#include "dino_game.h"

// =============================================================================
// 恐龙跑酷游戏实现（精致优化版）
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 像素画资源定义（Bitmap数据）
// -----------------------------------------------------------------------------

// Chrome T-Rex恐龙跑步动画帧1（12x11）- 完全复刻原版
static const uint8_t dino_run_frame1[] = {
    0x00, 0x00,  // ............  (第1行，两个字节)
    0x1E, 0x00,  // ...####.....  头部上方
    0x3F, 0x00,  // ..######....  头部
    0x7F, 0x00,  // .#######....  头部+眼睛
    0xFE, 0x00,  // #######.....  嘴巴
    0xFF, 0x00,  // ########....  身体
    0xFE, 0x00,  // #######.....  身体+尾巴
    0x7E, 0x00,  // .######.....  腿部连接
    0x3E, 0x00,  // ..#####.....  腿部
    0x1C, 0x00,  // ...###......  左腿
    0x18, 0x00,  // ...##.......  左脚
};

// Chrome T-Rex恐龙跑步动画帧2（12x11）- 跑步姿势
static const uint8_t dino_run_frame2[] = {
    0x00, 0x00,  // ............
    0x1E, 0x00,  // ...####.....  头部上方
    0x3F, 0x00,  // ..######....  头部
    0x7F, 0x00,  // .#######....  头部+眼睛
    0xFE, 0x00,  // #######.....  嘴巴
    0xFF, 0x00,  // ########....  身体
    0xFE, 0x00,  // #######.....  身体+尾巴
    0x7E, 0x00,  // .######.....  腿部连接
    0x3E, 0x00,  // ..#####.....  腿部
    0x38, 0x00,  // ..###.......  右腿
    0x30, 0x00,  // ..##........  右脚
};

// 仙人掌（6x10）- 降低高度
static const uint8_t cactus_bitmap[] = {
    0x30,  // ..##....
    0x30,  // ..##....
    0x78,  // .####...
    0x78,  // .####...
    0xFC,  // ######..
    0x30,  // ..##....
    0x30,  // ..##....
    0x30,  // ..##....
    0x30,  // ..##....
    0x30,  // ..##....
};

// 云朵（8x4）
static const uint8_t cloud_bitmap[] = {
    0x7E,  // .######.
    0xFF,  // ########
    0xFF,  // ########
    0x7E,  // .######.
};

// -----------------------------------------------------------------------------
// 2. 内部辅助函数声明
// -----------------------------------------------------------------------------

static void init_clouds(dino_game_t *game);
static void update_jump(dino_game_t *game);
static void update_animation(dino_game_t *game);
static void spawn_obstacle(dino_game_t *game);
static void update_obstacles(dino_game_t *game);
static void update_clouds(dino_game_t *game);
static uint8_t check_collision(dino_game_t *game);
static void update_score(dino_game_t *game);
static void update_speed(dino_game_t *game);
static void draw_ui(dino_game_t *game, u8g2_t *u8g2);
static void draw_dino(dino_game_t *game, u8g2_t *u8g2);
static void draw_obstacles(dino_game_t *game, u8g2_t *u8g2);
static void draw_clouds(dino_game_t *game, u8g2_t *u8g2);
static void draw_ground(u8g2_t *u8g2);
static void draw_game_over(dino_game_t *game, u8g2_t *u8g2);
static void draw_bitmap(u8g2_t *u8g2, int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h);

// -----------------------------------------------------------------------------
// 3. API函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化恐龙跑酷游戏
 */
