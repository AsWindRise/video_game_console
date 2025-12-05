#include "input_manager.h"

// =============================================================================
// 用户输入抽象层实现
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 私有宏定义
// -----------------------------------------------------------------------------

/** 输入事件队列容量 */
#define INPUT_EVENT_QUEUE_SIZE 16

/** 重复触发延迟（首次长按后多久开始重复） */
#define INPUT_REPEAT_DELAY_MS 400

/** 重复触发间隔（重复触发的周期） */
#define INPUT_REPEAT_INTERVAL_MS 100

// -----------------------------------------------------------------------------
// 2. 私有数据
// -----------------------------------------------------------------------------

/** 游戏模式标志（游戏模式下SK按键生效） */
static bool s_game_mode;

/** 当前帧各动作的按下状态 */
static bool s_current_state[INPUT_ACTION_MAX];

/** 上一帧各动作的按下状态 */
static bool s_previous_state[INPUT_ACTION_MAX];

/** 输入事件队列 */
static input_event_t s_event_queue[INPUT_EVENT_QUEUE_SIZE];
static uint8_t s_queue_head;
static uint8_t s_queue_tail;
static uint8_t s_queue_count;

/** 重复触发相关 */
static uint32_t s_press_start_time[INPUT_ACTION_MAX];
static uint32_t s_last_repeat_time[INPUT_ACTION_MAX];
static bool s_repeat_started[INPUT_ACTION_MAX];

// -----------------------------------------------------------------------------
// 3. 私有函数声明
// -----------------------------------------------------------------------------

static void push_event(input_action_t action, input_event_type_t type, uint8_t click_count);
static input_action_t map_button_to_action(uint16_t btn_id);
static input_action_t map_rocker_dir_to_action(rocker_direction_t dir);
static void process_button_event(uint16_t source_id, uint8_t event_type, uint32_t data);
static void process_rocker_event(uint8_t event_type, uint32_t data);
static void update_repeat(void);

// -----------------------------------------------------------------------------
// 4. API 实现
// -----------------------------------------------------------------------------

void input_manager_init(void)
{
    // 默认菜单模式
    s_game_mode = false;

    // 清空状态
    for (int i = 0; i < INPUT_ACTION_MAX; i++)
    {
        s_current_state[i] = false;
        s_previous_state[i] = false;
        s_press_start_time[i] = 0;
        s_last_repeat_time[i] = 0;
        s_repeat_started[i] = false;
    }

    // 清空事件队列
    s_queue_head = 0;
    s_queue_tail = 0;
    s_queue_count = 0;
}

void input_manager_process(void)
{
    app_event_t raw_evt;

    // 1. 保存上一帧状态
    for (int i = 0; i < INPUT_ACTION_MAX; i++)
    {
        s_previous_state[i] = s_current_state[i];
    }

    // 2. 处理底层事件队列
    while (event_queue_pop(&raw_evt))
    {
        if (raw_evt.source_id == ROCKER_SOURCE_ID)
        {
            process_rocker_event(raw_evt.event_type, raw_evt.data);
        }
        else
        {
            process_button_event(raw_evt.source_id, raw_evt.event_type, raw_evt.data);
        }
    }

    // 3. 处理重复触发（方向键长按）
    update_repeat();
}

void input_set_game_mode(bool enable)
{
    s_game_mode = enable;
}

bool input_is_game_mode(void)
{
    return s_game_mode;
}

bool input_is_pressed(input_action_t action)
{
    if (action >= INPUT_ACTION_MAX)
        return false;
    return s_current_state[action];
}

bool input_is_just_pressed(input_action_t action)
{
    if (action >= INPUT_ACTION_MAX)
        return false;
    return s_current_state[action] && !s_previous_state[action];
}

bool input_is_just_released(input_action_t action)
{
    if (action >= INPUT_ACTION_MAX)
        return false;
    return !s_current_state[action] && s_previous_state[action];
}

bool input_poll_event(input_event_t *evt_out)
{
    if (s_queue_count == 0)
        return false;

    *evt_out = s_event_queue[s_queue_head];
    s_queue_head = (s_queue_head + 1) % INPUT_EVENT_QUEUE_SIZE;
    s_queue_count--;

    return true;
}

void input_clear_events(void)
{
    s_queue_head = 0;
    s_queue_tail = 0;
    s_queue_count = 0;
}

const char *input_get_action_name(input_action_t action)
{
    static const char *names[] = {
        "NONE",
        "UP",
        "DOWN",
        "LEFT",
        "RIGHT",
        "CONFIRM",
        "CANCEL",
        "MENU",
        "BACK"};

    if (action < INPUT_ACTION_MAX)
        return names[action];
    return "UNKNOWN";
}

// -----------------------------------------------------------------------------
// 5. 私有函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 推送事件到内部队列
 */
static void push_event(input_action_t action, input_event_type_t type, uint8_t click_count)
{
    if (s_queue_count >= INPUT_EVENT_QUEUE_SIZE)
        return;

    input_event_t evt;
    evt.action = action;
    evt.type = type;
    evt.click_count = click_count;

    s_event_queue[s_queue_tail] = evt;
    s_queue_tail = (s_queue_tail + 1) % INPUT_EVENT_QUEUE_SIZE;
    s_queue_count++;
}

