/**
 * @file    test_sdcard.c
 * @brief   SD卡(TF卡)和FATFS测试实现
 * @note    完整的SDIO + FATFS功能测试套件
 */

#include "test_sdcard.h"
#include "fatfs.h"
#include "sdio.h"
#include "bsp_driver_sd.h"
#include "uart_driver.h"
#include "usart.h"
#include "tim.h"
#include "stm32f4xx_hal.h"  /* HAL_GetTick() */
#include <string.h>
#include <stdio.h>

/* 测试文件路径 */
#define TEST_FILE_PATH      "0:/test.txt"
#define TEST_LOG_PATH       "0:/log.txt"
#define TEST_DIR_PATH       "0:/testdir"
#define TEST_SAVE_PATH      "0:/saves"
#define TEST_GAME_SAVE_PATH "0:/saves/game1.sav"
#define TEST_SPEED_FILE     "0:/speedtest.bin"
#define TEST_NESTED_BASE    "0:/nested"
#define TEST_BATCH_DIR      "0:/batch"

/* 测试缓冲区 - 使用4KB对齐以获得最佳性能 */
static uint8_t s_test_buffer[4096] __attribute__((aligned(4)));
static char s_read_buf[128];

/* FATFS变量（外部定义在fatfs.c中） */
extern FATFS SDFatFS;
extern char SDPath[];

/* 挂载状态标记 */
static uint8_t s_mounted = 0;

/* 游戏存档结构（和LittleFS测试保持一致） */
typedef struct {
    uint32_t magic;           /* 魔数验证: 0xDEADBEEF */
    uint32_t version;         /* 存档版本号 */
    uint32_t high_score;      /* 最高分 */
    uint32_t current_level;   /* 当前关卡 */
    uint8_t  player_name[16]; /* 玩家名称 */
    uint32_t play_time;       /* 游戏时长（秒） */
    uint32_t checksum;        /* 校验和 */
} game_save_t;

#define SAVE_MAGIC 0xDEADBEEF

/*============================================================================*/
/*                              测试函数实现                                   */
/*============================================================================*/

int test_sdcard_init(void)
{
    my_printf(&huart1, "\r\n");
    my_printf(&huart1, "========================================\r\n");
    my_printf(&huart1, "       SD Card (FATFS) Test Suite       \r\n");
    my_printf(&huart1, "========================================\r\n");

    /* FATFS已在MX_FATFS_Init()中链接驱动，这里不需要额外初始化 */
    my_printf(&huart1, "[SDCARD] Test module initialized\r\n");

    return 0;
}

int test_sdcard_run_all(void)
{
    int failed = 0;

    my_printf(&huart1, "\r\n--- Running All SD Card Tests ---\r\n\r\n");

    /* 依次运行所有测试 */
    if (test_sdcard_detect() != 0)   failed++;
    if (test_sdcard_mount() != 0)    failed++;
    if (test_sdcard_write() != 0)    failed++;
    if (test_sdcard_read() != 0)     failed++;
    if (test_sdcard_append() != 0)   failed++;
    if (test_sdcard_directory() != 0) failed++;
    if (test_sdcard_capacity() != 0) failed++;
    if (test_sdcard_game_save() != 0) failed++;
    if (test_sdcard_delete() != 0)   failed++;

    /* 测试完成后卸载 */
    if (s_mounted) {
        f_mount(NULL, SDPath, 0);
        s_mounted = 0;
    }

    my_printf(&huart1, "\r\n========================================\r\n");
    if (failed == 0) {
        my_printf(&huart1, "  ALL TESTS PASSED!\r\n");
    } else {
        my_printf(&huart1, "  %d TEST(S) FAILED!\r\n", failed);
    }
    my_printf(&huart1, "========================================\r\n\r\n");

    return failed;
}

