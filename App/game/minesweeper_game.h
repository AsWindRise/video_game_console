/**
 * @file minesweeper_game.h
 * @brief 扫雷游戏（Minesweeper）
 * @details 经典扫雷游戏，智力挑战，递归展开，旗帜标记
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 网格：10列×8行（80格）
 * - 格子尺寸：12×7像素
 * - 难度：3档（10/16/20个地雷）
 *
 * 游戏规则：
 * - 翻开所有非地雷格子即胜利
 * - 翻开地雷格子即失败
 * - 数字表示周围8个格子的地雷数量
 * - 空白格子会自动递归展开
 * - 可以用旗帜标记疑似地雷
 *
 * 操作说明：
 * - 方向键：移动光标
 * - A键：翻开格子
 * - Y键：标记/取消标记旗帜
 * - START键：暂停/继续/重新开始
 * - B键：退出游戏
 */

#ifndef MINESWEEPER_GAME_H
#define MINESWEEPER_GAME_H

#include "mydefine.h"

/* ======================== 游戏参数定义 ======================== */

// 屏幕尺寸
#define MINE_SCREEN_WIDTH    128
#define MINE_SCREEN_HEIGHT   64

// UI区域
#define MINE_UI_HEIGHT       8      // 顶部UI高度（显示地雷数、时间）

// 网格参数
#define MINE_GRID_WIDTH      10     // 网格宽度（列数）
#define MINE_GRID_HEIGHT     8      // 网格高度（行数）
#define MINE_CELL_WIDTH      12     // 格子宽度（像素）
#define MINE_CELL_HEIGHT     7      // 格子高度（像素）
#define MINE_OFFSET_X        4      // 游戏区域X偏移（左边距）
#define MINE_OFFSET_Y        8      // 游戏区域Y偏移（顶部UI下方）

// 难度等级参数
#define MINE_EASY_MINES      10     // 简单：10个地雷（12.5%）
#define MINE_MEDIUM_MINES    16     // 中等：16个地雷（20%）
#define MINE_HARD_MINES      20     // 困难：20个地雷（25%）

// 游戏常量
#define MINE_MAX_CELLS       (MINE_GRID_WIDTH * MINE_GRID_HEIGHT)  // 总格子数：80

/* ======================== 枚举定义 ======================== */

/**
 * @brief 游戏状态
 */
typedef enum {
    MINE_STATE_READY = 0,    // 准备开始（选择难度）
    MINE_STATE_PLAYING,      // 游戏进行中
    MINE_STATE_PAUSED,       // 暂停
    MINE_STATE_WIN,          // 胜利
    MINE_STATE_LOSE          // 失败（踩雷）
} mine_game_state_t;

/**
 * @brief 难度等级
 */
typedef enum {
    MINE_DIFFICULTY_EASY = 0,    // 简单（10个地雷）
    MINE_DIFFICULTY_MEDIUM,      // 中等（16个地雷）
    MINE_DIFFICULTY_HARD         // 困难（20个地雷）
} mine_difficulty_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 格子数据
 */
typedef struct {
    uint8_t has_mine;       // 是否有地雷（1=有，0=无）
    uint8_t is_revealed;    // 是否已翻开（1=已翻开，0=未翻开）
    uint8_t is_flagged;     // 是否已标记旗帜（1=已标记，0=未标记）
    uint8_t neighbor_mines; // 周围地雷数量（0-8）
} mine_cell_t;

/**
 * @brief 扫雷游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                                  // 活跃标志
    void (*exit_callback)(void);                        // 退出回调

    // 游戏状态
    mine_game_state_t game_state;                       // 游戏状态
    mine_difficulty_t difficulty;                       // 难度等级

    // 地图数据
    mine_cell_t cells[MINE_GRID_HEIGHT][MINE_GRID_WIDTH]; // 格子数组
    int8_t cursor_x;                                    // 光标X坐标（0-9）
    int8_t cursor_y;                                    // 光标Y坐标（0-7）

    // 游戏数据
    uint8_t mines_total;                                // 总地雷数
    uint8_t flags_placed;                               // 已放置旗帜数
    uint8_t cells_revealed;                             // 已翻开格子数
    uint8_t first_click;                                // 是否是第一次点击（1=是，0=否）

    // 时间统计
    uint32_t game_start_time;                           // 游戏开始时间（ms）
    uint32_t game_time;                                 // 游戏时间（秒）

} minesweeper_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化扫雷游戏
 */
void minesweeper_game_init(minesweeper_game_t *game);

/**
 * @brief 激活游戏
 */
void minesweeper_game_activate(minesweeper_game_t *game);

/**
 * @brief 停用游戏
 */
void minesweeper_game_deactivate(minesweeper_game_t *game);

/**
 * @brief 设置退出回调
 */
void minesweeper_game_set_exit_callback(minesweeper_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入
 */
void minesweeper_game_update_input(minesweeper_game_t *game);

/**
 * @brief 更新游戏逻辑
 */
void minesweeper_game_update_logic(minesweeper_game_t *game);

/**
 * @brief 渲染游戏画面
 */
void minesweeper_game_render(minesweeper_game_t *game);

/**
 * @brief 游戏主任务
 */
void minesweeper_game_task(minesweeper_game_t *game);

#endif // MINESWEEPER_GAME_H
