#ifndef __TEST_INPUT_MANAGER_H__
#define __TEST_INPUT_MANAGER_H__

#include "mydefine.h"

// =============================================================================
// input_manager测试程序
// 功能：实时显示所有按键状态，验证input_manager功能
// 用法：在主循环中调用test_input_manager_task()
// =============================================================================

/**
 * @brief 初始化输入测试
 * @note  在input_manager_init()之后调用
 */
void test_input_manager_init(void);

/**
 * @brief 输入测试任务
 * @note  在主循环中调用，显示实时按键状态
 */
void test_input_manager_task(void);

#endif // __TEST_INPUT_MANAGER_H__
