/**
 * @file tetris_game.h
 * @brief 俄罗斯方块游戏实现
 * @details 经典俄罗斯方块游戏，10×10网格，7种方块，支持旋转、消行、得分系统
 *
 * 游戏参数：
 * - 屏幕分辨率：128×64像素
 * - 游戏网格：10列×10行（60×60像素游戏区域）
 * - 方块大小：6×6像素/格
 * - 目标帧率：10ms输入采样，动态下落间隔
 * - 初始速度：500ms/格，最快100ms/格
 *
 * 方块类型（Tetromino）：
 * - I型：一字长条（青色）
 * - O型：正方形（黄色）
 * - T型：T字形（紫色）
 * - S型：S字形（绿色）
 * - Z型：Z字形（红色）
 * - J型：J字形（蓝色）
 * - L型：L字形（橙色）
 *
 * 得分系统：
 * - 单行消除：100分
 * - 双行消除：300分
 * - 三行消除：500分
 * - 四行消除（Tetris）：800分
 * - 软降（向下键）：每格1分
 * - 硬降（未实现）：每格2分
 *
 * 等级系统：
 * - 每消除10行升1级
 * - 初始速度：500ms/格（Lv1）
 * - 每升1级加速50ms，最快100ms/格（Lv9+）
 *
 * 操作说明：
 * - 方向键左/右：移动方块
 * - 方向键下：软降（加速下落）
 * - A键：顺时针旋转
 * - START键：暂停/继续
 * - B键：退出游戏
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <mydefine.h>

/* ======================== 游戏参数定义 ======================== */

// 游戏网格尺寸
#define TETRIS_GRID_WIDTH       10      // 游戏区域宽度（格数）
#define TETRIS_GRID_HEIGHT      18      // 游戏区域高度（格数）- 标准俄罗斯方块高度
#define TETRIS_CELL_SIZE        3       // 每格像素大小（3×3，18行=54像素，10列=30像素）

// 游戏区域偏移（屏幕左上角为原点）
#define TETRIS_GRID_OFFSET_X    2       // 游戏区域X偏移（左侧留2像素边距）
#define TETRIS_GRID_OFFSET_Y    5       // 游戏区域Y偏移（顶部留5像素边距）

// 右侧信息区域
#define TETRIS_INFO_OFFSET_X    36      // 信息区域X偏移（游戏区域右侧：2+30+4=36）

// 方块矩阵大小
#define TETROMINO_SIZE          4       // 方块矩阵尺寸（4×4）

// 游戏速度参数
#define TETRIS_INITIAL_SPEED    500     // 初始下落速度（ms/格）
#define TETRIS_SPEED_DECREMENT  50      // 每升1级加速（ms）
#define TETRIS_MIN_SPEED        100     // 最快速度（ms/格）
#define TETRIS_SOFT_DROP_SPEED  50      // 软降速度（ms/格）

// 等级系统
#define TETRIS_LINES_PER_LEVEL  10      // 每消除N行升1级

// DAS (Delayed Auto Shift) 系统参数
#define TETRIS_DAS_DELAY        150     // DAS延迟（ms）- 按住后多久开始重复
#define TETRIS_DAS_REPEAT       50      // DAS重复间隔（ms）- 重复移动的间隔

/* ======================== 枚举定义 ======================== */

/**
 * @brief 方块类型（7种标准Tetromino）
 */
typedef enum {
    TETROMINO_I = 0,    // 一字长条（青色）
    TETROMINO_O,        // 正方形（黄色）
    TETROMINO_T,        // T字形（紫色）
    TETROMINO_S,        // S字形（绿色）
    TETROMINO_Z,        // Z字形（红色）
    TETROMINO_J,        // J字形（蓝色）
    TETROMINO_L,        // L字形（橙色）
    TETROMINO_COUNT     // 方块类型数量
} tetromino_type_t;

/**
 * @brief 游戏状态
 */
