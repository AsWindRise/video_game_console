/**
 * @file    lfs_port.h
 * @brief   LittleFS port layer for W25Q64 SPI Flash
 * @note    This file bridges LittleFS to the gd25qxx SPI Flash driver
 */

#ifndef LFS_PORT_H
#define LFS_PORT_H

#include "lfs.h"

/* W25Q64 Flash Configuration */
#define LFS_FLASH_BLOCK_SIZE    4096    /* Sector size: 4KB */
#define LFS_FLASH_BLOCK_COUNT   2048    /* Total sectors: 8MB / 4KB = 2048 */
#define LFS_FLASH_READ_SIZE     1       /* Minimum read size */
#define LFS_FLASH_PROG_SIZE     1       /* Minimum program size (could be 256 for page-aligned) */
#define LFS_FLASH_CACHE_SIZE    256     /* Cache size (typically = page size) */
#define LFS_FLASH_LOOKAHEAD_SIZE 16     /* Lookahead buffer size */
#define LFS_FLASH_BLOCK_CYCLES  500     /* Erase cycles before eviction */

/* Flash start address offset (if not using entire Flash for filesystem) */
#define LFS_FLASH_START_ADDR    0x000000

/**
 * @brief  Initialize LittleFS port layer
 * @note   Call this before using any LittleFS functions
 * @retval None
 */
void lfs_port_init(void);

/**
 * @brief  Get the LittleFS configuration structure
 * @note   Use this to pass to lfs_mount() or lfs_format()
 * @retval Pointer to the lfs_config structure
 */
const struct lfs_config* lfs_port_get_config(void);

/**
 * @brief  Get the LittleFS instance
 * @note   Use this to access the mounted filesystem
 * @retval Pointer to the lfs_t structure
 */
lfs_t* lfs_port_get_lfs(void);

/**
 * @brief  Mount the filesystem (format if necessary)
 * @note   Automatically formats on first mount or corruption
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
int lfs_port_mount(void);

/**
 * @brief  Unmount the filesystem
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
int lfs_port_unmount(void);

/**
 * @brief  Format the filesystem
 * @note   WARNING: This erases all data!
 * @retval LFS_ERR_OK on success, negative error code on failure
 */
int lfs_port_format(void);

#endif /* LFS_PORT_H */
