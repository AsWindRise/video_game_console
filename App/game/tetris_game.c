/**
 * @file tetris_game.c
 * @brief 俄罗斯方块游戏实现
 */

#include "tetris_game.h"

/* ======================== 方块形状数据定义（4×4矩阵）======================== */

/**
 * @brief 7种方块的初始形状（旋转状态0）
 * @note 使用4×4矩阵存储，1表示有方块，0表示空
 */
static const uint8_t g_tetromino_shapes[TETROMINO_COUNT][TETROMINO_SIZE][TETROMINO_SIZE] = {
    // I型（一字长条）- 青色
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // O型（正方形）- 黄色
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },

    // T型（T字形）- 紫色
    {
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },

    // S型（S字形）- 绿色
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    },

    // Z型（Z字形）- 红色
    {
        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },

    // J型（J字形）- 蓝色
    {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },

    // L型（L字形）- 橙色
    {
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    }
};

/* ======================== 内部函数声明 ======================== */

// 方块操作
static void load_tetromino(tetromino_t *tetromino, tetromino_type_t type);
static void rotate_tetromino_cw(tetromino_t *tetromino);
static tetromino_type_t get_random_tetromino_type(void);

// 碰撞检测
static uint8_t check_collision(tetris_game_t *game, tetromino_t *tetromino, int16_t x, int16_t y);

// 方块固定和消行
static void lock_piece(tetris_game_t *game);
static uint8_t check_and_clear_lines(tetris_game_t *game);
static void apply_gravity(tetris_game_t *game);

// 游戏控制
static void spawn_new_piece(tetris_game_t *game);
static void calculate_drop_interval(tetris_game_t *game);

// 渲染辅助
static void render_grid(tetris_game_t *game, u8g2_t *u8g2);
static void render_current_piece(tetris_game_t *game, u8g2_t *u8g2);
static void render_next_piece(tetris_game_t *game, u8g2_t *u8g2);
static void render_info(tetris_game_t *game, u8g2_t *u8g2);

/* ======================== 方块操作函数 ======================== */

/**
 * @brief 加载方块形状数据
 * @param tetromino 方块指针
 * @param type 方块类型
 */
static void load_tetromino(tetromino_t *tetromino, tetromino_type_t type)
{
    tetromino->type = type;
    tetromino->rotation = 0;

    // 复制形状数据
    for (uint8_t y = 0; y < TETROMINO_SIZE; y++) {
        for (uint8_t x = 0; x < TETROMINO_SIZE; x++) {
            tetromino->shape[y][x] = g_tetromino_shapes[type][y][x];
        }
    }
}

/**
 * @brief 顺时针旋转方块90度
 * @param tetromino 方块指针
 * @note 旋转算法：新[y][x] = 旧[TETROMINO_SIZE-1-x][y]
 */
static void rotate_tetromino_cw(tetromino_t *tetromino)
{
    uint8_t temp[TETROMINO_SIZE][TETROMINO_SIZE];

    // 复制当前形状
    memcpy(temp, tetromino->shape, sizeof(temp));

    // 顺时针旋转90度
    for (uint8_t y = 0; y < TETROMINO_SIZE; y++) {
        for (uint8_t x = 0; x < TETROMINO_SIZE; x++) {
            tetromino->shape[y][x] = temp[TETROMINO_SIZE - 1 - x][y];
        }
    }

    // 更新旋转状态
    tetromino->rotation = (tetromino->rotation + 1) % 4;
}

/**
 * @brief 生成随机方块类型
 * @return 随机方块类型
 */
static tetromino_type_t get_random_tetromino_type(void)
{
    return (tetromino_type_t)rng_get_random_range(0, TETROMINO_COUNT - 1);
}

/* ======================== 碰撞检测函数 ======================== */

/**
 * @brief 检测方块是否与网格或边界碰撞
 * @param game 游戏实例指针
 * @param tetromino 方块指针
 * @param x 测试X坐标
 * @param y 测试Y坐标
 * @return 1=碰撞，0=无碰撞
 */
