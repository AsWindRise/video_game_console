#ifndef __PLANE_GAME_H__
#define __PLANE_GAME_H__

#include "mydefine.h"

// =============================================================================
// 打飞机游戏 - 横向卷轴弹幕射击游戏（类似雷电、沙罗曼蛇）
// 特色：5种敌机、3种道具、武器升级、Boss战、爆炸动画
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 游戏配置宏定义
// -----------------------------------------------------------------------------

// 帧率控制
#define PLANE_TARGET_FPS            30      /*!< 目标帧率（30fps）*/
#define PLANE_FRAME_TIME_MS         (1000 / PLANE_TARGET_FPS)  /*!< 每帧时间（33ms）*/

// 屏幕和布局参数
#define PLANE_SCREEN_WIDTH          128     /*!< 屏幕宽度 */
#define PLANE_SCREEN_HEIGHT         64      /*!< 屏幕高度 */
#define PLANE_UI_HEIGHT             8       /*!< UI区域高度（底部状态栏）*/
#define PLANE_PLAYER_X              10      /*!< 玩家X坐标（固定在左侧）*/
#define PLANE_PLAYER_Y_MIN          8       /*!< 玩家Y活动范围最小值 */
#define PLANE_PLAYER_Y_MAX          48      /*!< 玩家Y活动范围最大值 */

// 玩家参数
#define PLANE_PLAYER_WIDTH          8       /*!< 玩家飞机宽度 */
#define PLANE_PLAYER_HEIGHT         8       /*!< 玩家飞机高度 */
#define PLANE_PLAYER_SPEED          2.5f    /*!< 玩家移动速度（像素/帧，上下移动）*/
#define PLANE_PLAYER_SHOOT_INTERVAL 120     /*!< 射击间隔（ms）- 优化后更爽快 */
#define PLANE_PLAYER_INITIAL_HP     3       /*!< 初始生命值 */

// 子弹参数
#define PLANE_BULLET_WIDTH          4       /*!< 子弹宽度（横向）*/
#define PLANE_BULLET_HEIGHT         2       /*!< 子弹高度 */
#define PLANE_PLAYER_BULLET_SPEED   5.0f    /*!< 玩家子弹速度（向右）*/
#define PLANE_ENEMY_BULLET_SPEED    -3.5f   /*!< 敌机子弹速度（向左）*/
#define MAX_PLAYER_BULLETS          12      /*!< 最大玩家子弹数量 */
#define MAX_ENEMY_BULLETS           20      /*!< 最大敌机子弹数量 */

// 敌机参数
#define MAX_ENEMIES                 8       /*!< 最大同时存在的敌机数量 */
#define PLANE_ENEMY_SPAWN_MIN       700     /*!< 敌机生成最小间隔（ms）- 优化后更紧凑 */
#define PLANE_ENEMY_SPAWN_MAX       1300    /*!< 敌机生成最大间隔（ms）- 优化后更紧凑 */

// 道具参数
#define MAX_POWERUPS                5       /*!< 最大道具数量 */
#define PLANE_POWERUP_SPEED         1.0f    /*!< 道具移动速度（向左）*/

// 爆炸参数
#define MAX_EXPLOSIONS              8       /*!< 最大爆炸数量 */
#define PLANE_EXPLOSION_FRAME_TIME  50      /*!< 爆炸动画帧时间（ms）*/

// Boss参数
#define PLANE_BOSS_HP               20      /*!< Boss生命值 */
#define PLANE_BOSS_SCORE_TRIGGER    500     /*!< 每多少分出现Boss */

// -----------------------------------------------------------------------------
// 2. 枚举定义
// -----------------------------------------------------------------------------

/**
 * @brief 游戏状态枚举
 */
typedef enum {
    PLANE_STATE_READY = 0,      /*!< 准备开始 */
    PLANE_STATE_RUNNING,        /*!< 运行中 */
    PLANE_STATE_GAME_OVER       /*!< 游戏结束 */
} plane_game_state_t;

/**
 * @brief 敌机类型枚举
 */
