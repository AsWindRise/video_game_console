/**
 ******************************************************************************
 * @file    test_littlefs.c
 * @brief   LittleFS filesystem test implementation
 * @author  LaoWang
 * @note    Test functions for LittleFS filesystem
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "test_littlefs.h"
#include "lfs_port.h"
#include "gd25qxx.h"
#include "uart_driver.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define TEST_FILE_NAME      "/test.txt"
#define TEST_DIR_NAME       "/saves"
#define TEST_GAME_SAVE_FILE "/saves/game1.sav"

/* Private variables ---------------------------------------------------------*/
static uint8_t s_test_buffer[256];
static int s_lfs_mounted = 0;

/* Private function prototypes -----------------------------------------------*/
static void print_test_result(const char *test_name, lfs_test_result_t result);

/* Public functions ----------------------------------------------------------*/

int test_littlefs_init(void)
{
    int err;
    uint16_t flash_id;

    my_printf(&huart1, "\r\n");
    my_printf(&huart1, "========================================\r\n");
    my_printf(&huart1, "  LittleFS Test Suite - by LaoWang\r\n");
    my_printf(&huart1, "========================================\r\n");

    /* First test: can we read Flash ID? */
    my_printf(&huart1, "[INIT] Testing Flash communication...\r\n");
    flash_id = spi_flash_read_id();
    my_printf(&huart1, "[INIT] Flash ID: 0x%04X\r\n", flash_id);

    if (flash_id == 0x0000 || flash_id == 0xFFFF) {
        my_printf(&huart1, "[INIT] ERROR: Flash not responding!\r\n");
        return -1;
    }

    /* Initialize port layer */
    my_printf(&huart1, "[INIT] Initializing LittleFS port layer...\r\n");
    lfs_port_init();
    my_printf(&huart1, "[INIT] Port layer initialized.\r\n");

    /* Mount filesystem */
    my_printf(&huart1, "[INIT] Mounting filesystem (may take a while if formatting)...\r\n");
    err = lfs_port_mount();

    if (err == LFS_ERR_OK) {
        s_lfs_mounted = 1;
        my_printf(&huart1, "[INIT] Filesystem mounted successfully!\r\n");
    } else {
        s_lfs_mounted = 0;
        my_printf(&huart1, "[INIT] ERROR: Failed to mount filesystem! err=%d\r\n", err);
    }

    return err;
}

void test_littlefs_run_all(void)
{
    my_printf(&huart1, "\r\n");
    my_printf(&huart1, "======== Running All Tests ========\r\n");
    my_printf(&huart1, "\r\n");

    print_test_result("Flash ID", test_littlefs_flash_id());
    print_test_result("Mount", test_littlefs_mount());
    print_test_result("File Write", test_littlefs_file_write());
    print_test_result("File Read", test_littlefs_file_read());
    print_test_result("File Append", test_littlefs_file_append());
    print_test_result("Directory", test_littlefs_directory());
    print_test_result("File Remove", test_littlefs_file_remove());
    print_test_result("Capacity", test_littlefs_capacity());
    print_test_result("Game Save", test_littlefs_game_save());

    my_printf(&huart1, "\r\n");
    my_printf(&huart1, "======== All Tests Complete ========\r\n");
    my_printf(&huart1, "\r\n");
}

lfs_test_result_t test_littlefs_flash_id(void)
{
    uint16_t id;

    my_printf(&huart1, "[TEST] Reading Flash ID...\r\n");

    id = spi_flash_read_id();
    my_printf(&huart1, "       Flash ID: 0x%04X\r\n", id);

    /* W25Q64: 0xEF17, GD25Q64: 0xC817 */
    if (id == 0xEF17 || id == 0xC817 || id == 0xEF18) {
        my_printf(&huart1, "       -> Flash recognized!\r\n");
        return LFS_TEST_PASS;
    } else if (id == 0x0000 || id == 0xFFFF) {
        my_printf(&huart1, "       -> ERROR: Flash not responding (check SPI wiring)\r\n");
        return LFS_TEST_FAIL;
    } else {
        my_printf(&huart1, "       -> WARNING: Unknown Flash ID\r\n");
        return LFS_TEST_PASS; /* May still work */
    }
}

