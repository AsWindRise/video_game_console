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
	// 系统各组件初始化
	scheduler_init();
	ebtn_driver_init();
	event_queue_init();
	rocker_adc_driver_init();
	test_rocker_adc_init();

	// 初始化u8g2显示组件
	u8g2_component_init();

	// 初始化u8g2测试
	test_u8g2_init();
}

/**
 * @brief 应用任务注册函数。
 * 职责：将所有应用层任务注册到调度器中。
 */
void system_assembly_register_tasks(void)
{
	scheduler_add_task(test_task, 10);
	scheduler_add_task(ebtn_process_task, 10);
	scheduler_add_task(sys_monitor_task, 10);
	scheduler_add_task(test_u8g2_task, 50);
}

void test_task(void)
{
	key_task();
	time++;
}
