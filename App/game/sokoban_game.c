/**
 * @file sokoban_game.c
 * @brief 推箱子游戏实现
 */

#include "sokoban_game.h"

/* ======================== 关卡数据定义 ======================== */

/**
 * @brief 关卡1：教学关（2个箱子）
 * @note  简单布局，便于新手熟悉规则
 *
 * ##########
 * #        #
 * #  . .   #
 * #  $ $   #
 * #   @    #
 * #        #
 * #        #
 * ##########
 */
static const tile_type_t level1_layout[SOKOBAN_HEIGHT][SOKOBAN_WIDTH] = {
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL}
};
static const position_t level1_player = {3, 4};

/**
 * @brief 关卡2：中等难度（3个箱子）
 * @note  增加墙壁障碍，需要规划路线
 *
 * ##########
 * #   .    #
 * # .$. $  #
 * #  $ #   #
 * #  @ #   #
 * #    #   #
 * #        #
 * ##########
 */
static const tile_type_t level2_layout[SOKOBAN_HEIGHT][SOKOBAN_WIDTH] = {
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_TARGET, TILE_BOX, TILE_TARGET, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL}
};
static const position_t level2_player = {3, 4};

/**
 * @brief 关卡3：困难关卡（4个箱子）
 * @note  复杂布局，需要精心规划每一步
 *
 * ##########
 * # .  .   #
 * #  ##    #
 * # $  $ . #
 * #  ##  . #
 * # $  $   #
 * #  @     #
 * ##########
 */
static const tile_type_t level3_layout[SOKOBAN_HEIGHT][SOKOBAN_WIDTH] = {
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_WALL, TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_WALL, TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_TARGET, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_FLOOR, TILE_BOX, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_FLOOR, TILE_WALL},
    {TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL, TILE_WALL}
};
static const position_t level3_player = {3, 6};

/* ======================== 内部函数声明 ======================== */

// 关卡管理
static void load_level(sokoban_game_t *game, uint8_t level);
static uint8_t count_boxes_and_targets(sokoban_game_t *game);

// 游戏逻辑
static uint8_t can_move_to(sokoban_game_t *game, int8_t x, int8_t y);
static uint8_t can_push_box(sokoban_game_t *game, int8_t box_x, int8_t box_y, int8_t dir_x, int8_t dir_y);
static void move_player(sokoban_game_t *game, int8_t dir_x, int8_t dir_y);
static void check_level_complete(sokoban_game_t *game);

// 渲染辅助
static void render_map(sokoban_game_t *game, u8g2_t *u8g2);
static void render_ui(sokoban_game_t *game, u8g2_t *u8g2);

/* ======================== 关卡管理函数 ======================== */

/**
 * @brief 加载关卡数据
 */
static void load_level(sokoban_game_t *game, uint8_t level)
{
    const tile_type_t (*layout)[SOKOBAN_WIDTH] = NULL;
    position_t player_pos = {0, 0};

    // 选择关卡布局
    switch (level) {
        case 1:
            layout = level1_layout;
            player_pos = level1_player;
            break;
        case 2:
            layout = level2_layout;
            player_pos = level2_player;
            break;
        case 3:
            layout = level3_layout;
            player_pos = level3_player;
            break;
        default:
            layout = level1_layout;
            player_pos = level1_player;
            break;
    }

    // 复制关卡数据到游戏状态
    for (uint8_t row = 0; row < SOKOBAN_HEIGHT; row++) {
        for (uint8_t col = 0; col < SOKOBAN_WIDTH; col++) {
            game->map[row][col] = layout[row][col];
        }
    }

    // 设置玩家初始位置
    game->player = player_pos;

    // 统计箱子和目标点数量
    game->total_boxes = count_boxes_and_targets(game);
    game->boxes_on_target = 0;

    // 计算已在目标点上的箱子
    for (uint8_t row = 0; row < SOKOBAN_HEIGHT; row++) {
        for (uint8_t col = 0; col < SOKOBAN_WIDTH; col++) {
            if (game->map[row][col] == TILE_BOX_ON_TARGET) {
                game->boxes_on_target++;
            }
        }
    }

    // 重置步数
    game->steps = 0;
}

