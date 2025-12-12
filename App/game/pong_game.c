/**
 * @file pong_game.c
 * @brief 乒乓球游戏实现
 */

#include "pong_game.h"

/* ======================== 内部函数声明 ======================== */

// 游戏控制
static void reset_ball(pong_game_t *game);
static void serve_ball(pong_game_t *game);
static void score_point(pong_game_t *game, uint8_t side);

// 挡板控制
static void update_player_paddle(pong_game_t *game);
static void update_ai_paddle(pong_game_t *game);

// 碰撞检测
static void check_wall_collision(pong_game_t *game);
static void check_paddle_collision(pong_game_t *game);
static void check_scoring(pong_game_t *game);

// 渲染辅助
static void render_paddles(pong_game_t *game, u8g2_t *u8g2);
static void render_ball(pong_game_t *game, u8g2_t *u8g2);
static void render_ui(pong_game_t *game, u8g2_t *u8g2);
static void render_center_line(u8g2_t *u8g2);

/* ======================== 游戏控制函数 ======================== */

/**
 * @brief 重置球到发球位置
 */
static void reset_ball(pong_game_t *game)
{
    float x = (game->serve_side == 0) ? PONG_PADDLE_OFFSET + 20.0f : PONG_SCREEN_WIDTH - PONG_PADDLE_OFFSET - 20.0f;
    float y = PONG_SCREEN_HEIGHT / 2.0f;

    ball_init(&game->ball, x, y, 0, 0, PONG_BALL_RADIUS);
}

/**
 * @brief 发球
 */
static void serve_ball(pong_game_t *game)
{
    // 根据发球方向设置球的初速度
    float vx = (game->serve_side == 0) ? PONG_BALL_SPEED : -PONG_BALL_SPEED;
    float vy = 0;  // 初始水平发球

    ball_init(&game->ball, game->ball.x, game->ball.y, vx, vy, PONG_BALL_RADIUS);
}

/**
 * @brief 得分处理
 * @param side 得分方（0=玩家，1=AI）
 */
static void score_point(pong_game_t *game, uint8_t side)
{
    if (side == 0) {
        game->player_score++;
    } else {
        game->ai_score++;
    }

    // 检查是否获胜
    if (game->player_score >= PONG_WIN_SCORE) {
        game->game_state = PONG_STATE_WIN;
        return;
    } else if (game->ai_score >= PONG_WIN_SCORE) {
        game->game_state = PONG_STATE_LOSE;
        return;
    }

    // 切换发球权
    game->serve_side = side;

    // 重置球
    reset_ball(game);
    game->game_state = PONG_STATE_SERVE;
}

/* ======================== 挡板控制函数 ======================== */

/**
 * @brief 更新玩家挡板
 */
static void update_player_paddle(pong_game_t *game)
{
    // 上下键移动
    if (input_is_pressed(INPUT_BTN_UP)) {
        game->player_y -= PONG_PADDLE_SPEED;
        if (game->player_y < PONG_PADDLE_HEIGHT / 2) {
            game->player_y = PONG_PADDLE_HEIGHT / 2;
        }
    }

    if (input_is_pressed(INPUT_BTN_DOWN)) {
        game->player_y += PONG_PADDLE_SPEED;
        if (game->player_y > PONG_SCREEN_HEIGHT - PONG_PADDLE_HEIGHT / 2) {
            game->player_y = PONG_SCREEN_HEIGHT - PONG_PADDLE_HEIGHT / 2;
        }
    }
}

/**
 * @brief 更新AI挡板（简单追踪）
 */
static void update_ai_paddle(pong_game_t *game)
{
    // AI追踪球的Y坐标
    int16_t target_y = (int16_t)game->ball.y;

    // AI反应速度（比玩家慢）
    int16_t ai_speed = PONG_PADDLE_SPEED - 1;

    if (game->ai_y < target_y - 2) {
        game->ai_y += ai_speed;
    } else if (game->ai_y > target_y + 2) {
        game->ai_y -= ai_speed;
    }

    // 限制范围
    if (game->ai_y < PONG_PADDLE_HEIGHT / 2) {
        game->ai_y = PONG_PADDLE_HEIGHT / 2;
    }
    if (game->ai_y > PONG_SCREEN_HEIGHT - PONG_PADDLE_HEIGHT / 2) {
        game->ai_y = PONG_SCREEN_HEIGHT - PONG_PADDLE_HEIGHT / 2;
    }
}

/* ======================== 碰撞检测函数 ======================== */

