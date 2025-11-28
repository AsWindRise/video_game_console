#ifndef __SYS_KEY_MONITOR_H__
#define __SYS_KEY_MONITOR_H__

#include "mydefine.h"
#include "ebtn_driver.h" // 引入按键 ID 和事件回调类型

// -----------------------------------------------------------------------------
// 1. 调试 API 声明
// -----------------------------------------------------------------------------

/**
 * @brief 周期性系统监控任务。
 * 职责：在调度器中运行，用于简单的调试打印或系统状态指示。
 */
void sys_monitor_task(void);

/**
 * @brief 临时的按键事件处理函数。
 * 职责：接收来自 ebtn_driver 的事件，通过串口打印信息，用于验证移植是否成功。
 *
 * @param key_id: 触发事件的按键 ID (button_id_t)。
 * @param evt: 触发的事件类型 (EBTN_EVT_ONCLICK, EBTN_EVT_KEEPALIVE, 等)。
 */
void app_key_event_handler(uint16_t key_id, ebtn_evt_t evt);


// -----------------------------------------------------------------------------
// 2. 状态变量声明
// -----------------------------------------------------------------------------

/**
 * @brief 全局事件计数器，用于验证是否收到事件。
 */
extern uint32_t g_event_count;


#endif // __SYS_KEY_MONITOR_H__