/**
 * @brief 统计箱子和目标点数量
 * @return 箱子总数（应该等于目标点数量）
 */
static uint8_t count_boxes_and_targets(sokoban_game_t *game)
{
    uint8_t box_count = 0;

    for (uint8_t row = 0; row < SOKOBAN_HEIGHT; row++) {
        for (uint8_t col = 0; col < SOKOBAN_WIDTH; col++) {
            tile_type_t tile = game->map[row][col];
            if (tile == TILE_BOX || tile == TILE_BOX_ON_TARGET) {
                box_count++;
            }
        }
    }

    return box_count;
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 检查玩家是否可以移动到指定位置
 * @return 1=可以移动，0=不能移动
 */
static uint8_t can_move_to(sokoban_game_t *game, int8_t x, int8_t y)
{
    // 超出边界
    if (x < 0 || x >= SOKOBAN_WIDTH || y < 0 || y >= SOKOBAN_HEIGHT) {
        return 0;
    }

    tile_type_t tile = game->map[y][x];

    // 墙壁不能通过
    if (tile == TILE_WALL) {
        return 0;
    }

    // 地板和目标点可以通过
    if (tile == TILE_FLOOR || tile == TILE_TARGET) {
        return 1;
    }

    // 箱子需要特殊处理（在move_player中处理）
    return 0;
}

/**
 * @brief 检查箱子是否可以被推
 * @param box_x 箱子X坐标
 * @param box_y 箱子Y坐标
 * @param dir_x 推动方向X（-1/0/1）
 * @param dir_y 推动方向Y（-1/0/1）
 * @return 1=可以推，0=不能推
 */
static uint8_t can_push_box(sokoban_game_t *game, int8_t box_x, int8_t box_y, int8_t dir_x, int8_t dir_y)
{
    // 计算箱子推动后的目标位置
    int8_t target_x = box_x + dir_x;
    int8_t target_y = box_y + dir_y;

    // 超出边界
    if (target_x < 0 || target_x >= SOKOBAN_WIDTH || target_y < 0 || target_y >= SOKOBAN_HEIGHT) {
        return 0;
    }

    tile_type_t target_tile = game->map[target_y][target_x];

    // 只有地板和目标点可以推箱子过去
    return (target_tile == TILE_FLOOR || target_tile == TILE_TARGET);
}

/**
 * @brief 移动玩家（包含推箱子逻辑）
 * @param dir_x 移动方向X（-1/0/1）
 * @param dir_y 移动方向Y（-1/0/1）
 */
static void move_player(sokoban_game_t *game, int8_t dir_x, int8_t dir_y)
{
    int8_t target_x = game->player.x + dir_x;
    int8_t target_y = game->player.y + dir_y;

    // 超出边界
    if (target_x < 0 || target_x >= SOKOBAN_WIDTH || target_y < 0 || target_y >= SOKOBAN_HEIGHT) {
        return;
    }

    tile_type_t target_tile = game->map[target_y][target_x];

    // 1. 目标是墙壁：不能移动
    if (target_tile == TILE_WALL) {
        return;
    }

    // 2. 目标是地板或目标点：直接移动
    if (target_tile == TILE_FLOOR || target_tile == TILE_TARGET) {
        game->player.x = target_x;
        game->player.y = target_y;
        game->steps++;
        return;
    }

    // 3. 目标是箱子：尝试推箱子
    if (target_tile == TILE_BOX || target_tile == TILE_BOX_ON_TARGET) {
        // 检查箱子是否可以被推
        if (!can_push_box(game, target_x, target_y, dir_x, dir_y)) {
            return;  // 箱子无法推动
        }

        // 推箱子：箱子的新位置
        int8_t box_new_x = target_x + dir_x;
        int8_t box_new_y = target_y + dir_y;
        tile_type_t box_new_tile = game->map[box_new_y][box_new_x];

        // 更新箱子的旧位置
        if (target_tile == TILE_BOX) {
            game->map[target_y][target_x] = TILE_FLOOR;  // 箱子原来在地板上
        } else {  // TILE_BOX_ON_TARGET
            game->map[target_y][target_x] = TILE_TARGET;  // 箱子原来在目标点上
            game->boxes_on_target--;  // 箱子离开目标点
        }

        // 更新箱子的新位置
        if (box_new_tile == TILE_TARGET) {
            game->map[box_new_y][box_new_x] = TILE_BOX_ON_TARGET;  // 箱子推到目标点上
            game->boxes_on_target++;
        } else {  // TILE_FLOOR
            game->map[box_new_y][box_new_x] = TILE_BOX;  // 箱子推到地板上
        }

        // 玩家移动到箱子原来的位置
        game->player.x = target_x;
        game->player.y = target_y;
        game->steps++;

        // 检查过关
        check_level_complete(game);
    }
}

/**
 * @brief 检查关卡是否完成
 */
static void check_level_complete(sokoban_game_t *game)
{
    if (game->boxes_on_target == game->total_boxes) {
        game->game_state = SOKOBAN_STATE_LEVEL_CLEAR;
        game->level_clear_start_time = HAL_GetTick();
    }
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化游戏
 */
void sokoban_game_init(sokoban_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空游戏状态
    memset(game, 0, sizeof(sokoban_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    game->game_state = SOKOBAN_STATE_READY;
    game->current_level = 1;

    // 加载关卡
    load_level(game, game->current_level);
}

/**
 * @brief 激活游戏
 */
void sokoban_game_activate(sokoban_game_t *game)
{
    game->is_active = 1;
    game->game_state = SOKOBAN_STATE_PLAYING;
}

/**
 * @brief 停用游戏
 */
void sokoban_game_deactivate(sokoban_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void sokoban_game_set_exit_callback(sokoban_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void sokoban_game_update_input(sokoban_game_t *game)
{
    if (!game->is_active) return;

    // B键：退出游戏
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();
        }
        return;
    }

    // READY状态：按任意键开始
    if (game->game_state == SOKOBAN_STATE_READY) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = SOKOBAN_STATE_PLAYING;
        }
        return;
    }

    // WIN状态：按START重新开始
    if (game->game_state == SOKOBAN_STATE_WIN) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            sokoban_game_init(game);
            sokoban_game_activate(game);
        }
        return;
    }

    // LEVEL_CLEAR状态：按A键进入下一关
    if (game->game_state == SOKOBAN_STATE_LEVEL_CLEAR) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->current_level++;
            if (game->current_level > SOKOBAN_MAX_LEVELS) {
                game->game_state = SOKOBAN_STATE_WIN;
            } else {
                load_level(game, game->current_level);
                game->game_state = SOKOBAN_STATE_PLAYING;
            }
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == SOKOBAN_STATE_PLAYING) {
            game->game_state = SOKOBAN_STATE_PAUSED;
        } else if (game->game_state == SOKOBAN_STATE_PAUSED) {
            game->game_state = SOKOBAN_STATE_PLAYING;
        }
        return;
    }

    // A键：重新开始当前关卡
    if (game->game_state == SOKOBAN_STATE_PLAYING && input_is_just_pressed(INPUT_BTN_A)) {
        load_level(game, game->current_level);
        return;
    }

    // 暂停状态不处理移动
    if (game->game_state == SOKOBAN_STATE_PAUSED) {
        return;
    }

    // 方向键移动（只在PLAYING状态）
    if (game->game_state == SOKOBAN_STATE_PLAYING) {
        if (input_is_just_pressed(INPUT_BTN_UP)) {
            move_player(game, 0, -1);
        } else if (input_is_just_pressed(INPUT_BTN_DOWN)) {
            move_player(game, 0, 1);
        } else if (input_is_just_pressed(INPUT_BTN_LEFT)) {
            move_player(game, -1, 0);
        } else if (input_is_just_pressed(INPUT_BTN_RIGHT)) {
            move_player(game, 1, 0);
        }
    }
}

