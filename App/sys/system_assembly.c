#include "system_assembly.h"

// -----------------------------------------------------------------------------
// 系统初始化和任务注册函数实现 (装配逻辑)
// -----------------------------------------------------------------------------
unsigned int time;
/**
 * @brief 系统的主要初始化函数。
 */
void system_assembly_init(void)
{
    // ... 初始化调用保持不变 ...
    scheduler_init(); 
		ebtn_driver_init();
		// 【注册事件回调】 
    // 这是将 ebtn 事件转发到您的打印函数 app_key_event_handler 的关键！
    ebtn_driver_register_callback(app_key_event_handler);
    // ...
}

/**
 * @brief 应用任务注册函数。
 * 职责：将所有应用层任务注册到调度器中。
 */
void system_assembly_register_tasks(void)
{
		scheduler_add_task(test_task, 10);
		scheduler_add_task(ebtn_process_task, 10);
		scheduler_add_task(sys_monitor_task, 500);
}

void test_task(void)
{
	key_task();
	time++;
}