int test_sdcard_detect(void)
{
    my_printf(&huart1, "[TEST] SD Card Detection... ");

    /* 检测SD卡是否插入 */
    if (BSP_SD_IsDetected() != SD_PRESENT) {
        my_printf(&huart1, "FAILED (No card detected)\r\n");
        return -1;
    }

    my_printf(&huart1, "PASSED (Card present)\r\n");
    return 0;
}

int test_sdcard_mount(void)
{
    FRESULT res;

    my_printf(&huart1, "[TEST] FATFS Mount... ");

    /* 挂载文件系统，1表示立即挂载 */
    res = f_mount(&SDFatFS, SDPath, 1);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (err=%d)\r\n", res);
        return -1;
    }

    s_mounted = 1;
    my_printf(&huart1, "PASSED\r\n");
    return 0;
}

int test_sdcard_write(void)
{
    FIL file;
    FRESULT res;
    UINT bytes_written;

    my_printf(&huart1, "[TEST] File Write... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建并写入测试文件 */
    res = f_open(&file, TEST_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open err=%d)\r\n", res);
        return -1;
    }

    /* 写入测试数据 */
    const char *test_str = "Hello SD Card from STM32F407!";
    res = f_write(&file, test_str, strlen(test_str), &bytes_written);
    f_close(&file);

    if (res != FR_OK || bytes_written != strlen(test_str)) {
        my_printf(&huart1, "FAILED (write err=%d, written=%u)\r\n", res, bytes_written);
        return -1;
    }

    my_printf(&huart1, "PASSED (%u bytes)\r\n", bytes_written);
    return 0;
}

int test_sdcard_read(void)
{
    FIL file;
    FRESULT res;
    UINT bytes_read;

    my_printf(&huart1, "[TEST] File Read... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 打开测试文件 */
    res = f_open(&file, TEST_FILE_PATH, FA_READ);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open err=%d)\r\n", res);
        return -1;
    }

    /* 读取内容 */
    memset(s_read_buf, 0, sizeof(s_read_buf));
    res = f_read(&file, s_read_buf, sizeof(s_read_buf) - 1, &bytes_read);
    f_close(&file);

    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (read err=%d)\r\n", res);
        return -1;
    }

    my_printf(&huart1, "PASSED (%u bytes: \"%s\")\r\n", bytes_read, s_read_buf);
    return 0;
}

int test_sdcard_append(void)
{
    FIL file;
    FRESULT res;
    UINT bytes_written;

    my_printf(&huart1, "[TEST] File Append... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 以追加模式打开日志文件 */
    res = f_open(&file, TEST_LOG_PATH, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK) {
        /* 文件不存在则创建 */
        res = f_open(&file, TEST_LOG_PATH, FA_CREATE_NEW | FA_WRITE);
        if (res != FR_OK) {
            my_printf(&huart1, "FAILED (open err=%d)\r\n", res);
            return -1;
        }
    }

    /* 追加日志 */
    const char *log_str = "[LOG] Test entry\r\n";
    res = f_write(&file, log_str, strlen(log_str), &bytes_written);
    f_close(&file);

    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (write err=%d)\r\n", res);
        return -1;
    }

    my_printf(&huart1, "PASSED (%u bytes appended)\r\n", bytes_written);
    return 0;
}

int test_sdcard_directory(void)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int file_count = 0;

    my_printf(&huart1, "[TEST] Directory Operations... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建测试目录（如果已存在会返回FR_EXIST，忽略） */
    res = f_mkdir(TEST_DIR_PATH);
    if (res != FR_OK && res != FR_EXIST) {
        my_printf(&huart1, "FAILED (mkdir err=%d)\r\n", res);
        return -1;
    }

    /* 遍历根目录 */
    res = f_opendir(&dir, "0:/");
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (opendir err=%d)\r\n", res);
        return -1;
    }

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Root directory contents:\r\n");

    while (1) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;

        if (fno.fattrib & AM_DIR) {
            my_printf(&huart1, "    [DIR]  %s\r\n", fno.fname);
        } else {
            my_printf(&huart1, "    [FILE] %s (%lu bytes)\r\n", fno.fname, fno.fsize);
        }
        file_count++;
    }

    f_closedir(&dir);
    my_printf(&huart1, "  Total: %d items\r\n", file_count);

    return 0;
}