/**
 * @brief 更新游戏逻辑
 */
void sokoban_game_update_logic(sokoban_game_t *game)
{
    if (!game->is_active) return;

    // 推箱子游戏主要是输入驱动，逻辑层没有太多需要更新的内容
    // 如果需要添加动画、音效等，可以在这里处理
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染地图
 */
static void render_map(sokoban_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t row = 0; row < SOKOBAN_HEIGHT; row++) {
        for (uint8_t col = 0; col < SOKOBAN_WIDTH; col++) {
            int16_t x = SOKOBAN_OFFSET_X + col * SOKOBAN_CELL_WIDTH;
            int16_t y = SOKOBAN_OFFSET_Y + row * SOKOBAN_CELL_HEIGHT;

            tile_type_t tile = game->map[row][col];

            switch (tile) {
                case TILE_WALL:
                    // 墙壁：实心方块
                    u8g2_DrawBox(u8g2, x, y, SOKOBAN_CELL_WIDTH, SOKOBAN_CELL_HEIGHT);
                    break;

                case TILE_TARGET:
                    // 目标点：小圆圈
                    u8g2_DrawCircle(u8g2, x + SOKOBAN_CELL_WIDTH/2, y + SOKOBAN_CELL_HEIGHT/2, 2, U8G2_DRAW_ALL);
                    break;

                case TILE_BOX:
                    // 箱子：空心方块
                    u8g2_DrawFrame(u8g2, x + 1, y + 1, SOKOBAN_CELL_WIDTH - 2, SOKOBAN_CELL_HEIGHT - 2);
                    break;

                case TILE_BOX_ON_TARGET:
                    // 箱子在目标点上：实心方块（表示已到位）
                    u8g2_DrawBox(u8g2, x + 1, y + 1, SOKOBAN_CELL_WIDTH - 2, SOKOBAN_CELL_HEIGHT - 2);
                    break;

                case TILE_FLOOR:
                case TILE_EMPTY:
                default:
                    // 地板和空地：不绘制
                    break;
            }

            // 绘制玩家（在玩家位置上层）
            if (game->player.x == col && game->player.y == row) {
                // 玩家：小实心圆
                u8g2_DrawDisc(u8g2, x + SOKOBAN_CELL_WIDTH/2, y + SOKOBAN_CELL_HEIGHT/2, 3, U8G2_DRAW_ALL);
            }
        }
    }
}

