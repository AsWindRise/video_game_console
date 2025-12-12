/**
 * @file pacman_game.c
 * @brief 吃豆人游戏实现
 */

#include "pacman_game.h"

/* ======================== 迷宫地图定义 ======================== */

/**
 * @brief 经典迷宫布局（16×8）
 * @note  1=墙壁，0=通道（初始放置豆子），2=能量豆位置
 */
static const uint8_t g_maze_layout[PACMAN_GRID_HEIGHT][PACMAN_GRID_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  // 顶部墙壁
    {1, 2, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 2, 1},  // 能量豆在角落
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},  // 内部墙壁
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},  // 中间通道
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},  // 中间通道
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},  // 内部墙壁
    {1, 2, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 2, 1},  // 能量豆在角落
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}   // 底部墙壁
};

// 吃豆人初始位置（左下角通道）
static const pacman_position_t g_pacman_start = {3, 6};

// 幽灵初始位置（右侧通道）
static const pacman_position_t g_ghost_starts[PACMAN_MAX_GHOSTS] = {
    {12, 3},  // 幽灵1：右侧通道
    {13, 3}   // 幽灵2：右侧通道
};

/* ======================== 内部函数声明 ======================== */

// 地图初始化
static void load_maze(pacman_game_t *game);
static uint8_t count_dots(pacman_game_t *game);

// 移动辅助
static uint8_t can_move_to(pacman_game_t *game, int8_t x, int8_t y);
static void try_move_pacman(pacman_game_t *game);
static void try_move_ghost(pacman_game_t *game, pacman_ghost_t *ghost);

// 游戏逻辑
static void collect_item(pacman_game_t *game, int8_t x, int8_t y);
static void check_ghost_collision(pacman_game_t *game);
static void check_win(pacman_game_t *game);
static void lose_life(pacman_game_t *game);

// 渲染辅助
static void render_maze(pacman_game_t *game, u8g2_t *u8g2);
static void render_pacman(pacman_game_t *game, u8g2_t *u8g2);
static void render_ghosts(pacman_game_t *game, u8g2_t *u8g2);
static void render_ui(pacman_game_t *game, u8g2_t *u8g2);

/* ======================== 地图初始化函数 ======================== */

/**
 * @brief 加载迷宫地图
 */
static void load_maze(pacman_game_t *game)
{
    for (uint8_t y = 0; y < PACMAN_GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < PACMAN_GRID_WIDTH; x++) {
            uint8_t layout = g_maze_layout[y][x];

            if (layout == 1) {
                game->map[y][x] = PACMAN_TILE_WALL;
            } else if (layout == 2) {
                game->map[y][x] = PACMAN_TILE_POWER;  // 能量豆
            } else {
                game->map[y][x] = PACMAN_TILE_DOT;    // 小豆子
            }
        }
    }

    // 统计豆子总数
    game->total_dots = count_dots(game);
    game->dots_remaining = game->total_dots;
}

/**
 * @brief 统计豆子总数
 */
static uint8_t count_dots(pacman_game_t *game)
{
    uint8_t count = 0;

    for (uint8_t y = 0; y < PACMAN_GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < PACMAN_GRID_WIDTH; x++) {
            if (game->map[y][x] == PACMAN_TILE_DOT || game->map[y][x] == PACMAN_TILE_POWER) {
                count++;
            }
        }
    }

    return count;
}

/* ======================== 移动辅助函数 ======================== */

/**
 * @brief 检查位置是否可以移动
 */
static uint8_t can_move_to(pacman_game_t *game, int8_t x, int8_t y)
{
    // 边界检查
    if (x < 0 || x >= PACMAN_GRID_WIDTH || y < 0 || y >= PACMAN_GRID_HEIGHT) {
        return 0;
    }

    // 墙壁检测
    return (game->map[y][x] != PACMAN_TILE_WALL);
}

/**
 * @brief 尝试移动吃豆人
 */