/**
 * @brief 检测墙壁碰撞
 */
static void check_wall_collision(pong_game_t *game)
{
    // 上下墙壁
    if (game->ball.y - game->ball.radius <= 0) {
        game->ball.y = game->ball.radius;
        ball_reflect_vertical(&game->ball);
    } else if (game->ball.y + game->ball.radius >= PONG_SCREEN_HEIGHT) {
        game->ball.y = PONG_SCREEN_HEIGHT - game->ball.radius;
        ball_reflect_vertical(&game->ball);
    }
}

/**
 * @brief 检测挡板碰撞
 */
static void check_paddle_collision(pong_game_t *game)
{
    // 玩家挡板（左侧）
    rect_t player_paddle = {
        .x = PONG_PADDLE_OFFSET,
        .y = game->player_y - PONG_PADDLE_HEIGHT / 2,
        .width = PONG_PADDLE_WIDTH,
        .height = PONG_PADDLE_HEIGHT
    };

    // 只有球向左运动时才检测玩家挡板
    if (game->ball.vx < 0 && ball_collides_with_rect(&game->ball, &player_paddle)) {
        // 计算击球点相对于挡板中心的位置（-1.0 ~ 1.0）
        float hit_pos = (game->ball.y - game->player_y) / (PONG_PADDLE_HEIGHT / 2.0f);

        // 反弹，并根据击球位置调整Y速度
        ball_reflect_horizontal(&game->ball);
        game->ball.vy = hit_pos * PONG_BALL_SPEED;

        // 微调球位置，避免陷入挡板
        game->ball.x = PONG_PADDLE_OFFSET + PONG_PADDLE_WIDTH + game->ball.radius + 1;
    }

    // AI挡板（右侧）
    rect_t ai_paddle = {
        .x = PONG_SCREEN_WIDTH - PONG_PADDLE_OFFSET - PONG_PADDLE_WIDTH,
        .y = game->ai_y - PONG_PADDLE_HEIGHT / 2,
        .width = PONG_PADDLE_WIDTH,
        .height = PONG_PADDLE_HEIGHT
    };

    // 只有球向右运动时才检测AI挡板
    if (game->ball.vx > 0 && ball_collides_with_rect(&game->ball, &ai_paddle)) {
        // 计算击球点相对于挡板中心的位置
        float hit_pos = (game->ball.y - game->ai_y) / (PONG_PADDLE_HEIGHT / 2.0f);

        // 反弹，并根据击球位置调整Y速度
        ball_reflect_horizontal(&game->ball);
        game->ball.vy = hit_pos * PONG_BALL_SPEED;

        // 微调球位置
        game->ball.x = PONG_SCREEN_WIDTH - PONG_PADDLE_OFFSET - PONG_PADDLE_WIDTH - game->ball.radius - 1;
    }
}

/**
 * @brief 检测得分（球出界）
 */
