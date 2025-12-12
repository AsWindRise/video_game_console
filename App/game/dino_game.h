#ifndef __DINO_GAME_H__
#define __DINO_GAME_H__

#include "mydefine.h"

// =============================================================================
// 恐龙跑酷游戏 - Chrome断网小游戏复刻版（精致优化版）
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 游戏配置宏定义
// -----------------------------------------------------------------------------

// 帧率控制
#define DINO_TARGET_FPS         30      /*!< 目标帧率（30fps，流畅且不卡顿）*/
#define DINO_FRAME_TIME_MS      (1000 / DINO_TARGET_FPS)  /*!< 每帧时间（33ms）*/

// 屏幕和布局参数
#define DINO_SCREEN_WIDTH       128     /*!< 屏幕宽度 */
#define DINO_SCREEN_HEIGHT      64      /*!< 屏幕高度 */
#define DINO_UI_HEIGHT          10      /*!< UI区域高度（顶部分数显示）*/
#define DINO_GROUND_Y           50      /*!< 地面Y坐标 */

// 恐龙参数
#define DINO_X                  16      /*!< 恐龙X坐标（固定位置）*/
#define DINO_WIDTH              12      /*!< 恐龙宽度（像素画，Chrome原版）*/
#define DINO_HEIGHT             11      /*!< 恐龙高度（像素画，Chrome原版）*/
#define DINO_JUMP_HEIGHT        20      /*!< 普通跳跃高度（像素）- 平衡性优化 */
#define DINO_JUMP_HEIGHT_HIGH   28      /*!< 长按跳跃高度（像素）- 平衡性优化 */
#define DINO_JUMP_DURATION      450     /*!< 跳跃总时长（毫秒）*/
#define DINO_LONG_PRESS_TIME    80      /*!< 长按判定时间（ms）*/

// 障碍物参数
#define DINO_OBSTACLE_WIDTH     6       /*!< 障碍物宽度（像素画仙人掌）*/
#define DINO_OBSTACLE_HEIGHT    10      /*!< 障碍物高度（降低高度，更容易跳过）*/
#define DINO_MAX_OBSTACLES      3       /*!< 最多同时存在的障碍物数量 */
#define DINO_OBSTACLE_MIN_DELAY 1000    /*!< 障碍物生成最小间隔（ms）- 降低难度 */
#define DINO_OBSTACLE_MAX_DELAY 2000    /*!< 障碍物生成最大间隔（ms）- 降低难度 */

// 速度参数
#define DINO_INITIAL_SPEED      3.5f    /*!< 初始速度（像素/帧）- 再次提速 */
#define DINO_MAX_SPEED          6.5f    /*!< 最大速度（像素/帧）- 再次提速 */
#define DINO_SPEED_INCREMENT    0.2f    /*!< 每次加速的增量 */
#define DINO_SPEED_UP_INTERVAL  150     /*!< 每多少分加速一次 */

// 分数参数
#define DINO_SCORE_INTERVAL     100     /*!< 分数增加间隔（ms）*/

// 动画参数
#define DINO_RUN_ANIM_INTERVAL  150     /*!< 跑步动画切换间隔（ms）*/

// 云朵参数
#define DINO_MAX_CLOUDS         3       /*!< 最多云朵数量 */
#define DINO_CLOUD_SPEED        0.3f    /*!< 云朵移动速度（像素/帧）*/

// -----------------------------------------------------------------------------
// 2. 枚举定义
// -----------------------------------------------------------------------------

/**
 * @brief 游戏状态枚举
 */
typedef enum {
    DINO_STATE_READY = 0,       /*!< 准备开始（显示提示信息）*/
    DINO_STATE_RUNNING,         /*!< 运行中 */
    DINO_STATE_GAME_OVER        /*!< 游戏结束 */
} dino_game_state_t;

/**
 * @brief 恐龙跳跃状态枚举
 */
typedef enum {
    DINO_JUMP_IDLE = 0,         /*!< 站立（在地面上）*/
    DINO_JUMP_RISING,           /*!< 上升中 */
    DINO_JUMP_FALLING           /*!< 下降中 */
} dino_jump_state_t;

/**
 * @brief 障碍物类型
 */
typedef enum {
    OBSTACLE_TYPE_CACTUS_SMALL = 0,  /*!< 小型仙人掌 */
    OBSTACLE_TYPE_CACTUS_LARGE,      /*!< 大型仙人掌 */
} obstacle_type_t;

// -----------------------------------------------------------------------------
// 3. 数据结构定义
// -----------------------------------------------------------------------------

