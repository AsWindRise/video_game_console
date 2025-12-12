/**
 * @file    lfs_port.c
 * @brief   LittleFS port layer implementation for W25Q64 SPI Flash
 * @note    Bridges LittleFS callbacks to gd25qxx SPI Flash driver
 */

#include "lfs_port.h"
#include "gd25qxx.h"
#include "uart_driver.h"
#include "usart.h"
#include <string.h>

/* LittleFS instance and configuration */
static lfs_t s_lfs;
static struct lfs_config s_lfs_config;

/* Static buffers for LittleFS (avoid malloc) */
static uint8_t s_read_buffer[LFS_FLASH_CACHE_SIZE];
static uint8_t s_prog_buffer[LFS_FLASH_CACHE_SIZE];
static uint8_t s_lookahead_buffer[LFS_FLASH_LOOKAHEAD_SIZE];

/**
 * @brief  Flash read callback for LittleFS
 * @param  c      Pointer to LittleFS config
 * @param  block  Block number to read from
 * @param  off    Offset within the block
 * @param  buffer Buffer to store read data
 * @param  size   Number of bytes to read
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
static int flash_read(const struct lfs_config *c, lfs_block_t block,
                      lfs_off_t off, void *buffer, lfs_size_t size)
{
    (void)c; /* Unused parameter */

    /* Calculate absolute address */
    uint32_t addr = LFS_FLASH_START_ADDR + (block * LFS_FLASH_BLOCK_SIZE) + off;

    /* Use gd25qxx driver to read */
    spi_flash_buffer_read((uint8_t *)buffer, addr, (uint16_t)size);

    return LFS_ERR_OK;
}

/**
 * @brief  Flash program callback for LittleFS
 * @param  c      Pointer to LittleFS config
 * @param  block  Block number to program
 * @param  off    Offset within the block
 * @param  buffer Data to write
 * @param  size   Number of bytes to write
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
static int flash_prog(const struct lfs_config *c, lfs_block_t block,
                      lfs_off_t off, const void *buffer, lfs_size_t size)
{
    (void)c; /* Unused parameter */

    /* Calculate absolute address */
    uint32_t addr = LFS_FLASH_START_ADDR + (block * LFS_FLASH_BLOCK_SIZE) + off;

    /* Use gd25qxx driver to write (handles page boundaries internally) */
    spi_flash_buffer_write((uint8_t *)buffer, addr, (uint16_t)size);

    return LFS_ERR_OK;
}

/**
 * @brief  Flash erase callback for LittleFS
 * @param  c     Pointer to LittleFS config
 * @param  block Block number to erase
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
static int flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    (void)c; /* Unused parameter */

    /* Calculate sector address */
    uint32_t addr = LFS_FLASH_START_ADDR + (block * LFS_FLASH_BLOCK_SIZE);

    /* Use gd25qxx driver to erase sector */
    spi_flash_sector_erase(addr);

    return LFS_ERR_OK;
}

/**
 * @brief  Flash sync callback for LittleFS
 * @param  c Pointer to LittleFS config
 * @retval LFS_ERR_OK (SPI Flash doesn't need explicit sync)
 */
static int flash_sync(const struct lfs_config *c)
{
    (void)c; /* Unused parameter */

    /* SPI Flash operations are synchronous, nothing to do here */
    return LFS_ERR_OK;
}

void lfs_port_init(void)
{
    my_printf(&huart1, "[LFS] lfs_port_init start\r\n");

    /* Initialize SPI Flash driver (CS pin high, etc.) */
    spi_flash_init();
    my_printf(&huart1, "[LFS] spi_flash_init done\r\n");

    /* Configure LittleFS */
    memset(&s_lfs_config, 0, sizeof(s_lfs_config));

    /* Block device operations */
    s_lfs_config.read  = flash_read;
    s_lfs_config.prog  = flash_prog;
    s_lfs_config.erase = flash_erase;
    s_lfs_config.sync  = flash_sync;

    /* Block device configuration */
    s_lfs_config.read_size      = LFS_FLASH_READ_SIZE;
    s_lfs_config.prog_size      = LFS_FLASH_PROG_SIZE;
    s_lfs_config.block_size     = LFS_FLASH_BLOCK_SIZE;
    s_lfs_config.block_count    = LFS_FLASH_BLOCK_COUNT;
    s_lfs_config.block_cycles   = LFS_FLASH_BLOCK_CYCLES;
    s_lfs_config.cache_size     = LFS_FLASH_CACHE_SIZE;
    s_lfs_config.lookahead_size = LFS_FLASH_LOOKAHEAD_SIZE;

    /* Static buffers (avoid dynamic allocation) */
    s_lfs_config.read_buffer      = s_read_buffer;
    s_lfs_config.prog_buffer      = s_prog_buffer;
    s_lfs_config.lookahead_buffer = s_lookahead_buffer;

    my_printf(&huart1, "[LFS] lfs_port_init done\r\n");
}

const struct lfs_config* lfs_port_get_config(void)
{
    return &s_lfs_config;
}

lfs_t* lfs_port_get_lfs(void)
{
    return &s_lfs;
}

int lfs_port_mount(void)
{
    int err;

    my_printf(&huart1, "[LFS] Trying to mount...\r\n");

    /* Try to mount existing filesystem */
    err = lfs_mount(&s_lfs, &s_lfs_config);
    my_printf(&huart1, "[LFS] Mount result: %d\r\n", err);

    /* If mount fails, format and try again */
    if (err != LFS_ERR_OK) {
        my_printf(&huart1, "[LFS] Mount failed, formatting (this may take a while)...\r\n");

        /* Format the filesystem */
        err = lfs_format(&s_lfs, &s_lfs_config);
        my_printf(&huart1, "[LFS] Format result: %d\r\n", err);

        if (err != LFS_ERR_OK) {
            return err;
        }

        my_printf(&huart1, "[LFS] Trying mount again...\r\n");
        /* Try mounting again */
        err = lfs_mount(&s_lfs, &s_lfs_config);
        my_printf(&huart1, "[LFS] Second mount result: %d\r\n", err);
    }

    return err;
}

int lfs_port_unmount(void)
{
    return lfs_unmount(&s_lfs);
}

int lfs_port_format(void)
{
    return lfs_format(&s_lfs, &s_lfs_config);
}