void dino_game_init(dino_game_t *game)
{
    // 保存重要的回调和状态（不能被清空）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;
    uint32_t saved_high_score = game->high_score;

    // 清空整个游戏状态
    memset(game, 0, sizeof(dino_game_t));

    // 恢复保存的数据
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;
    game->high_score = saved_high_score;

    // 设置初始状态
    game->game_state = DINO_STATE_READY;
    game->jump_state = DINO_JUMP_IDLE;
    game->dino_y = DINO_GROUND_Y;
    game->run_anim_frame = 0;
    game->current_jump_height = DINO_JUMP_HEIGHT;  // 默认跳跃高度
    game->jump_button_press_time = 0;

    // 初始化速度和分数
    game->speed = DINO_INITIAL_SPEED;
    game->score = 0;
    game->last_speed_up_score = 0;

    // 初始化时间戳
    uint32_t now = HAL_GetTick();
    game->last_frame_time = now;
    game->last_logic_update_time = now;
    game->last_score_time = now;
    game->last_obstacle_time = now;
    game->last_anim_time = now;
    game->next_obstacle_delay = DINO_OBSTACLE_MIN_DELAY;

    // 清空所有障碍物
    for (uint8_t i = 0; i < DINO_MAX_OBSTACLES; i++)
    {
        game->obstacles[i].active = 0;
    }

    // 初始化云朵
    init_clouds(game);
}

/**
 * @brief 处理玩家输入
 */
void dino_game_update_input(dino_game_t *game)
{
    // 退出游戏（B键）
    if (input_is_just_pressed(INPUT_BTN_B))
    {
        if (game->exit_callback != NULL)
        {
            game->exit_callback();
        }
        return;
    }

    if (game->game_state == DINO_STATE_READY)
    {
        // 按A键开始游戏
        if (input_is_just_pressed(INPUT_BTN_A))
        {
            game->game_state = DINO_STATE_RUNNING;
            game->score = 0;
            game->speed = DINO_INITIAL_SPEED;
            game->last_score_time = HAL_GetTick();
            game->last_obstacle_time = HAL_GetTick();
        }
    }
    else if (game->game_state == DINO_STATE_RUNNING)
    {
        // 跳跃（只能在地面时跳）
        if (input_is_just_pressed(INPUT_BTN_A))
        {
            if (game->jump_state == DINO_JUMP_IDLE)
            {
                game->jump_state = DINO_JUMP_RISING;
                game->jump_start_time = HAL_GetTick();
                game->jump_button_press_time = HAL_GetTick();
                game->current_jump_height = DINO_JUMP_HEIGHT;  // 先设置为普通跳跃
            }
        }

        // 检测长按A键（跳得更高）
        if (game->jump_state == DINO_JUMP_RISING && input_is_pressed(INPUT_BTN_A))
        {
            uint32_t hold_time = HAL_GetTick() - game->jump_button_press_time;
            if (hold_time >= DINO_LONG_PRESS_TIME)
            {
                game->current_jump_height = DINO_JUMP_HEIGHT_HIGH;  // 升级为高跳
            }
        }
    }
    else if (game->game_state == DINO_STATE_GAME_OVER)
    {
        // 重新开始（按A键）
        if (input_is_just_pressed(INPUT_BTN_A))
        {
            dino_game_init(game);
            game->game_state = DINO_STATE_RUNNING;
            game->last_score_time = HAL_GetTick();
            game->last_obstacle_time = HAL_GetTick();
        }
    }
}

/**
 * @brief 更新游戏逻辑
 */
void dino_game_update_logic(dino_game_t *game)
{
    if (game->game_state != DINO_STATE_RUNNING)
    {
        return;  // 只在运行状态更新逻辑
    }

    uint32_t now = HAL_GetTick();

    // 1. 更新跳跃
    update_jump(game);

    // 2. 更新跑步动画
    update_animation(game);

    // 3. 更新障碍物位置
    update_obstacles(game);

    // 4. 更新云朵位置
    update_clouds(game);

    // 5. 生成新障碍物（根据时间间隔）
    if (now - game->last_obstacle_time >= game->next_obstacle_delay)
    {
        spawn_obstacle(game);
    }

    // 6. 碰撞检测
    if (check_collision(game))
    {
        game->game_state = DINO_STATE_GAME_OVER;

        // 更新最高分
        if (game->score > game->high_score)
        {
            game->high_score = game->score;
        }
        return;
    }

    // 7. 更新分数
    update_score(game);

    // 8. 更新速度
    update_speed(game);
}

/**
 * @brief 渲染游戏画面
 */