/**
 * @brief 障碍物结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活（1=激活，0=未使用）*/
    int16_t x;                  /*!< X坐标（屏幕坐标）*/
    uint8_t width;              /*!< 宽度 */
    uint8_t height;             /*!< 高度 */
    obstacle_type_t type;       /*!< 障碍物类型 */
} dino_obstacle_t;

/**
 * @brief 云朵结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活 */
    int16_t x;                  /*!< X坐标 */
    uint8_t y;                  /*!< Y坐标 */
} dino_cloud_t;

/**
 * @brief 恐龙跑酷游戏状态结构体
 */
typedef struct {
    // 游戏状态
    dino_game_state_t game_state;       /*!< 游戏当前状态 */
    uint8_t is_active;                  /*!< 游戏是否活跃（1=活跃，0=停止）*/
    void (*exit_callback)(void);        /*!< 退出回调函数（按B键返回菜单）*/

    // 恐龙状态
    dino_jump_state_t jump_state;       /*!< 跳跃状态 */
    int16_t dino_y;                     /*!< 恐龙Y坐标（脚底坐标）*/
    uint32_t jump_start_time;           /*!< 跳跃开始时间戳（毫秒）*/
    uint32_t jump_button_press_time;    /*!< A键按下时间（用于判定长按）*/
    uint16_t current_jump_height;       /*!< 当前跳跃的高度（普通/高跳）*/
    uint8_t run_anim_frame;             /*!< 跑步动画帧（0或1）*/
    uint32_t last_anim_time;            /*!< 上次动画切换时间 */

    // 障碍物系统
    dino_obstacle_t obstacles[DINO_MAX_OBSTACLES];  /*!< 障碍物数组 */
    uint32_t last_obstacle_time;        /*!< 上次生成障碍物时间 */
    uint16_t next_obstacle_delay;       /*!< 下次生成延迟（ms）*/

    // 云朵系统
    dino_cloud_t clouds[DINO_MAX_CLOUDS];  /*!< 云朵数组 */

    // 分数和速度
    uint32_t score;                     /*!< 当前分数 */
    uint32_t high_score;                /*!< 历史最高分 */
    float speed;                        /*!< 当前速度（像素/帧）*/
    uint32_t last_speed_up_score;       /*!< 上次加速时的分数 */
    uint32_t last_score_time;           /*!< 上次增加分数的时间 */

    // 帧时间控制
    uint32_t last_frame_time;           /*!< 上一帧时间戳 */
    uint32_t last_logic_update_time;    /*!< 上次逻辑更新时间 */
} dino_game_t;

// -----------------------------------------------------------------------------
// 4. API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化恐龙跑酷游戏
 * @param game: 游戏状态结构体指针
 * @note  设置游戏初始状态，清空障碍物，重置分数
 */
void dino_game_init(dino_game_t *game);

/**
 * @brief 处理玩家输入
 * @param game: 游戏状态结构体指针
 * @note  10ms周期调用，处理跳跃和退出输入
 */
void dino_game_update_input(dino_game_t *game);

/**
 * @brief 更新游戏逻辑
 * @param game: 游戏状态结构体指针
 * @note  处理跳跃物理、障碍物移动、碰撞检测、分数更新
 */
void dino_game_update_logic(dino_game_t *game);

/**
 * @brief 渲染游戏画面
 * @param game: 游戏状态结构体指针
 * @note  使用u8g2绘制恐龙、障碍物、地面、UI
 */
void dino_game_render(dino_game_t *game);

/**
 * @brief 游戏主任务（周期调用）
 * @param game: 游戏状态结构体指针
 * @note  集成输入、逻辑、渲染的完整游戏循环，带帧率控制
 */
void dino_game_task(dino_game_t *game);

/**
 * @brief 激活游戏
 * @param game: 游戏状态结构体指针
 * @note  从菜单进入游戏时调用，使游戏开始运行
 */
void dino_game_activate(dino_game_t *game);

/**
 * @brief 停用游戏
 * @param game: 游戏状态结构体指针
 * @note  返回菜单时调用，使游戏停止运行
 */
void dino_game_deactivate(dino_game_t *game);

/**
 * @brief 设置游戏退出回调
 * @param game: 游戏状态结构体指针
 * @param callback: 退出回调函数（按B键时调用）
 * @note  用于从游戏返回菜单
 */
void dino_game_set_exit_callback(dino_game_t *game, void (*callback)(void));

#endif // __DINO_GAME_H__