static uint8_t check_collision(tetris_game_t *game, tetromino_t *tetromino, int16_t x, int16_t y)
{
    for (uint8_t ty = 0; ty < TETROMINO_SIZE; ty++) {
        for (uint8_t tx = 0; tx < TETROMINO_SIZE; tx++) {
            // 跳过空格
            if (tetromino->shape[ty][tx] == 0) {
                continue;
            }

            // 计算网格坐标
            int16_t grid_x = x + tx;
            int16_t grid_y = y + ty;

            // 边界检测
            if (grid_x < 0 || grid_x >= TETRIS_GRID_WIDTH ||
                grid_y < 0 || grid_y >= TETRIS_GRID_HEIGHT) {
                return 1;  // 碰撞
            }

            // 网格碰撞检测
            if (game->grid[grid_y][grid_x] != 0) {
                return 1;  // 碰撞
            }
        }
    }

    return 0;  // 无碰撞
}

/* ======================== 方块固定和消行 ======================== */

/**
 * @brief 固定当前方块到网格
 * @param game 游戏实例指针
 */
static void lock_piece(tetris_game_t *game)
{
    for (uint8_t ty = 0; ty < TETROMINO_SIZE; ty++) {
        for (uint8_t tx = 0; tx < TETROMINO_SIZE; tx++) {
            if (game->current_piece.tetromino.shape[ty][tx] != 0) {
                int16_t grid_x = game->current_piece.x + tx;
                int16_t grid_y = game->current_piece.y + ty;

                // 确保在网格范围内
                if (grid_x >= 0 && grid_x < TETRIS_GRID_WIDTH &&
                    grid_y >= 0 && grid_y < TETRIS_GRID_HEIGHT) {
                    // 存储方块类型（1-7）
                    game->grid[grid_y][grid_x] = game->current_piece.tetromino.type + 1;
                }
            }
        }
    }
}

/**
 * @brief 检测并标记满行
 * @param game 游戏实例指针
 * @return 满行数量
 */
static uint8_t check_and_clear_lines(tetris_game_t *game)
{
    uint8_t lines_found = 0;

    // 清空标记
    memset(game->clearing_lines, 0, sizeof(game->clearing_lines));

    // 从下往上检测满行
    for (int16_t y = TETRIS_GRID_HEIGHT - 1; y >= 0; y--) {
        uint8_t full = 1;

        // 检查该行是否满
        for (uint8_t x = 0; x < TETRIS_GRID_WIDTH; x++) {
            if (game->grid[y][x] == 0) {
                full = 0;
                break;
            }
        }

        // 标记满行
        if (full) {
            game->clearing_lines[y] = 1;
            lines_found++;
        }
    }

    return lines_found;
}

/**
 * @brief 应用重力，消除满行
 * @param game 游戏实例指针
 */
static void apply_gravity(tetris_game_t *game)
{
    // 从下往上处理
    int16_t write_y = TETRIS_GRID_HEIGHT - 1;

    for (int16_t read_y = TETRIS_GRID_HEIGHT - 1; read_y >= 0; read_y--) {
        // 跳过待清除行
        if (game->clearing_lines[read_y]) {
            continue;
        }

        // 复制非满行到write_y
        if (read_y != write_y) {
            for (uint8_t x = 0; x < TETRIS_GRID_WIDTH; x++) {
                game->grid[write_y][x] = game->grid[read_y][x];
            }
        }

        write_y--;
    }

    // 清空顶部空出的行
    while (write_y >= 0) {
        for (uint8_t x = 0; x < TETRIS_GRID_WIDTH; x++) {
            game->grid[write_y][x] = 0;
        }
        write_y--;
    }

    // 清除标记
    memset(game->clearing_lines, 0, sizeof(game->clearing_lines));
}

/* ======================== 游戏控制函数 ======================== */

/**
 * @brief 生成新方块
 * @param game 游戏实例指针
 */
