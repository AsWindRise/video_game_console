#ifndef __SYSTEM_ASSEMBLY_H__
#define __SYSTEM_ASSEMBLY_H__

// 包含全局头文件，确保具备标准类型和调度器 API 等依赖
#include "mydefine.h"

/**
 * @brief 系统核心组件初始化函数
 * 职责：初始化调度器、定时器、外设等核心初始化逻辑
 */
void system_assembly_init(void);

/**
 * @brief 应用任务注册函数
 * 职责：将所有应用层任务注册到调度器中。
 */
void system_assembly_register_tasks(void);
void test_task(void);
#endif // __SYSTEM_ASSEMBLY_H__