typedef enum {
    ENEMY_TYPE_SMALL = 0,       /*!< 小型战斗机（1HP，快速）*/
    ENEMY_TYPE_MEDIUM,          /*!< 中型轰炸机（2HP，中速）*/
    ENEMY_TYPE_HEAVY,           /*!< 重型装甲（3HP，慢速，频繁开火）*/
    ENEMY_TYPE_FAST,            /*!< 快速敌机（1HP，超快，波浪移动）*/
    ENEMY_TYPE_BOSS             /*!< Boss（20HP，特殊攻击）*/
} enemy_type_t;

/**
 * @brief 道具类型枚举
 */
typedef enum {
    POWERUP_WEAPON = 0,         /*!< 火力升级（P道具）*/
    POWERUP_SHIELD,             /*!< 护盾（S道具）*/
    POWERUP_BOMB                /*!< 清屏炸弹（B道具）*/
} powerup_type_t;

/**
 * @brief 爆炸类型枚举
 */
typedef enum {
    EXPLOSION_SMALL = 0,        /*!< 小型爆炸 */
    EXPLOSION_MEDIUM,           /*!< 中型爆炸 */
    EXPLOSION_LARGE             /*!< 大型爆炸（Boss）*/
} explosion_type_t;

// -----------------------------------------------------------------------------
// 3. 数据结构定义
// -----------------------------------------------------------------------------

/**
 * @brief 子弹结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活（1=使用中，0=未使用）*/
    int16_t x;                  /*!< X坐标 */
    int16_t y;                  /*!< Y坐标 */
    float vx;                   /*!< X方向速度 */
    float vy;                   /*!< Y方向速度 */
    uint8_t damage;             /*!< 伤害值 */
} bullet_t;

/**
 * @brief 敌机结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活 */
    int16_t x;                  /*!< X坐标 */
    int16_t y;                  /*!< Y坐标 */
    float vx;                   /*!< X方向速度 */
    float vy;                   /*!< Y方向速度 */
    uint8_t hp;                 /*!< 当前生命值 */
    uint8_t max_hp;             /*!< 最大生命值 */
    enemy_type_t type;          /*!< 敌机类型 */
    uint32_t last_shoot_time;   /*!< 上次射击时间 */
    uint8_t width;              /*!< 碰撞箱宽度 */
    uint8_t height;             /*!< 碰撞箱高度 */
    uint32_t spawn_time;        /*!< 生成时间（用于波浪移动）*/
} enemy_t;

/**
 * @brief 道具结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活 */
    int16_t x;                  /*!< X坐标 */
    int16_t y;                  /*!< Y坐标 */
    float vx;                   /*!< X方向速度 */
    powerup_type_t type;        /*!< 道具类型 */
} powerup_t;

/**
 * @brief 爆炸结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活 */
    int16_t x;                  /*!< X坐标 */
    int16_t y;                  /*!< Y坐标 */
    explosion_type_t type;      /*!< 爆炸类型 */
    uint8_t frame;              /*!< 当前动画帧（0-2）*/
    uint32_t last_frame_time;   /*!< 上次帧切换时间 */
} explosion_t;

/**
 * @brief Boss结构体
 */
typedef struct {
    uint8_t active;             /*!< 是否激活 */
    int16_t x;                  /*!< X坐标 */
    int16_t y;                  /*!< Y坐标 */
    float vy;                   /*!< Y方向速度 */
    uint8_t hp;                 /*!< 当前生命值 */
    uint8_t max_hp;             /*!< 最大生命值 */
    uint8_t phase;              /*!< 当前阶段（1-3）*/
    uint32_t last_attack_time;  /*!< 上次攻击时间 */
    uint32_t spawn_time;        /*!< 生成时间（用于移动）*/
} boss_t;

/**
 * @brief 打飞机游戏状态结构体
 */