static void spawn_new_piece(tetris_game_t *game)
{
    // 加载下一个方块
    load_tetromino(&game->current_piece.tetromino, game->next_piece_type);

    // 设置初始位置（顶部中央）
    game->current_piece.x = (TETRIS_GRID_WIDTH - TETROMINO_SIZE) / 2 + 1;
    game->current_piece.y = 0;

    // 生成新的下一个方块
    game->next_piece_type = get_random_tetromino_type();

    // 检测游戏结束（新方块生成时就碰撞）
    if (check_collision(game, &game->current_piece.tetromino,
                        game->current_piece.x, game->current_piece.y)) {
        game->game_state = TETRIS_STATE_GAME_OVER;
    }
}

/**
 * @brief 根据等级计算下落间隔
 * @param game 游戏实例指针
 */
static void calculate_drop_interval(tetris_game_t *game)
{
    uint32_t interval = TETRIS_INITIAL_SPEED - (game->level - 1) * TETRIS_SPEED_DECREMENT;

    if (interval < TETRIS_MIN_SPEED) {
        interval = TETRIS_MIN_SPEED;
    }

    game->drop_interval = interval;
}

/* ======================== 生命周期函数 ======================== */

/**
 * @brief 初始化俄罗斯方块游戏
 */
void tetris_game_init(tetris_game_t *game)
{
    // 保存关键字段（避免被memset清零）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;

    // 清空结构体
    memset(game, 0, sizeof(tetris_game_t));

    // 恢复关键字段
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;

    // 初始化游戏状态
    game->game_state = TETRIS_STATE_READY;

    // 初始化游戏数据
    game->level = 1;
    game->score = 0;
    game->lines_cleared = 0;

    // 计算初始下落速度
    calculate_drop_interval(game);

    // 生成第一个和下一个方块
    game->next_piece_type = get_random_tetromino_type();
    spawn_new_piece(game);
}

/**
 * @brief 激活游戏
 */
void tetris_game_activate(tetris_game_t *game)
{
    game->is_active = 1;
    game->game_state = TETRIS_STATE_RUNNING;
    game->last_drop_time = HAL_GetTick();
}

/**
 * @brief 停用游戏
 */