int test_sdcard_delete(void)
{
    FRESULT res;

    my_printf(&huart1, "[TEST] File Delete... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 删除测试文件 */
    res = f_unlink(TEST_FILE_PATH);
    if (res != FR_OK && res != FR_NO_FILE) {
        my_printf(&huart1, "FAILED (err=%d)\r\n", res);
        return -1;
    }

    /* 删除日志文件 */
    f_unlink(TEST_LOG_PATH);

    /* 删除游戏存档 */
    f_unlink(TEST_GAME_SAVE_PATH);

    /* 删除目录（必须为空） */
    f_unlink(TEST_SAVE_PATH);
    f_unlink(TEST_DIR_PATH);

    my_printf(&huart1, "PASSED\r\n");
    return 0;
}

int test_sdcard_capacity(void)
{
    FATFS *fs;
    DWORD free_clusters;
    FRESULT res;
    uint32_t total_kb, free_kb;

    my_printf(&huart1, "[TEST] SD Card Capacity... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 获取空闲簇数量 */
    res = f_getfree(SDPath, &free_clusters, &fs);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (err=%d)\r\n", res);
        return -1;
    }

    /* 计算容量（KB） */
    /* 总容量 = 总簇数 * 每簇扇区数 * 每扇区字节数 */
    total_kb = ((fs->n_fatent - 2) * fs->csize) / 2;  /* 除以2是因为扇区512字节 */
    free_kb = (free_clusters * fs->csize) / 2;

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Total: %lu KB (%lu MB)\r\n", total_kb, total_kb / 1024);
    my_printf(&huart1, "  Free:  %lu KB (%lu MB)\r\n", free_kb, free_kb / 1024);
    my_printf(&huart1, "  Used:  %lu KB (%lu MB)\r\n", total_kb - free_kb, (total_kb - free_kb) / 1024);

    return 0;
}

int test_sdcard_game_save(void)
{
    FIL file;
    FRESULT res;
    UINT bytes;
    game_save_t save_write, save_read;

    my_printf(&huart1, "[TEST] Game Save Simulation... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建saves目录 */
    f_mkdir(TEST_SAVE_PATH);

    /* 准备存档数据 */
    memset(&save_write, 0, sizeof(save_write));
    save_write.magic = SAVE_MAGIC;
    save_write.version = 1;
    save_write.high_score = 88888;
    save_write.current_level = 15;
    strcpy((char*)save_write.player_name, "Player1");
    save_write.play_time = 7200;
    save_write.checksum = save_write.magic ^ save_write.high_score ^ save_write.current_level;

    /* 写入存档 */
    res = f_open(&file, TEST_GAME_SAVE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (create err=%d)\r\n", res);
        return -1;
    }

    res = f_write(&file, &save_write, sizeof(game_save_t), &bytes);
    f_close(&file);

    if (res != FR_OK || bytes != sizeof(game_save_t)) {
        my_printf(&huart1, "FAILED (write err=%d)\r\n", res);
        return -1;
    }

    /* 读取并验证存档 */
    res = f_open(&file, TEST_GAME_SAVE_PATH, FA_READ);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open err=%d)\r\n", res);
        return -1;
    }

    memset(&save_read, 0, sizeof(save_read));
    res = f_read(&file, &save_read, sizeof(game_save_t), &bytes);
    f_close(&file);

    if (res != FR_OK || bytes != sizeof(game_save_t)) {
        my_printf(&huart1, "FAILED (read err=%d)\r\n", res);
        return -1;
    }

    /* 验证魔数 */
    if (save_read.magic != SAVE_MAGIC) {
        my_printf(&huart1, "FAILED (bad magic)\r\n");
        return -1;
    }

    /* 验证校验和 */
    uint32_t expected = save_read.magic ^ save_read.high_score ^ save_read.current_level;
    if (save_read.checksum != expected) {
        my_printf(&huart1, "FAILED (bad checksum)\r\n");
        return -1;
    }

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Player: %s\r\n", save_read.player_name);
    my_printf(&huart1, "  Score:  %lu\r\n", save_read.high_score);
    my_printf(&huart1, "  Level:  %lu\r\n", save_read.current_level);
    my_printf(&huart1, "  Time:   %lu sec\r\n", save_read.play_time);

    return 0;
}

