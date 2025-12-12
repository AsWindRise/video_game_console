#ifndef __SYS_KEY_MONITOR_H__
#define __SYS_KEY_MONITOR_H__

#include "mydefine.h"

/**
 * @brief 周期性系统监控任务。
 */
void sys_monitor_task(void);

/**
 * @brief 临时的按键事件处理函数 (推入队列)。
 */
void app_key_event_handler(uint16_t key_id, ebtn_evt_t evt);

/**
 * @brief 全局事件计数器，用于验证是否收到事件。
 */
extern uint32_t g_event_count;


#endif // __SYS_KEY_MONITOR_H__