static void check_scoring(pong_game_t *game)
{
    // 球飞出左边界：AI得分
    if (game->ball.x - game->ball.radius < 0) {
        score_point(game, 1);  // AI得分
    }
    // 球飞出右边界：玩家得分
    else if (game->ball.x + game->ball.radius > PONG_SCREEN_WIDTH) {
        score_point(game, 0);  // 玩家得分
    }
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化乒乓球游戏
 */
void pong_game_init(pong_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空游戏状态
    memset(game, 0, sizeof(pong_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    // 初始化游戏状态
    game->game_state = PONG_STATE_READY;

    // 初始化挡板位置（居中）
    game->player_y = PONG_SCREEN_HEIGHT / 2;
    game->ai_y = PONG_SCREEN_HEIGHT / 2;

    // 初始化分数
    game->player_score = 0;
    game->ai_score = 0;

    // 玩家先发球
    game->serve_side = 0;

    // 初始化球
    reset_ball(game);
}

/**
 * @brief 激活游戏
 */
void pong_game_activate(pong_game_t *game)
{
    game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void pong_game_deactivate(pong_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void pong_game_set_exit_callback(pong_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void pong_game_update_input(pong_game_t *game)
{
    if (!game->is_active) return;

    // B键：退出游戏（全局可用）
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();
        }
        return;
    }

    // READY状态：按A键开始
    if (game->game_state == PONG_STATE_READY) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = PONG_STATE_SERVE;
        }
        return;
    }

    // WIN/LOSE状态：按START重新开始
    if (game->game_state == PONG_STATE_WIN || game->game_state == PONG_STATE_LOSE) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            pong_game_init(game);
            pong_game_activate(game);
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == PONG_STATE_PLAYING) {
            game->game_state = PONG_STATE_PAUSED;
        } else if (game->game_state == PONG_STATE_PAUSED) {
            game->game_state = PONG_STATE_PLAYING;
        }
        return;
    }

    // SERVE状态：按A键发球
    if (game->game_state == PONG_STATE_SERVE) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            serve_ball(game);
            game->game_state = PONG_STATE_PLAYING;
        }
    }

    // 暂停状态不处理移动
    if (game->game_state == PONG_STATE_PAUSED) {
        return;
    }

    // 更新玩家挡板（SERVE和PLAYING状态都可以移动）
    if (game->game_state == PONG_STATE_SERVE || game->game_state == PONG_STATE_PLAYING) {
        update_player_paddle(game);
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 更新游戏逻辑
 */
void pong_game_update_logic(pong_game_t *game)
{
    if (!game->is_active) return;

    // 只有PLAYING状态更新逻辑
    if (game->game_state != PONG_STATE_PLAYING) {
        return;
    }

    // 更新球位置
    ball_update(&game->ball);

    // 碰撞检测
    check_wall_collision(game);
    check_paddle_collision(game);
    check_scoring(game);

    // 更新AI挡板
    update_ai_paddle(game);
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染中线
 */
static void render_center_line(u8g2_t *u8g2)
{
    // 虚线中线
    for (int16_t y = 0; y < PONG_SCREEN_HEIGHT; y += 4) {
        u8g2_DrawVLine(u8g2, PONG_SCREEN_WIDTH / 2, y, 2);
    }
}

/**
 * @brief 渲染挡板
 */
static void render_paddles(pong_game_t *game, u8g2_t *u8g2)
{
    // 玩家挡板（左侧）
    int16_t player_y = game->player_y - PONG_PADDLE_HEIGHT / 2;
    u8g2_DrawBox(u8g2, PONG_PADDLE_OFFSET, player_y, PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT);

    // AI挡板（右侧）
    int16_t ai_y = game->ai_y - PONG_PADDLE_HEIGHT / 2;
    u8g2_DrawBox(u8g2, PONG_SCREEN_WIDTH - PONG_PADDLE_OFFSET - PONG_PADDLE_WIDTH, ai_y,
                 PONG_PADDLE_WIDTH, PONG_PADDLE_HEIGHT);
}

/**
 * @brief 渲染球
 */
static void render_ball(pong_game_t *game, u8g2_t *u8g2)
{
    u8g2_DrawDisc(u8g2, (int16_t)game->ball.x, (int16_t)game->ball.y, game->ball.radius, U8G2_DRAW_ALL);
}

/**
 * @brief 渲染UI信息
 */
static void render_ui(pong_game_t *game, u8g2_t *u8g2)
{
    char buf[8];

    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);

    // 玩家分数（左上角）
    snprintf(buf, sizeof(buf), "%u", game->player_score);
    u8g2_DrawStr(u8g2, 30, 12, buf);

    // AI分数（右上角）
    snprintf(buf, sizeof(buf), "%u", game->ai_score);
    u8g2_DrawStr(u8g2, 94, 12, buf);
}

/**
 * @brief 渲染游戏画面
 */
void pong_game_render(pong_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    // READY状态
    if (game->game_state == PONG_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 44, 26, "PONG");
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 20, 40, "Press A Start");
        u8g2_SendBuffer(u8g2);
        return;
    }

    // WIN状态
    if (game->game_state == PONG_STATE_WIN) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 22, 26, "YOU WIN!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %u - %u", game->player_score, game->ai_score);
        u8g2_DrawStr(u8g2, 24, 40, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // LOSE状态
    if (game->game_state == PONG_STATE_LOSE) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 22, 26, "YOU LOSE");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %u - %u", game->player_score, game->ai_score);
        u8g2_DrawStr(u8g2, 24, 40, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == PONG_STATE_PAUSED) {
        render_center_line(u8g2);
        render_paddles(game, u8g2);
        render_ball(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 32, 35, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // SERVE/PLAYING状态
    render_center_line(u8g2);
    render_paddles(game, u8g2);
    render_ball(game, u8g2);
    render_ui(game, u8g2);

    // SERVE状态提示
    if (game->game_state == PONG_STATE_SERVE) {
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 34, 58, "Press A");
    }

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void pong_game_task(pong_game_t *game)
{
    if (!game->is_active) return;

    pong_game_update_input(game);
    pong_game_update_logic(game);
    pong_game_render(game);
}
