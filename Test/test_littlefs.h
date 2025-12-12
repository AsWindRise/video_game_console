/**
 ******************************************************************************
 * @file    test_littlefs.h
 * @brief   LittleFS文件系统测试代码头文件
 * @author  老王
 * @note    这个文件提供LittleFS文件系统的各种测试函数
 *          包括挂载、文件读写、目录操作等测试
 ******************************************************************************
 */

#ifndef __TEST_LITTLEFS_H__
#define __TEST_LITTLEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mydefine.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief LittleFS测试结果枚举
 */
typedef enum {
    LFS_TEST_PASS = 0,    // 测试通过
    LFS_TEST_FAIL,        // 测试失败
    LFS_TEST_SKIP         // 测试跳过
} lfs_test_result_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief LittleFS测试初始化函数
 * @note  初始化LittleFS并挂载文件系统
 * @return 0成功，负数失败
 */
int test_littlefs_init(void);

/**
 * @brief 运行所有LittleFS测试
 * @note  依次运行所有测试用例并打印结果
 */
void test_littlefs_run_all(void);

/* ========== 各种测试函数 ========== */

/**
 * @brief Flash ID读取测试
 * @note  读取Flash ID验证SPI通信正常
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_flash_id(void);

/**
 * @brief 文件系统挂载测试
 * @note  测试挂载/卸载文件系统
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_mount(void);

/**
 * @brief 文件创建与写入测试
 * @note  测试创建新文件并写入数据
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_file_write(void);

/**
 * @brief 文件读取测试
 * @note  测试读取文件内容
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_file_read(void);

/**
 * @brief 文件追加测试
 * @note  测试向文件追加数据
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_file_append(void);

/**
 * @brief 目录操作测试
 * @note  测试创建、遍历、删除目录
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_directory(void);

/**
 * @brief 文件删除测试
 * @note  测试删除文件
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_file_remove(void);

/**
 * @brief 掉电安全测试(模拟)
 * @note  测试写入过程中断后的数据完整性
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_power_loss(void);

/**
 * @brief 文件系统容量测试
 * @note  获取文件系统使用情况
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_capacity(void);

/**
 * @brief 游戏存档读写测试
 * @note  模拟游戏存档的保存和读取
 * @return 测试结果
 */
lfs_test_result_t test_littlefs_game_save(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_LITTLEFS_H__ */
