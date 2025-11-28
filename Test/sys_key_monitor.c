#include "sys_key_monitor.h"

// -----------------------------------------------------------------------------
// 1. 全局变量定义
// -----------------------------------------------------------------------------

/**
 * @brief 全局事件计数器定义。
 */
uint32_t g_event_count = 0;


// -----------------------------------------------------------------------------
// 2. 调试辅助函数 (用于打印)
// -----------------------------------------------------------------------------

/**
 * @brief 将按键 ID 转换为可读的字符串。
 */
static const char* get_key_name(uint16_t key_id)
{
    // 使用 ebtn_driver.h 中定义的按键 ID
    switch (key_id)
    {
        case BTN_SW1: return "BTN_SW1";
        case BTN_SW2: return "BTN_SW2";
        case BTN_SW3: return "BTN_SW3";
        case BTN_SW4: return "BTN_SW4";
        case BTN_SK:  return "BTN_SK";
        case BTN_COMBO_0: return "COMBO_SW1+SW2";
        case BTN_COMBO_1: return "COMBO_SW1+SW3";
        case BTN_COMBO_2: return "COMBO_SW2+SW3";
        default: return "UNKNOWN_KEY";
    }
}

/**
 * @brief 将事件类型转换为可读的字符串。
 */
static const char* get_event_name(ebtn_evt_t evt)
{
    switch (evt)
    {
        case EBTN_EVT_ONPRESS:    return "ONPRESS";
        case EBTN_EVT_ONRELEASE:  return "ONRELEASE";
        case EBTN_EVT_ONCLICK:    return "ONCLICK";
        case EBTN_EVT_KEEPALIVE:  return "KEEPALIVE";
        default: return "OTHER_EVENT";
    }
}


// -----------------------------------------------------------------------------
// 3. 调试任务实现
// -----------------------------------------------------------------------------

/**
 * @brief 周期性系统监控任务。
 * 职责：检查系统是否存活，并打印事件接收状态。
 */
void sys_monitor_task(void)
{
    // 每 500ms 打印一次心跳和事件接收状态
    // *** 使用用户指定的调试句柄 &huart1 ***
    my_printf(&huart1, "[SYS] Heartbeat. Total events: %lu\r\n", g_event_count);
}

/**
 * @brief 临时的按键事件处理函数 (ebtn 回调)。
 * 职责：接收来自 ebtn_driver 的事件，用于验证移植是否成功。
 */
void app_key_event_handler(uint16_t key_id, ebtn_evt_t evt)
{
    // 增加事件计数
    g_event_count++; 
    
    // 打印详细的事件信息
    // *** 使用用户指定的调试句柄 &huart1 ***
    my_printf(&huart1, 
              "[EBTN_VERIFY] #%lu - Key: %s, Event: %s\r\n", 
              g_event_count, 
              get_key_name(key_id), 
              get_event_name(evt));
}
