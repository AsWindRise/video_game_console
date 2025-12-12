#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "mydefine.h"

// 环形事件队列的容量由两个参数构成：
// 槽数存储 16 个事件，每个事件 8 字节（结构体大小）
#define EVENT_QUEUE_CAPACITY_SLOTS 16
#define EVENT_QUEUE_BUFFER_SIZE (EVENT_QUEUE_CAPACITY_SLOTS * sizeof(app_event_t))


// -----------------------------------------------------------------------------
// 1. 统一事件结构体 (app_event_t)
// -----------------------------------------------------------------------------

/**
 * @brief 统一应用层事件结构体。
 * 职责：封装各个输入源 (按键, 摇杆) 的信息。
 */
typedef struct
{
    uint16_t source_id;     /*!< 事件来源 ID (BTN_SW1, ROCKER_UP 等) */
    uint8_t event_type;     /*!< 事件类型 (EBTN_EVT_ONCLICK, DIRECTION_MOVE 等) */
    uint32_t data;          /*!< 附加数据，用于传递额外信息 (如按键次数, 摇杆原始值) */
} app_event_t;


// -----------------------------------------------------------------------------
// 2. 事件队列 API 声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化事件队列组件。
 */
void event_queue_init(void);

/**
 * @brief 向队列中推入一个事件 (Push)。
 * 职责：供底层驱动层 (如 ebtn_driver) 调用。
 * @param evt: 要推入的事件实例。
 * @return bool: 成功返回 true，队列已满返回 false。
 */
bool event_queue_push(app_event_t evt);

/**
 * @brief 从队列中取出一个事件 (Pop)。
 * 职责：供应用层调用 (如 menu_task, game_task) 读取并处理。
 * @param evt_out: 指向存储返回事件的缓冲区指针。
 * @return bool: 队列非空且成功取出一个事件返回 true，队列为空返回 false。
 */
bool event_queue_pop(app_event_t *evt_out);

/**
 * @brief 清空事件队列中的所有事件
 * @note  用于场景切换时清除残留事件，避免新场景处理旧事件
 */
void event_queue_clear(void);


#endif // __EVENT_QUEUE_H__
