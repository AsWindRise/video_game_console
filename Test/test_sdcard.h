/**
 * @file    test_sdcard.h
 * @brief   SD卡(TF卡)和FATFS测试头文件
 * @note    SDIO + FATFS功能测试套件
 */

#ifndef __TEST_SDCARD_H
#define __TEST_SDCARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  初始化SD卡测试模块
 * @retval 0成功，负数失败
 */
int test_sdcard_init(void);

/**
 * @brief  运行所有SD卡测试
 * @retval 失败的测试数量（0表示全部通过）
 */
int test_sdcard_run_all(void);

/**
 * @brief  测试SD卡检测
 * @retval 0成功，-1失败
 */
int test_sdcard_detect(void);

/**
 * @brief  测试FATFS挂载/卸载
 * @retval 0成功，-1失败
 */
int test_sdcard_mount(void);

/**
 * @brief  测试文件写入
 * @retval 0成功，-1失败
 */
int test_sdcard_write(void);

/**
 * @brief  测试文件读取
 * @retval 0成功，-1失败
 */
int test_sdcard_read(void);

/**
 * @brief  测试文件追加写入
 * @retval 0成功，-1失败
 */
int test_sdcard_append(void);

/**
 * @brief  测试目录操作
 * @retval 0成功，-1失败
 */
int test_sdcard_directory(void);

/**
 * @brief  测试文件删除
 * @retval 0成功，-1失败
 */
int test_sdcard_delete(void);

/**
 * @brief  测试SD卡容量信息
 * @retval 0成功，-1失败
 */
int test_sdcard_capacity(void);

/**
 * @brief  测试游戏存档模拟
 * @retval 0成功，-1失败
 */
int test_sdcard_game_save(void);

/*============================================================================*/
/*                           高级测试功能                                      */
/*============================================================================*/

/**
 * @brief  运行高级测试套件
 * @retval 失败的测试数量（0表示全部通过）
 */
int test_sdcard_run_advanced(void);

/**
 * @brief  大文件写入速度测试
 * @param  size_kb 文件大小（KB）
 * @retval 0成功，-1失败
 */
int test_sdcard_write_speed(uint32_t size_kb);

/**
 * @brief  大文件读取速度测试
 * @param  size_kb 文件大小（KB）
 * @retval 0成功，-1失败
 */
int test_sdcard_read_speed(uint32_t size_kb);

/**
 * @brief  多层目录递归创建测试
 * @retval 0成功，-1失败
 */
int test_sdcard_nested_dirs(void);

/**
 * @brief  批量小文件创建测试
 * @param  count 文件数量
 * @retval 0成功，-1失败
 */
int test_sdcard_batch_files(uint32_t count);

/**
 * @brief  文件重命名测试
 * @retval 0成功，-1失败
 */
int test_sdcard_rename(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_SDCARD_H */
