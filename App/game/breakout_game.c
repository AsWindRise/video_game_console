/**
 * @file breakout_game.c
 * @brief 打砖块游戏实现
 */

#include "breakout_game.h"

/* ======================== 关卡数据定义 ======================== */

/**
 * @brief 关卡1：基础布局
 * @note  3行普通砖块，便于新手熟悉操作
 */
static const brick_type_t level1_layout[BREAKOUT_BRICK_ROWS][BREAKOUT_BRICK_COLS] = {
    {BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE},
    {BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE},
};

/**
 * @brief 关卡2：中等难度（含坚固砖块）
 */
static const brick_type_t level2_layout[BREAKOUT_BRICK_ROWS][BREAKOUT_BRICK_COLS] = {
    {BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE},
    {BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG},
    {BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE},
};

/**
 * @brief 关卡3：困难（含不可破坏砖块）
 */
static const brick_type_t level3_layout[BREAKOUT_BRICK_ROWS][BREAKOUT_BRICK_COLS] = {
    {BRICK_UNBREAKABLE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_UNBREAKABLE},
    {BRICK_STRONG, BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG, BRICK_STRONG},
    {BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG},
    {BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL},
    {BRICK_STRONG, BRICK_STRONG, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_NORMAL, BRICK_STRONG, BRICK_STRONG},
    {BRICK_UNBREAKABLE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_NONE, BRICK_UNBREAKABLE},
};

/* ======================== 内部函数声明 ======================== */

// 关卡管理
static void load_level(breakout_game_t *game, uint8_t level);
static uint8_t count_bricks(breakout_game_t *game);

// 游戏控制
static void reset_ball(breakout_game_t *game);
static void launch_ball(breakout_game_t *game);
static void lose_life(breakout_game_t *game);

// 碰撞处理
static void check_wall_collision(breakout_game_t *game);
static void check_paddle_collision(breakout_game_t *game);
static void check_brick_collision(breakout_game_t *game);

// 渲染辅助
static void render_paddle(breakout_game_t *game, u8g2_t *u8g2);
static void render_ball(breakout_game_t *game, u8g2_t *u8g2);
static void render_bricks(breakout_game_t *game, u8g2_t *u8g2);
static void render_ui(breakout_game_t *game, u8g2_t *u8g2);

/* ======================== 关卡管理函数 ======================== */

/**
 * @brief 加载关卡数据
 */
static void load_level(breakout_game_t *game, uint8_t level)
{
    const brick_type_t (*layout)[BREAKOUT_BRICK_COLS] = NULL;

    // 选择关卡布局
    switch (level) {
        case 1:
            layout = level1_layout;
            break;
        case 2:
            layout = level2_layout;
            break;
        case 3:
            layout = level3_layout;
            break;
        default:
            layout = level1_layout;
            break;
    }

    // 复制关卡数据到游戏状态
    for (uint8_t row = 0; row < BREAKOUT_BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BREAKOUT_BRICK_COLS; col++) {
            brick_type_t type = layout[row][col];
            game->bricks[row][col].type = type;

            // 设置击破次数
            if (type == BRICK_NORMAL) {
                game->bricks[row][col].hits_remaining = 1;
            } else if (type == BRICK_STRONG) {
                game->bricks[row][col].hits_remaining = 2;
            } else if (type == BRICK_UNBREAKABLE) {
                game->bricks[row][col].hits_remaining = 255;  // 无限
            } else {
                game->bricks[row][col].hits_remaining = 0;
            }
        }
    }

    // 统计砖块数量
    game->bricks_remaining = count_bricks(game);
}

/**
 * @brief 统计剩余可破坏砖块数量
 */
static uint8_t count_bricks(breakout_game_t *game)
{
    uint8_t count = 0;

    for (uint8_t row = 0; row < BREAKOUT_BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BREAKOUT_BRICK_COLS; col++) {
            brick_type_t type = game->bricks[row][col].type;
            // 只统计可破坏砖块
            if (type != BRICK_NONE && type != BRICK_UNBREAKABLE) {
                count++;
            }
        }
    }

    return count;
}

/* ======================== 游戏控制函数 ======================== */

/**
 * @brief 重置球到挡板上
 */
static void reset_ball(breakout_game_t *game)
{
    game->ball_attached = 1;
    ball_init(&game->ball,
              game->paddle_x,
              BREAKOUT_PADDLE_Y - BREAKOUT_BALL_RADIUS - 1,
              0, 0,
              BREAKOUT_BALL_RADIUS);
}

/**
 * @brief 发射球
 */
