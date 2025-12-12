/**
 * @file minesweeper_game.c
 * @brief 扫雷游戏实现
 */

#include "minesweeper_game.h"

/* ======================== 内部函数声明 ======================== */

// 游戏初始化
static void reset_game(minesweeper_game_t *game);
static void generate_mines(minesweeper_game_t *game, int8_t safe_x, int8_t safe_y);
static void calculate_neighbor_mines(minesweeper_game_t *game);

// 游戏逻辑
static void reveal_cell(minesweeper_game_t *game, int8_t x, int8_t y);
static void reveal_cell_recursive(minesweeper_game_t *game, int8_t x, int8_t y);
static void toggle_flag(minesweeper_game_t *game, int8_t x, int8_t y);
static void check_win(minesweeper_game_t *game);

// 辅助函数
static uint8_t is_valid_pos(int8_t x, int8_t y);
static uint8_t count_neighbor_mines(minesweeper_game_t *game, int8_t x, int8_t y);

// 渲染辅助
static void render_grid(minesweeper_game_t *game, u8g2_t *u8g2);
static void render_ui(minesweeper_game_t *game, u8g2_t *u8g2);
static void render_cursor(minesweeper_game_t *game, u8g2_t *u8g2);

/* ======================== 游戏初始化函数 ======================== */

/**
 * @brief 重置游戏状态
 */
static void reset_game(minesweeper_game_t *game)
{
    // 清空地图
    memset(game->cells, 0, sizeof(game->cells));

    // 重置光标到中心
    game->cursor_x = MINE_GRID_WIDTH / 2;
    game->cursor_y = MINE_GRID_HEIGHT / 2;

    // 重置游戏数据
    game->flags_placed = 0;
    game->cells_revealed = 0;
    game->first_click = 1;
    game->game_time = 0;
    game->game_start_time = 0;

    // 设置地雷总数（根据难度）
    switch (game->difficulty) {
        case MINE_DIFFICULTY_EASY:
            game->mines_total = MINE_EASY_MINES;
            break;
        case MINE_DIFFICULTY_MEDIUM:
            game->mines_total = MINE_MEDIUM_MINES;
            break;
        case MINE_DIFFICULTY_HARD:
            game->mines_total = MINE_HARD_MINES;
            break;
        default:
            game->mines_total = MINE_EASY_MINES;
            break;
    }
}

/**
 * @brief 生成地雷（确保安全区域无地雷）
 * @param safe_x 安全区域中心X
 * @param safe_y 安全区域中心Y
 */
static void generate_mines(minesweeper_game_t *game, int8_t safe_x, int8_t safe_y)
{
    uint8_t mines_placed = 0;

    // 随机放置地雷
    while (mines_placed < game->mines_total) {
        // 生成随机位置
        int8_t x = rng_get_random_range(0, MINE_GRID_WIDTH - 1);
        int8_t y = rng_get_random_range(0, MINE_GRID_HEIGHT - 1);

        // 检查是否在安全区域内（首次点击位置及周围8格）
        int8_t dx = x - safe_x;
        int8_t dy = y - safe_y;
        if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1) {
            continue;  // 在安全区域，跳过
        }

        // 检查该位置是否已有地雷
        if (game->cells[y][x].has_mine) {
            continue;  // 已有地雷，跳过
        }

        // 放置地雷
        game->cells[y][x].has_mine = 1;
        mines_placed++;
    }

    // 计算所有格子的邻居地雷数
    calculate_neighbor_mines(game);
}

/**
 * @brief 计算所有格子的邻居地雷数
 */