/**
 * @brief 按键ID映射到逻辑动作
 * 映射关系：
 *   SW1 → MENU (菜单)
 *   SW2 → CONFIRM (确定)
 *   SW3 → CANCEL (取消)
 *   SW4 → BACK (返回)
 *   SK  → CONFIRM (仅游戏模式)
 */
static input_action_t map_button_to_action(uint16_t btn_id)
{
    switch (btn_id)
    {
    case BTN_SW1:
        return INPUT_ACTION_MENU;
    case BTN_SW2:
        return INPUT_ACTION_CONFIRM;
    case BTN_SW3:
        return INPUT_ACTION_CANCEL;
    case BTN_SW4:
        return INPUT_ACTION_BACK;
    case BTN_SK:
        // SK只在游戏模式下生效
        return s_game_mode ? INPUT_ACTION_CONFIRM : INPUT_ACTION_NONE;
    default:
        // 组合键暂不处理
        return INPUT_ACTION_NONE;
    }
}

/**
 * @brief 摇杆方向映射到逻辑动作
 * 对角线方向映射为主方向（UP/DOWN优先）
 */
static input_action_t map_rocker_dir_to_action(rocker_direction_t dir)
{
    switch (dir)
    {
    case ROCKER_DIR_UP:
    case ROCKER_DIR_UP_LEFT:
    case ROCKER_DIR_UP_RIGHT:
        return INPUT_ACTION_UP;
    case ROCKER_DIR_DOWN:
    case ROCKER_DIR_DOWN_LEFT:
    case ROCKER_DIR_DOWN_RIGHT:
        return INPUT_ACTION_DOWN;
    case ROCKER_DIR_LEFT:
        return INPUT_ACTION_LEFT;
    case ROCKER_DIR_RIGHT:
        return INPUT_ACTION_RIGHT;
    default:
        return INPUT_ACTION_NONE;
    }
}

/**
 * @brief 处理按键事件
 */
static void process_button_event(uint16_t source_id, uint8_t event_type, uint32_t data)
{
    input_action_t action = map_button_to_action(source_id);
    if (action == INPUT_ACTION_NONE)
        return;

    ebtn_evt_t evt = (ebtn_evt_t)event_type;

    switch (evt)
    {
    case EBTN_EVT_ONPRESS:
        s_current_state[action] = true;
        s_press_start_time[action] = HAL_GetTick();
        s_repeat_started[action] = false;
        push_event(action, INPUT_EVT_PRESS, 0);
        break;

    case EBTN_EVT_ONRELEASE:
        s_current_state[action] = false;
        push_event(action, INPUT_EVT_RELEASE, 0);
        break;

    case EBTN_EVT_ONCLICK:
    {
        // data字段低8位存放连击次数（由ebtn_driver传递）
        uint8_t click_cnt = (uint8_t)(data & 0xFF);
        if (click_cnt == 0)
            click_cnt = 1; // 兼容旧版本
        push_event(action, INPUT_EVT_CLICK, click_cnt);
        break;
    }

    case EBTN_EVT_KEEPALIVE:
        // 长按保持事件，update_repeat会处理重复触发
        break;

    default:
        break;
    }
}

/**
 * @brief 处理摇杆事件
 */
static void process_rocker_event(uint8_t event_type, uint32_t data)
{
    rocker_event_type_t evt = (rocker_event_type_t)event_type;
    rocker_direction_t dir = ROCKER_EVT_UNPACK_DIR(data);

    input_action_t action = map_rocker_dir_to_action(dir);
    if (action == INPUT_ACTION_NONE && evt != ROCKER_EVT_DIR_LEAVE)
        return;

    switch (evt)
    {
    case ROCKER_EVT_DIR_ENTER:
        if (action != INPUT_ACTION_NONE)
        {
            s_current_state[action] = true;
            s_press_start_time[action] = HAL_GetTick();
            s_repeat_started[action] = false;
            push_event(action, INPUT_EVT_PRESS, 0);
        }
        break;

    case ROCKER_EVT_DIR_LEAVE:
        action = map_rocker_dir_to_action(dir);
        if (action != INPUT_ACTION_NONE)
        {
            s_current_state[action] = false;
            push_event(action, INPUT_EVT_RELEASE, 0);
        }
        break;

    case ROCKER_EVT_DIR_HOLD:
        // 摇杆保持，update_repeat会处理
        break;

    default:
        break;
    }
}

/**
 * @brief 更新重复触发逻辑
 * 方向键长按时周期性触发REPEAT事件，用于菜单导航
 */
static void update_repeat(void)
{
    uint32_t now = HAL_GetTick();

    // 只对方向键做重复触发
    for (input_action_t act = INPUT_ACTION_UP; act <= INPUT_ACTION_RIGHT; act++)
    {
        if (!s_current_state[act])
        {
            s_repeat_started[act] = false;
            continue;
        }

        uint32_t held_time = now - s_press_start_time[act];

        if (!s_repeat_started[act])
        {
            if (held_time >= INPUT_REPEAT_DELAY_MS)
            {
                s_repeat_started[act] = true;
                s_last_repeat_time[act] = now;
                push_event(act, INPUT_EVT_REPEAT, 0);
            }
        }
        else
        {
            if ((now - s_last_repeat_time[act]) >= INPUT_REPEAT_INTERVAL_MS)
            {
                s_last_repeat_time[act] = now;
                push_event(act, INPUT_EVT_REPEAT, 0);
            }
        }
    }
}
