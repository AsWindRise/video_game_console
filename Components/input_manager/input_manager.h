#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include "mydefine.h"

// =============================================================================
// 输入管理器 - 统一输入抽象层（薄抽象，只做硬件统一，不做逻辑映射）
// 职责：
// 1. 统一按键（ebtn）和摇杆（rocker）的输入源
// 2. 提供物理按键查询接口（告诉应用层"哪个物理按键被按下"）
// 3. 不做逻辑映射（不定义"确认""取消"，让游戏自己决定按键含义）
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 物理按键枚举
// -----------------------------------------------------------------------------

/**
 * @brief 统一的物理按键枚举
 * @note  这些是物理按键，不是逻辑功能！
 *        游戏/菜单自己决定每个按键的含义
 */
typedef enum
{
    // 方向键（来自摇杆）
    INPUT_BTN_UP = 0, /*!< 摇杆向上 */
    INPUT_BTN_DOWN,   /*!< 摇杆向下 */
    INPUT_BTN_LEFT,   /*!< 摇杆向左 */
    INPUT_BTN_RIGHT,  /*!< 摇杆向右 */

    // 功能键（来自物理按键）
    INPUT_BTN_A, /*!< 对应SW3 - 通常用作主要确认键 */
    INPUT_BTN_B, /*!< 对应SW4 - 通常用作取消/返回键 */
    INPUT_BTN_X, /*!< 对应SW2 - 扩展功能键 */
    INPUT_BTN_Y, /*!< 对应SW1 - 扩展功能键 */

    // 特殊键
    INPUT_BTN_START, /*!< 对应SK（摇杆中心按键）- 通常用作开始/暂停 */

    INPUT_BTN_MAX /*!< 按键总数 */
} input_button_t;

// -----------------------------------------------------------------------------
// 2. 按键状态枚举
// -----------------------------------------------------------------------------

/**
 * @brief 按键状态类型
 */
typedef enum
{
    INPUT_STATE_IDLE = 0,      /*!< 空闲（未按下） */
    INPUT_STATE_PRESSED,       /*!< 按下中 */
    INPUT_STATE_JUST_PRESSED,  /*!< 刚刚按下（边缘触发，只在按下瞬间为真） */
    INPUT_STATE_JUST_RELEASED, /*!< 刚刚释放（边缘触发，只在释放瞬间为真） */
} input_state_t;

// -----------------------------------------------------------------------------
// 3. API声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化输入管理器
 * @note  在系统初始化时调用，必须在ebtn_driver和rocker初始化之后调用
 */
void input_manager_init(void);

/**
 * @brief 输入管理器任务处理函数
 * @note  定期调用（建议10ms），用于更新按键状态
 *        从event_queue获取事件并更新内部状态
 */
void input_manager_task(void);

/**
 * @brief 清空输入管理器的所有状态
 * @note  用于场景切换时清除残留的按键状态，避免新场景收到旧按键事件
 */
void input_manager_clear(void);

/**
 * @brief 查询按键是否正在被按下
 * @param btn: 要查询的按键
 * @retval 1: 按键按下中, 0: 按键未按下
 * @note   用于持续检测（如：长按移动）
 * @example if(input_is_pressed(INPUT_BTN_LEFT)) { player_move_left(); }
 */
uint8_t input_is_pressed(input_button_t btn);

/**
 * @brief 查询按键是否刚刚按下（边缘触发）
 * @param btn: 要查询的按键
 * @retval 1: 刚按下, 0: 否
 * @note   只在按下瞬间返回1，下一帧自动清除
 *         用于单次触发事件（如：菜单选择、射击）
 * @example if(input_is_just_pressed(INPUT_BTN_A)) { menu_confirm(); }
 */
uint8_t input_is_just_pressed(input_button_t btn);

/**
 * @brief 查询按键是否刚刚释放（边缘触发）
 * @param btn: 要查询的按键
 * @retval 1: 刚释放, 0: 否
 * @note   只在释放瞬间返回1，下一帧自动清除
 * @example if(input_is_just_released(INPUT_BTN_A)) { jump_end(); }
 */
uint8_t input_is_just_released(input_button_t btn);

/**
 * @brief 获取按键的完整状态
 * @param btn: 要查询的按键
 * @retval input_state_t: 按键状态
 */
input_state_t input_get_state(input_button_t btn);

/**
 * @brief 获取任意方向键是否按下
 * @retval 1: 任一方向键按下, 0: 无方向键按下
 * @note   用于快速判断是否有方向输入
 */
uint8_t input_any_direction_pressed(void);

/**
 * @brief 获取任意功能键是否按下
 * @retval 1: 任一功能键按下, 0: 无功能键按下
 */
uint8_t input_any_button_pressed(void);

/**
 * @brief 查询按键是否双击（边缘触发）
 * @param btn: 要查询的按键
 * @retval 1: 检测到双击, 0: 否
 * @note   只在双击瞬间返回1，下一帧自动清除
 *         用于双击触发事件（如：二段跳、快速冲刺）
 * @example if(input_is_double_click(INPUT_BTN_A)) { player_double_jump(); }
 */
uint8_t input_is_double_click(input_button_t btn);

#endif // __INPUT_MANAGER_H__