void tetris_game_deactivate(tetris_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置退出回调
 */
void tetris_game_set_exit_callback(tetris_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

/* ======================== 输入处理函数 ======================== */

/**
 * @brief 处理用户输入
 */
void tetris_game_update_input(tetris_game_t *game)
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
    if (game->game_state == TETRIS_STATE_READY) {
        if (input_any_button_pressed() || input_any_direction_pressed()) {
            game->game_state = TETRIS_STATE_RUNNING;
            game->last_drop_time = HAL_GetTick();
        }
        return;
    }

    // GAME_OVER状态：按START重新开始
    if (game->game_state == TETRIS_STATE_GAME_OVER) {
        if (input_is_just_pressed(INPUT_BTN_START)) {
            tetris_game_init(game);
            tetris_game_activate(game);
        }
        return;
    }

    // START键：暂停/继续
    if (input_is_just_pressed(INPUT_BTN_START)) {
        if (game->game_state == TETRIS_STATE_RUNNING) {
            game->game_state = TETRIS_STATE_PAUSED;
        } else if (game->game_state == TETRIS_STATE_PAUSED) {
            game->game_state = TETRIS_STATE_RUNNING;
            game->last_drop_time = HAL_GetTick();  // 重置时间，避免暂停后瞬间下落
        }
        return;
    }

    // 暂停或消行动画期间不处理移动/旋转
    if (game->game_state != TETRIS_STATE_RUNNING || game->clearing_animation) {
        return;
    }

    uint32_t now = HAL_GetTick();

    // === DAS (Delayed Auto Shift) 系统 - 连续移动机制 ===

    // 左键处理
    if (input_is_just_pressed(INPUT_BTN_LEFT)) {
        // 刚按下：立即移动
        int16_t new_x = game->current_piece.x - 1;
        if (!check_collision(game, &game->current_piece.tetromino, new_x, game->current_piece.y)) {
            game->current_piece.x = new_x;
        }

        // 启动DAS
        game->das_left_active = 1;
        game->das_start_time = now;
        game->das_last_move_time = now;
    } else if (input_is_pressed(INPUT_BTN_LEFT) && game->das_left_active) {
        // 持续按住：DAS重复移动
        uint32_t hold_time = now - game->das_start_time;

        if (hold_time >= TETRIS_DAS_DELAY) {
            // 延迟后开始重复
            uint32_t since_last_move = now - game->das_last_move_time;

            if (since_last_move >= TETRIS_DAS_REPEAT) {
                int16_t new_x = game->current_piece.x - 1;
                if (!check_collision(game, &game->current_piece.tetromino, new_x, game->current_piece.y)) {
                    game->current_piece.x = new_x;
                }
                game->das_last_move_time = now;
            }
        }
    } else {
        // 释放：停止DAS
        game->das_left_active = 0;
    }

    // 右键处理
    if (input_is_just_pressed(INPUT_BTN_RIGHT)) {
        // 刚按下：立即移动
        int16_t new_x = game->current_piece.x + 1;
        if (!check_collision(game, &game->current_piece.tetromino, new_x, game->current_piece.y)) {
            game->current_piece.x = new_x;
        }

        // 启动DAS
        game->das_right_active = 1;
        game->das_start_time = now;
        game->das_last_move_time = now;
    } else if (input_is_pressed(INPUT_BTN_RIGHT) && game->das_right_active) {
        // 持续按住：DAS重复移动
        uint32_t hold_time = now - game->das_start_time;

        if (hold_time >= TETRIS_DAS_DELAY) {
            // 延迟后开始重复
            uint32_t since_last_move = now - game->das_last_move_time;

            if (since_last_move >= TETRIS_DAS_REPEAT) {
                int16_t new_x = game->current_piece.x + 1;
                if (!check_collision(game, &game->current_piece.tetromino, new_x, game->current_piece.y)) {
                    game->current_piece.x = new_x;
                }
                game->das_last_move_time = now;
            }
        }
    } else {
        // 释放：停止DAS
        game->das_right_active = 0;
    }

    // 下键：禁用（根据用户需求，防止误触）
    // 原软降功能已删除
    game->soft_drop_active = 0;

    // A键：顺时针旋转（带Wall Kick）
    if (input_is_just_pressed(INPUT_BTN_A)) {
        // 备份当前方块
        tetromino_t backup = game->current_piece.tetromino;

        // 旋转
        rotate_tetromino_cw(&game->current_piece.tetromino);

        // 尝试Wall Kick（依次尝试：原位 → 左移1格 → 右移1格 → 上移1格）
        int16_t kick_offsets[][2] = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}};
        uint8_t success = 0;

        for (uint8_t i = 0; i < 4; i++) {
            int16_t test_x = game->current_piece.x + kick_offsets[i][0];
            int16_t test_y = game->current_piece.y + kick_offsets[i][1];

            if (!check_collision(game, &game->current_piece.tetromino, test_x, test_y)) {
                game->current_piece.x = test_x;
                game->current_piece.y = test_y;
                success = 1;
                break;
            }
        }

        // Wall Kick失败，恢复原方块
        if (!success) {
            game->current_piece.tetromino = backup;
        }
    }
}

/* ======================== 游戏逻辑函数 ======================== */

/**
 * @brief 更新游戏逻辑
 */
