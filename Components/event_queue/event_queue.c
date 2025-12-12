#include "event_queue.h"

// -----------------------------------------------------------------------------
// 1. 静态数据结构定义 (基于 RT-Thread Ring Buffer 实现)
// -----------------------------------------------------------------------------

// 静态分配的内存
static rt_uint8_t event_queue_buffer[EVENT_QUEUE_BUFFER_SIZE];

// 环形缓冲区控制块
static struct rt_ringbuffer rb_event_queue;


// -----------------------------------------------------------------------------
// 2. 队列 API 实现 (封装 RT-Thread API)
// -----------------------------------------------------------------------------

/**
 * @brief 初始化事件队列组件。
 * 职责：初始化 RT-Thread 环形缓冲区控制块。
 */
void event_queue_init(void)
{
    // 调用 RT-Thread 的初始化函数
    // 注意：这里将 size 参数设为缓冲区字节大小
    rt_ringbuffer_init(&rb_event_queue, event_queue_buffer, EVENT_QUEUE_BUFFER_SIZE);
}

/**
 * @brief 向队列中推入一个事件 (Push)。
 * 职责：供底层驱动层调用。
 * @param evt: 要推入的事件实例。
 * @return bool: 成功返回 true，队列已满返回 false。
 */
bool event_queue_push(app_event_t evt)
{
    rt_size_t put_len;

    // **关键：中断保护，防止竞态条件**
    // 生产者和消费者在不同任务或中断中，需要保护环形缓冲区的读写指针
    __disable_irq();

    // 调用 RT-Thread 的 put 函数，注意长度是结构体的大小
    put_len = rt_ringbuffer_put(&rb_event_queue, (const rt_uint8_t *)&evt, sizeof(app_event_t));

    __enable_irq();

    // 如果实际放入的字节数等于结构体大小，视为成功
    return put_len == sizeof(app_event_t);
}

/**
 * @brief 从队列中取出一个事件 (Pop)。
 * 职责：供应用层调用。
 * @param evt_out: 指向存储返回事件的缓冲区指针。
 * @return bool: 队列非空且成功取出一个事件返回 true，队列为空返回 false。
 */
bool event_queue_pop(app_event_t *evt_out)
{
    rt_size_t get_len;

    // **关键：中断保护，防止竞态条件**
    __disable_irq();

    // 检查是否有足够的空间来取出一个完整的事件结构体
    if (rt_ringbuffer_data_len(&rb_event_queue) < sizeof(app_event_t))
    {
        __enable_irq();
        return false;
    }

    // 调用 RT-Thread 的 get 函数
    get_len = rt_ringbuffer_get(&rb_event_queue, (rt_uint8_t *)evt_out, sizeof(app_event_t));

    __enable_irq();

    // 如果实际读取的字节数等于结构体大小，视为成功
    return get_len == sizeof(app_event_t);
}

/**
 * @brief 清空事件队列中的所有事件
 * @note  用于场景切换时清除残留事件，避免新场景处理旧事件
 */
void event_queue_clear(void)
{
    __disable_irq();
    rt_ringbuffer_reset(&rb_event_queue);  // 重置环形缓冲区
    __enable_irq();
}
