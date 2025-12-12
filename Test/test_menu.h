#ifndef __TEST_MENU_H__
#define __TEST_MENU_H__

#include "mydefine.h"

// =============================================================================
// 菜单系统测试程序
// 功能：
// 1. 演示多级菜单导航
// 2. 演示动作函数绑定
// 3. 演示动态值显示
// 4. 验证菜单系统完整功能
// =============================================================================

/**
 * @brief 初始化测试菜单
 * @note  在system_assembly_init中调用
 */
void test_menu_init(void);

/**
 * @brief 测试菜单任务
 * @note  在调度器中注册，10ms周期调用
 */
void test_menu_task(void);

#endif // __TEST_MENU_H__