static void launch_ball(breakout_game_t *game)
{
    game->ball_attached = 0;
    // 发射角度：略微向右上
    ball_init(&game->ball,
              game->ball.x, game->ball.y,
              BREAKOUT_BALL_SPEED * 0.5f,
              -BREAKOUT_BALL_SPEED,
              BREAKOUT_BALL_RADIUS);
}

/**
 * @brief 失去一条命
 */
static void lose_life(breakout_game_t *game)
{
    game->lives--;

    if (game->lives == 0) {
        // 游戏结束
        game->game_state = BREAKOUT_STATE_GAME_OVER;
    } else {
        // 重置球
        reset_ball(game);
        game->game_state = BREAKOUT_STATE_AIMING;
    }

    // 清空连击
    game->combo = 0;
}

/* ======================== 碰撞检测函数 ======================== */

/**
 * @brief 检测墙壁碰撞
 */
static void check_wall_collision(breakout_game_t *game)
{
    // 左右墙壁
    if (game->ball.x - game->ball.radius <= 0) {
        game->ball.x = game->ball.radius;
        ball_reflect_horizontal(&game->ball);
    } else if (game->ball.x + game->ball.radius >= BREAKOUT_SCREEN_WIDTH) {
        game->ball.x = BREAKOUT_SCREEN_WIDTH - game->ball.radius;
        ball_reflect_horizontal(&game->ball);
    }

    // 顶部墙壁
    if (game->ball.y - game->ball.radius <= 0) {
        game->ball.y = game->ball.radius;
        ball_reflect_vertical(&game->ball);
    }

    // 底部（失去生命）
    if (game->ball.y - game->ball.radius > BREAKOUT_SCREEN_HEIGHT) {
        lose_life(game);
    }
}

/**
 * @brief 检测挡板碰撞
 */
static void check_paddle_collision(breakout_game_t *game)
{
    rect_t paddle_rect = {
        .x = game->paddle_x - BREAKOUT_PADDLE_WIDTH / 2,
        .y = BREAKOUT_PADDLE_Y,
        .width = BREAKOUT_PADDLE_WIDTH,
        .height = BREAKOUT_PADDLE_HEIGHT
    };

    // 检测碰撞（只有球向下运动时才检测）
    if (game->ball.vy > 0 && ball_collides_with_rect(&game->ball, &paddle_rect)) {
        // 挡板反弹（带角度调整）
        ball_reflect_paddle(&game->ball, game->paddle_x, BREAKOUT_PADDLE_WIDTH);

        // 微调球的位置，避免陷入挡板
        game->ball.y = BREAKOUT_PADDLE_Y - game->ball.radius - 1;
    }
}

/**
 * @brief 检测砖块碰撞
 */
static void check_brick_collision(breakout_game_t *game)
{
    for (uint8_t row = 0; row < BREAKOUT_BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BREAKOUT_BRICK_COLS; col++) {
            brick_t *brick = &game->bricks[row][col];

            // 跳过无砖块和已破坏砖块
            if (brick->type == BRICK_NONE || brick->hits_remaining == 0) {
                continue;
            }

            // 计算砖块矩形
            rect_t brick_rect = {
                .x = BREAKOUT_BRICK_OFFSET_X + col * (BREAKOUT_BRICK_WIDTH + BREAKOUT_BRICK_GAP_X),
                .y = BREAKOUT_BRICK_OFFSET_Y + row * (BREAKOUT_BRICK_HEIGHT + BREAKOUT_BRICK_GAP_Y),
                .width = BREAKOUT_BRICK_WIDTH,
                .height = BREAKOUT_BRICK_HEIGHT
            };

            // 检测碰撞
            uint8_t hit_top, hit_bottom, hit_left, hit_right;
            if (ball_collides_with_rect_detailed(&game->ball, &brick_rect,
                                                  &hit_top, &hit_bottom,
                                                  &hit_left, &hit_right)) {

                // 不可破坏砖块：只反弹不破坏
                if (brick->type == BRICK_UNBREAKABLE) {
                    if (hit_top || hit_bottom) {
                        ball_reflect_vertical(&game->ball);
                    } else {
                        ball_reflect_horizontal(&game->ball);
                    }
                    return;
                }

                // 可破坏砖块：扣血
                brick->hits_remaining--;

                // 更新连击
                game->combo++;
                game->combo_timer = HAL_GetTick();

                // 得分（连击加成）
                uint16_t base_score = (brick->type == BRICK_STRONG) ? 20 : 10;
                game->score += base_score + game->combo * 5;

                // 破坏砖块
                if (brick->hits_remaining == 0) {
                    brick->type = BRICK_NONE;
                    game->bricks_remaining--;

                    // 检查关卡完成
                    if (game->bricks_remaining == 0) {
                        game->game_state = BREAKOUT_STATE_LEVEL_CLEAR;
                        game->level_clear_start_time = HAL_GetTick();
                    }
                }

                // 反弹
                if (hit_top || hit_bottom) {
                    ball_reflect_vertical(&game->ball);
                } else {
                    ball_reflect_horizontal(&game->ball);
                }

                return;  // 每帧只处理一个砖块碰撞
            }
        }
    }
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化游戏
 */
