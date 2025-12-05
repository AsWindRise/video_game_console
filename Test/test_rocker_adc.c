#include "test_rocker_adc.h"
#include "rocker_adc_driver.h" // 摇杆ADC驱动
#include "rocker.h"            // 摇杆组件
#include "event_queue.h"       // 事件队列
#include "uart_driver.h"       // 串口打印
#include "scheduler.h"         // 调度器

// =============================================================================
// 摇杆组件 + 事件队列 测试示例
// =============================================================================

// -----------------------------------------------------------------------------
// 外部句柄声明
// -----------------------------------------------------------------------------
extern UART_HandleTypeDef huart1;

// -----------------------------------------------------------------------------
// 私有变量
// -----------------------------------------------------------------------------
static bool s_calibration_done = false;
static uint8_t s_init_counter = 0;

// -----------------------------------------------------------------------------
// 私有函数声明
// -----------------------------------------------------------------------------
static void rocker_raw_update_task(void);
static void rocker_event_handler_task(void);

// -----------------------------------------------------------------------------
// 任务1：摇杆原始数据采集 + 组件更新（高频，如20ms）
// -----------------------------------------------------------------------------
static void rocker_raw_update_task(void)
{
    // 1. 从ADC驱动获取原始数据
    rocker_data_t raw = rocker_adc_get_raw_value();

    // 2. 上电校准（等待ADC稳定后自动校准中心点）
    if (!s_calibration_done)
    {
        s_init_counter++;
        if (s_init_counter >= 10) // 等待约200ms
        {
            rocker_calibrate_center((uint16_t)raw.x_raw_value,
                                    (uint16_t)raw.y_raw_value);
            s_calibration_done = true;
            my_printf(&huart1, "[ROCKER] 中心点校准完成: X=%lu, Y=%lu\r\n",
                      raw.x_raw_value, raw.y_raw_value);
        }
        return;
    }

    // 3. 更新摇杆组件（内部自动推送事件到队列）
    rocker_update((uint16_t)raw.x_raw_value, (uint16_t)raw.y_raw_value);
}

// -----------------------------------------------------------------------------
// 任务2：事件处理（可以放在主循环或低频任务中）
// -----------------------------------------------------------------------------
static void rocker_event_handler_task(void)
{
    app_event_t evt;

    // 循环处理队列中的所有事件
    while (event_queue_pop(&evt))
    {
        // 只处理摇杆事件
        if (evt.source_id == ROCKER_SOURCE_ID)
        {
            // 解包数据
            rocker_direction_t dir = ROCKER_EVT_UNPACK_DIR(evt.data);
            uint8_t mag = ROCKER_EVT_UNPACK_MAG(evt.data);

            // 根据事件类型处理
            switch (evt.event_type)
            {
            case ROCKER_EVT_DIR_ENTER:
                // 摇杆进入某个方向
                my_printf(&huart1, "[EVENT] 进入方向: %-10s (幅度:%3u%%)\r\n",
                          rocker_get_direction_name(dir), mag);
                break;

            case ROCKER_EVT_DIR_LEAVE:
                // 摇杆离开某个方向（回到中心）
                my_printf(&huart1, "[EVENT] 离开方向: %-10s\r\n",
                          rocker_get_direction_name(dir));
                break;

            case ROCKER_EVT_DIR_HOLD:
                // 摇杆持续保持某个方向（仅当启用HOLD事件时触发）
                my_printf(&huart1, "[EVENT] 保持方向: %-10s (幅度:%3u%%)\r\n",
                          rocker_get_direction_name(dir), mag);
                break;

            default:
                break;
            }
        }
        // 这里可以继续处理按键等其他事件源
        // else if (evt.source_id == BTN_SOURCE_ID) { ... }
    }
}

// -----------------------------------------------------------------------------
// 测试模块初始化
// -----------------------------------------------------------------------------
void test_rocker_adc_init(void)
{
    // 1. 初始化摇杆组件（使用默认配置）
    //    默认：死区100，滤波4次采样，输出范围-100~+100
    rocker_init(NULL);

    // 2. 启用事件队列推送
    rocker_event_enable(true);

    // 3. (可选) 启用HOLD事件，每300ms触发一次
    //    如果不需要持续触发，可以注释掉这行
    rocker_event_hold_enable(true, 300);

    // 4. 注册任务到调度器
    //    - 摇杆更新任务：20ms执行一次（高频采样）
    //    - 事件处理任务：50ms执行一次（处理事件队列）
    scheduler_add_task(rocker_raw_update_task, 20);
    scheduler_add_task(rocker_event_handler_task, 50);

    my_printf(&huart1, "\r\n========================================\r\n");
    my_printf(&huart1, "  摇杆组件 + 事件队列 测试启动\r\n");
    my_printf(&huart1, "  - 事件推送: 已启用\r\n");
    my_printf(&huart1, "  - HOLD事件: 已启用 (300ms间隔)\r\n");
    my_printf(&huart1, "========================================\r\n\r\n");
}

// -----------------------------------------------------------------------------
// (可选) 原有的测试任务，直接打印状态（用于调试）
// -----------------------------------------------------------------------------
void test_rocker_task(void)
{
    if (!s_calibration_done)
        return;

    rocker_state_t state = rocker_get_state();

    my_printf(&huart1,
              "[STATE] X:%4d, Y:%4d | Dir:%-10s | Mag:%3u%% | Dead:%d\r\n",
              state.x,
              state.y,
              rocker_get_direction_name(state.direction),
              state.magnitude,
              state.in_deadzone);
}
