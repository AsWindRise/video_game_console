#ifndef __ROCKER_APP_H__
#define __ROCKER_APP_H__

#include "mydefine.h"

/**
 * @brief 摇杆应用层初始化
 * @note  负责初始化ADC驱动、摇杆组件、启用事件推送
 */
void rocker_app_init(void);

/**
 * @brief 摇杆处理任务
 * @note  10ms周期调用，负责采集ADC数据、校准、更新摇杆组件状态
 */
void rocker_process_task(void);

#endif // __ROCKER_APP_H__
