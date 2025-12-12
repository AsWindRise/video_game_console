#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

// 引入全局配置头文件，包含必要的标准类型和HAL库引用
#include "mydefine.h" 

// -----------------------------------------------------------------------------
// 调度器组件核心 API
// -----------------------------------------------------------------------------

/**
 * @brief 初始化调度器组件。
 */
void scheduler_init(void);

/**
 * @brief 运行调度器主循环。该函数应在 main() 的 while(1) 循环中被调用。
 */
void scheduler_run(void);

/**
 * @brief 向调度器添加一个任务。
 * @param task_func: 任务函数指针。
 * @param rate_ms: 任务的执行周期（单位：毫秒）。
 * @return bool: 任务添加成功返回 true，失败（如任务数组已满）返回 false。
 */
bool scheduler_add_task(void (*task_func)(void), uint32_t rate_ms);

#endif // __SCHEDULER_H__
