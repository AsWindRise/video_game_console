#ifndef __TEST_ROCKER_ADC_H__
#define __TEST_ROCKER_ADC_H__

#include "mydefine.h"

// -----------------------------------------------------------------------------
// Test API Declaration
// -----------------------------------------------------------------------------

/**
 * @brief 测试模块初始化函数。
 * 职责：注册 test_rocker_task 到调度器。
 */
void test_rocker_adc_init(void);

/**
 * @brief 测试任务。
 * 职责：周期性地读取摇杆驱动，并将原始 ADC 值通过 UART 打印输出。
 */
void test_rocker_task(void);

#endif // __TEST_ROCKER_ADC_H__