void dino_game_render(dino_game_t *game)
{
    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    if (game->game_state == DINO_STATE_READY)
    {
        // 显示"Press A to Start"
        u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(u8g2, 10, 32, "Press A to Start");
    }
    else if (game->game_state == DINO_STATE_RUNNING)
    {
        // 1. 绘制UI（分数）
        draw_ui(game, u8g2);

        // 2. 绘制云朵（背景）
        draw_clouds(game, u8g2);

        // 3. 绘制地面
        draw_ground(u8g2);

        // 4. 绘制恐龙
        draw_dino(game, u8g2);

        // 5. 绘制障碍物
        draw_obstacles(game, u8g2);
    }
    else if (game->game_state == DINO_STATE_GAME_OVER)
    {
        // 绘制完整场景
        draw_ui(game, u8g2);
        draw_clouds(game, u8g2);
        draw_ground(u8g2);
        draw_dino(game, u8g2);
        draw_obstacles(game, u8g2);

        // 显示"Game Over"
        draw_game_over(game, u8g2);
    }

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务（周期调用，带帧率控制）
 */
void dino_game_task(dino_game_t *game)
{
    // 检查活跃状态
    if (!game->is_active)
    {
        return;
    }

    uint32_t now = HAL_GetTick();

    // 帧率控制：30fps（33ms一帧）
    if (now - game->last_frame_time < DINO_FRAME_TIME_MS)
    {
        // 输入仍然需要快速响应（10ms周期）
        dino_game_update_input(game);
        return;  // 未到渲染时间，直接返回
    }

    // 更新帧时间
    game->last_frame_time = now;

    // 完整游戏循环（30fps）
    dino_game_update_input(game);
    dino_game_update_logic(game);
    dino_game_render(game);
}

/**
 * @brief 激活游戏
 */
void dino_game_activate(dino_game_t *game)
{
    game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void dino_game_deactivate(dino_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置游戏退出回调
 */
void dino_game_set_exit_callback(dino_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

// -----------------------------------------------------------------------------
// 4. 内部辅助函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化云朵
 */
static void init_clouds(dino_game_t *game)
{
    for (uint8_t i = 0; i < DINO_MAX_CLOUDS; i++)
    {
        game->clouds[i].active = 1;
        game->clouds[i].x = 40 + i * 50;  // 均匀分布
        game->clouds[i].y = 15 + (i * 5);  // 不同高度
    }
}

/**
 * @brief 更新跳跃物理（抛物线模拟）
 */
static void update_jump(dino_game_t *game)
{
    if (game->jump_state == DINO_JUMP_IDLE)
    {
        return;  // 在地面上，无需更新
    }

    uint32_t now = HAL_GetTick();
    uint32_t elapsed = now - game->jump_start_time;

    if (game->jump_state == DINO_JUMP_RISING)
    {
        // 上升阶段（0-200ms）
        if (elapsed < DINO_JUMP_DURATION / 2)
        {
            // 抛物线上升（使用可变的跳跃高度）
            float progress = (float)elapsed / (DINO_JUMP_DURATION / 2);
            game->dino_y = DINO_GROUND_Y - (int16_t)(progress * game->current_jump_height);
        }
        else
        {
            // 切换到下降阶段
            game->jump_state = DINO_JUMP_FALLING;
        }
    }
    else if (game->jump_state == DINO_JUMP_FALLING)
    {
        // 下降阶段（200-400ms）
        uint32_t fall_time = elapsed - (DINO_JUMP_DURATION / 2);
        if (fall_time < DINO_JUMP_DURATION / 2)
        {
            // 抛物线下降（使用可变的跳跃高度）
            float progress = (float)fall_time / (DINO_JUMP_DURATION / 2);
            game->dino_y = DINO_GROUND_Y - game->current_jump_height +
                           (int16_t)(progress * game->current_jump_height);
        }
        else
        {
            // 落地
            game->dino_y = DINO_GROUND_Y;
            game->jump_state = DINO_JUMP_IDLE;
        }
    }
}

/**
 * @brief 更新跑步动画
 */
static void update_animation(dino_game_t *game)
{
    // 只在地面上才播放跑步动画
    if (game->jump_state != DINO_JUMP_IDLE)
    {
        return;
    }

    uint32_t now = HAL_GetTick();
    if (now - game->last_anim_time >= DINO_RUN_ANIM_INTERVAL)
    {
        game->run_anim_frame = 1 - game->run_anim_frame;  // 0和1切换
        game->last_anim_time = now;
    }
}

/**
 * @brief 生成新障碍物
 */
static void spawn_obstacle(dino_game_t *game)
{
    // 查找空闲槽位
    for (uint8_t i = 0; i < DINO_MAX_OBSTACLES; i++)
    {
        if (!game->obstacles[i].active)
        {
            game->obstacles[i].active = 1;
            game->obstacles[i].x = DINO_SCREEN_WIDTH;  // 从屏幕右侧出现
            game->obstacles[i].width = DINO_OBSTACLE_WIDTH;
            game->obstacles[i].height = DINO_OBSTACLE_HEIGHT;
            game->obstacles[i].type = OBSTACLE_TYPE_CACTUS_SMALL;

            // 随机下次生成延迟
            game->next_obstacle_delay = (uint16_t)rng_get_random_range(
                DINO_OBSTACLE_MIN_DELAY, DINO_OBSTACLE_MAX_DELAY);
            game->last_obstacle_time = HAL_GetTick();
            break;
        }
    }
}

/**
 * @brief 更新障碍物位置
 */
static void update_obstacles(dino_game_t *game)
{
    for (uint8_t i = 0; i < DINO_MAX_OBSTACLES; i++)
    {
        if (!game->obstacles[i].active)
        {
            continue;
        }

        // 向左移动障碍物
        game->obstacles[i].x -= (int16_t)game->speed;

        // 移出屏幕后清除
        if (game->obstacles[i].x < -DINO_OBSTACLE_WIDTH)
        {
            game->obstacles[i].active = 0;
        }
    }
}

/**
 * @brief 更新云朵位置
 */
static void update_clouds(dino_game_t *game)
{
    for (uint8_t i = 0; i < DINO_MAX_CLOUDS; i++)
    {
        if (!game->clouds[i].active)
        {
            continue;
        }

        // 向左移动云朵（慢速）
        game->clouds[i].x -= (int16_t)(DINO_CLOUD_SPEED);

        // 移出屏幕后从右侧重新出现
        if (game->clouds[i].x < -8)
        {
            game->clouds[i].x = DINO_SCREEN_WIDTH + (rng_get_random_byte() % 30);
            game->clouds[i].y = 15 + (rng_get_random_byte() % 15);
        }
    }
}

/**
 * @brief 碰撞检测（AABB算法，收紧碰撞箱）
 */
static uint8_t check_collision(dino_game_t *game)
{
    // 恐龙碰撞箱（收紧2像素提升手感）
    int16_t dino_x1 = DINO_X + 2;
    int16_t dino_y1 = game->dino_y - DINO_HEIGHT + 2;
    int16_t dino_x2 = DINO_X + DINO_WIDTH - 2;
    int16_t dino_y2 = game->dino_y - 2;

    // 检测所有障碍物
    for (uint8_t i = 0; i < DINO_MAX_OBSTACLES; i++)
    {
        if (!game->obstacles[i].active)
        {
            continue;
        }

        dino_obstacle_t *obs = &game->obstacles[i];
        int16_t obs_x1 = obs->x + 1;
        int16_t obs_y1 = DINO_GROUND_Y - obs->height + 1;
        int16_t obs_x2 = obs->x + obs->width - 1;
        int16_t obs_y2 = DINO_GROUND_Y - 1;

        // AABB碰撞检测
        if (dino_x1 < obs_x2 && dino_x2 > obs_x1 &&
            dino_y1 < obs_y2 && dino_y2 > obs_y1)
        {
            return 1;  // 发生碰撞
        }
    }

    return 0;  // 无碰撞
}

/**
 * @brief 更新分数
 */
static void update_score(dino_game_t *game)
{
    uint32_t now = HAL_GetTick();

    // 每100ms增加1分
    if (now - game->last_score_time >= DINO_SCORE_INTERVAL)
    {
        game->score++;
        game->last_score_time = now;
    }
}

/**
 * @brief 更新速度（每100分加速一次）
 */
static void update_speed(dino_game_t *game)
{
    // 检查是否该加速
    if (game->score > game->last_speed_up_score &&
        (game->score - game->last_speed_up_score) >= DINO_SPEED_UP_INTERVAL)
    {
        game->last_speed_up_score = game->score;

        // 增加速度
        game->speed += DINO_SPEED_INCREMENT;
        if (game->speed > DINO_MAX_SPEED)
        {
            game->speed = DINO_MAX_SPEED;
        }
    }
}

/**
 * @brief 绘制UI（分数显示）
 */
static void draw_ui(dino_game_t *game, u8g2_t *u8g2)
{
    char buf[32];

    // 设置字体
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 绘制当前分数（左上角）
    sprintf(buf, "%05u", (unsigned int)game->score);
    u8g2_DrawStr(u8g2, 2, 7, buf);

    // 绘制最高分（右上角）
    sprintf(buf, "HI:%05u", (unsigned int)game->high_score);
    u8g2_DrawStr(u8g2, 78, 7, buf);
}

/**
 * @brief 绘制恐龙（像素画+跑步动画）
 */
static void draw_dino(dino_game_t *game, u8g2_t *u8g2)
{
    // 根据动画帧选择bitmap
    const uint8_t *bitmap = (game->run_anim_frame == 0) ?
                            dino_run_frame1 : dino_run_frame2;

    // 绘制恐龙
    draw_bitmap(u8g2, DINO_X, game->dino_y - DINO_HEIGHT,
                bitmap, DINO_WIDTH, DINO_HEIGHT);
}

/**
 * @brief 绘制障碍物（像素画仙人掌）
 */
static void draw_obstacles(dino_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < DINO_MAX_OBSTACLES; i++)
    {
        if (!game->obstacles[i].active)
        {
            continue;
        }

        dino_obstacle_t *obs = &game->obstacles[i];

        // 绘制仙人掌
        draw_bitmap(u8g2, obs->x, DINO_GROUND_Y - obs->height,
                    cactus_bitmap, obs->width, obs->height);
    }
}

/**
 * @brief 绘制云朵
 */
static void draw_clouds(dino_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < DINO_MAX_CLOUDS; i++)
    {
        if (!game->clouds[i].active)
        {
            continue;
        }

        // 绘制云朵
        draw_bitmap(u8g2, game->clouds[i].x, game->clouds[i].y,
                    cloud_bitmap, 8, 4);
    }
}

/**
 * @brief 绘制地面（带装饰）
 */
static void draw_ground(u8g2_t *u8g2)
{
    // 主地面线
    u8g2_DrawHLine(u8g2, 0, DINO_GROUND_Y, DINO_SCREEN_WIDTH);
    u8g2_DrawHLine(u8g2, 0, DINO_GROUND_Y + 1, DINO_SCREEN_WIDTH);

    // 地面装饰（小草点）
    for (uint8_t i = 0; i < 10; i++)
    {
        uint8_t x = i * 13;
        u8g2_DrawPixel(u8g2, x, DINO_GROUND_Y + 2);
        u8g2_DrawPixel(u8g2, x + 1, DINO_GROUND_Y + 3);
    }
}

/**
 * @brief 绘制游戏结束画面
 */
static void draw_game_over(dino_game_t *game, u8g2_t *u8g2)
{
    // 设置字体
    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);

    // 绘制"GAME OVER"（居中）
    u8g2_DrawStr(u8g2, 30, 28, "GAME OVER");

    // 绘制重新开始提示
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(u8g2, 20, 40, "Press A to Retry");
}

/**
 * @brief 绘制Bitmap（像素画，支持多字节宽度）
 */
static void draw_bitmap(u8g2_t *u8g2, int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h)
{
    // 计算每行需要多少字节（向上取整）
    uint8_t bytes_per_row = (w + 7) / 8;

    for (uint8_t row = 0; row < h; row++)
    {
        // 遍历当前行的所有像素
        for (uint8_t col = 0; col < w; col++)
        {
            // 计算当前像素在bitmap中的位置
            uint8_t byte_index = row * bytes_per_row + (col / 8);
            uint8_t bit_index = 7 - (col % 8);

            // 检查对应的bit是否为1
            if (bitmap[byte_index] & (1 << bit_index))
            {
                u8g2_DrawPixel(u8g2, x + col, y + row);
            }
        }
    }
}