/*============================================================================*/
/*                           高级测试功能实现                                   */
/*============================================================================*/

/**
 * @brief  获取系统tick（用于计时）
 * @note   使用HAL_GetTick()获取毫秒级时间戳
 */
static uint32_t get_tick_ms(void)
{
    return HAL_GetTick();
}

int test_sdcard_run_advanced(void)
{
    int failed = 0;

    my_printf(&huart1, "\r\n--- Running Advanced SD Card Tests ---\r\n\r\n");

    /* 确保文件系统已挂载 */
    if (!s_mounted) {
        FRESULT res = f_mount(&SDFatFS, SDPath, 1);
        if (res != FR_OK) {
            my_printf(&huart1, "[ERROR] Failed to mount (err=%d)\r\n", res);
            return -1;
        }
        s_mounted = 1;
    }

    /* 运行高级测试 */
    if (test_sdcard_write_speed(64) != 0)   failed++;
    if (test_sdcard_read_speed(64) != 0)    failed++;
    if (test_sdcard_nested_dirs() != 0)     failed++;
    if (test_sdcard_batch_files(10) != 0)   failed++;
    if (test_sdcard_rename() != 0)          failed++;

    /* 清理测试文件 */
    f_unlink(TEST_SPEED_FILE);

    my_printf(&huart1, "\r\n========================================\r\n");
    if (failed == 0) {
        my_printf(&huart1, "  ALL ADVANCED TESTS PASSED!\r\n");
    } else {
        my_printf(&huart1, "  %d ADVANCED TEST(S) FAILED!\r\n", failed);
    }
    my_printf(&huart1, "========================================\r\n\r\n");

    return failed;
}

int test_sdcard_write_speed(uint32_t size_kb)
{
    FIL file;
    FRESULT res;
    UINT bytes_written;
    uint32_t total_bytes = size_kb * 1024;
    uint32_t written = 0;
    uint32_t start_tick, end_tick, elapsed;
    uint32_t speed_kbps;

    my_printf(&huart1, "[TEST] Write Speed (%lu KB)... ", size_kb);

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 填充测试数据 */
    for (uint32_t i = 0; i < sizeof(s_test_buffer); i++) {
        s_test_buffer[i] = (uint8_t)(i & 0xFF);
    }

    /* 创建测试文件 */
    res = f_open(&file, TEST_SPEED_FILE, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open err=%d)\r\n", res);
        return -1;
    }

    /* 开始计时写入 */
    start_tick = get_tick_ms();

    while (written < total_bytes) {
        uint32_t chunk = (total_bytes - written > sizeof(s_test_buffer))
                        ? sizeof(s_test_buffer) : (total_bytes - written);
        res = f_write(&file, s_test_buffer, chunk, &bytes_written);
        if (res != FR_OK || bytes_written != chunk) {
            f_close(&file);
            my_printf(&huart1, "FAILED (write err=%d at %lu)\r\n", res, written);
            return -1;
        }
        written += bytes_written;
    }

    /* 确保数据刷入 */
    f_sync(&file);
    f_close(&file);

    end_tick = get_tick_ms();
    elapsed = end_tick - start_tick;

    /* 计算速度 (KB/s) */
    if (elapsed > 0) {
        speed_kbps = (size_kb * 1000) / elapsed;
    } else {
        speed_kbps = 0;
    }

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Written: %lu KB in %lu ms\r\n", size_kb, elapsed);
    my_printf(&huart1, "  Speed:   %lu KB/s\r\n", speed_kbps);

    return 0;
}

