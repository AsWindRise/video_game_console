/**
 * @file breakout_game.h
 * @brief 打砖块游戏（Breakout/Arkanoid风格）
 * @details 经典打砖块游戏，挡板控制、物理反弹、砖块消除、关卡系统
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 挡板：24×3像素，摇杆左右控制
 * - 球：半径2像素，物理反弹
 * - 砖块：12×4像素，10列×6行
 * - 关卡：3个关卡，不同布局
 *
 * 游戏规则：
 * - 初始3条命
 * - 球掉落失去1条命
 * - 清除所有砖块进入下一关
 * - 通关全部3关卡或生命为0游戏结束
 *
 * 操作说明：
 * - 方向键左/右：移动挡板
 * - A键：发球（游戏开始或重新发球）
 * - START键：暂停/继续
 * - B键：退出游戏
 */

#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include "mydefine.h"

/* ======================== 游戏参数定义 ======================== */

// 屏幕尺寸
#define BREAKOUT_SCREEN_WIDTH   128
#define BREAKOUT_SCREEN_HEIGHT  64

// 挡板参数
#define BREAKOUT_PADDLE_WIDTH   24
#define BREAKOUT_PADDLE_HEIGHT  3
#define BREAKOUT_PADDLE_Y       60
#define BREAKOUT_PADDLE_SPEED   3      // 每帧移动像素数

// 球参数
#define BREAKOUT_BALL_RADIUS    2
#define BREAKOUT_BALL_SPEED     2.0f   // 初始速度（像素/帧）

// 砖块参数
#define BREAKOUT_BRICK_COLS     10     // 砖块列数
#define BREAKOUT_BRICK_ROWS     6      // 砖块行数
#define BREAKOUT_BRICK_WIDTH    12     // 砖块宽度
#define BREAKOUT_BRICK_HEIGHT   4      // 砖块高度
#define BREAKOUT_BRICK_OFFSET_X 2      // 砖块区域X偏移
#define BREAKOUT_BRICK_OFFSET_Y 4      // 砖块区域Y偏移
#define BREAKOUT_BRICK_GAP_X    1      // 砖块横向间距
#define BREAKOUT_BRICK_GAP_Y    1      // 砖块纵向间距

// 游戏参数
#define BREAKOUT_MAX_LIVES      3      // 最大生命数
#define BREAKOUT_MAX_LEVELS     3      // 最大关卡数

/* ======================== 枚举定义 ======================== */

/**
 * @brief 游戏状态
 */
typedef enum {
    BREAKOUT_STATE_READY = 0,    // 准备开始
    BREAKOUT_STATE_AIMING,       // 瞄准中（球附着在挡板上）
    BREAKOUT_STATE_PLAYING,      // 游戏进行中
    BREAKOUT_STATE_PAUSED,       // 暂停
    BREAKOUT_STATE_LEVEL_CLEAR,  // 关卡通过
    BREAKOUT_STATE_GAME_OVER,    // 游戏结束
    BREAKOUT_STATE_WIN           // 通关全部关卡
} breakout_game_state_t;

/**
 * @brief 砖块类型
 */
typedef enum {
    BRICK_NONE = 0,              // 无砖块
    BRICK_NORMAL,                // 普通砖块（1次击破）
    BRICK_STRONG,                // 坚固砖块（2次击破）
    BRICK_UNBREAKABLE            // 不可破坏砖块
} brick_type_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 砖块数据
 */
typedef struct {
    brick_type_t type;           // 砖块类型
    uint8_t hits_remaining;      // 剩余击打次数
} brick_t;

/**
 * @brief 打砖块游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                              // 活跃标志
    void (*exit_callback)(void);                    // 退出回调

    // 游戏状态
    breakout_game_state_t game_state;               // 游戏状态

    // 挡板
    int16_t paddle_x;                               // 挡板中心X坐标

    // 球
    ball_physics_t ball;                            // 球的物理状态
    uint8_t ball_attached;                          // 球是否附着在挡板上

    // 砖块
    brick_t bricks[BREAKOUT_BRICK_ROWS][BREAKOUT_BRICK_COLS];  // 砖块数组
    uint8_t bricks_remaining;                       // 剩余砖块数量

    // 游戏数据
    uint8_t lives;                                  // 剩余生命数
    uint8_t level;                                  // 当前关卡（1-3）
    uint32_t score;                                 // 当前分数
    uint8_t combo;                                  // 连击数（连续击破砖块）
    uint32_t combo_timer;                           // 连击计时器

    // 动画
    uint32_t level_clear_start_time;                // 关卡通过动画开始时间

} breakout_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化打砖块游戏
 */
void breakout_game_init(breakout_game_t *game);

/**
 * @brief 激活游戏
 */
void breakout_game_activate(breakout_game_t *game);

/**
 * @brief 停用游戏
 */
void breakout_game_deactivate(breakout_game_t *game);

/**
 * @brief 设置退出回调
 */
void breakout_game_set_exit_callback(breakout_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入
 */
void breakout_game_update_input(breakout_game_t *game);

/**
 * @brief 更新游戏逻辑
 */
void breakout_game_update_logic(breakout_game_t *game);

/**
 * @brief 渲染游戏画面
 */
void breakout_game_render(breakout_game_t *game);

/**
 * @brief 游戏主任务
 */
void breakout_game_task(breakout_game_t *game);

#endif // BREAKOUT_GAME_H