static void try_move_pacman(pacman_game_t *game)
{
    // 计算目标位置
    int8_t new_x = game->pacman_pos.x;
    int8_t new_y = game->pacman_pos.y;

    // 优先尝试缓冲方向（转弯预输入）
    pacman_direction_t try_dir = (game->pacman_next_dir != PACMAN_DIR_NONE) ? game->pacman_next_dir : game->pacman_dir;

    // 根据方向计算新位置
    switch (try_dir) {
        case PACMAN_DIR_UP:    new_y--; break;
        case PACMAN_DIR_DOWN:  new_y++; break;
        case PACMAN_DIR_LEFT:  new_x--; break;
        case PACMAN_DIR_RIGHT: new_x++; break;
        default: return;
    }

    // 检查是否可以移动
    if (can_move_to(game, new_x, new_y)) {
        game->pacman_pos.x = new_x;
        game->pacman_pos.y = new_y;

        // 如果成功使用了缓冲方向，更新实际方向
        if (try_dir == game->pacman_next_dir) {
            game->pacman_dir = game->pacman_next_dir;
            game->pacman_next_dir = PACMAN_DIR_NONE;
        }

        // 收集物品
        collect_item(game, new_x, new_y);

        // 更新动画帧
        game->pacman_anim_frame = (game->pacman_anim_frame + 1) % 2;
    }
}

/**
 * @brief 尝试移动幽灵（简单追踪AI）
 */
static void try_move_ghost(pacman_game_t *game, pacman_ghost_t *ghost)
{
    // 惊吓模式：随机移动
    if (ghost->is_frightened) {
        pacman_direction_t dirs[] = {PACMAN_DIR_UP, PACMAN_DIR_DOWN, PACMAN_DIR_LEFT, PACMAN_DIR_RIGHT};
        pacman_direction_t try_dir = dirs[rng_get_random_range(0, 3)];

        int8_t new_x = ghost->pos.x;
        int8_t new_y = ghost->pos.y;

        switch (try_dir) {
            case PACMAN_DIR_UP:    new_y--; break;
            case PACMAN_DIR_DOWN:  new_y++; break;
            case PACMAN_DIR_LEFT:  new_x--; break;
            case PACMAN_DIR_RIGHT: new_x++; break;
            default: break;
        }

        if (can_move_to(game, new_x, new_y)) {
            ghost->pos.x = new_x;
            ghost->pos.y = new_y;
            ghost->dir = try_dir;
        }
        return;
    }

    // 正常模式：简单追踪（朝吃豆人方向移动）
    int8_t dx = game->pacman_pos.x - ghost->pos.x;
    int8_t dy = game->pacman_pos.y - ghost->pos.y;

    pacman_direction_t prefer_dir = PACMAN_DIR_NONE;

    // 优先朝距离更远的轴移动（使用三元运算符避免abs函数）
    int8_t abs_dx = (dx < 0) ? -dx : dx;
    int8_t abs_dy = (dy < 0) ? -dy : dy;

    if (abs_dx > abs_dy) {
        prefer_dir = (dx > 0) ? PACMAN_DIR_RIGHT : PACMAN_DIR_LEFT;
    } else {
        prefer_dir = (dy > 0) ? PACMAN_DIR_DOWN : PACMAN_DIR_UP;
    }

    // 尝试首选方向
    int8_t new_x = ghost->pos.x;
    int8_t new_y = ghost->pos.y;

    switch (prefer_dir) {
        case PACMAN_DIR_UP:    new_y--; break;
        case PACMAN_DIR_DOWN:  new_y++; break;
        case PACMAN_DIR_LEFT:  new_x--; break;
        case PACMAN_DIR_RIGHT: new_x++; break;
        default: break;
    }

    if (can_move_to(game, new_x, new_y)) {
        ghost->pos.x = new_x;
        ghost->pos.y = new_y;
        ghost->dir = prefer_dir;
    } else {
        // 首选方向失败，尝试其他方向
        pacman_direction_t alt_dirs[] = {PACMAN_DIR_UP, PACMAN_DIR_DOWN, PACMAN_DIR_LEFT, PACMAN_DIR_RIGHT};
        for (uint8_t i = 0; i < 4; i++) {
            if (alt_dirs[i] == prefer_dir) continue;

            new_x = ghost->pos.x;
            new_y = ghost->pos.y;

            switch (alt_dirs[i]) {
                case PACMAN_DIR_UP:    new_y--; break;
                case PACMAN_DIR_DOWN:  new_y++; break;
                case PACMAN_DIR_LEFT:  new_x--; break;
                case PACMAN_DIR_RIGHT: new_x++; break;
                default: break;
            }

            if (can_move_to(game, new_x, new_y)) {
                ghost->pos.x = new_x;
                ghost->pos.y = new_y;
                ghost->dir = alt_dirs[i];
                break;
            }
        }
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 收集物品
 */
static void collect_item(pacman_game_t *game, int8_t x, int8_t y)
{
    pacman_tile_type_t tile = game->map[y][x];

    if (tile == PACMAN_TILE_DOT) {
        // 吃掉小豆子
        game->map[y][x] = PACMAN_TILE_EMPTY;
        game->score += PACMAN_SCORE_DOT;
        game->dots_remaining--;
        check_win(game);
    } else if (tile == PACMAN_TILE_POWER) {
        // 吃掉能量豆
        game->map[y][x] = PACMAN_TILE_EMPTY;
        game->score += PACMAN_SCORE_POWER;
        game->dots_remaining--;

        // 激活能量豆效果
        game->power_active = 1;
        game->power_start_time = HAL_GetTick();

        // 所有幽灵进入惊吓模式
        for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
            game->ghosts[i].is_frightened = 1;
        }

        check_win(game);
    }
}

/**
 * @brief 检查与幽灵的碰撞
 */
static void check_ghost_collision(pacman_game_t *game)
{
    for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        pacman_ghost_t *ghost = &game->ghosts[i];

        // 检查是否在同一位置
        if (ghost->pos.x == game->pacman_pos.x && ghost->pos.y == game->pacman_pos.y) {
            if (ghost->is_frightened) {
                // 吃掉幽灵
                game->score += PACMAN_SCORE_GHOST;

                // 重置幽灵位置
                ghost->pos = g_ghost_starts[i];
                ghost->is_frightened = 0;
            } else {
                // 被幽灵吃掉
                lose_life(game);
                return;
            }
        }
    }
}