int test_sdcard_read_speed(uint32_t size_kb)
{
    FIL file;
    FRESULT res;
    UINT bytes_read;
    uint32_t total_bytes = size_kb * 1024;
    uint32_t read_total = 0;
    uint32_t start_tick, end_tick, elapsed;
    uint32_t speed_kbps;

    my_printf(&huart1, "[TEST] Read Speed (%lu KB)... ", size_kb);

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 打开测试文件（应该在write_speed中创建） */
    res = f_open(&file, TEST_SPEED_FILE, FA_READ);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open err=%d, run write_speed first)\r\n", res);
        return -1;
    }

    /* 开始计时读取 */
    start_tick = get_tick_ms();

    while (read_total < total_bytes) {
        uint32_t chunk = (total_bytes - read_total > sizeof(s_test_buffer))
                        ? sizeof(s_test_buffer) : (total_bytes - read_total);
        res = f_read(&file, s_test_buffer, chunk, &bytes_read);
        if (res != FR_OK) {
            f_close(&file);
            my_printf(&huart1, "FAILED (read err=%d at %lu)\r\n", res, read_total);
            return -1;
        }
        if (bytes_read == 0) break;  /* EOF */
        read_total += bytes_read;
    }

    f_close(&file);

    end_tick = get_tick_ms();
    elapsed = end_tick - start_tick;

    /* 计算速度 (KB/s) */
    if (elapsed > 0) {
        speed_kbps = (read_total / 1024 * 1000) / elapsed;
    } else {
        speed_kbps = 0;
    }

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Read:  %lu KB in %lu ms\r\n", read_total / 1024, elapsed);
    my_printf(&huart1, "  Speed: %lu KB/s\r\n", speed_kbps);

    return 0;
}

int test_sdcard_nested_dirs(void)
{
    FRESULT res;
    char path[64];
    int depth = 5;

    my_printf(&huart1, "[TEST] Nested Directories (%d levels)... ", depth);

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建多层嵌套目录 */
    strcpy(path, TEST_NESTED_BASE);
    res = f_mkdir(path);
    if (res != FR_OK && res != FR_EXIST) {
        my_printf(&huart1, "FAILED (mkdir base err=%d)\r\n", res);
        return -1;
    }

    for (int i = 1; i <= depth; i++) {
        char level[16];
        sprintf(level, "/L%d", i);
        strcat(path, level);

        res = f_mkdir(path);
        if (res != FR_OK && res != FR_EXIST) {
            my_printf(&huart1, "FAILED (mkdir L%d err=%d)\r\n", i, res);
            return -1;
        }
    }

    /* 在最深层创建文件验证 */
    strcat(path, "/deep.txt");
    FIL file;
    UINT bytes;
    res = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (create file err=%d)\r\n", res);
        return -1;
    }

    const char *msg = "Deep nested file test";
    f_write(&file, msg, strlen(msg), &bytes);
    f_close(&file);

    /* 清理 - 从最深层开始删除 */
    f_unlink(path);  /* 删除文件 */

    /* 逐层删除目录 */
    strcpy(path, TEST_NESTED_BASE);
    for (int i = depth; i >= 1; i--) {
        char full_path[64];
        strcpy(full_path, TEST_NESTED_BASE);
        for (int j = 1; j <= i; j++) {
            char level[16];
            sprintf(level, "/L%d", j);
            strcat(full_path, level);
        }
        f_unlink(full_path);
    }
    f_unlink(TEST_NESTED_BASE);

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Created %d nested directories\r\n", depth);

    return 0;
}

