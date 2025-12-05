#ifndef __EBTN_DRIVER_H__
#define __EBTN_DRIVER_H__

// 引入全局配置头文件和核心库
#include "mydefine.h"
#include "ebtn.h" // 引用核心库 API (需要 ebtn_evt_t, ebtn_btn_t 等类型)
#include  "event_queue.h"// 引用组件库event_queue的API

// -----------------------------------------------------------------------------
// 1. 按键 ID 定义
// -----------------------------------------------------------------------------

/**
 * @brief 物理按键和静态组合按键的唯一标识符。
 * * 注意：ID 必须保持唯一性，组合键 ID 必须与物理按键 ID 错开。
 */
typedef enum
{
    // 物理按键 ID (索引 0 开始)
    BTN_SW1 = 0,    // 对应 SW1 (PE0)
    BTN_SW2,        // 对应 SW2 (PE1)
    BTN_SW3,        // 对应 SW3 (PE2)
    BTN_SW4,        // 对应 SW4 (PE3)
    BTN_SK,         // 对应 SK_Pin (PE4)
    BTN_MAX_COUNT,  // 总独立按键数

    // 静态组合按键 ID (从 101 开始，避免与物理按键冲突)
    BTN_COMBO_0 = 101, // 假设: SW1 + SW2 组合键
    BTN_COMBO_1,       // 假设: SW1 + SW3 组合键
    BTN_COMBO_2,       // 假设: SW2 + SW3 组合键
    BTN_COMBO_MAX,     // 组合按键最大 ID
} button_id_t;


// -----------------------------------------------------------------------------
// 2. 驱动 API 声明
// -----------------------------------------------------------------------------

/**
 * @brief ebtn 驱动初始化函数。
 * 职责：初始化 ebtn 库，配置按键数组和回调。该函数在 system_assembly_init 中调用。
 */
void ebtn_driver_init(void); 

/**
 * @brief ebtn 库处理任务 (周期性任务)。
 * 职责：由调度器中周期性运行 (如 10ms)，驱动 ebtn 状态机。
 */
void ebtn_process_task(void);

#endif // __EBTN_DRIVER_H__