void tetris_game_update_logic(tetris_game_t *game)
{
    if (!game->is_active) return;

    // 非运行状态不更新逻辑
    if (game->game_state != TETRIS_STATE_RUNNING) {
        return;
    }

    uint32_t now = HAL_GetTick();

    // 消行动画处理
    if (game->clearing_animation) {
        // 动画持续200ms
        if (now - game->clearing_start_time >= 200) {
            // 应用重力（消除满行）
            apply_gravity(game);

            // 生成新方块
            spawn_new_piece(game);

            // 结束动画
            game->clearing_animation = 0;
        }
        return;
    }

    // 方块下落逻辑
    uint32_t current_interval = game->soft_drop_active ? TETRIS_SOFT_DROP_SPEED : game->drop_interval;

    if (now - game->last_drop_time >= current_interval) {
        game->last_drop_time = now;

        // 尝试下落
        int16_t new_y = game->current_piece.y + 1;

        if (!check_collision(game, &game->current_piece.tetromino, game->current_piece.x, new_y)) {
            // 下落成功
            game->current_piece.y = new_y;

            // 软降得分
            if (game->soft_drop_active) {
                game->score += 1;
            }
        } else {
            // 下落失败，固定方块
            lock_piece(game);

            // 检测消行
            uint8_t lines = check_and_clear_lines(game);

            if (lines > 0) {
                // 启动消行动画
                game->clearing_animation = 1;
                game->clearing_start_time = now;

                // 更新统计
                game->lines_cleared += lines;

                // 计算得分（单行100，双行300，三行500，四行800）
                const uint16_t scores[] = {0, 100, 300, 500, 800};
                game->score += scores[lines];

                // 检查升级
                game->level = (game->lines_cleared / TETRIS_LINES_PER_LEVEL) + 1;
                calculate_drop_interval(game);
            } else {
                // 无消行，直接生成新方块
                spawn_new_piece(game);
            }
        }
    }
}

/* ======================== 渲染函数 ======================== */

/**
 * @brief 渲染游戏网格（已固定的方块）
 */
static void render_grid(tetris_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t y = 0; y < TETRIS_GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < TETRIS_GRID_WIDTH; x++) {
            if (game->grid[y][x] != 0) {
                // 计算像素坐标
                uint8_t px = TETRIS_GRID_OFFSET_X + x * TETRIS_CELL_SIZE;
                uint8_t py = TETRIS_GRID_OFFSET_Y + y * TETRIS_CELL_SIZE;

                // 消行动画：闪烁效果
                if (game->clearing_animation && game->clearing_lines[y]) {
                    // 每100ms切换一次显示/隐藏
                    uint32_t elapsed = HAL_GetTick() - game->clearing_start_time;
                    if ((elapsed / 100) % 2 == 0) {
                        continue;  // 隐藏
                    }
                }

                // 绘制实心方块（5×5，留1像素边距）
                u8g2_DrawBox(u8g2, px, py, TETRIS_CELL_SIZE - 1, TETRIS_CELL_SIZE - 1);
            }
        }
    }
}

/**
 * @brief 渲染当前下落方块
 */
static void render_current_piece(tetris_game_t *game, u8g2_t *u8g2)
{
    // 消行动画期间不显示当前方块
    if (game->clearing_animation) {
        return;
    }

    for (uint8_t ty = 0; ty < TETROMINO_SIZE; ty++) {
        for (uint8_t tx = 0; tx < TETROMINO_SIZE; tx++) {
            if (game->current_piece.tetromino.shape[ty][tx] != 0) {
                int16_t grid_x = game->current_piece.x + tx;
                int16_t grid_y = game->current_piece.y + ty;

                // 只绘制在网格范围内的部分
                if (grid_x >= 0 && grid_x < TETRIS_GRID_WIDTH &&
                    grid_y >= 0 && grid_y < TETRIS_GRID_HEIGHT) {

                    uint8_t px = TETRIS_GRID_OFFSET_X + grid_x * TETRIS_CELL_SIZE;
                    uint8_t py = TETRIS_GRID_OFFSET_Y + grid_y * TETRIS_CELL_SIZE;

                    // 绘制实心方块
                    u8g2_DrawBox(u8g2, px, py, TETRIS_CELL_SIZE - 1, TETRIS_CELL_SIZE - 1);
                }
            }
        }
    }
}

