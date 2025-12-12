/**
 ******************************************************************************
 * @file    test_u8g2.h
 * @brief   u8g2图形库测试代码头文件
 * @author  老王
 * @note    这个文件提供u8g2图形库的各种测试函数
 *          包括基本图形、文本、动画等测试
 ******************************************************************************
 */

#ifndef __TEST_U8G2_H__
#define __TEST_U8G2_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mydefine.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief u8g2测试模式枚举
 */
typedef enum {
    U8G2_TEST_BASIC_SHAPES = 0,  // 基本图形测试
    U8G2_TEST_TEXT,              // 文本显示测试
    U8G2_TEST_ANIMATION,         // 动画测试
    U8G2_TEST_MENU,              // 菜单UI测试
    U8G2_TEST_MIXED,             // 混合显示测试
    U8G2_TEST_MODE_MAX           // 测试模式数量
} u8g2_test_mode_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief u8g2测试初始化函数
 * @note  初始化测试相关的变量和状态
 */
void test_u8g2_init(void);

/**
 * @brief u8g2测试任务(周期性调用)
 * @note  这个函数会被调度器周期性调用,用于显示动态内容
 *        可以在scheduler中注册这个任务
 */
void test_u8g2_task(void);

/**
 * @brief 切换测试模式
 * @note  切换到下一个测试模式,用于演示不同的显示效果
 */
void test_u8g2_next_mode(void);

/**
 * @brief 获取当前测试模式
 * @return 当前的测试模式
 */
u8g2_test_mode_t test_u8g2_get_mode(void);

/* ========== 各种测试函数 ========== */

/**
 * @brief 基本图形测试
 * @note  测试绘制点、线、矩形、圆形、三角形等基本图形
 */
void test_u8g2_basic_shapes(void);

/**
 * @brief 文本显示测试
 * @note  测试不同字体、不同大小的文本显示
 */
void test_u8g2_text_display(void);

/**
 * @brief 动画测试
 * @note  测试简单的动画效果(如弹跳球、进度条等)
 */
void test_u8g2_animation(void);

/**
 * @brief 菜单UI测试
 * @note  测试简单的菜单界面显示
 */
void test_u8g2_menu_ui(void);

/**
 * @brief 混合显示测试
 * @note  测试图形+文本的混合显示效果
 */
void test_u8g2_mixed_display(void);

/**
 * @brief 性能测试
 * @note  测试u8g2的刷新性能和帧率
 */
void test_u8g2_performance(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_U8G2_H__ */