typedef struct {
    // ========== 核心状态管理 ==========
    plane_game_state_t game_state;      /*!< 游戏当前状态 */
    uint8_t is_active;                  /*!< 游戏是否活跃（1=活跃，0=停止）*/
    void (*exit_callback)(void);        /*!< 退出回调函数（按B键返回菜单）*/

    // ========== 玩家 ==========
    int16_t player_x;                   /*!< 玩家X坐标（固定在左侧）*/
    int16_t player_y;                   /*!< 玩家Y坐标（上下移动）*/
    uint8_t player_hp;                  /*!< 玩家生命值 */
    uint8_t player_shield;              /*!< 护盾（0=无，1=有）*/
    uint8_t weapon_level;               /*!< 武器等级（1-3）*/
    uint32_t last_shoot_time;           /*!< 上次射击时间 */

    // ========== 对象池 ==========
    bullet_t player_bullets[MAX_PLAYER_BULLETS];    /*!< 玩家子弹数组 */
    enemy_t enemies[MAX_ENEMIES];                   /*!< 敌机数组 */
    bullet_t enemy_bullets[MAX_ENEMY_BULLETS];      /*!< 敌机子弹数组 */
    powerup_t powerups[MAX_POWERUPS];               /*!< 道具数组 */
    explosion_t explosions[MAX_EXPLOSIONS];         /*!< 爆炸数组 */

    // ========== Boss系统 ==========
    boss_t boss;                        /*!< Boss数据 */
    uint8_t boss_warning;               /*!< Boss警告标志（1=显示警告）*/
    uint32_t boss_warning_start_time;   /*!< Boss警告开始时间 */

    // ========== 时间戳系统 ==========
    uint32_t last_frame_time;           /*!< 上一帧时间戳 */
    uint32_t last_enemy_spawn_time;     /*!< 上次生成敌机时间 */

    // ========== 游戏数据 ==========
    uint32_t score;                     /*!< 当前分数 */
    uint32_t high_score;                /*!< 历史最高分 */
    uint16_t next_enemy_delay;          /*!< 下次生成敌机延迟（ms）*/
    uint32_t last_boss_score;           /*!< 上次Boss出现时的分数 */
    uint8_t difficulty_level;           /*!< 难度等级（每击败1次Boss +1）*/
    uint8_t boss_count;                 /*!< 已击败Boss数量 */
} plane_game_t;

// -----------------------------------------------------------------------------
// 4. API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化打飞机游戏
 * @param game: 游戏状态结构体指针
 * @note  设置游戏初始状态，清空所有对象池
 */
void plane_game_init(plane_game_t *game);

/**
 * @brief 处理玩家输入
 * @param game: 游戏状态结构体指针
 * @note  10ms周期调用，处理移动、射击、退出输入
 */
void plane_game_update_input(plane_game_t *game);

/**
 * @brief 更新游戏逻辑
 * @param game: 游戏状态结构体指针
 * @note  处理移动、射击、碰撞检测、分数更新
 */
void plane_game_update_logic(plane_game_t *game);

/**
 * @brief 渲染游戏画面
 * @param game: 游戏状态结构体指针
 * @note  使用u8g2绘制所有游戏元素
 */
void plane_game_render(plane_game_t *game);

/**
 * @brief 游戏主任务（周期调用）
 * @param game: 游戏状态结构体指针
 * @note  集成输入、逻辑、渲染的完整游戏循环，带帧率控制
 */
void plane_game_task(plane_game_t *game);

/**
 * @brief 激活游戏
 * @param game: 游戏状态结构体指针
 * @note  从菜单进入游戏时调用，使游戏开始运行
 */
void plane_game_activate(plane_game_t *game);

/**
 * @brief 停用游戏
 * @param game: 游戏状态结构体指针
 * @note  返回菜单时调用，使游戏停止运行
 */
void plane_game_deactivate(plane_game_t *game);

/**
 * @brief 设置游戏退出回调
 * @param game: 游戏状态结构体指针
 * @param callback: 退出回调函数（按B键时调用）
 * @note  用于从游戏返回菜单
 */
void plane_game_set_exit_callback(plane_game_t *game, void (*callback)(void));

#endif // __PLANE_GAME_H__
