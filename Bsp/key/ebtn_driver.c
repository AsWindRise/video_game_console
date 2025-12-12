#include "ebtn_driver.h"
#include "gpio.h"

// -----------------------------------------------------------------------------
// 0. 内部函数声明与全局变量
// -----------------------------------------------------------------------------

// 内部函数声明
static int btn_combos_init(void);
static uint8_t prv_get_state_callback(struct ebtn_btn *btn);
static void prv_btn_event_callback(struct ebtn_btn *btn, ebtn_evt_t evt);

// -----------------------------------------------------------------------------
// 1. 静态参数与按键列表
// -----------------------------------------------------------------------------

/**
 * @brief 默认按键参数配置实例。
 * 所有的静态按键都将使用此配置。
 */
// 参数宏: EBTN_PARAMS_INIT(按下消抖, 释放消抖, 单击最短按下, 单击最长按下, 多次单击最大间隔, 长按周期, 最大连击数)
const ebtn_btn_param_t default_ebtn_param = EBTN_PARAMS_INIT(
    20,  // time_debounce: 按下稳定 20ms (去抖时间)
    20,  // time_debounce_release: 释放稳定 20ms (释放去抖时间)
    50,  // time_click_pressed_min: 有效单击最短按下时间
    500, // time_click_pressed_max: 有效单击最长按下时间 (超过则视为长按或无效)
    300, // time_click_multi_max: 多次单击之间的最大间隔时间
    500, // time_keepalive_period: 长按事件的周期 (每 500ms 触发一次 KEEPALIVE)
    5    // max_consecutive: 最大连续点击次数
);

/**
 * @brief 静态独立按键列表。
 */
static ebtn_btn_t btns[] = {
    EBTN_BUTTON_INIT(BTN_SW1, &default_ebtn_param),
    EBTN_BUTTON_INIT(BTN_SW2, &default_ebtn_param),
    EBTN_BUTTON_INIT(BTN_SW3, &default_ebtn_param),
    EBTN_BUTTON_INIT(BTN_SW4, &default_ebtn_param),
    EBTN_BUTTON_INIT(BTN_SK, &default_ebtn_param),
};

/**
 * @brief 静态组合按键列表。
 */
ebtn_btn_combo_t btn_combos[] = {
    EBTN_BUTTON_COMBO_INIT(BTN_COMBO_0, &default_ebtn_param),
    EBTN_BUTTON_COMBO_INIT(BTN_COMBO_1, &default_ebtn_param),
    EBTN_BUTTON_COMBO_INIT(BTN_COMBO_2, &default_ebtn_param),
};

// -----------------------------------------------------------------------------
// 2. 底层驱动连接回调实现
// -----------------------------------------------------------------------------

/**
 * @brief HAL 状态获取回调函数 (供 ebtn 库调用)。
 * @return 1 为按下 (低电平活动)，0 为释放。
 */
static uint8_t prv_get_state_callback(struct ebtn_btn *btn)
{
    switch (btn->key_id)
    {
    case BTN_SW1:
        return !HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin);
    case BTN_SW2:
        return !HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin);
    case BTN_SW3:
        return !HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin);
    case BTN_SW4:
        return !HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin);
    case BTN_SK:
        return !HAL_GPIO_ReadPin(SK_GPIO_Port, SK_Pin);
    default:
        return 0;
    }
}

/**
 * @brief 按钮事件回调函数 (事件转发)。
 * 职责：将 ebtn 核心事件打包为 app_event_t，并推入 Event Queue。
 */
static void prv_btn_event_callback(struct ebtn_btn *btn, ebtn_evt_t evt)
{
    // 1. 打包事件 (Pack the Event)

    // 声明事件结构体变量。注意：使用 static 关键字需要考虑并发安全。
    auto app_event_t ebtn_event_t;

    // 将按键 ID (事件来源) 存入事件结构体
    ebtn_event_t.source_id = btn->key_id;

    // 将事件类型存入事件结构体 (需要显式类型转换，确保类型匹配 uint8_t)
    ebtn_event_t.event_type = (uint8_t)evt;

    // 附加数据字段 (暂时设为 0，未来用于连击数或摇杆值)
    if (evt == EBTN_EVT_ONCLICK)
    {
        ebtn_event_t.data = (uint32_t)btn->click_cnt;
    }
    else
    {
        ebtn_event_t.data = 0;
    }

    // 2. 推入队列 (Push to Channel)

    // 将打包好的事件发送到队列，完成事件从驱动层到组件层的转发。
    event_queue_push(ebtn_event_t);
}

// -----------------------------------------------------------------------------
// 3. 组合键绑定实现
// -----------------------------------------------------------------------------

/**
 * @brief 组合按键绑定初始化。
 */
static int btn_combos_init()
{
    int key1_index = ebtn_get_btn_index_by_key_id(BTN_SW1);
    int key2_index = ebtn_get_btn_index_by_key_id(BTN_SW2);
    int key3_index = ebtn_get_btn_index_by_key_id(BTN_SW3);

    // BTN_COMBO_0: BTN_SW1 + BTN_SW2
    if (key1_index >= 0 && key2_index >= 0)
    {
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[0], key1_index);
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[0], key2_index);
    }
    // BTN_COMBO_1: BTN_SW1 + BTN_SW3
    if (key1_index >= 0 && key3_index >= 0)
    {
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[1], key1_index);
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[1], key3_index);
    }
    // BTN_COMBO_2: BTN_SW2 + BTN_SW3
    if (key2_index >= 0 && key3_index >= 0)
    {
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[2], key2_index);
        ebtn_combo_btn_add_btn_by_idx(&btn_combos[2], key3_index);
    }
    return 0;
}

// -----------------------------------------------------------------------------
// 4. 驱动 API 实现
// -----------------------------------------------------------------------------

/**
 * @brief ebtn 驱动初始化函数。
 */
void ebtn_driver_init(void)
{
    // 1. 调用核心库初始化
    ebtn_init(
        btns,
        EBTN_ARRAY_SIZE(btns),
        btn_combos,
        EBTN_ARRAY_SIZE(btn_combos),
        prv_get_state_callback,
        prv_btn_event_callback);
    // 2. 绑定组合键
    btn_combos_init();
}

/**
 * @brief ebtn 库处理任务。
 */
void ebtn_process_task(void)
{
    // 传入当前毫秒系统时间 (HAL_GetTick())
    ebtn_process(HAL_GetTick());
}
