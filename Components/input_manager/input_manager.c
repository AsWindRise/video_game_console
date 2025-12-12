#include "input_manager.h"
#include "event_queue.h"
#include "ebtn_driver.h"
#include "rocker.h"

// -----------------------------------------------------------------------------
// 事件源ID分配规则
// -----------------------------------------------------------------------------
// ebtn（按键）: 0 ~ (ROCKER_SOURCE_ID - 1)
//   - 单键: BTN_SW1(0) ~ BTN_SK(4)
//   - 组合键: BTN_COMBO_0(101) ~ BTN_COMBO_MAX
// rocker（摇杆）: ROCKER_SOURCE_ID (0x0100 = 256)
//
// 判断方法: evt.source_id < ROCKER_SOURCE_ID 则为按键事件

// -----------------------------------------------------------------------------
// 私有变量
// -----------------------------------------------------------------------------

// 按键状态数组
static uint8_t btn_pressed[INPUT_BTN_MAX] = {0};       // 当前是否按下
static uint8_t btn_just_pressed[INPUT_BTN_MAX] = {0};  // 刚刚按下标志
static uint8_t btn_just_released[INPUT_BTN_MAX] = {0}; // 刚刚释放标志
static uint8_t btn_double_click[INPUT_BTN_MAX] = {0};  // 双击标志

// -----------------------------------------------------------------------------
// 私有函数
// -----------------------------------------------------------------------------

/**
 * @brief 清除所有"刚刚"状态标志
 * @note  每帧开始时调用，确保边缘触发只维持一帧
 */
static void clear_edge_flags(void)
{
    for (uint8_t i = 0; i < INPUT_BTN_MAX; i++)
    {
        btn_just_pressed[i] = 0;
        btn_just_released[i] = 0;
        btn_double_click[i] = 0;  // 清除双击标志
    }
}

/**
 * @brief 将硬件按键ID映射到input_button_t
 * @param source_id: 来自event_queue的source_id（BTN_SW1等）
 * @retval 映射后的input_button_t，失败返回INPUT_BTN_MAX
 */
static input_button_t map_button_id(uint16_t source_id)
{
    switch (source_id)
    {
    case BTN_SW1:
        return INPUT_BTN_Y;
    case BTN_SW2:
        return INPUT_BTN_X;
    case BTN_SW3:
        return INPUT_BTN_A;
    case BTN_SW4:
        return INPUT_BTN_B;
    case BTN_SK:
        return INPUT_BTN_START;  // 摇杆中心按键
    default:
        return INPUT_BTN_MAX; // 无效ID
    }
}

/**
 * @brief 将摇杆方向映射到input_button_t
 * @param direction: 摇杆方向枚举
 * @retval 映射后的input_button_t，中心位置返回INPUT_BTN_MAX
 */
static input_button_t map_rocker_direction(rocker_direction_t direction)
{
    switch (direction)
    {
    case ROCKER_DIR_UP:
        return INPUT_BTN_UP;
    case ROCKER_DIR_DOWN:
        return INPUT_BTN_DOWN;
    case ROCKER_DIR_LEFT:
        return INPUT_BTN_LEFT;
    case ROCKER_DIR_RIGHT:
        return INPUT_BTN_RIGHT;

    // 对角方向优先映射为主要方向（可根据游戏需求调整）
    case ROCKER_DIR_UP_LEFT:
        return INPUT_BTN_UP;
    case ROCKER_DIR_UP_RIGHT:
        return INPUT_BTN_UP;
    case ROCKER_DIR_DOWN_LEFT:
        return INPUT_BTN_DOWN;
    case ROCKER_DIR_DOWN_RIGHT:
        return INPUT_BTN_DOWN;

    default:
        return INPUT_BTN_MAX; // 中心位置
    }
}

/**
 * @brief 处理按键事件
 * @param btn: 按键枚举
 * @param is_press: 1=按下, 0=释放
 */
static void handle_button_event(input_button_t btn, uint8_t is_press)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return; // 无效按键，忽略
    }

    if (is_press)
    {
        // 按下事件
        if (!btn_pressed[btn]) // 之前未按下，这是新的按下
        {
            btn_pressed[btn] = 1;
            btn_just_pressed[btn] = 1;
        }
    }
    else
    {
        // 释放事件
        if (btn_pressed[btn]) // 之前是按下的，这是新的释放
        {
            btn_pressed[btn] = 0;
            btn_just_released[btn] = 1;
        }
    }
}

/**
 * @brief 处理ebtn按键事件
 * @param evt: 事件结构体指针
 */