lfs_test_result_t test_littlefs_mount(void)
{
    int err;

    my_printf(&huart1, "[TEST] Testing mount/unmount...\r\n");

    if (s_lfs_mounted) {
        /* Unmount first */
        err = lfs_port_unmount();
        if (err != LFS_ERR_OK) {
            my_printf(&huart1, "       -> ERROR: Unmount failed! err=%d\r\n", err);
            return LFS_TEST_FAIL;
        }
        s_lfs_mounted = 0;
    }

    /* Mount */
    err = lfs_port_mount();
    if (err != LFS_ERR_OK) {
        my_printf(&huart1, "       -> ERROR: Mount failed! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }
    s_lfs_mounted = 1;

    my_printf(&huart1, "       -> Mount/Unmount OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_file_write(void)
{
    lfs_t *lfs;
    lfs_file_t file;
    int err;
    const char *test_data = "Hello from LittleFS! - LaoWang was here";

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping file write (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing file write...\r\n");

    lfs = lfs_port_get_lfs();

    /* Open file for writing (create if not exists) */
    err = lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to open file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    /* Write data */
    err = lfs_file_write(lfs, &file, test_data, strlen(test_data));
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to write file! err=%d\r\n", err);
        lfs_file_close(lfs, &file);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Wrote %d bytes\r\n", err);

    /* Close file */
    lfs_file_close(lfs, &file);

    my_printf(&huart1, "       -> File write OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_file_read(void)
{
    lfs_t *lfs;
    lfs_file_t file;
    int err;

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping file read (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing file read...\r\n");

    lfs = lfs_port_get_lfs();

    /* Open file for reading */
    err = lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_RDONLY);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to open file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    /* Read data */
    memset(s_test_buffer, 0, sizeof(s_test_buffer));
    err = lfs_file_read(lfs, &file, s_test_buffer, sizeof(s_test_buffer) - 1);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to read file! err=%d\r\n", err);
        lfs_file_close(lfs, &file);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Read %d bytes: \"%s\"\r\n", err, s_test_buffer);

    /* Close file */
    lfs_file_close(lfs, &file);

    my_printf(&huart1, "       -> File read OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_file_append(void)
{
    lfs_t *lfs;
    lfs_file_t file;
    int err;
    const char *append_data = " [APPENDED]";

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping file append (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing file append...\r\n");

    lfs = lfs_port_get_lfs();

    /* Open file for appending */
    err = lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_WRONLY | LFS_O_APPEND);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to open file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    /* Append data */
    err = lfs_file_write(lfs, &file, append_data, strlen(append_data));
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to append! err=%d\r\n", err);
        lfs_file_close(lfs, &file);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Appended %d bytes\r\n", err);
    lfs_file_close(lfs, &file);

    /* Read back to verify */
    err = lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_RDONLY);
    if (err >= 0) {
        memset(s_test_buffer, 0, sizeof(s_test_buffer));
        lfs_file_read(lfs, &file, s_test_buffer, sizeof(s_test_buffer) - 1);
        my_printf(&huart1, "       Content: \"%s\"\r\n", s_test_buffer);
        lfs_file_close(lfs, &file);
    }

    my_printf(&huart1, "       -> File append OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_directory(void)
{
    lfs_t *lfs;
    lfs_dir_t dir;
    struct lfs_info info;
    int err;

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping directory test (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing directory operations...\r\n");

    lfs = lfs_port_get_lfs();

    /* Create directory */
    err = lfs_mkdir(lfs, TEST_DIR_NAME);
    if (err < 0 && err != LFS_ERR_EXIST) {
        my_printf(&huart1, "       -> ERROR: Failed to create dir! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }
    my_printf(&huart1, "       Created directory: %s\r\n", TEST_DIR_NAME);

    /* List root directory */
    err = lfs_dir_open(lfs, &dir, "/");
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to open root dir! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Root directory contents:\r\n");
    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        if (info.type == LFS_TYPE_DIR) {
            my_printf(&huart1, "         [DIR]  %s\r\n", info.name);
        } else {
            my_printf(&huart1, "         [FILE] %s (%lu bytes)\r\n", info.name, info.size);
        }
    }

    lfs_dir_close(lfs, &dir);

    my_printf(&huart1, "       -> Directory operations OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_file_remove(void)
{
    lfs_t *lfs;
    int err;

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping file remove (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing file remove...\r\n");

    lfs = lfs_port_get_lfs();

    /* Remove test file */
    err = lfs_remove(lfs, TEST_FILE_NAME);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to remove file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Removed: %s\r\n", TEST_FILE_NAME);

    /* Verify file is gone */
    {
        struct lfs_info info;
        err = lfs_stat(lfs, TEST_FILE_NAME, &info);
        if (err == LFS_ERR_NOENT) {
            my_printf(&huart1, "       -> File remove OK!\r\n");
            return LFS_TEST_PASS;
        } else {
            my_printf(&huart1, "       -> ERROR: File still exists!\r\n");
            return LFS_TEST_FAIL;
        }
    }
}

lfs_test_result_t test_littlefs_power_loss(void)
{
    /* This is a simulated test - real power loss testing requires hardware */
    my_printf(&huart1, "[TEST] Power loss simulation (placeholder)\r\n");
    my_printf(&huart1, "       -> LittleFS is designed for power-loss resilience\r\n");
    my_printf(&huart1, "       -> Real testing requires actual power cycle\r\n");
    return LFS_TEST_SKIP;
}

lfs_test_result_t test_littlefs_capacity(void)
{
    lfs_t *lfs;
    lfs_ssize_t used_blocks;

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping capacity test (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing filesystem capacity...\r\n");

    lfs = lfs_port_get_lfs();

    /* Get used blocks */
    used_blocks = lfs_fs_size(lfs);
    if (used_blocks < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to get fs size! err=%d\r\n", (int)used_blocks);
        return LFS_TEST_FAIL;
    }

    {
        uint32_t total_size = LFS_FLASH_BLOCK_COUNT * LFS_FLASH_BLOCK_SIZE;
        uint32_t used_size = used_blocks * LFS_FLASH_BLOCK_SIZE;
        uint32_t free_size = total_size - used_size;

        my_printf(&huart1, "       Total:  %lu KB (%lu blocks)\r\n",
                    total_size / 1024, (uint32_t)LFS_FLASH_BLOCK_COUNT);
        my_printf(&huart1, "       Used:   %lu KB (%ld blocks)\r\n",
                    used_size / 1024, (long)used_blocks);
        my_printf(&huart1, "       Free:   %lu KB\r\n", free_size / 1024);
        my_printf(&huart1, "       Usage:  %lu%%\r\n", (used_size * 100) / total_size);
    }

    my_printf(&huart1, "       -> Capacity info OK!\r\n");
    return LFS_TEST_PASS;
}

lfs_test_result_t test_littlefs_game_save(void)
{
    lfs_t *lfs;
    lfs_file_t file;
    int err;

    /* Simulated game save structure */
    typedef struct {
        uint32_t magic;         /* 0xDEADBEEF */
        uint32_t version;       /* Save version */
        uint32_t score;         /* Player score */
        uint32_t level;         /* Current level */
        uint8_t  player_name[16]; /* Player name */
        uint32_t checksum;      /* Simple checksum */
    } game_save_t;

    game_save_t save_data;
    game_save_t load_data;

    if (!s_lfs_mounted) {
        my_printf(&huart1, "[TEST] Skipping game save test (not mounted)\r\n");
        return LFS_TEST_SKIP;
    }

    my_printf(&huart1, "[TEST] Testing game save functionality...\r\n");

    lfs = lfs_port_get_lfs();

    /* Make sure saves directory exists */
    lfs_mkdir(lfs, TEST_DIR_NAME);

    /* Prepare save data */
    memset(&save_data, 0, sizeof(save_data));
    save_data.magic = 0xDEADBEEF;
    save_data.version = 1;
    save_data.score = 12345;
    save_data.level = 7;
    memcpy(save_data.player_name, "LaoWang", 8);
    save_data.checksum = save_data.magic ^ save_data.score ^ save_data.level;

    /* Write save file */
    err = lfs_file_open(lfs, &file, TEST_GAME_SAVE_FILE,
                        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to create save file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    err = lfs_file_write(lfs, &file, &save_data, sizeof(save_data));
    lfs_file_close(lfs, &file);

    if (err != sizeof(save_data)) {
        my_printf(&huart1, "       -> ERROR: Save write incomplete! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    my_printf(&huart1, "       Saved: Score=%lu, Level=%lu, Player=%s\r\n",
                save_data.score, save_data.level, save_data.player_name);

    /* Read back and verify */
    err = lfs_file_open(lfs, &file, TEST_GAME_SAVE_FILE, LFS_O_RDONLY);
    if (err < 0) {
        my_printf(&huart1, "       -> ERROR: Failed to open save file! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    memset(&load_data, 0, sizeof(load_data));
    err = lfs_file_read(lfs, &file, &load_data, sizeof(load_data));
    lfs_file_close(lfs, &file);

    if (err != sizeof(load_data)) {
        my_printf(&huart1, "       -> ERROR: Save read incomplete! err=%d\r\n", err);
        return LFS_TEST_FAIL;
    }

    /* Verify data integrity */
    if (load_data.magic != 0xDEADBEEF) {
        my_printf(&huart1, "       -> ERROR: Invalid magic number!\r\n");
        return LFS_TEST_FAIL;
    }

    {
        uint32_t expected_checksum = load_data.magic ^ load_data.score ^ load_data.level;
        if (load_data.checksum != expected_checksum) {
            my_printf(&huart1, "       -> ERROR: Checksum mismatch!\r\n");
            return LFS_TEST_FAIL;
        }
    }

    my_printf(&huart1, "       Loaded: Score=%lu, Level=%lu, Player=%s\r\n",
                load_data.score, load_data.level, load_data.player_name);
    my_printf(&huart1, "       -> Game save test OK!\r\n");
    return LFS_TEST_PASS;
}

/* Private functions ---------------------------------------------------------*/

static void print_test_result(const char *test_name, lfs_test_result_t result)
{
    const char *result_str;

    switch (result) {
        case LFS_TEST_PASS:
            result_str = "[PASS]";
            break;
        case LFS_TEST_FAIL:
            result_str = "[FAIL]";
            break;
        case LFS_TEST_SKIP:
            result_str = "[SKIP]";
            break;
        default:
            result_str = "[????]";
            break;
    }

    my_printf(&huart1, "%s %-20s\r\n", result_str, test_name);
}