typedef enum {
    TETRIS_STATE_READY = 0,     // 准备开始（显示提示信息）
    TETRIS_STATE_RUNNING,       // 游戏运行中
    TETRIS_STATE_PAUSED,        // 游戏暂停
    TETRIS_STATE_GAME_OVER      // 游戏结束
} tetris_game_state_t;

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 方块定义（4×4矩阵）
 */
typedef struct {
    uint8_t shape[TETROMINO_SIZE][TETROMINO_SIZE];  // 方块形状（1=有方块，0=空）
    tetromino_type_t type;                          // 方块类型
    uint8_t rotation;                               // 当前旋转状态（0-3）
} tetromino_t;

/**
 * @brief 当前下落方块
 */
typedef struct {
    tetromino_t tetromino;  // 方块数据
    int16_t x;              // 网格X坐标（左上角）
    int16_t y;              // 网格Y坐标（左上角）
} falling_piece_t;

/**
 * @brief 俄罗斯方块游戏状态
 */
typedef struct {
    // 活跃状态控制（场景切换）
    uint8_t is_active;                              // 活跃标志（1=前台运行，0=后台停止）
    void (*exit_callback)(void);                    // 退出回调（返回菜单）

    // 游戏状态
    tetris_game_state_t game_state;                 // 游戏状态（准备/运行/暂停/结束）

    // 游戏区域（已固定的方块）
    uint8_t grid[TETRIS_GRID_HEIGHT][TETRIS_GRID_WIDTH];  // 网格数据（0=空，1-7=方块类型）

    // 当前方块
    falling_piece_t current_piece;                  // 当前下落方块
    tetromino_type_t next_piece_type;               // 下一个方块类型

    // 游戏逻辑
    uint32_t last_drop_time;                        // 上次下落时间（ms）
    uint32_t drop_interval;                         // 当前下落间隔（ms）
    uint8_t soft_drop_active;                       // 软降激活标志（按下向下键）

    // DAS (Delayed Auto Shift) 系统 - 连续移动机制
    uint8_t das_left_active;                        // 左键DAS激活标志
    uint8_t das_right_active;                       // 右键DAS激活标志
    uint32_t das_start_time;                        // DAS开始时间（ms）
    uint32_t das_last_move_time;                    // DAS上次移动时间（ms）

    // 游戏数据
    uint32_t score;                                 // 当前分数
    uint8_t level;                                  // 当前等级（1-9+）
    uint16_t lines_cleared;                         // 已消除行数

    // 动画控制
    uint8_t clearing_lines[TETRIS_GRID_HEIGHT];    // 待清除行标记（1=待清除，0=正常）
    uint8_t clearing_animation;                     // 消行动画标志
    uint32_t clearing_start_time;                   // 消行动画开始时间

} tetris_game_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化俄罗斯方块游戏
 * @param game 游戏实例指针
 */
void tetris_game_init(tetris_game_t *game);

/**
 * @brief 激活游戏（场景切换）
 * @param game 游戏实例指针
 */
void tetris_game_activate(tetris_game_t *game);

/**
 * @brief 停用游戏（场景切换）
 * @param game 游戏实例指针
 */
void tetris_game_deactivate(tetris_game_t *game);

/**
 * @brief 设置退出回调
 * @param game 游戏实例指针
 * @param callback 退出回调函数
 */
void tetris_game_set_exit_callback(tetris_game_t *game, void (*callback)(void));

/**
 * @brief 处理用户输入（10ms周期调用）
 * @param game 游戏实例指针
 */
void tetris_game_update_input(tetris_game_t *game);

/**
 * @brief 更新游戏逻辑（10ms周期调用）
 * @param game 游戏实例指针
 */
void tetris_game_update_logic(tetris_game_t *game);

/**
 * @brief 渲染游戏画面
 * @param game 游戏实例指针
 */
void tetris_game_render(tetris_game_t *game);

/**
 * @brief 游戏主任务（10ms周期调用，集成输入+逻辑+渲染）
 * @param game 游戏实例指针
 */
void tetris_game_task(tetris_game_t *game);

#endif // TETRIS_GAME_H
