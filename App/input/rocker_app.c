#include "rocker_app.h"

/**
 * @brief 摇杆应用层初始化
 * @note  负责初始化整个摇杆系统（Bsp层驱动 + Components层组件）
 */
void rocker_app_init(void)
{
	// 1. 初始化ADC驱动（Bsp层）
	rocker_adc_driver_init();

	// 2. 初始化摇杆组件（Components层）
	rocker_init(NULL);           // 使用默认配置

	// 3. 启用事件推送（关键！没有这个就不会推送事件到event_queue）
	rocker_event_enable(true);
}

/**
 * @brief 摇杆处理任务
 * @note  10ms周期调用
 *
 * 职责：
 * 1. 从Bsp层获取ADC原始数据
 * 2. 上电自动校准中心点
 * 3. 调用Components层更新摇杆状态（自动推送事件到event_queue）
 */
void rocker_process_task(void)
{
	// 静态变量用于中心校准
	static bool calibration_done = false;
	static uint8_t init_counter = 0;

	// 1. 从ADC驱动获取原始数据（Bsp层）
	rocker_data_t raw = rocker_adc_get_raw_value();

	// 2. 上电校准（等待ADC稳定后自动校准中心点）
	if (!calibration_done)
	{
		init_counter++;
		if (init_counter >= 10) // 等待约100ms
		{
			rocker_calibrate_center((uint16_t)raw.x_raw_value,
			                        (uint16_t)raw.y_raw_value);
			calibration_done = true;
		}
		return;
	}

	// 3. 更新摇杆组件状态（滤波、死区、映射、方向检测，处理完成后自动推送事件）
	rocker_update((uint16_t)raw.x_raw_value, (uint16_t)raw.y_raw_value);
}
