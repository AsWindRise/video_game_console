#include "sys_key_monitor.h"
#include "uart_driver.h"
#include "event_queue.h" // <-- 引入全局事件队列接口
#include "rocker.h"      // <-- 引入摇杆SOURCE_ID定义，用于事件过滤

// -----------------------------------------------------------------------------
// 1. 全局变量定义
// -----------------------------------------------------------------------------

/**
 * @brief 全局事件计数器变量
 */
uint32_t g_event_count = 0;

// -----------------------------------------------------------------------------
// 2. 辅助函数定义 (用于打印)
// -----------------------------------------------------------------------------

/**
 * @brief 将按键 ID 转换为可读的字符串
 */
static const char *get_key_name(uint16_t key_id)
{
    // 使用 ebtn_driver.h 中定义的按键 ID
    switch (key_id)
    {
    case BTN_SW1:
        return "BTN_SW1";
    case BTN_SW2:
        return "BTN_SW2";
    case BTN_SW3:
        return "BTN_SW3";
    case BTN_SW4:
        return "BTN_SW4";
    case BTN_SK:
        return "BTN_SK";
    case BTN_COMBO_0:
        return "COMBO_SW1+SW2";
    case BTN_COMBO_1:
        return "COMBO_SW1+SW3";
    case BTN_COMBO_2:
        return "COMBO_SW2+SW3";
    default:
        return "UNKNOWN_KEY";
    }
}

/**
 * @brief 将按键事件类型转换为可读的字符串
 */
static const char *get_event_name(ebtn_evt_t evt)
{
    switch (evt)
    {
    case EBTN_EVT_ONPRESS:
        return "ONPRESS";
    case EBTN_EVT_ONRELEASE:
        return "ONRELEASE";
    case EBTN_EVT_ONCLICK:
        return "ONCLICK";
    case EBTN_EVT_KEEPALIVE:
        return "KEEPALIVE";
    default:
        return "OTHER_EVENT";
    }
}

// -----------------------------------------------------------------------------
// 3. 任务函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 监控系统事件处理
 * 职责：监控系统按键事件，遇到摇杆事件则放回队列留给专用处理任务
 *
 * ⚠️ 重要：本任务只处理按键事件（source_id < 0x0100）
 *          摇杆事件（source_id = ROCKER_SOURCE_ID）会被放回队列
 */
void sys_monitor_task(void)
{
    // 1. 打印心跳 (低频)
    //    my_printf(&huart1, "[SYS] Heartbeat. Total events PUSHED: %lu\r\n", g_event_count);

    // 2. 事件队列循环 (高频/轮询)
    app_event_t received_event;

    // 只要队列不为空，就不断地弹出事件 (这是轮询方式)
    while (event_queue_pop(&received_event))
    {
        // ========== 艹！关键修改：过滤摇杆事件 ==========
        // 如果是摇杆事件，放回队列，留给 rocker_event_handler_task 处理
        if (received_event.source_id == ROCKER_SOURCE_ID)
        {
            event_queue_push(received_event); // 重新放回队列
            break;                            // 停止继续pop，避免死循环
        }

        // ========== 只处理按键事件 ==========
        my_printf(&huart1,
                  "[EQ_TEST] Key: %s, Event: %s, Data: %lu\r\n",
                  get_key_name(received_event.source_id),
                  get_event_name((ebtn_evt_t)received_event.event_type),
                  received_event.data);
    }
}

/**
 * @brief 过时的按键事件回调函数 (ebtn 回调)
 * 职责：不再打印，而是将事件推送到 Event Queue
 */
void app_key_event_handler(uint16_t key_id, ebtn_evt_t evt)
{
    g_event_count++;

    // 1. 构造事件
    app_event_t new_event;
    new_event.source_id = key_id;
    new_event.event_type = (uint8_t)evt;
    new_event.data = 0;

    // 2. 推送到 Event Queue
    event_queue_push(new_event);

    // 注意：我们移除了原有的打印，因为统一打印放在 sys_monitor_task 中
}