int test_sdcard_batch_files(uint32_t count)
{
    FRESULT res;
    FIL file;
    UINT bytes;
    char filename[64];
    uint32_t start_tick, end_tick;
    uint32_t created = 0;

    my_printf(&huart1, "[TEST] Batch Files (%lu files)... ", count);

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建批量测试目录 */
    res = f_mkdir(TEST_BATCH_DIR);
    if (res != FR_OK && res != FR_EXIST) {
        my_printf(&huart1, "FAILED (mkdir err=%d)\r\n", res);
        return -1;
    }

    start_tick = get_tick_ms();

    /* 批量创建小文件 */
    for (uint32_t i = 0; i < count; i++) {
        sprintf(filename, "%s/file%04lu.txt", TEST_BATCH_DIR, i);

        res = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
        if (res != FR_OK) {
            my_printf(&huart1, "FAILED (create #%lu err=%d)\r\n", i, res);
            goto cleanup;
        }

        /* 写入少量数据 */
        char content[32];
        sprintf(content, "File #%lu content", i);
        res = f_write(&file, content, strlen(content), &bytes);
        f_close(&file);

        if (res != FR_OK) {
            my_printf(&huart1, "FAILED (write #%lu err=%d)\r\n", i, res);
            goto cleanup;
        }
        created++;
    }

    end_tick = get_tick_ms();

    my_printf(&huart1, "PASSED\r\n");
    my_printf(&huart1, "  Created %lu files in %lu ms\r\n", created, end_tick - start_tick);
    my_printf(&huart1, "  Avg: %lu ms/file\r\n", (end_tick - start_tick) / count);

cleanup:
    /* 清理所有创建的文件 */
    for (uint32_t i = 0; i < created; i++) {
        sprintf(filename, "%s/file%04lu.txt", TEST_BATCH_DIR, i);
        f_unlink(filename);
    }
    f_unlink(TEST_BATCH_DIR);

    return (created == count) ? 0 : -1;
}

int test_sdcard_rename(void)
{
    FRESULT res;
    FIL file;
    UINT bytes;
    const char *old_name = "0:/rename_old.txt";
    const char *new_name = "0:/rename_new.txt";

    my_printf(&huart1, "[TEST] File Rename... ");

    if (!s_mounted) {
        my_printf(&huart1, "SKIPPED (not mounted)\r\n");
        return -1;
    }

    /* 创建原始文件 */
    res = f_open(&file, old_name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (create err=%d)\r\n", res);
        return -1;
    }

    const char *content = "Rename test content";
    f_write(&file, content, strlen(content), &bytes);
    f_close(&file);

    /* 确保目标文件不存在 */
    f_unlink(new_name);

    /* 执行重命名 */
    res = f_rename(old_name, new_name);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (rename err=%d)\r\n", res);
        f_unlink(old_name);
        return -1;
    }

    /* 验证新文件存在且内容正确 */
    res = f_open(&file, new_name, FA_READ);
    if (res != FR_OK) {
        my_printf(&huart1, "FAILED (open new err=%d)\r\n", res);
        return -1;
    }

    memset(s_read_buf, 0, sizeof(s_read_buf));
    res = f_read(&file, s_read_buf, sizeof(s_read_buf) - 1, &bytes);
    f_close(&file);

    if (res != FR_OK || strcmp(s_read_buf, content) != 0) {
        my_printf(&huart1, "FAILED (content mismatch)\r\n");
        f_unlink(new_name);
        return -1;
    }

    /* 验证旧文件已不存在 */
    res = f_open(&file, old_name, FA_READ);
    if (res == FR_OK) {
        f_close(&file);
        my_printf(&huart1, "FAILED (old file still exists)\r\n");
        f_unlink(old_name);
        f_unlink(new_name);
        return -1;
    }

    /* 清理 */
    f_unlink(new_name);

    my_printf(&huart1, "PASSED\r\n");
    return 0;
}
