/**
 * @file pong_game.h
 * @brief 乒乓球游戏（Pong）
 * @details 电子游戏鼻祖，双挡板对战，简单但经典
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 挡板尺寸：2×16像素
 * - 球半径：2像素
 * - 胜利分数：11分
 *
 * 游戏规则：
 * - 玩家控制左挡板，AI控制右挡板
 * - 球碰到挡板反弹，碰到上下墙壁反弹
 * - 球飞出左右边界则对手得分
 * - 先到11分获胜
 *
 * 操作说明：
 * - 上/下键：移动挡板
 * - A键：开始/发球
 * - START键：暂停/继续
 * - B键：退出游戏
 */

#ifndef PONG_GAME_H
#define PONG_GAME_H

#include "mydefine.h"

/* ======================== 游戏参数定义 ======================== */

// 屏幕尺寸
#define PONG_SCREEN_WIDTH    128
#define PONG_SCREEN_HEIGHT   64

// 挡板参数
#define PONG_PADDLE_WIDTH    2       // 挡板宽度
#define PONG_PADDLE_HEIGHT   16      // 挡板高度
#define PONG_PADDLE_SPEED    3       // 挡板移动速度（像素/帧）
#define PONG_PADDLE_OFFSET   8       // 挡板距离边界的距离

// 球参数
#define PONG_BALL_RADIUS     2       // 球半径
#define PONG_BALL_SPEED      2.0f    // 球速度（像素/帧）

// 游戏规则
#define PONG_WIN_SCORE       11      // 获胜分数

/* ======================== 枚举定义 ======================== */

/**
 * @brief 游戏状态
 */
typedef enum {
    PONG_STATE_READY = 0,    // 准备开始
    PONG_STATE_SERVE,        // 发球等待
    PONG_STATE_PLAYING,      // 游戏进行中
    PONG_STATE_PAUSED,       // 暂停
    PONG_STATE_WIN,          // 玩家胜利
    PONG_STATE_LOSE          // 玩家失败（AI获胜）
} pong_game_state_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 乒乓球游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                                  // 活跃标志
    void (*exit_callback)(void);                        // 退出回调

    // 游戏状态
    pong_game_state_t game_state;                       // 游戏状态

    // 球数据（复用ball_physics组件）
    ball_physics_t ball;                                // 球物理对象

    // 挡板数据
    int16_t player_y;                                   // 玩家挡板Y坐标（中心点）
    int16_t ai_y;                                       // AI挡板Y坐标（中心点）

    // 分数
    uint8_t player_score;                               // 玩家分数
    uint8_t ai_score;                                   // AI分数

    // 发球权（0=玩家，1=AI）
    uint8_t serve_side;

} pong_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化乒乓球游戏
 */
void pong_game_init(pong_game_t *game);

/**
 * @brief 激活游戏
 */
void pong_game_activate(pong_game_t *game);

/**
 * @brief 停用游戏
 */
void pong_game_deactivate(pong_game_t *game);

/**
 * @brief 设置退出回调
 */
void pong_game_set_exit_callback(pong_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入
 */
void pong_game_update_input(pong_game_t *game);

/**
 * @brief 更新游戏逻辑
 */
void pong_game_update_logic(pong_game_t *game);

/**
 * @brief 渲染游戏画面
 */
void pong_game_render(pong_game_t *game);

/**
 * @brief 游戏主任务
 */
void pong_game_task(pong_game_t *game);

#endif // PONG_GAME_H
