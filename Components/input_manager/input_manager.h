#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include "mydefine.h"

// =============================================================================
// 用户输入抽象层 - 统一按键和摇杆输入
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 逻辑输入动作定义
// -----------------------------------------------------------------------------

/**
 * @brief 逻辑输入动作枚举
 * 游戏/菜单使用这些抽象动作，不关心具体是哪个按键或摇杆
 *
 * 默认按键映射：
 *   SW1 → MENU (菜单)
 *   SW2 → CONFIRM (确定)
 *   SW3 → CANCEL (取消)
 *   SW4 → BACK (返回)
 *   SK  → CONFIRM (仅游戏模式)
 */
typedef enum
{
    INPUT_ACTION_NONE = 0,

    /* 方向动作 (摇杆) */
    INPUT_ACTION_UP,
    INPUT_ACTION_DOWN,
    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,

    /* 功能动作 (按键) */
    INPUT_ACTION_CONFIRM, /*!< 确定 (SW2, 游戏中SK也可) */
    INPUT_ACTION_CANCEL,  /*!< 取消 (SW3) */
    INPUT_ACTION_MENU,    /*!< 菜单 (SW1) */
    INPUT_ACTION_BACK,    /*!< 返回 (SW4) */

    INPUT_ACTION_MAX
} input_action_t;

/**
 * @brief 输入事件类型
 */
typedef enum
{
    INPUT_EVT_NONE = 0,
    INPUT_EVT_PRESS,   /*!< 按下 */
    INPUT_EVT_RELEASE, /*!< 释放 */
    INPUT_EVT_CLICK,   /*!< 单击（支持连击，click_count表示连击次数） */
    INPUT_EVT_REPEAT,  /*!< 重复触发（长按时周期触发，用于菜单导航） */
} input_event_type_t;

/**
 * @brief 输入事件结构
 */
typedef struct
{
    input_action_t action;   /*!< 逻辑动作 */
    input_event_type_t type; /*!< 事件类型 */
    uint8_t click_count;     /*!< 连击次数（1=单击, 2=双击, 仅CLICK事件有效） */
} input_event_t;

// -----------------------------------------------------------------------------
// 2. API 声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化输入管理器
 * 在 ebtn_driver_init() 和 rocker_init() 之后调用
 */
void input_manager_init(void);

/**
 * @brief 输入管理器处理函数
 * 从 event_queue 读取底层事件并转换为逻辑输入
 * 建议 10ms 周期调用
 */
void input_manager_process(void);

// -----------------------------------------------------------------------------
// 2.1 模式控制
// -----------------------------------------------------------------------------

/**
 * @brief 设置游戏模式
 * 游戏模式下SK按键映射为CONFIRM，非游戏模式下SK被忽略
 * @param enable: true=游戏模式, false=菜单模式
 */
void input_set_game_mode(bool enable);

/**
 * @brief 获取当前是否为游戏模式
 */
bool input_is_game_mode(void);

// -----------------------------------------------------------------------------
// 2.2 轮询接口 - 查询当前输入状态
// -----------------------------------------------------------------------------

/**
 * @brief 检查某个动作是否正在被按住
 * @param action: 逻辑动作
 * @return true=按住中, false=未按
 */
bool input_is_pressed(input_action_t action);

/**
 * @brief 检查某个动作是否刚刚按下（本帧按下，上帧未按）
 * @param action: 逻辑动作
 * @return true=刚按下, false=否
 */
bool input_is_just_pressed(input_action_t action);

/**
 * @brief 检查某个动作是否刚刚释放（本帧释放，上帧按住）
 * @param action: 逻辑动作
 * @return true=刚释放, false=否
 */
bool input_is_just_released(input_action_t action);

// -----------------------------------------------------------------------------
// 2.3 事件接口 - 弹出输入事件
// -----------------------------------------------------------------------------

/**
 * @brief 弹出一个输入事件
 * @param evt_out: 输出事件指针
 * @return true=有事件, false=队列空
 */
bool input_poll_event(input_event_t *evt_out);

/**
 * @brief 清空输入事件队列
 */
void input_clear_events(void);

// -----------------------------------------------------------------------------
// 2.4 辅助函数
// -----------------------------------------------------------------------------

/**
 * @brief 获取动作名称字符串（调试用）
 */
const char *input_get_action_name(input_action_t action);

#endif // __INPUT_MANAGER_H__
