#ifndef __SNAKE_GAME_H__
#define __SNAKE_GAME_H__

#include "mydefine.h"

// =============================================================================
// 贪吃蛇游戏 - 经典网格游戏实现
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 游戏配置宏定义
// -----------------------------------------------------------------------------

#define SNAKE_GRID_WIDTH      16    /*!< 网格宽度（格子数） */
#define SNAKE_GRID_HEIGHT     8     /*!< 网格高度（格子数） */
#define SNAKE_PIXEL_SIZE      8     /*!< 每格像素大小 (128/16=8, 64/8=8) */
#define SNAKE_MAX_LENGTH      128   /*!< 蛇最大长度（满屏） */

// 动态速度系统配置
#define SNAKE_SPEED_INITIAL   250   /*!< 初始速度：250ms/格（4格/秒）*/
#define SNAKE_SPEED_MIN       100   /*!< 最快速度：100ms/格（10格/秒）*/
#define SNAKE_SPEED_DECREASE  10    /*!< 每次加速减少的时间：10ms */
#define SNAKE_SPEED_INTERVAL  30    /*!< 每吃多少分加速一次（30分=3个食物）*/

// -----------------------------------------------------------------------------
// 2. 枚举定义
// -----------------------------------------------------------------------------

/**
 * @brief 移动方向枚举
 */
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} direction_t;

/**
 * @brief 游戏状态枚举
 */
typedef enum {
    GAME_STATE_INIT = 0,      /*!< 初始化状态 */
    GAME_STATE_RUNNING,       /*!< 运行中 */
    GAME_STATE_PAUSED,        /*!< 暂停 */
    GAME_STATE_GAME_OVER      /*!< 游戏结束 */
} game_state_t;

// -----------------------------------------------------------------------------
// 3. 数据结构定义
// -----------------------------------------------------------------------------

/**
 * @brief 退出回调函数类型
 * @note  当游戏请求退出时调用（按SELECT键）
 */
typedef void (*snake_exit_callback_t)(void);

/**
 * @brief 贪吃蛇游戏状态结构体
 */
typedef struct {
    // 蛇身数据
    uint8_t body_x[SNAKE_MAX_LENGTH];   /*!< 蛇身X坐标数组（索引0为头部）*/
    uint8_t body_y[SNAKE_MAX_LENGTH];   /*!< 蛇身Y坐标数组（索引0为头部）*/
    uint8_t length;                     /*!< 当前蛇长度 */

    // 方向控制（方案A：下一方向锁存机制）
    direction_t direction;              /*!< 当前移动方向（本次移动使用）*/
    direction_t next_direction;         /*!< 下一个方向（下次移动使用）*/

    // 食物数据
    uint8_t food_x;                     /*!< 食物X坐标 */
    uint8_t food_y;                     /*!< 食物Y坐标 */

    // 游戏状态
    game_state_t game_state;            /*!< 游戏当前状态 */
    uint16_t score;                     /*!< 当前分数 */
    uint16_t last_speed_up_score;       /*!< 上次加速时的分数（用于判断是否该加速）*/

    // 时间控制（动态速度系统）
    uint32_t last_update_time;          /*!< 上次更新时间戳（毫秒）*/
    uint16_t update_interval;           /*!< 当前更新间隔（毫秒），动态调整 */

    // 活动状态控制（菜单集成所需）
    uint8_t is_active;                  /*!< 游戏是否活跃（1=活跃，0=停止）*/
    snake_exit_callback_t exit_callback; /*!< 退出回调函数（按SELECT键返回菜单）*/
} snake_game_t;

// -----------------------------------------------------------------------------
// 4. API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化贪吃蛇游戏
 * @param game: 游戏状态结构体指针
 * @note  设置蛇初始位置、方向、生成第一个食物
 */
void snake_game_init(snake_game_t *game);

/**
 * @brief 处理玩家输入
 * @param game: 游戏状态结构体指针
 * @note  使用方案A锁存机制，将输入暂存到next_direction
 *        应该被频繁调用（10ms），不会丢失输入
 */
void snake_game_update_input(snake_game_t *game);

/**
 * @brief 更新游戏逻辑
 * @param game: 游戏状态结构体指针
 * @note  100ms周期调用，应用next_direction，移动蛇，碰撞检测
 */
void snake_game_update_logic(snake_game_t *game);

/**
 * @brief 渲染游戏画面
 * @param game: 游戏状态结构体指针
 * @note  使用u8g2绘制蛇、食物、分数
 */
void snake_game_render(snake_game_t *game);

/**
 * @brief 游戏主任务（周期调用）
 * @param game: 游戏状态结构体指针
 * @note  集成输入、逻辑、渲染的完整游戏循环
 */
void snake_game_task(snake_game_t *game);

/**
 * @brief 激活游戏
 * @param game: 游戏状态结构体指针
 * @note  从菜单进入游戏时调用，使游戏开始运行
 */
void snake_game_activate(snake_game_t *game);

/**
 * @brief 停用游戏
 * @param game: 游戏状态结构体指针
 * @note  返回菜单时调用，使游戏停止运行
 */
void snake_game_deactivate(snake_game_t *game);

/**
 * @brief 设置游戏退出回调
 * @param game: 游戏状态结构体指针
 * @param callback: 退出回调函数（按SELECT键时调用）
 * @note  用于从游戏返回菜单
 */
void snake_game_set_exit_callback(snake_game_t *game, snake_exit_callback_t callback);

#endif // __SNAKE_GAME_H__