static void process_ebtn_event(app_event_t *evt)
{
    input_button_t btn = map_button_id(evt->source_id);

    if (btn >= INPUT_BTN_MAX)
    {
        return; // 无效按键
    }

    // 根据事件类型处理
    switch (evt->event_type)
    {
    case EBTN_EVT_ONPRESS:
        handle_button_event(btn, 1);
        break;

    case EBTN_EVT_ONRELEASE:
        handle_button_event(btn, 0);
        break;

    case EBTN_EVT_ONCLICK:
        // 处理单击/双击事件
        // evt->data包含点击次数（来自ebtn的click_cnt字段）
        {
            uint16_t click_count = evt->data;  // 获取点击次数
            if (click_count >= 2)
            {
                // 检测到双击
                btn_double_click[btn] = 1;
            }
        }
        break;

    // KEEPALIVE事件不需要处理
    default:
        break;
    }
}

/**
 * @brief 处理摇杆事件
 * @param evt: 事件结构体指针
 */
static void process_rocker_event(app_event_t *evt)
{
    // 解包摇杆事件数据
    rocker_direction_t dir = ROCKER_EVT_UNPACK_DIR(evt->data);
    input_button_t btn = map_rocker_direction(dir);

    if (btn >= INPUT_BTN_MAX)
    {
        return; // 中心位置，不处理
    }

    // 根据事件类型处理
    switch (evt->event_type)
    {
    case ROCKER_EVT_DIR_ENTER:
        handle_button_event(btn, 1);
        break;

    case ROCKER_EVT_DIR_LEAVE:
        handle_button_event(btn, 0);
        break;

    // HOLD事件不需要特殊处理，状态已经是按下
    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// 公共函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化输入管理器
 */
void input_manager_init(void)
{
    // 清空所有状态
    for (uint8_t i = 0; i < INPUT_BTN_MAX; i++)
    {
        btn_pressed[i] = 0;
        btn_just_pressed[i] = 0;
        btn_just_released[i] = 0;
        btn_double_click[i] = 0;
    }
}

/**
 * @brief 清空输入管理器的所有状态
 * @note  用于场景切换时清除残留的按键状态
 */
void input_manager_clear(void)
{
    // 清空所有状态（与init相同）
    for (uint8_t i = 0; i < INPUT_BTN_MAX; i++)
    {
        btn_pressed[i] = 0;
        btn_just_pressed[i] = 0;
        btn_just_released[i] = 0;
        btn_double_click[i] = 0;
    }
}

/**
 * @brief 输入管理器任务处理函数
 */
void input_manager_task(void)
{
    // 清除上一帧的边缘触发标志
    clear_edge_flags();

    // 处理事件队列中的所有事件
    app_event_t evt;
    while (event_queue_pop(&evt))
    {
        // 判断事件来源（根据ID范围区分）
        if (evt.source_id < ROCKER_SOURCE_ID)
        {
            // 来自ebtn（按键）：ID < 256
            process_ebtn_event(&evt);
        }
        else if (evt.source_id == ROCKER_SOURCE_ID)
        {
            // 来自rocker（摇杆）：ID = 256
            process_rocker_event(&evt);
        }
        // 其他事件源忽略
    }
}

/**
 * @brief 查询按键是否正在被按下
 */
uint8_t input_is_pressed(input_button_t btn)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return 0;
    }
    return btn_pressed[btn];
}

/**
 * @brief 查询按键是否刚刚按下
 */
uint8_t input_is_just_pressed(input_button_t btn)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return 0;
    }
    return btn_just_pressed[btn];
}

/**
 * @brief 查询按键是否刚刚释放
 */
uint8_t input_is_just_released(input_button_t btn)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return 0;
    }
    return btn_just_released[btn];
}

/**
 * @brief 获取按键的完整状态
 */
input_state_t input_get_state(input_button_t btn)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return INPUT_STATE_IDLE;
    }

    if (btn_just_pressed[btn])
    {
        return INPUT_STATE_JUST_PRESSED;
    }
    else if (btn_just_released[btn])
    {
        return INPUT_STATE_JUST_RELEASED;
    }
    else if (btn_pressed[btn])
    {
        return INPUT_STATE_PRESSED;
    }
    else
    {
        return INPUT_STATE_IDLE;
    }
}

/**
 * @brief 获取任意方向键是否按下
 */
uint8_t input_any_direction_pressed(void)
{
    return btn_pressed[INPUT_BTN_UP] ||
           btn_pressed[INPUT_BTN_DOWN] ||
           btn_pressed[INPUT_BTN_LEFT] ||
           btn_pressed[INPUT_BTN_RIGHT];
}

/**
 * @brief 获取任意功能键是否按下
 */
uint8_t input_any_button_pressed(void)
{
    return btn_pressed[INPUT_BTN_A] ||
           btn_pressed[INPUT_BTN_B] ||
           btn_pressed[INPUT_BTN_X] ||
           btn_pressed[INPUT_BTN_Y] ||
           btn_pressed[INPUT_BTN_START];
}

/**
 * @brief 查询按键是否双击
 */
uint8_t input_is_double_click(input_button_t btn)
{
    if (btn >= INPUT_BTN_MAX)
    {
        return 0;
    }
    return btn_double_click[btn];
}
