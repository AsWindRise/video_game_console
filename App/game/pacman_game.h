/**
 * @file pacman_game.h
 * @brief 吃豆人游戏（Pac-Man）
 * @details 经典街机游戏，迷宫冒险，吃豆躲鬼，紧张刺激
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 网格：16列×8行（128格）
 * - 格子尺寸：8×8像素
 * - 幽灵数量：2个
 * - 初始生命：3条
 *
 * 游戏规则：
 * - 吃掉所有小豆子即胜利
 * - 碰到幽灵失去生命
 * - 吃能量豆后进入无敌模式，可反杀幽灵
 * - 无敌模式持续5秒
 *
 * 操作说明：
 * - 方向键：移动吃豆人
 * - START键：暂停/继续/重新开始
 * - B键：退出游戏
 */

#ifndef PACMAN_GAME_H
#define PACMAN_GAME_H

#include "mydefine.h"

/* ======================== 游戏参数定义 ======================== */

// 屏幕尺寸
#define PACMAN_SCREEN_WIDTH    128
#define PACMAN_SCREEN_HEIGHT   64

// 网格参数
#define PACMAN_GRID_WIDTH      16      // 网格宽度（列数）
#define PACMAN_GRID_HEIGHT     8       // 网格高度（行数）
#define PACMAN_CELL_SIZE       8       // 格子尺寸（正方形）

// 游戏常量
#define PACMAN_MAX_GHOSTS      2       // 幽灵数量
#define PACMAN_MAX_LIVES       3       // 最大生命数
#define PACMAN_POWER_DURATION  5000    // 能量豆效果持续时间（ms）
#define PACMAN_SPEED           150     // 吃豆人移动速度（ms/格）
#define PACMAN_GHOST_SPEED     250     // 幽灵移动速度（ms/格，比吃豆人慢）

// 分数
#define PACMAN_SCORE_DOT       10      // 小豆子分数
#define PACMAN_SCORE_POWER     50      // 能量豆分数
#define PACMAN_SCORE_GHOST     200     // 吃幽灵分数

/* ======================== 枚举定义 ======================== */

/**
 * @brief 游戏状态
 */
typedef enum {
    PACMAN_STATE_READY = 0,    // 准备开始
    PACMAN_STATE_PLAYING,      // 游戏进行中
    PACMAN_STATE_PAUSED,       // 暂停
    PACMAN_STATE_WIN,          // 胜利
    PACMAN_STATE_LOSE          // 失败
} pacman_game_state_t;

/**
 * @brief 格子类型
 */
typedef enum {
    PACMAN_TILE_EMPTY = 0,      // 空地
    PACMAN_TILE_WALL,           // 墙壁
    PACMAN_TILE_DOT,            // 小豆子
    PACMAN_TILE_POWER           // 能量豆
} pacman_tile_type_t;

/**
 * @brief 移动方向
 */
typedef enum {
    PACMAN_DIR_NONE = 0,        // 无方向
    PACMAN_DIR_UP,              // 向上
    PACMAN_DIR_DOWN,            // 向下
    PACMAN_DIR_LEFT,            // 向左
    PACMAN_DIR_RIGHT            // 向右
} pacman_direction_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 位置坐标
 */
typedef struct {
    int8_t x;            // X坐标
    int8_t y;            // Y坐标
} pacman_position_t;

/**
 * @brief 幽灵数据
 */
typedef struct {
    pacman_position_t pos;              // 当前位置
    pacman_direction_t dir;             // 移动方向
    uint8_t is_frightened;              // 是否处于惊吓模式（能量豆效果）
    uint32_t last_move_time;            // 上次移动时间
} pacman_ghost_t;

/**
 * @brief 吃豆人游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                                  // 活跃标志
    void (*exit_callback)(void);                        // 退出回调

    // 游戏状态
    pacman_game_state_t game_state;                     // 游戏状态

    // 地图数据
    pacman_tile_type_t map[PACMAN_GRID_HEIGHT][PACMAN_GRID_WIDTH];  // 地图网格

    // 吃豆人数据
    pacman_position_t pacman_pos;                       // 吃豆人位置
    pacman_direction_t pacman_dir;                      // 移动方向
    pacman_direction_t pacman_next_dir;                 // 缓冲方向（转弯预输入）
    uint8_t pacman_anim_frame;                          // 动画帧（张嘴/闭嘴）
    uint32_t pacman_last_move_time;                     // 上次移动时间

    // 幽灵数据
    pacman_ghost_t ghosts[PACMAN_MAX_GHOSTS];           // 幽灵数组

    // 能量豆效果
    uint8_t power_active;                               // 能量豆是否激活
    uint32_t power_start_time;                          // 能量豆激活时间

    // 游戏数据
    uint8_t lives;                                      // 剩余生命
    uint16_t score;                                     // 分数
    uint8_t dots_remaining;                             // 剩余豆子数量
    uint8_t total_dots;                                 // 总豆子数量

} pacman_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化吃豆人游戏
 */
void pacman_game_init(pacman_game_t *game);

/**
 * @brief 激活游戏
 */
void pacman_game_activate(pacman_game_t *game);

/**
 * @brief 停用游戏
 */
void pacman_game_deactivate(pacman_game_t *game);

/**
 * @brief 设置退出回调
 */
void pacman_game_set_exit_callback(pacman_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入
 */
void pacman_game_update_input(pacman_game_t *game);

/**
 * @brief 更新游戏逻辑
 */
void pacman_game_update_logic(pacman_game_t *game);

/**
 * @brief 渲染游戏画面
 */
void pacman_game_render(pacman_game_t *game);

/**
 * @brief 游戏主任务
 */
void pacman_game_task(pacman_game_t *game);

#endif // PACMAN_GAME_H