static void calculate_neighbor_mines(minesweeper_game_t *game)
{
    for (int8_t y = 0; y < MINE_GRID_HEIGHT; y++) {
        for (int8_t x = 0; x < MINE_GRID_WIDTH; x++) {
            if (!game->cells[y][x].has_mine) {
                game->cells[y][x].neighbor_mines = count_neighbor_mines(game, x, y);
            }
        }
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 翻开格子
 */
static void reveal_cell(minesweeper_game_t *game, int8_t x, int8_t y)
{
    // 边界检查
    if (!is_valid_pos(x, y)) {
        return;
    }

    mine_cell_t *cell = &game->cells[y][x];

    // 已翻开或已标记的格子不能翻开
    if (cell->is_revealed || cell->is_flagged) {
        return;
    }

    // 首次点击：生成地雷
    if (game->first_click) {
        game->first_click = 0;
        generate_mines(game, x, y);
        game->game_start_time = HAL_GetTick();
    }

    // 翻开格子
    cell->is_revealed = 1;
    game->cells_revealed++;

    // 踩到地雷：游戏结束
    if (cell->has_mine) {
        game->game_state = MINE_STATE_LOSE;
        return;
    }

    // 如果是空白格子（neighbor_mines == 0），递归展开
    if (cell->neighbor_mines == 0) {
        reveal_cell_recursive(game, x, y);
    }

    // 检查是否胜利
    check_win(game);
}

/**
 * @brief 递归展开空白格子
 */
static void reveal_cell_recursive(minesweeper_game_t *game, int8_t x, int8_t y)
{
    // 8个方向
    const int8_t dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int8_t dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (uint8_t i = 0; i < 8; i++) {
        int8_t nx = x + dx[i];
        int8_t ny = y + dy[i];

        // 边界检查
        if (!is_valid_pos(nx, ny)) {
            continue;
        }

        mine_cell_t *neighbor = &game->cells[ny][nx];

        // 跳过已翻开、已标记或有地雷的格子
        if (neighbor->is_revealed || neighbor->is_flagged || neighbor->has_mine) {
            continue;
        }

        // 翻开邻居格子
        neighbor->is_revealed = 1;
        game->cells_revealed++;

        // 如果邻居也是空白格子，继续递归
        if (neighbor->neighbor_mines == 0) {
            reveal_cell_recursive(game, nx, ny);
        }
    }
}

/**
 * @brief 标记/取消标记旗帜
 */
static void toggle_flag(minesweeper_game_t *game, int8_t x, int8_t y)
{
    // 边界检查
    if (!is_valid_pos(x, y)) {
        return;
    }

    mine_cell_t *cell = &game->cells[y][x];

    // 已翻开的格子不能标记
    if (cell->is_revealed) {
        return;
    }

    // 切换标记状态
    if (cell->is_flagged) {
        cell->is_flagged = 0;
        game->flags_placed--;
    } else {
        // 限制旗帜数量不超过地雷总数
        if (game->flags_placed < game->mines_total) {
            cell->is_flagged = 1;
            game->flags_placed++;
        }
    }
}

/**
 * @brief 检查是否胜利
 */
static void check_win(minesweeper_game_t *game)
{
    // 胜利条件：所有非地雷格子都已翻开
    uint8_t total_cells = MINE_MAX_CELLS;
    uint8_t target_revealed = total_cells - game->mines_total;

    if (game->cells_revealed >= target_revealed) {
        game->game_state = MINE_STATE_WIN;
    }
}

/* ======================== 辅助函数 ======================== */

/**
 * @brief 检查位置是否有效
 */
static uint8_t is_valid_pos(int8_t x, int8_t y)
{
    return (x >= 0 && x < MINE_GRID_WIDTH && y >= 0 && y < MINE_GRID_HEIGHT);
}

/**
 * @brief 统计指定格子周围的地雷数量
 */
static uint8_t count_neighbor_mines(minesweeper_game_t *game, int8_t x, int8_t y)
{
    uint8_t count = 0;

    // 8个方向
    const int8_t dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int8_t dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    for (uint8_t i = 0; i < 8; i++) {
        int8_t nx = x + dx[i];
        int8_t ny = y + dy[i];

        if (is_valid_pos(nx, ny) && game->cells[ny][nx].has_mine) {
            count++;
        }
    }

    return count;
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化扫雷游戏
 */
void minesweeper_game_init(minesweeper_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空游戏状态
    memset(game, 0, sizeof(minesweeper_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    // 初始化游戏状态
    game->game_state = MINE_STATE_READY;
    game->difficulty = MINE_DIFFICULTY_EASY;  // 默认简单难度

    // 重置游戏
    reset_game(game);
}

/**
 * @brief 激活游戏
 */
void minesweeper_game_activate(minesweeper_game_t *game)
{
    game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void minesweeper_game_deactivate(minesweeper_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void minesweeper_game_set_exit_callback(minesweeper_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void minesweeper_game_update_input(minesweeper_game_t *game)
{
    if (!game->is_active) return;

    // B键：退出游戏（全局可用）
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();
        }
        return;
    }

    // READY状态：选择难度
    if (game->game_state == MINE_STATE_READY) {
        // 左右键：切换难度
        if (input_is_just_pressed(INPUT_BTN_LEFT)) {
            if (game->difficulty > MINE_DIFFICULTY_EASY) {
                game->difficulty--;
                reset_game(game);
            }
        } else if (input_is_just_pressed(INPUT_BTN_RIGHT)) {
            if (game->difficulty < MINE_DIFFICULTY_HARD) {
                game->difficulty++;
                reset_game(game);
            }
        }

        // A键：开始游戏
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = MINE_STATE_PLAYING;
        }
        return;
    }

    // WIN/LOSE状态：按START重新开始
    if (game->game_state == MINE_STATE_WIN || game->game_state == MINE_STATE_LOSE) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            minesweeper_game_init(game);
            minesweeper_game_activate(game);
            game->game_state = MINE_STATE_READY;
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == MINE_STATE_PLAYING) {
            game->game_state = MINE_STATE_PAUSED;
        } else if (game->game_state == MINE_STATE_PAUSED) {
            game->game_state = MINE_STATE_PLAYING;
        }
        return;
    }

    // 暂停状态不处理移动
    if (game->game_state == MINE_STATE_PAUSED) {
        return;
    }

    // PLAYING状态：方向键移动光标
    if (game->game_state == MINE_STATE_PLAYING) {
        if (input_is_just_pressed(INPUT_BTN_UP)) {
            if (game->cursor_y > 0) {
                game->cursor_y--;
            }
        } else if (input_is_just_pressed(INPUT_BTN_DOWN)) {
            if (game->cursor_y < MINE_GRID_HEIGHT - 1) {
                game->cursor_y++;
            }
        } else if (input_is_just_pressed(INPUT_BTN_LEFT)) {
            if (game->cursor_x > 0) {
                game->cursor_x--;
            }
        } else if (input_is_just_pressed(INPUT_BTN_RIGHT)) {
            if (game->cursor_x < MINE_GRID_WIDTH - 1) {
                game->cursor_x++;
            }
        }

        // A键：翻开格子
        if (input_is_just_pressed(INPUT_BTN_A)) {
            reveal_cell(game, game->cursor_x, game->cursor_y);
        }

        // Y键：标记/取消标记旗帜
        if (input_is_just_pressed(INPUT_BTN_Y)) {
            toggle_flag(game, game->cursor_x, game->cursor_y);
        }
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 更新游戏逻辑
 */
void minesweeper_game_update_logic(minesweeper_game_t *game)
{
    if (!game->is_active) return;

    // 只有PLAYING状态更新时间
    if (game->game_state == MINE_STATE_PLAYING && game->game_start_time > 0) {
        uint32_t elapsed = HAL_GetTick() - game->game_start_time;
        game->game_time = elapsed / 1000;  // 转换为秒
    }
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染网格
 */
static void render_grid(minesweeper_game_t *game, u8g2_t *u8g2)
{
    for (int8_t y = 0; y < MINE_GRID_HEIGHT; y++) {
        for (int8_t x = 0; x < MINE_GRID_WIDTH; x++) {
            mine_cell_t *cell = &game->cells[y][x];

            int16_t px = MINE_OFFSET_X + x * MINE_CELL_WIDTH;
            int16_t py = MINE_OFFSET_Y + y * MINE_CELL_HEIGHT;

            // 未翻开的格子
            if (!cell->is_revealed) {
                // 绘制空心方块
                u8g2_DrawFrame(u8g2, px, py, MINE_CELL_WIDTH - 1, MINE_CELL_HEIGHT - 1);

                // 如果有旗帜，绘制旗帜标记（小三角）
                if (cell->is_flagged) {
                    u8g2_DrawTriangle(u8g2,
                                      px + 2, py + MINE_CELL_HEIGHT - 3,
                                      px + MINE_CELL_WIDTH - 3, py + 2,
                                      px + MINE_CELL_WIDTH - 3, py + MINE_CELL_HEIGHT - 3);
                }
            }
            // 已翻开的格子
            else {
                // 显示地雷（失败状态）
                if (cell->has_mine) {
                    u8g2_DrawBox(u8g2, px + 2, py + 2, MINE_CELL_WIDTH - 4, MINE_CELL_HEIGHT - 4);
                }
                // 显示数字
                else if (cell->neighbor_mines > 0) {
                    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
                    char buf[2];
                    buf[0] = '0' + cell->neighbor_mines;
                    buf[1] = '\0';
                    u8g2_DrawStr(u8g2, px + 3, py + 6, buf);
                }
                // 空白格子（neighbor_mines == 0）：什么都不画
            }
        }
    }
}

/**
 * @brief 渲染光标
 */
static void render_cursor(minesweeper_game_t *game, u8g2_t *u8g2)
{
    // 只在PLAYING状态显示光标
    if (game->game_state != MINE_STATE_PLAYING) {
        return;
    }

    int16_t px = MINE_OFFSET_X + game->cursor_x * MINE_CELL_WIDTH;
    int16_t py = MINE_OFFSET_Y + game->cursor_y * MINE_CELL_HEIGHT;

    // 绘制高亮方框（反色）
    u8g2_SetDrawColor(u8g2, 2);  // XOR模式
    u8g2_DrawBox(u8g2, px, py, MINE_CELL_WIDTH - 1, MINE_CELL_HEIGHT - 1);
    u8g2_SetDrawColor(u8g2, 1);  // 恢复正常模式
}

/**
 * @brief 渲染UI信息
 */
static void render_ui(minesweeper_game_t *game, u8g2_t *u8g2)
{
    char buf[16];

    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 左上角：剩余地雷数（总数 - 已标记旗帜数）
    uint8_t remaining_mines = game->mines_total - game->flags_placed;
    snprintf(buf, sizeof(buf), "M:%u", remaining_mines);
    u8g2_DrawStr(u8g2, 2, 6, buf);

    // 右上角：时间
    if (game->game_time < 999) {
        snprintf(buf, sizeof(buf), "T:%lu", game->game_time);
    } else {
        snprintf(buf, sizeof(buf), "T:999");
    }
    u8g2_DrawStr(u8g2, 100, 6, buf);
}

/**
 * @brief 渲染游戏画面
 */
void minesweeper_game_render(minesweeper_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    // READY状态：难度选择
    if (game->game_state == MINE_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 18, 20, "MINESWEEPER");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 20, 34, "Select Difficulty:");

        // 显示难度选项
        const char *difficulty_names[] = {"Easy", "Medium", "Hard"};
        u8g2_DrawStr(u8g2, 35, 46, difficulty_names[game->difficulty]);

        u8g2_DrawStr(u8g2, 20, 58, "Press A to Start");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // WIN状态
    if (game->game_state == MINE_STATE_WIN) {
        render_grid(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 30, 35, "YOU WIN!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // LOSE状态
    if (game->game_state == MINE_STATE_LOSE) {
        // 显示所有地雷
        render_grid(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 22, 35, "GAME OVER");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == MINE_STATE_PAUSED) {
        render_grid(game, u8g2);
        render_ui(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 35, 35, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PLAYING状态
    render_grid(game, u8g2);
    render_cursor(game, u8g2);
    render_ui(game, u8g2);

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void minesweeper_game_task(minesweeper_game_t *game)
{
    if (!game->is_active) return;

    minesweeper_game_update_input(game);
    minesweeper_game_update_logic(game);
    minesweeper_game_render(game);
}