/**
 * @brief 检查是否胜利
 */
static void check_win(pacman_game_t *game)
{
    if (game->dots_remaining == 0) {
        game->game_state = PACMAN_STATE_WIN;
    }
}

/**
 * @brief 失去一条命
 */
static void lose_life(pacman_game_t *game)
{
    game->lives--;

    if (game->lives == 0) {
        game->game_state = PACMAN_STATE_LOSE;
    } else {
        // 重置位置
        game->pacman_pos = g_pacman_start;
        game->pacman_dir = PACMAN_DIR_NONE;
        game->pacman_next_dir = PACMAN_DIR_NONE;

        for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
            game->ghosts[i].pos = g_ghost_starts[i];
            game->ghosts[i].dir = PACMAN_DIR_NONE;
            game->ghosts[i].is_frightened = 0;
        }

        // 清除能量豆效果
        game->power_active = 0;
    }
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化吃豆人游戏
 */
void pacman_game_init(pacman_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空游戏状态
    memset(game, 0, sizeof(pacman_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    // 初始化游戏状态
    game->game_state = PACMAN_STATE_READY;
    game->lives = PACMAN_MAX_LIVES;
    game->score = 0;

    // 加载迷宫
    load_maze(game);

    // 初始化吃豆人
    game->pacman_pos = g_pacman_start;
    game->pacman_dir = PACMAN_DIR_NONE;
    game->pacman_next_dir = PACMAN_DIR_NONE;
    game->pacman_anim_frame = 0;

    // 初始化幽灵
    for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        game->ghosts[i].pos = g_ghost_starts[i];
        game->ghosts[i].dir = PACMAN_DIR_NONE;
        game->ghosts[i].is_frightened = 0;
        game->ghosts[i].last_move_time = 0;
    }

    // 清除能量豆效果
    game->power_active = 0;
}

/**
 * @brief 激活游戏
 */
void pacman_game_activate(pacman_game_t *game)
{
    game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void pacman_game_deactivate(pacman_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void pacman_game_set_exit_callback(pacman_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void pacman_game_update_input(pacman_game_t *game)
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
    if (game->game_state == PACMAN_STATE_READY) {
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = PACMAN_STATE_PLAYING;
            game->pacman_last_move_time = HAL_GetTick();
        }
        return;
    }

    // WIN/LOSE状态：按START重新开始
    if (game->game_state == PACMAN_STATE_WIN || game->game_state == PACMAN_STATE_LOSE) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            pacman_game_init(game);
            pacman_game_activate(game);
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == PACMAN_STATE_PLAYING) {
            game->game_state = PACMAN_STATE_PAUSED;
        } else if (game->game_state == PACMAN_STATE_PAUSED) {
            game->game_state = PACMAN_STATE_PLAYING;
            game->pacman_last_move_time = HAL_GetTick();
        }
        return;
    }

    // 暂停状态不处理移动
    if (game->game_state == PACMAN_STATE_PAUSED) {
        return;
    }

    // PLAYING状态：方向键控制（缓冲输入）
    if (game->game_state == PACMAN_STATE_PLAYING) {
        if (input_is_pressed(INPUT_BTN_UP)) {
            game->pacman_next_dir = PACMAN_DIR_UP;
        } else if (input_is_pressed(INPUT_BTN_DOWN)) {
            game->pacman_next_dir = PACMAN_DIR_DOWN;
        } else if (input_is_pressed(INPUT_BTN_LEFT)) {
            game->pacman_next_dir = PACMAN_DIR_LEFT;
        } else if (input_is_pressed(INPUT_BTN_RIGHT)) {
            game->pacman_next_dir = PACMAN_DIR_RIGHT;
        }
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 更新游戏逻辑
 */
void pacman_game_update_logic(pacman_game_t *game)
{
    if (!game->is_active) return;

    // 只有PLAYING状态更新逻辑
    if (game->game_state != PACMAN_STATE_PLAYING) {
        return;
    }

    uint32_t now = HAL_GetTick();

    // 更新吃豆人移动
    if (now - game->pacman_last_move_time >= PACMAN_SPEED) {
        game->pacman_last_move_time = now;
        try_move_pacman(game);
    }

    // 更新幽灵移动（幽灵比吃豆人慢）
    for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        pacman_ghost_t *ghost = &game->ghosts[i];

        if (now - ghost->last_move_time >= PACMAN_GHOST_SPEED) {
            ghost->last_move_time = now;
            try_move_ghost(game, ghost);
        }
    }

    // 检查碰撞
    check_ghost_collision(game);

    // 更新能量豆效果
    if (game->power_active) {
        if (now - game->power_start_time >= PACMAN_POWER_DURATION) {
            // 能量豆效果结束
            game->power_active = 0;

            for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
                game->ghosts[i].is_frightened = 0;
            }
        }
    }
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染迷宫
 */
static void render_maze(pacman_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t y = 0; y < PACMAN_GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < PACMAN_GRID_WIDTH; x++) {
            int16_t px = x * PACMAN_CELL_SIZE;
            int16_t py = y * PACMAN_CELL_SIZE;

            pacman_tile_type_t tile = game->map[y][x];

            switch (tile) {
                case PACMAN_TILE_WALL:
                    // 墙壁：实心方块
                    u8g2_DrawBox(u8g2, px, py, PACMAN_CELL_SIZE, PACMAN_CELL_SIZE);
                    break;

                case PACMAN_TILE_DOT:
                    // 小豆子：小点
                    u8g2_DrawPixel(u8g2, px + PACMAN_CELL_SIZE / 2, py + PACMAN_CELL_SIZE / 2);
                    break;

                case PACMAN_TILE_POWER:
                    // 能量豆：大圆点
                    u8g2_DrawDisc(u8g2, px + PACMAN_CELL_SIZE / 2, py + PACMAN_CELL_SIZE / 2, 2, U8G2_DRAW_ALL);
                    break;

                case PACMAN_TILE_EMPTY:
                default:
                    // 空地：不绘制
                    break;
            }
        }
    }
}