void breakout_game_init(breakout_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空游戏状态
    memset(game, 0, sizeof(breakout_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    game->game_state = BREAKOUT_STATE_READY;
    game->lives = BREAKOUT_MAX_LIVES;
    game->level = 1;
    game->score = 0;
    game->combo = 0;

    game->paddle_x = BREAKOUT_SCREEN_WIDTH / 2;

    // 加载关卡
    load_level(game, game->level);

    // 重置球
    reset_ball(game);
}

/**
 * @brief 激活游戏
 */
void breakout_game_activate(breakout_game_t *game)
{
    game->is_active = 1;
    game->game_state = BREAKOUT_STATE_AIMING;
}

/**
 * @brief 停用游戏
 */
void breakout_game_deactivate(breakout_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void breakout_game_set_exit_callback(breakout_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void breakout_game_update_input(breakout_game_t *game)
{
    if (!game->is_active) return;

    // B键：退出游戏
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();
        }
        return;
    }

    // READY状态：按A键开始
    if (game->game_state == BREAKOUT_STATE_READY) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = BREAKOUT_STATE_AIMING;
        }
        return;
    }

    // GAME_OVER/WIN状态：按START重新开始
    if (game->game_state == BREAKOUT_STATE_GAME_OVER ||
        game->game_state == BREAKOUT_STATE_WIN) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            breakout_game_init(game);
            breakout_game_activate(game);
        }
        return;
    }

    // LEVEL_CLEAR状态：按A键进入下一关
    if (game->game_state == BREAKOUT_STATE_LEVEL_CLEAR) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->level++;
            if (game->level > BREAKOUT_MAX_LEVELS) {
                game->game_state = BREAKOUT_STATE_WIN;
            } else {
                load_level(game, game->level);
                reset_ball(game);
                game->game_state = BREAKOUT_STATE_AIMING;
            }
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == BREAKOUT_STATE_PLAYING) {
            game->game_state = BREAKOUT_STATE_PAUSED;
        } else if (game->game_state == BREAKOUT_STATE_PAUSED) {
            game->game_state = BREAKOUT_STATE_PLAYING;
        }
        return;
    }

    // 暂停状态不处理移动
    if (game->game_state == BREAKOUT_STATE_PAUSED) {
        return;
    }

    // 挡板左右移动
    if (input_is_pressed(INPUT_BTN_LEFT)) {
        game->paddle_x -= BREAKOUT_PADDLE_SPEED;
        if (game->paddle_x < BREAKOUT_PADDLE_WIDTH / 2) {
            game->paddle_x = BREAKOUT_PADDLE_WIDTH / 2;
        }

        // 球附着时跟随移动
        if (game->ball_attached) {
            game->ball.x = game->paddle_x;
        }
    }

    if (input_is_pressed(INPUT_BTN_RIGHT)) {
        game->paddle_x += BREAKOUT_PADDLE_SPEED;
        if (game->paddle_x > BREAKOUT_SCREEN_WIDTH - BREAKOUT_PADDLE_WIDTH / 2) {
            game->paddle_x = BREAKOUT_SCREEN_WIDTH - BREAKOUT_PADDLE_WIDTH / 2;
        }

        // 球附着时跟随移动
        if (game->ball_attached) {
            game->ball.x = game->paddle_x;
        }
    }

    // A键：发球
    if (game->game_state == BREAKOUT_STATE_AIMING && input_is_just_pressed(INPUT_BTN_A)) {
        launch_ball(game);
        game->game_state = BREAKOUT_STATE_PLAYING;
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 更新游戏逻辑
 */
void breakout_game_update_logic(breakout_game_t *game)
{
    if (!game->is_active) return;

    // 只有PLAYING状态更新逻辑
    if (game->game_state != BREAKOUT_STATE_PLAYING) {
        return;
    }

    // 更新球的位置
    if (!game->ball_attached) {
        ball_update(&game->ball);

        // 碰撞检测
        check_wall_collision(game);
        check_paddle_collision(game);
        check_brick_collision(game);
    }

    // 连击超时检测（1秒无击砖则清空连击）
    if (game->combo > 0) {
        uint32_t now = HAL_GetTick();
        if (now - game->combo_timer > 1000) {
            game->combo = 0;
        }
    }
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染挡板
 */
static void render_paddle(breakout_game_t *game, u8g2_t *u8g2)
{
    int16_t x = game->paddle_x - BREAKOUT_PADDLE_WIDTH / 2;
    u8g2_DrawBox(u8g2, x, BREAKOUT_PADDLE_Y, BREAKOUT_PADDLE_WIDTH, BREAKOUT_PADDLE_HEIGHT);
}

/**
 * @brief 渲染球
 */
static void render_ball(breakout_game_t *game, u8g2_t *u8g2)
{
    u8g2_DrawDisc(u8g2, (int16_t)game->ball.x, (int16_t)game->ball.y, game->ball.radius, U8G2_DRAW_ALL);
}

/**
 * @brief 渲染砖块
 */
static void render_bricks(breakout_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t row = 0; row < BREAKOUT_BRICK_ROWS; row++) {
        for (uint8_t col = 0; col < BREAKOUT_BRICK_COLS; col++) {
            brick_t *brick = &game->bricks[row][col];

            if (brick->type == BRICK_NONE || brick->hits_remaining == 0) {
                continue;
            }

            int16_t x = BREAKOUT_BRICK_OFFSET_X + col * (BREAKOUT_BRICK_WIDTH + BREAKOUT_BRICK_GAP_X);
            int16_t y = BREAKOUT_BRICK_OFFSET_Y + row * (BREAKOUT_BRICK_HEIGHT + BREAKOUT_BRICK_GAP_Y);

            // 不可破坏砖块：填充图案
            if (brick->type == BRICK_UNBREAKABLE) {
                u8g2_DrawBox(u8g2, x, y, BREAKOUT_BRICK_WIDTH, BREAKOUT_BRICK_HEIGHT);
            }
            // 坚固砖块：空心（第一次击中后）
            else if (brick->type == BRICK_STRONG && brick->hits_remaining == 1) {
                u8g2_DrawFrame(u8g2, x, y, BREAKOUT_BRICK_WIDTH, BREAKOUT_BRICK_HEIGHT);
            }
            // 普通砖块和完整坚固砖块：实心
            else {
                u8g2_DrawBox(u8g2, x, y, BREAKOUT_BRICK_WIDTH, BREAKOUT_BRICK_HEIGHT);
            }
        }
    }
}

/**
 * @brief 渲染UI信息
 */
static void render_ui(breakout_game_t *game, u8g2_t *u8g2)
{
    char buf[16];

    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 左上角：生命
    for (uint8_t i = 0; i < game->lives; i++) {
        u8g2_DrawDisc(u8g2, 4 + i * 6, 1, 2, U8G2_DRAW_ALL);
    }

    // 右上角：分数
    snprintf(buf, sizeof(buf), "%lu", game->score);
    u8g2_DrawStr(u8g2, 90, 6, buf);

    // 连击提示（右侧）
    if (game->combo >= 2) {
        snprintf(buf, sizeof(buf), "x%u", game->combo);
        u8g2_DrawStr(u8g2, 110, 6, buf);
    }
}

/**
 * @brief 渲染游戏画面
 */
void breakout_game_render(breakout_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    // READY状态
    if (game->game_state == BREAKOUT_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 28, 26, "BREAKOUT");
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 20, 40, "Press A Start");
        u8g2_SendBuffer(u8g2);
        return;
    }

    // GAME_OVER状态
    if (game->game_state == BREAKOUT_STATE_GAME_OVER) {
        render_bricks(game, u8g2);
        render_paddle(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 16, 30, "GAME OVER");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %lu", game->score);
        u8g2_DrawStr(u8g2, 28, 44, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // WIN状态
    if (game->game_state == BREAKOUT_STATE_WIN) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 10, 26, "YOU WIN!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %lu", game->score);
        u8g2_DrawStr(u8g2, 28, 40, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // LEVEL_CLEAR状态
    if (game->game_state == BREAKOUT_STATE_LEVEL_CLEAR) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 10, 26, "LEVEL CLEAR!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Level %u", game->level);
        u8g2_DrawStr(u8g2, 38, 40, buf);

        u8g2_DrawStr(u8g2, 22, 58, "Press A Next");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == BREAKOUT_STATE_PAUSED) {
        render_bricks(game, u8g2);
        render_paddle(game, u8g2);
        render_ball(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 32, 35, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PLAYING/AIMING状态
    render_bricks(game, u8g2);
    render_paddle(game, u8g2);
    render_ball(game, u8g2);
    render_ui(game, u8g2);

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void breakout_game_task(breakout_game_t *game)
{
    if (!game->is_active) return;

    breakout_game_update_input(game);
    breakout_game_update_logic(game);
    breakout_game_render(game);
}