/**
 * @brief 渲染UI信息
 */
static void render_ui(sokoban_game_t *game, u8g2_t *u8g2)
{
    char buf[16];

    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 左上角：关卡号
    snprintf(buf, sizeof(buf), "L%u", game->current_level);
    u8g2_DrawStr(u8g2, 2, 6, buf);

    // 中间：步数
    snprintf(buf, sizeof(buf), "S:%u", game->steps);
    u8g2_DrawStr(u8g2, 40, 6, buf);

    // 右上角：箱子到位情况
    snprintf(buf, sizeof(buf), "%u/%u", game->boxes_on_target, game->total_boxes);
    u8g2_DrawStr(u8g2, 100, 6, buf);
}

/**
 * @brief 渲染游戏画面
 */
void sokoban_game_render(sokoban_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    // READY状态
    if (game->game_state == SOKOBAN_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 30, 26, "SOKOBAN");
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 20, 40, "Press A Start");
        u8g2_SendBuffer(u8g2);
        return;
    }

    // WIN状态
    if (game->game_state == SOKOBAN_STATE_WIN) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 22, 26, "YOU WIN!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Steps: %u", game->steps);
        u8g2_DrawStr(u8g2, 32, 40, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // LEVEL_CLEAR状态
    if (game->game_state == SOKOBAN_STATE_LEVEL_CLEAR) {
        render_map(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 10, 35, "LEVEL CLEAR!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 22, 50, "Press A Next");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == SOKOBAN_STATE_PAUSED) {
        render_map(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 32, 35, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PLAYING状态
    render_map(game, u8g2);
    render_ui(game, u8g2);

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void sokoban_game_task(sokoban_game_t *game)
{
    if (!game->is_active) return;

    sokoban_game_update_input(game);
    sokoban_game_update_logic(game);
    sokoban_game_render(game);
}