/**
 * @brief 渲染吃豆人
 */
static void render_pacman(pacman_game_t *game, u8g2_t *u8g2)
{
    int16_t px = game->pacman_pos.x * PACMAN_CELL_SIZE + PACMAN_CELL_SIZE / 2;
    int16_t py = game->pacman_pos.y * PACMAN_CELL_SIZE + PACMAN_CELL_SIZE / 2;

    // 吃豆人：圆形，带张嘴闭嘴动画
    if (game->pacman_anim_frame == 0) {
        // 闭嘴：完整圆形（半径3）
        u8g2_DrawDisc(u8g2, px, py, 3, U8G2_DRAW_ALL);
    } else {
        // 张嘴：画一个缺口的圆（用弧形或多个点模拟）
        // 简化版：画圆但中间挖空一个三角形区域表示嘴巴
        u8g2_DrawDisc(u8g2, px, py, 3, U8G2_DRAW_ALL);

        // 根据方向绘制嘴巴缺口（清除部分像素）
        u8g2_SetDrawColor(u8g2, 0);  // 黑色（擦除）
        switch (game->pacman_dir) {
            case PACMAN_DIR_RIGHT:
                u8g2_DrawTriangle(u8g2, px, py, px + 3, py - 2, px + 3, py + 2);
                break;
            case PACMAN_DIR_LEFT:
                u8g2_DrawTriangle(u8g2, px, py, px - 3, py - 2, px - 3, py + 2);
                break;
            case PACMAN_DIR_UP:
                u8g2_DrawTriangle(u8g2, px, py, px - 2, py - 3, px + 2, py - 3);
                break;
            case PACMAN_DIR_DOWN:
                u8g2_DrawTriangle(u8g2, px, py, px - 2, py + 3, px + 2, py + 3);
                break;
            default:
                u8g2_DrawTriangle(u8g2, px, py, px + 3, py - 2, px + 3, py + 2);
                break;
        }
        u8g2_SetDrawColor(u8g2, 1);  // 恢复白色
    }
}

