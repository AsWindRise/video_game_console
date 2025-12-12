/**
 * @file sokoban_game.h
 * @brief 推箱子游戏（Sokoban/倉庫番）
 * @details 经典益智游戏，网格系统核心，推箱子到目标点
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 网格：10列×8行
 * - 格子尺寸：12×7像素
 * - 关卡数：3个（简单→中等→困难）
 *
 * 游戏规则：
 * - 玩家可以推箱子（不能拉）
 * - 一次只能推一个箱子
 * - 箱子不能推到墙壁或其他箱子上
 * - 所有箱子到达目标点即过关
 *
 * 操作说明：
 * - 方向键：移动玩家
 * - A键：重新开始当前关卡
 * - START键：暂停/继续
 * - B键：退出游戏
 */

#ifndef SOKOBAN_GAME_H
#define SOKOBAN_GAME_H

#include "mydefine.h"

/* ======================== 游戏参数定义 ======================== */

// 屏幕尺寸
#define SOKOBAN_SCREEN_WIDTH    128
#define SOKOBAN_SCREEN_HEIGHT   64

// UI区域
#define SOKOBAN_UI_HEIGHT       8      // 顶部UI高度（显示关卡号、步数）

// 网格参数
#define SOKOBAN_WIDTH           10     // 网格宽度（列数）
#define SOKOBAN_HEIGHT          8      // 网格高度（行数）
#define SOKOBAN_CELL_WIDTH      12     // 格子宽度（像素）
#define SOKOBAN_CELL_HEIGHT     7      // 格子高度（像素）
#define SOKOBAN_OFFSET_X        4      // 游戏区域X偏移（左边距）
#define SOKOBAN_OFFSET_Y        8      // 游戏区域Y偏移（顶部UI下方）

// 游戏参数
#define SOKOBAN_MAX_LEVELS      3      // 最大关卡数
#define SOKOBAN_MAX_STEPS       999    // 最大步数（显示用）

/* ======================== 枚举定义 ======================== */

/**
 * @brief 地图元素类型
 */
typedef enum {
    TILE_EMPTY = 0,         // 空地（墙外区域，不可见）
    TILE_FLOOR,             // 地板（可以走）
    TILE_WALL,              // 墙壁（不可通过）
    TILE_TARGET,            // 目标点（箱子的目标位置）
    TILE_BOX,               // 箱子（在地板上）
    TILE_BOX_ON_TARGET      // 箱子在目标点上（已到位）
} tile_type_t;

/**
 * @brief 游戏状态
 */
typedef enum {
    SOKOBAN_STATE_READY = 0,    // 准备开始
    SOKOBAN_STATE_PLAYING,      // 游戏进行中
    SOKOBAN_STATE_PAUSED,       // 暂停
    SOKOBAN_STATE_LEVEL_CLEAR,  // 关卡通过
    SOKOBAN_STATE_WIN           // 通关全部关卡
} sokoban_game_state_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 位置坐标
 */
typedef struct {
    int8_t x;   // 列（0-9）
    int8_t y;   // 行（0-7）
} position_t;

/**
 * @brief 推箱子游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                              // 活跃标志
    void (*exit_callback)(void);                    // 退出回调

    // 游戏状态
    sokoban_game_state_t game_state;                // 游戏状态

    // 关卡数据
    tile_type_t map[SOKOBAN_HEIGHT][SOKOBAN_WIDTH]; // 地图数据
    position_t player;                              // 玩家位置
    uint8_t boxes_on_target;                        // 已到位的箱子数
    uint8_t total_boxes;                            // 总箱子数

    // 游戏数据
    uint8_t current_level;                          // 当前关卡（1-3）
    uint16_t steps;                                 // 当前步数

    // 动画
    uint32_t level_clear_start_time;                // 关卡通过动画开始时间

} sokoban_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化推箱子游戏
 */
void sokoban_game_init(sokoban_game_t *game);

/**
 * @brief 激活游戏
 */
void sokoban_game_activate(sokoban_game_t *game);

/**
 * @brief 停用游戏
 */
void sokoban_game_deactivate(sokoban_game_t *game);

/**
 * @brief 设置退出回调
 */
void sokoban_game_set_exit_callback(sokoban_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入
 */
void sokoban_game_update_input(sokoban_game_t *game);

/**
 * @brief 更新游戏逻辑
 */
void sokoban_game_update_logic(sokoban_game_t *game);

/**
 * @brief 渲染游戏画面
 */
void sokoban_game_render(sokoban_game_t *game);

/**
 * @brief 游戏主任务
 */
void sokoban_game_task(sokoban_game_t *game);

#endif // SOKOBAN_GAME_H