/**
 * @brief 渲染下一个方块预览
 */
static void render_next_piece(tetris_game_t *game, u8g2_t *u8g2)
{
    // 加载下一个方块形状
    tetromino_t next;
    load_tetromino(&next, game->next_piece_type);

    // 预览区域位置
    uint8_t preview_x = TETRIS_INFO_OFFSET_X + 8;
    uint8_t preview_y = 16;

    // 标题
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(u8g2, TETRIS_INFO_OFFSET_X + 2, 12, "NEXT");

    // 绘制下一个方块（缩小版，4像素/格）
    for (uint8_t ty = 0; ty < TETROMINO_SIZE; ty++) {
        for (uint8_t tx = 0; tx < TETROMINO_SIZE; tx++) {
            if (next.shape[ty][tx] != 0) {
                uint8_t px = preview_x + tx * 4;
                uint8_t py = preview_y + ty * 4;
                u8g2_DrawBox(u8g2, px, py, 3, 3);
            }
        }
    }
}

/**
 * @brief 渲染游戏信息（分数、等级、行数）
 */
static void render_info(tetris_game_t *game, u8g2_t *u8g2)
{
    char buf[16];

    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 分数
    u8g2_DrawStr(u8g2, TETRIS_INFO_OFFSET_X + 2, 40, "SCORE");
    snprintf(buf, sizeof(buf), "%lu", game->score);
    u8g2_DrawStr(u8g2, TETRIS_INFO_OFFSET_X + 2, 48, buf);

    // 等级
    u8g2_DrawStr(u8g2, TETRIS_INFO_OFFSET_X + 2, 56, "LEVEL");
    snprintf(buf, sizeof(buf), "%u", game->level);
    u8g2_DrawStr(u8g2, TETRIS_INFO_OFFSET_X + 2, 64, buf);
}

/**
 * @brief 渲染游戏画面
 */
void tetris_game_render(tetris_game_t *game)
{
    if (!game->is_active) return;

    u8g2_t *u8g2 = u8g2_get_instance();

    u8g2_ClearBuffer(u8g2);

    // 绘制游戏区域边框
    u8g2_DrawFrame(u8g2, TETRIS_GRID_OFFSET_X - 1, TETRIS_GRID_OFFSET_Y - 1,
                   TETRIS_GRID_WIDTH * TETRIS_CELL_SIZE + 1,
                   TETRIS_GRID_HEIGHT * TETRIS_CELL_SIZE + 1);

    // READY状态
    if (game->game_state == TETRIS_STATE_READY) {
        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 20, 32, "TETRIS");
        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        u8g2_DrawStr(u8g2, 10, 48, "Press Any Key");
        u8g2_SendBuffer(u8g2);
        return;
    }

    // GAME_OVER状态
    if (game->game_state == TETRIS_STATE_GAME_OVER) {
        render_grid(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 12, 28, "GAME OVER");

        u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
        char buf[20];
        snprintf(buf, sizeof(buf), "Score: %lu", game->score);
        u8g2_DrawStr(u8g2, 18, 40, buf);

        u8g2_DrawStr(u8g2, 8, 56, "START: Restart");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // PAUSED状态
    if (game->game_state == TETRIS_STATE_PAUSED) {
        render_grid(game, u8g2);
        render_current_piece(game, u8g2);

        u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
        u8g2_DrawStr(u8g2, 20, 32, "PAUSED");

        u8g2_SendBuffer(u8g2);
        return;
    }

    // RUNNING状态
    render_grid(game, u8g2);
    render_current_piece(game, u8g2);
    render_next_piece(game, u8g2);
    render_info(game, u8g2);

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务
 */
void tetris_game_task(tetris_game_t *game)
{
    if (!game->is_active) return;

    // 处理输入
    tetris_game_update_input(game);

    // 更新逻辑
    tetris_game_update_logic(game);

    // 渲染画面
    tetris_game_render(game);
}