/**
 * @brief 渲染幽灵
 */
static void render_ghosts(pacman_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < PACMAN_MAX_GHOSTS; i++) {
        pacman_ghost_t *ghost = &game->ghosts[i];

        int16_t px = ghost->pos.x * PACMAN_CELL_SIZE + PACMAN_CELL_SIZE / 2;
        int16_t py = ghost->pos.y * PACMAN_CELL_SIZE + PACMAN_CELL_SIZE / 2;

        if (ghost->is_frightened) {
            // 惊吓模式：空心方块
            u8g2_DrawFrame(u8g2, px - 3, py - 3, 6, 6);
        } else {
            // 正常模式：小三角形
            u8g2_DrawTriangle(u8g2,
                              px, py - 3,
                              px - 3, py + 3,
                              px + 3, py + 3);
        }
    }
}

/**
 * @brief 渲染UI信息
 */
static void render_ui(pacman_game_t *game, u8g2_t *u8g2)
{
    // 由于屏幕太小，UI信息覆盖在游戏区域上方
    // 这里暂时不渲染UI，保持游戏区域完整
    // 可以在暂停/结束状态显示分数和生命
}

/**
 * @brief 渲染游戏画面
 */
void pacman_game_render(pacman_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    // READY状态
    if (game->game_state == PACMAN_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 28, 26, "PAC-MAN");
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 20, 40, "Press A Start");
        u8g2_SendBuffer(u8g2);
        return;
    }

    // WIN状态
    if (game->game_state == PACMAN_STATE_WIN) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 22, 20, "YOU WIN!");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %u", game->score);
        u8g2_DrawStr(u8g2, 28, 34, buf);

        snprintf(buf, sizeof(buf), "Lives: %u", game->lives);
        u8g2_DrawStr(u8g2, 32, 46, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // LOSE状态
    if (game->game_state == PACMAN_STATE_LOSE) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 16, 20, "GAME OVER");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %u", game->score);
        u8g2_DrawStr(u8g2, 28, 34, buf);

        u8g2_DrawStr(u8g2, 10, 58, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == PACMAN_STATE_PAUSED) {
        render_maze(game, u8g2);
        render_pacman(game, u8g2);
        render_ghosts(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 32, 35, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PLAYING状态
    render_maze(game, u8g2);
    render_pacman(game, u8g2);
    render_ghosts(game, u8g2);

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void pacman_game_task(pacman_game_t *game)
{
    if (!game->is_active) return;

    pacman_game_update_input(game);
    pacman_game_update_logic(game);
    pacman_game_render(game);
}
