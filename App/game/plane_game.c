#include "plane_game.h"

// =============================================================================
// 打飞机游戏实现 - 横向卷轴弹幕射击
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 内部辅助函数声明
// -----------------------------------------------------------------------------

// 游戏逻辑
static void update_player_movement(plane_game_t *game);
static void player_shoot(plane_game_t *game);
static void player_take_damage(plane_game_t *game);

// 子弹系统
static void spawn_player_bullet(plane_game_t *game, int16_t x, int16_t y, float vx, float vy);
static void spawn_enemy_bullet(plane_game_t *game, int16_t x, int16_t y, float vx, float vy);
static void update_player_bullets(plane_game_t *game);
static void update_enemy_bullets(plane_game_t *game);

// 敌机系统
static void spawn_enemy(plane_game_t *game);
static void update_enemies(plane_game_t *game);
static void enemy_shoot(plane_game_t *game, enemy_t *enemy);

// Boss系统
static void spawn_boss(plane_game_t *game);
static void update_boss(plane_game_t *game);
static void boss_attack(plane_game_t *game);

// 道具系统
static void spawn_powerup(plane_game_t *game, int16_t x, int16_t y, powerup_type_t type);
static void update_powerups(plane_game_t *game);
static void apply_powerup(plane_game_t *game, powerup_type_t type);

// 爆炸系统
static void spawn_explosion(plane_game_t *game, int16_t x, int16_t y, explosion_type_t type);
static void update_explosions(plane_game_t *game);

// 碰撞检测
static void check_bullet_enemy_collision(plane_game_t *game);
static void check_bullet_boss_collision(plane_game_t *game);
static void check_enemy_bullet_player_collision(plane_game_t *game);
static void check_enemy_player_collision(plane_game_t *game);
static void check_powerup_player_collision(plane_game_t *game);
static uint8_t aabb_collision(int16_t x1, int16_t y1, uint8_t w1, uint8_t h1,
                               int16_t x2, int16_t y2, uint8_t w2, uint8_t h2);

// 渲染函数
static void draw_player(plane_game_t *game, u8g2_t *u8g2);
static void draw_bullets(plane_game_t *game, u8g2_t *u8g2);
static void draw_enemies(plane_game_t *game, u8g2_t *u8g2);
static void draw_boss(plane_game_t *game, u8g2_t *u8g2);
static void draw_powerups(plane_game_t *game, u8g2_t *u8g2);
static void draw_explosions(plane_game_t *game, u8g2_t *u8g2);
static void draw_ui(plane_game_t *game, u8g2_t *u8g2);
static void draw_game_over(plane_game_t *game, u8g2_t *u8g2);
static void draw_bitmap(u8g2_t *u8g2, int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h);

// -----------------------------------------------------------------------------
// 2. Sprite数据定义
// -----------------------------------------------------------------------------

/**
 * @brief 玩家飞机sprite（8x8，机头朝右）
 */
static const uint8_t sprite_player[] = {
    0x80, // #.......
    0xC0, // ##......
    0xF0, // ####....
    0xFF, // ########
    0xFF, // ########
    0xF0, // ####....
    0xC0, // ##......
    0x80, // #.......
};

/**
 * @brief 玩家子弹sprite（4x2，横向）
 */
static const uint8_t sprite_player_bullet[] = {
    0xF0, // ####
    0xF0, // ####
};

/**
 * @brief 小型敌机sprite（7x6，战斗机，机头朝左）
 * 设计：小型战斗机，带机翼
 */
static const uint8_t sprite_enemy_small[] = {
    0x38, // ..###...
    0x7C, // .#####..
    0xFE, // #######.
    0xFE, // #######.
    0x7C, // .#####..
    0x38, // ..###...
};

/**
 * @brief 中型敌机sprite（9x8，轰炸机，机头朝左）
 * 设计：中型轰炸机，带宽机翼和引擎
 */
static const uint8_t sprite_enemy_medium[] = {
    0x1C, 0x00, // ...###..  ........
    0x3E, 0x00, // ..#####.  ........
    0x7F, 0x00, // .#######  ........
    0xFF, 0x80, // ########  #.......
    0xFF, 0x80, // ########  #.......
    0x7F, 0x00, // .#######  ........
    0x3E, 0x00, // ..#####.  ........
    0x1C, 0x00, // ...###..  ........
};

/**
 * @brief 重型敌机sprite（11x10，装甲机，机头朝左）
 * 设计：重型装甲机，厚实机身，双引擎
 */
static const uint8_t sprite_enemy_heavy[] = {
    0x0F, 0x00, // ....####  ........
    0x1F, 0x80, // ...#####  #.......
    0x3F, 0xC0, // ..######  ##......
    0x7F, 0xE0, // .#######  ###.....
    0xFF, 0xF0, // ########  ####....
    0xFF, 0xF0, // ########  ####....
    0x7F, 0xE0, // .#######  ###.....
    0x3F, 0xC0, // ..######  ##......
    0x1F, 0x80, // ...#####  #.......
    0x0F, 0x00, // ....####  ........
};

/**
 * @brief 快速敌机sprite（6x5，箭型机，机头朝左）
 * 设计：超快速箭型机，尖锐机头
 */
static const uint8_t sprite_enemy_fast[] = {
    0x60, // .##.....
    0xF0, // ####....
    0xF8, // #####...
    0xF0, // ####....
    0x60, // .##.....
};

/**
 * @brief P道具sprite（8x8，武器升级）
 * 设计：方框内大写"P"
 */
static const uint8_t sprite_powerup_weapon[] = {
    0xFF, // ########
    0x81, // #......#
    0xBD, // #.####.#
    0xBD, // #.####.#
    0xBD, // #.####.#
    0xB1, // #.##...#
    0xB1, // #.##...#
    0xFF, // ########
};

/**
 * @brief S道具sprite（8x8，护盾）
 * 设计：方框内大写"S"
 */
static const uint8_t sprite_powerup_shield[] = {
    0xFF, // ########
    0x81, // #......#
    0xBD, // #.####.#
    0xB1, // #.##...#
    0x87, // #....###
    0xBD, // #.####.#
    0x81, // #......#
    0xFF, // ########
};

/**
 * @brief B道具sprite（8x8，炸弹）
 * 设计：方框内大写"B"
 */
static const uint8_t sprite_powerup_bomb[] = {
    0xFF, // ########
    0x81, // #......#
    0xBD, // #.####.#
    0xBD, // #.####.#
    0xB9, // #.###..#
    0xBD, // #.####.#
    0xBD, // #.####.#
    0xFF, // ########
};

/**
 * @brief 爆炸动画帧1（8x8，初始爆炸）
 * 设计：完整的爆炸圆形
 */
static const uint8_t sprite_explosion_frame1[] = {
    0x18, // ...##...
    0x3C, // ..####..
    0x7E, // .######.
    0xFF, // ########
    0xFF, // ########
    0x7E, // .######.
    0x3C, // ..####..
    0x18, // ...##...
};

/**
 * @brief 爆炸动画帧2（8x8，扩散）
 * 设计：爆炸扩散，出现空隙
 */
static const uint8_t sprite_explosion_frame2[] = {
    0x00, // ........
    0x24, // ..#..#..
    0x5A, // .#.##.#.
    0xBD, // #.####.#
    0xBD, // #.####.#
    0x5A, // .#.##.#.
    0x24, // ..#..#..
    0x00, // ........
};

/**
 * @brief 爆炸动画帧3（8x8，消散）
 * 设计：稀疏的碎片
 */
static const uint8_t sprite_explosion_frame3[] = {
    0x00, // ........
    0x00, // ........
    0x42, // .#....#.
    0x24, // ..#..#..
    0x24, // ..#..#..
    0x42, // .#....#.
    0x00, // ........
    0x00, // ........
};

/**
 * @brief Boss战舰sprite（16x16，超大型）
 * 设计：重型战舰，多层装甲，双引擎
 */
static const uint8_t sprite_boss[] = {
    0x03, 0xC0, // ......##  ##......
    0x07, 0xE0, // .....###  ###.....
    0x0F, 0xF0, // ....####  ####....
    0x1F, 0xF8, // ...#####  #####...
    0x3F, 0xFC, // ..######  ######..
    0x7F, 0xFE, // .#######  #######.
    0xFF, 0xFF, // ########  ########
    0xFF, 0xFF, // ########  ########
    0xFF, 0xFF, // ########  ########
    0xFF, 0xFF, // ########  ########
    0x7F, 0xFE, // .#######  #######.
    0x3F, 0xFC, // ..######  ######..
    0x1F, 0xF8, // ...#####  #####...
    0x0F, 0xF0, // ....####  ####....
    0x07, 0xE0, // .....###  ###.....
    0x03, 0xC0, // ......##  ##......
};

// -----------------------------------------------------------------------------
// 3. API函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化打飞机游戏
 */
void plane_game_init(plane_game_t *game)
{
    // 保存重要的回调和状态（不能被清空）
    void (*saved_exit_callback)(void) = game->exit_callback;
    uint8_t saved_is_active = game->is_active;
    uint32_t saved_high_score = game->high_score;

    // 清空整个游戏状态
    memset(game, 0, sizeof(plane_game_t));

    // 恢复保存的数据
    game->exit_callback = saved_exit_callback;
    game->is_active = saved_is_active;
    game->high_score = saved_high_score;

    // 设置初始状态
    game->game_state = PLANE_STATE_READY;
    game->player_x = PLANE_PLAYER_X;
    game->player_y = (PLANE_PLAYER_Y_MIN + PLANE_PLAYER_Y_MAX) / 2;  // 居中
    game->player_hp = PLANE_PLAYER_INITIAL_HP;
    game->player_shield = 0;
    game->weapon_level = 1;  // 初始武器等级1

    // 初始化分数
    game->score = 0;
    game->last_boss_score = 0;

    // 初始化难度系统
    game->difficulty_level = 0;  // 初始难度等级0
    game->boss_count = 0;        // 未击败任何Boss

    // 初始化时间戳
    uint32_t now = HAL_GetTick();
    game->last_frame_time = now;
    game->last_shoot_time = now;
    game->last_enemy_spawn_time = now;
    game->next_enemy_delay = (uint16_t)rng_get_random_range(
        PLANE_ENEMY_SPAWN_MIN, PLANE_ENEMY_SPAWN_MAX);

    // 清空所有对象池
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        game->player_bullets[i].active = 0;
    }
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        game->enemies[i].active = 0;
    }
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        game->enemy_bullets[i].active = 0;
    }
    for (uint8_t i = 0; i < MAX_POWERUPS; i++) {
        game->powerups[i].active = 0;
    }
    for (uint8_t i = 0; i < MAX_EXPLOSIONS; i++) {
        game->explosions[i].active = 0;
    }

    // Boss初始化
    game->boss.active = 0;
}

/**
 * @brief 处理玩家输入
 */
void plane_game_update_input(plane_game_t *game)
{
    // 退出游戏（B键）
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();
        }
        return;
    }

    if (game->game_state == PLANE_STATE_READY) {
        // 按A键开始游戏
        if (input_is_just_pressed(INPUT_BTN_A)) {
            game->game_state = PLANE_STATE_RUNNING;
            game->score = 0;
            game->last_enemy_spawn_time = HAL_GetTick();
        }
    }
    else if (game->game_state == PLANE_STATE_RUNNING) {
        // 玩家移动（上下）
        update_player_movement(game);

        // 玩家射击（A键）
        if (input_is_pressed(INPUT_BTN_A)) {
            player_shoot(game);
        }
    }
    else if (game->game_state == PLANE_STATE_GAME_OVER) {
        // 重新开始（按A键）
        if (input_is_just_pressed(INPUT_BTN_A)) {
            plane_game_init(game);
            game->game_state = PLANE_STATE_RUNNING;
            game->last_enemy_spawn_time = HAL_GetTick();
        }
    }
}

/**
 * @brief 更新游戏逻辑
 */
void plane_game_update_logic(plane_game_t *game)
{
    if (game->game_state != PLANE_STATE_RUNNING) {
        return;  // 只在运行状态更新逻辑
    }

    uint32_t now = HAL_GetTick();

    // 1. 更新子弹
    update_player_bullets(game);
    update_enemy_bullets(game);

    // 2. 更新敌机
    update_enemies(game);

    // 3. 更新Boss
    if (game->boss.active) {
        update_boss(game);
    }

    // 4. 更新道具
    update_powerups(game);

    // 5. 更新爆炸动画
    update_explosions(game);

    // 6. 生成新敌机（根据时间间隔）
    if (now - game->last_enemy_spawn_time >= game->next_enemy_delay) {
        // 检查是否应该生成Boss
        if (game->score > game->last_boss_score &&
            (game->score - game->last_boss_score) >= PLANE_BOSS_SCORE_TRIGGER) {
            if (!game->boss.active) {
                spawn_boss(game);
                game->last_boss_score = game->score;
            }
        } else {
            spawn_enemy(game);
        }
    }

    // 7. 碰撞检测
    check_bullet_enemy_collision(game);
    check_bullet_boss_collision(game);
    check_enemy_bullet_player_collision(game);
    check_enemy_player_collision(game);
    check_powerup_player_collision(game);

    // 8. 检查游戏结束
    if (game->player_hp <= 0) {
        game->game_state = PLANE_STATE_GAME_OVER;

        // 更新最高分
        if (game->score > game->high_score) {
            game->high_score = game->score;
        }
    }
}

/**
 * @brief 渲染游戏画面
 */
void plane_game_render(plane_game_t *game)
{
    u8g2_t *u8g2 = u8g2_get_instance();
    u8g2_ClearBuffer(u8g2);

    if (game->game_state == PLANE_STATE_READY) {
        // 显示"Press A to Start"
        u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(u8g2, 10, 32, "Press A to Start");
    }
    else if (game->game_state == PLANE_STATE_RUNNING) {
        // 1. 绘制玩家
        draw_player(game, u8g2);

        // 2. 绘制子弹
        draw_bullets(game, u8g2);

        // 3. 绘制敌机
        draw_enemies(game, u8g2);

        // 4. 绘制Boss
        if (game->boss.active) {
            draw_boss(game, u8g2);
        }

        // 5. 绘制道具
        draw_powerups(game, u8g2);

        // 6. 绘制爆炸
        draw_explosions(game, u8g2);

        // 7. 绘制UI
        draw_ui(game, u8g2);

        // 8. 绘制Boss警告（如果激活）
        if (game->boss_warning) {
            uint32_t now = HAL_GetTick();
            uint32_t elapsed = now - game->boss_warning_start_time;

            // 警告显示2秒后自动关闭
            if (elapsed < 2000) {
                // 闪烁效果：每300ms切换一次显示状态
                if ((elapsed / 300) % 2 == 0) {
                    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
                    u8g2_DrawStr(u8g2, 12, 32, "WARNING!");
                    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
                    u8g2_DrawStr(u8g2, 8, 44, "BOSS INCOMING");
                }
            } else {
                game->boss_warning = 0;  // 关闭警告
            }
        }
    }
    else if (game->game_state == PLANE_STATE_GAME_OVER) {
        // 绘制完整场景
        draw_player(game, u8g2);
        draw_bullets(game, u8g2);
        draw_enemies(game, u8g2);
        draw_powerups(game, u8g2);
        draw_explosions(game, u8g2);
        draw_ui(game, u8g2);

        // 显示"Game Over"
        draw_game_over(game, u8g2);
    }

    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务（周期调用，带帧率控制）
 */
void plane_game_task(plane_game_t *game)
{
    // 检查活跃状态
    if (!game->is_active) {
        return;
    }

    uint32_t now = HAL_GetTick();

    // 帧率控制：30fps（33ms一帧）
    if (now - game->last_frame_time < PLANE_FRAME_TIME_MS) {
        // 输入仍然需要快速响应（10ms周期）
        plane_game_update_input(game);
        return;  // 未到渲染时间，直接返回
    }

    // 更新帧时间
    game->last_frame_time = now;

    // 完整游戏循环（30fps）
    plane_game_update_input(game);
    plane_game_update_logic(game);
    plane_game_render(game);
}

/**
 * @brief 激活游戏
 */
void plane_game_activate(plane_game_t *game)
{
    game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void plane_game_deactivate(plane_game_t *game)
{
    game->is_active = 0;
}

/**
 * @brief 设置游戏退出回调
 */
void plane_game_set_exit_callback(plane_game_t *game, void (*callback)(void))
{
    game->exit_callback = callback;
}

// -----------------------------------------------------------------------------
// 3. 内部辅助函数实现（占位符，后续阶段实现）
// -----------------------------------------------------------------------------

static void update_player_movement(plane_game_t *game)
{
    // 摇杆上下控制（横向游戏，玩家只能上下移动）
    if (input_is_pressed(INPUT_BTN_UP)) {
        game->player_y -= (int16_t)PLANE_PLAYER_SPEED;
    }
    if (input_is_pressed(INPUT_BTN_DOWN)) {
        game->player_y += (int16_t)PLANE_PLAYER_SPEED;
    }

    // 边界限制（防止飞出屏幕）
    if (game->player_y < PLANE_PLAYER_Y_MIN) {
        game->player_y = PLANE_PLAYER_Y_MIN;
    }
    if (game->player_y > PLANE_PLAYER_Y_MAX) {
        game->player_y = PLANE_PLAYER_Y_MAX;
    }
}

static void player_shoot(plane_game_t *game)
{
    uint32_t now = HAL_GetTick();

    // 射击冷却检测
    if (now - game->last_shoot_time < PLANE_PLAYER_SHOOT_INTERVAL) {
        return;  // 未到射击间隔
    }

    game->last_shoot_time = now;

    // 根据武器等级发射不同数量的子弹
    if (game->weapon_level == 1) {
        // Lv1：单发（居中）
        int16_t bullet_x = game->player_x + PLANE_PLAYER_WIDTH;
        int16_t bullet_y = game->player_y + PLANE_PLAYER_HEIGHT / 2 - 1;
        spawn_player_bullet(game, bullet_x, bullet_y, PLANE_PLAYER_BULLET_SPEED, 0);
    }
    else if (game->weapon_level == 2) {
        // Lv2：双发（上下）
        int16_t bullet_x = game->player_x + PLANE_PLAYER_WIDTH;
        spawn_player_bullet(game, bullet_x, game->player_y, PLANE_PLAYER_BULLET_SPEED, 0);
        spawn_player_bullet(game, bullet_x, game->player_y + PLANE_PLAYER_HEIGHT - 2,
                            PLANE_PLAYER_BULLET_SPEED, 0);
    }
    else if (game->weapon_level >= 3) {
        // Lv3：三发（上中下）
        int16_t bullet_x = game->player_x + PLANE_PLAYER_WIDTH;
        spawn_player_bullet(game, bullet_x, game->player_y, PLANE_PLAYER_BULLET_SPEED, 0);
        spawn_player_bullet(game, bullet_x, game->player_y + PLANE_PLAYER_HEIGHT / 2 - 1,
                            PLANE_PLAYER_BULLET_SPEED, 0);
        spawn_player_bullet(game, bullet_x, game->player_y + PLANE_PLAYER_HEIGHT - 2,
                            PLANE_PLAYER_BULLET_SPEED, 0);
    }
}

static void player_take_damage(plane_game_t *game)
{
    // 检查护盾
    if (game->player_shield) {
        // 护盾抵消一次伤害
        game->player_shield = 0;
        // TODO: 阶段6添加护盾破碎动画
        return;
    }

    // 扣血
    if (game->player_hp > 0) {
        game->player_hp--;
    }

    // TODO: 阶段6添加受击动画/闪烁效果

    // 检查游戏结束
    // 游戏结束判定在update_logic中统一处理
}

static void spawn_player_bullet(plane_game_t *game, int16_t x, int16_t y, float vx, float vy)
{
    // 查找空闲子弹槽
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!game->player_bullets[i].active) {
            game->player_bullets[i].active = 1;
            game->player_bullets[i].x = x;
            game->player_bullets[i].y = y;
            game->player_bullets[i].vx = vx;
            game->player_bullets[i].vy = vy;
            game->player_bullets[i].damage = 1;
            return;
        }
    }
    // 如果没有空闲槽位，子弹不生成（对象池已满）
}

static void spawn_enemy_bullet(plane_game_t *game, int16_t x, int16_t y, float vx, float vy)
{
    // 查找空闲子弹槽
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!game->enemy_bullets[i].active) {
            game->enemy_bullets[i].active = 1;
            game->enemy_bullets[i].x = x;
            game->enemy_bullets[i].y = y;
            game->enemy_bullets[i].vx = vx;
            game->enemy_bullets[i].vy = vy;
            game->enemy_bullets[i].damage = 1;
            return;
        }
    }
    // 如果没有空闲槽位，子弹不生成
}

static void update_player_bullets(plane_game_t *game)
{
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!game->player_bullets[i].active) continue;

        bullet_t *bullet = &game->player_bullets[i];

        // 更新位置
        bullet->x += (int16_t)bullet->vx;
        bullet->y += (int16_t)bullet->vy;

        // 超出屏幕右侧销毁
        if (bullet->x > PLANE_SCREEN_WIDTH) {
            bullet->active = 0;
        }
    }
}

static void update_enemy_bullets(plane_game_t *game)
{
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!game->enemy_bullets[i].active) continue;

        bullet_t *bullet = &game->enemy_bullets[i];

        // 更新位置
        bullet->x += (int16_t)bullet->vx;
        bullet->y += (int16_t)bullet->vy;

        // 超出屏幕左侧销毁
        if (bullet->x < -5) {
            bullet->active = 0;
        }
    }
}

static void spawn_enemy(plane_game_t *game)
{
    // 查找空闲敌机槽
    uint8_t slot = 0xFF;
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == 0xFF) return;  // 对象池已满

    enemy_t *enemy = &game->enemies[slot];

    // 根据分数阶段和难度等级选择敌机类型
    enemy_type_t type;
    uint8_t rand = (uint8_t)(rng_get_random_range(0, 100));

    // 根据难度等级调整敌机类型概率（难度越高，重型/快速敌机越多）
    uint8_t difficulty_bonus = game->difficulty_level * 5;  // 每级难度+5%重型/快速概率

    if (game->score < 100) {
        // 前期：小型和中型为主
        uint8_t threshold = 80 - difficulty_bonus;  // 难度提升后中型敌机更多
        type = (rand < threshold) ? ENEMY_TYPE_SMALL : ENEMY_TYPE_MEDIUM;
    }
    else if (game->score < 300) {
        // 中期：引入重型和快速
        uint8_t heavy_threshold = 70 + difficulty_bonus;  // 难度提升后重型更多
        if (rand < 40)               type = ENEMY_TYPE_SMALL;
        else if (rand < 70)          type = ENEMY_TYPE_MEDIUM;
        else if (rand < heavy_threshold) type = ENEMY_TYPE_HEAVY;
        else                         type = ENEMY_TYPE_FAST;
    }
    else {
        // 后期：全类型混合（难度越高，危险敌机越多）
        uint8_t heavy_threshold = 55 + difficulty_bonus;
        uint8_t fast_threshold = 80 + (difficulty_bonus / 2);
        if (rand < 30)                  type = ENEMY_TYPE_SMALL;
        else if (rand < 55)             type = ENEMY_TYPE_MEDIUM;
        else if (rand < heavy_threshold) type = ENEMY_TYPE_HEAVY;
        else if (rand < fast_threshold)  type = ENEMY_TYPE_FAST;
        else                            type = ENEMY_TYPE_HEAVY;  // 超高难度：更多重型
    }

    // 根据类型设置敌机属性
    enemy->active = 1;
    enemy->type = type;
    enemy->x = PLANE_SCREEN_WIDTH;  // 从屏幕右侧生成
    enemy->spawn_time = HAL_GetTick();
    enemy->last_shoot_time = HAL_GetTick();

    // 计算难度速度倍率（每级难度+8%速度，最高+80%）
    float speed_multiplier = 1.0f + (game->difficulty_level * 0.08f);
    if (speed_multiplier > 1.8f) {
        speed_multiplier = 1.8f;  // 限制最高1.8倍速度
    }

    // 根据类型设置不同参数
    switch (type) {
        case ENEMY_TYPE_SMALL:
            enemy->hp = enemy->max_hp = 1;
            enemy->vx = -2.0f * speed_multiplier;  // 向左移动，根据难度加速
            enemy->vy = 0;
            enemy->width = 7;
            enemy->height = 6;
            enemy->y = (int16_t)rng_get_random_range(PLANE_PLAYER_Y_MIN, PLANE_PLAYER_Y_MAX - 6);
            break;

        case ENEMY_TYPE_MEDIUM:
            enemy->hp = enemy->max_hp = 2;
            enemy->vx = -1.5f * speed_multiplier;
            enemy->vy = 0;
            enemy->width = 9;
            enemy->height = 8;
            enemy->y = (int16_t)rng_get_random_range(PLANE_PLAYER_Y_MIN, PLANE_PLAYER_Y_MAX - 8);
            break;

        case ENEMY_TYPE_HEAVY:
            enemy->hp = enemy->max_hp = 3;
            enemy->vx = -1.0f * speed_multiplier;
            enemy->vy = 0;
            enemy->width = 11;
            enemy->height = 10;
            enemy->y = (int16_t)rng_get_random_range(PLANE_PLAYER_Y_MIN, PLANE_PLAYER_Y_MAX - 10);
            break;

        case ENEMY_TYPE_FAST:
            enemy->hp = enemy->max_hp = 1;
            enemy->vx = -3.5f * speed_multiplier;  // 超快速，难度加成更明显
            enemy->vy = 0;  // 波浪移动在update_enemies中实现
            enemy->width = 6;
            enemy->height = 5;
            enemy->y = (int16_t)rng_get_random_range(PLANE_PLAYER_Y_MIN, PLANE_PLAYER_Y_MAX - 5);
            break;

        default:
            break;
    }

    // 更新生成时间和下次延迟（根据难度缩短间隔）
    game->last_enemy_spawn_time = HAL_GetTick();

    // 计算难度调整后的生成间隔（难度越高，生成越快）
    uint16_t min_delay = PLANE_ENEMY_SPAWN_MIN - (game->difficulty_level * 40);
    uint16_t max_delay = PLANE_ENEMY_SPAWN_MAX - (game->difficulty_level * 70);

    // 限制最小值，避免过于疯狂
    if (min_delay < 400) min_delay = 400;  // 最快400ms
    if (max_delay < 600) max_delay = 600;  // 最快600ms
    if (max_delay < min_delay + 100) max_delay = min_delay + 100;  // 保证至少100ms差值

    game->next_enemy_delay = (uint16_t)rng_get_random_range(min_delay, max_delay);
}

static void update_enemies(plane_game_t *game)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) continue;

        enemy_t *enemy = &game->enemies[i];

        // 基础横向移动（向左）
        enemy->x += (int16_t)enemy->vx;

        // FAST类型做波浪移动
        if (enemy->type == ENEMY_TYPE_FAST) {
            uint32_t time_offset = HAL_GetTick() - enemy->spawn_time;
            float wave = sinf((float)time_offset * 0.01f) * 1.5f;
            enemy->y += (int16_t)wave;

            // 限制Y边界
            if (enemy->y < PLANE_PLAYER_Y_MIN) enemy->y = PLANE_PLAYER_Y_MIN;
            if (enemy->y > PLANE_PLAYER_Y_MAX - enemy->height) {
                enemy->y = PLANE_PLAYER_Y_MAX - enemy->height;
            }
        }

        // 超出屏幕左侧销毁
        if (enemy->x < -enemy->width) {
            enemy->active = 0;
            continue;
        }

        // 随机射击
        enemy_shoot(game, enemy);
    }
}

static void enemy_shoot(plane_game_t *game, enemy_t *enemy)
{
    uint32_t now = HAL_GetTick();

    // 射击冷却检测（至少500ms间隔）
    if (now - enemy->last_shoot_time < 500) {
        return;
    }

    // 根据类型随机射击（概率不同）
    uint8_t shoot_chance = 0;
    switch (enemy->type) {
        case ENEMY_TYPE_SMALL:  shoot_chance = 3;  break;  // 3% per frame
        case ENEMY_TYPE_MEDIUM: shoot_chance = 5;  break;  // 5%
        case ENEMY_TYPE_HEAVY:  shoot_chance = 8;  break;  // 8% 频繁开火
        case ENEMY_TYPE_FAST:   shoot_chance = 1;  break;  // 1% 太快降低射击
        default: break;
    }

    uint8_t rand = (uint8_t)(rng_get_random_range(0, 100));
    if (rand >= shoot_chance) {
        return;  // 未触发射击
    }

    // 发射子弹（从敌机中心向左）
    int16_t bullet_x = enemy->x;
    int16_t bullet_y = enemy->y + enemy->height / 2;
    spawn_enemy_bullet(game, bullet_x, bullet_y, PLANE_ENEMY_BULLET_SPEED, 0);

    enemy->last_shoot_time = now;
}

static void spawn_boss(plane_game_t *game)
{
    // 清空所有普通敌机（Boss战开始）
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) {
            spawn_explosion(game, game->enemies[i].x, game->enemies[i].y, EXPLOSION_SMALL);
            game->enemies[i].active = 0;
        }
    }

    // 初始化Boss
    game->boss.active = 1;
    game->boss.x = PLANE_SCREEN_WIDTH + 10;  // 从屏幕右侧外生成
    game->boss.y = (PLANE_PLAYER_Y_MIN + PLANE_PLAYER_Y_MAX) / 2 - 8;  // 居中（16x16）
    game->boss.vy = 0;

    // 启动Boss警告（显示2秒）
    game->boss_warning = 1;
    game->boss_warning_start_time = HAL_GetTick();

    // 根据击败Boss数量提升HP（无尽模式难度递增）
    uint8_t boss_hp = PLANE_BOSS_HP + (game->boss_count * 5);  // 每次+5 HP
    if (boss_hp > 50) {
        boss_hp = 50;  // 限制最高50 HP
    }

    game->boss.hp = boss_hp;
    game->boss.max_hp = boss_hp;
    game->boss.phase = 1;  // 初始阶段1
    game->boss.spawn_time = HAL_GetTick();
    game->boss.last_attack_time = HAL_GetTick();
}

static void update_boss(plane_game_t *game)
{
    if (!game->boss.active) return;

    uint32_t now = HAL_GetTick();
    uint32_t alive_time = now - game->boss.spawn_time;

    // 阶段1：进场动画（前2秒）
    if (alive_time < 2000) {
        // 从右侧飞入到固定位置（X=100）
        if (game->boss.x > 100) {
            game->boss.x -= 2;
        }
    }
    else {
        // 阶段2+：固定在X=100，上下缓慢移动
        game->boss.x = 100;

        // 上下波浪移动
        float wave = sinf((float)alive_time * 0.002f) * 1.2f;
        game->boss.y += (int16_t)wave;

        // 边界限制
        if (game->boss.y < PLANE_PLAYER_Y_MIN) {
            game->boss.y = PLANE_PLAYER_Y_MIN;
        }
        if (game->boss.y > PLANE_PLAYER_Y_MAX - 16) {
            game->boss.y = PLANE_PLAYER_Y_MAX - 16;
        }

        // 根据血量更新阶段
        if (game->boss.hp > 14) {
            game->boss.phase = 1;  // 阶段1（HP 20-15）
        }
        else if (game->boss.hp > 7) {
            game->boss.phase = 2;  // 阶段2（HP 14-8）
        }
        else {
            game->boss.phase = 3;  // 阶段3（HP 7-1）狂暴
        }

        // 执行攻击
        boss_attack(game);
    }
}

static void boss_attack(plane_game_t *game)
{
    uint32_t now = HAL_GetTick();

    // 根据阶段设置攻击间隔
    uint32_t attack_interval;
    if (game->boss.phase == 1) {
        attack_interval = 500;       // 阶段1：500ms
    }
    else if (game->boss.phase == 2) {
        attack_interval = 600;       // 阶段2：600ms
    }
    else {
        attack_interval = 300;       // 阶段3：300ms（狂暴）
    }

    // 攻击冷却检测
    if (now - game->boss.last_attack_time < attack_interval) {
        return;  // 未到攻击时间
    }

    game->boss.last_attack_time = now;

    // 根据阶段执行不同攻击模式
    if (game->boss.phase == 1) {
        // 阶段1：单发瞄准弹（向玩家方向）
        float dx = (float)(game->player_x - game->boss.x);
        float dy = (float)(game->player_y - game->boss.y);
        float len = sqrtf(dx*dx + dy*dy);

        if (len > 0.1f) {  // 避免除零
            float bullet_vx = PLANE_ENEMY_BULLET_SPEED * (dx/len);
            float bullet_vy = PLANE_ENEMY_BULLET_SPEED * (dy/len);
            spawn_enemy_bullet(game, game->boss.x, game->boss.y + 8, bullet_vx, bullet_vy);
        }
    }
    else if (game->boss.phase == 2) {
        // 阶段2：扇形弹幕（5发，向左扩散）
        for (int i = -2; i <= 2; i++) {
            float angle = (float)i * 0.3f;  // 扩散角度（弧度）
            float bullet_vx = PLANE_ENEMY_BULLET_SPEED * cosf(angle);
            float bullet_vy = PLANE_ENEMY_BULLET_SPEED * sinf(angle);
            spawn_enemy_bullet(game, game->boss.x, game->boss.y + 8, bullet_vx, bullet_vy);
        }
    }
    else {
        // 阶段3：圆形弹幕（8发，全方向）
        for (int i = 0; i < 8; i++) {
            float angle = ((float)i / 8.0f) * 2.0f * 3.14159265f;  // 圆周均匀分布
            float bullet_vx = PLANE_ENEMY_BULLET_SPEED * cosf(angle);
            float bullet_vy = PLANE_ENEMY_BULLET_SPEED * sinf(angle);
            spawn_enemy_bullet(game, game->boss.x + 8, game->boss.y + 8, bullet_vx, bullet_vy);
        }
    }
}

static void spawn_powerup(plane_game_t *game, int16_t x, int16_t y, powerup_type_t type)
{
    // 查找空闲道具槽
    for (uint8_t i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerups[i].active) {
            game->powerups[i].active = 1;
            game->powerups[i].x = x;
            game->powerups[i].y = y;
            game->powerups[i].vx = -1.0f;  // 向左缓慢移动
            game->powerups[i].type = type;
            return;
        }
    }
    // 如果没有空闲槽位，道具不生成
}

static void update_powerups(plane_game_t *game)
{
    for (uint8_t i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerups[i].active) continue;

        powerup_t *powerup = &game->powerups[i];

        // 向左缓慢移动
        powerup->x += (int16_t)powerup->vx;

        // 超出屏幕左侧销毁
        if (powerup->x < -8) {
            powerup->active = 0;
        }
    }
}

static void apply_powerup(plane_game_t *game, powerup_type_t type)
{
    switch (type) {
        case POWERUP_WEAPON:
            // 武器升级（最高Lv3）
            if (game->weapon_level < 3) {
                game->weapon_level++;
            }
            break;

        case POWERUP_SHIELD:
            // 获得护盾（免死一次）
            game->player_shield = 1;
            break;

        case POWERUP_BOMB:
            // 清屏炸弹：销毁所有敌机和子弹
            // 销毁所有敌机
            for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
                if (game->enemies[i].active) {
                    // 给分（减半）
                    switch (game->enemies[i].type) {
                        case ENEMY_TYPE_SMALL:  game->score += 5; break;
                        case ENEMY_TYPE_MEDIUM: game->score += 10; break;
                        case ENEMY_TYPE_HEAVY:  game->score += 15; break;
                        case ENEMY_TYPE_FAST:   game->score += 7; break;
                        default: break;
                    }
                    // 爆炸动画
                    spawn_explosion(game, game->enemies[i].x, game->enemies[i].y, EXPLOSION_SMALL);
                    game->enemies[i].active = 0;
                }
            }
            // 销毁所有敌机子弹
            for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
                game->enemy_bullets[i].active = 0;
            }
            break;

        default:
            break;
    }
}

static void spawn_explosion(plane_game_t *game, int16_t x, int16_t y, explosion_type_t type)
{
    // 查找空闲爆炸槽
    for (uint8_t i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!game->explosions[i].active) {
            game->explosions[i].active = 1;
            game->explosions[i].x = x;
            game->explosions[i].y = y;
            game->explosions[i].type = type;
            game->explosions[i].frame = 0;  // 从第0帧开始
            game->explosions[i].last_frame_time = HAL_GetTick();
            return;
        }
    }
    // 如果没有空闲槽位，爆炸不生成
}

static void update_explosions(plane_game_t *game)
{
    uint32_t now = HAL_GetTick();

    for (uint8_t i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!game->explosions[i].active) continue;

        explosion_t *exp = &game->explosions[i];

        // 检查是否到切换帧的时间（50ms/帧）
        if (now - exp->last_frame_time >= PLANE_EXPLOSION_FRAME_TIME) {
            exp->frame++;  // 切换到下一帧
            exp->last_frame_time = now;

            // 播放完3帧后销毁
            if (exp->frame >= 3) {
                exp->active = 0;
            }
        }
    }
}

static void check_bullet_enemy_collision(plane_game_t *game)
{
    // 遍历所有玩家子弹
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!game->player_bullets[i].active) continue;

        bullet_t *bullet = &game->player_bullets[i];

        // 遍历所有敌机
        for (uint8_t j = 0; j < MAX_ENEMIES; j++) {
            if (!game->enemies[j].active) continue;

            enemy_t *enemy = &game->enemies[j];

            // AABB碰撞检测
            if (aabb_collision(bullet->x, bullet->y, PLANE_BULLET_WIDTH, PLANE_BULLET_HEIGHT,
                               enemy->x, enemy->y, enemy->width, enemy->height)) {

                // 敌机减血
                enemy->hp -= bullet->damage;

                // 子弹销毁
                bullet->active = 0;

                // 检查敌机是否死亡
                if (enemy->hp <= 0) {
                    // 根据类型给分
                    switch (enemy->type) {
                        case ENEMY_TYPE_SMALL:  game->score += 10; break;
                        case ENEMY_TYPE_MEDIUM: game->score += 20; break;
                        case ENEMY_TYPE_HEAVY:  game->score += 30; break;
                        case ENEMY_TYPE_FAST:   game->score += 15; break;
                        default: break;
                    }

                    // 爆炸动画
                    spawn_explosion(game, enemy->x, enemy->y, EXPLOSION_SMALL);

                    // 道具掉落系统（30%总概率）
                    uint8_t rand = (uint8_t)rng_get_random_range(0, 100);
                    if (rand < 5) {
                        // 5% 炸弹
                        spawn_powerup(game, enemy->x, enemy->y, POWERUP_BOMB);
                    }
                    else if (rand < 15) {
                        // 10% 护盾
                        spawn_powerup(game, enemy->x, enemy->y, POWERUP_SHIELD);
                    }
                    else if (rand < 30) {
                        // 15% 火力
                        spawn_powerup(game, enemy->x, enemy->y, POWERUP_WEAPON);
                    }

                    // 敌机销毁
                    enemy->active = 0;
                }

                break;  // 子弹已销毁，跳出内层循环
            }
        }
    }
}

static void check_bullet_boss_collision(plane_game_t *game)
{
    // Boss未激活则跳过
    if (!game->boss.active) {
        return;
    }

    // 遍历所有玩家子弹
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!game->player_bullets[i].active) continue;

        bullet_t *bullet = &game->player_bullets[i];

        // AABB碰撞检测（Boss是16x16）
        if (aabb_collision(bullet->x, bullet->y, PLANE_BULLET_WIDTH, PLANE_BULLET_HEIGHT,
                           game->boss.x, game->boss.y, 16, 16)) {

            // Boss减血
            if (game->boss.hp > 0) {
                game->boss.hp -= bullet->damage;
            }

            // 子弹销毁
            bullet->active = 0;

            // 检查Boss是否死亡
            if (game->boss.hp <= 0) {
                // 给分（200分）
                game->score += 200;

                // 大型爆炸动画
                spawn_explosion(game, game->boss.x, game->boss.y, EXPLOSION_SMALL);
                spawn_explosion(game, game->boss.x + 8, game->boss.y, EXPLOSION_SMALL);
                spawn_explosion(game, game->boss.x, game->boss.y + 8, EXPLOSION_SMALL);
                spawn_explosion(game, game->boss.x + 8, game->boss.y + 8, EXPLOSION_SMALL);

                // 提升难度（无尽模式）
                game->difficulty_level++;  // 难度等级+1
                game->boss_count++;        // Boss击败数+1

                // Boss销毁
                game->boss.active = 0;
            }

            break;  // 子弹已销毁，跳出循环
        }
    }
}

static void check_enemy_bullet_player_collision(plane_game_t *game)
{
    // 遍历所有敌机子弹
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!game->enemy_bullets[i].active) continue;

        bullet_t *bullet = &game->enemy_bullets[i];

        // AABB碰撞检测（玩家碰撞箱收紧2px，提升手感）
        if (aabb_collision(bullet->x, bullet->y, 3, 3,
                           game->player_x + 2, game->player_y + 2,
                           PLANE_PLAYER_WIDTH - 4, PLANE_PLAYER_HEIGHT - 4)) {

            // 子弹销毁
            bullet->active = 0;

            // 玩家受伤
            player_take_damage(game);

            break;  // 只处理一次碰撞
        }
    }
}

static void check_enemy_player_collision(plane_game_t *game)
{
    // 遍历所有敌机
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) continue;

        enemy_t *enemy = &game->enemies[i];

        // AABB碰撞检测（玩家碰撞箱收紧2px）
        if (aabb_collision(enemy->x, enemy->y, enemy->width, enemy->height,
                           game->player_x + 2, game->player_y + 2,
                           PLANE_PLAYER_WIDTH - 4, PLANE_PLAYER_HEIGHT - 4)) {

            // 爆炸动画
            spawn_explosion(game, enemy->x, enemy->y, EXPLOSION_SMALL);

            // 敌机销毁
            enemy->active = 0;

            // 玩家受伤
            player_take_damage(game);

            break;  // 只处理一次碰撞
        }
    }
}

static void check_powerup_player_collision(plane_game_t *game)
{
    // 遍历所有道具
    for (uint8_t i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerups[i].active) continue;

        powerup_t *powerup = &game->powerups[i];

        // AABB碰撞检测（8x8道具）
        if (aabb_collision(powerup->x, powerup->y, 8, 8,
                           game->player_x, game->player_y,
                           PLANE_PLAYER_WIDTH, PLANE_PLAYER_HEIGHT)) {

            // 应用道具效果
            apply_powerup(game, powerup->type);

            // 道具销毁
            powerup->active = 0;

            break;  // 一次只拾取一个道具
        }
    }
}

static uint8_t aabb_collision(int16_t x1, int16_t y1, uint8_t w1, uint8_t h1,
                               int16_t x2, int16_t y2, uint8_t w2, uint8_t h2)
{
    // AABB碰撞检测：两个矩形是否重叠
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

static void draw_player(plane_game_t *game, u8g2_t *u8g2)
{
    // 绘制玩家飞机（8x8 sprite）
    draw_bitmap(u8g2, game->player_x, game->player_y, sprite_player,
                PLANE_PLAYER_WIDTH, PLANE_PLAYER_HEIGHT);

    // 如果有护盾，绘制护盾框
    if (game->player_shield) {
        u8g2_DrawFrame(u8g2, game->player_x - 1, game->player_y - 1,
                       PLANE_PLAYER_WIDTH + 2, PLANE_PLAYER_HEIGHT + 2);
    }
}

static void draw_bullets(plane_game_t *game, u8g2_t *u8g2)
{
    // 绘制玩家子弹
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!game->player_bullets[i].active) continue;

        bullet_t *bullet = &game->player_bullets[i];
        draw_bitmap(u8g2, bullet->x, bullet->y, sprite_player_bullet,
                    PLANE_BULLET_WIDTH, PLANE_BULLET_HEIGHT);
    }

    // 绘制敌机子弹（阶段3实现）
    for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!game->enemy_bullets[i].active) continue;

        bullet_t *bullet = &game->enemy_bullets[i];
        // 简单用矩形表示敌机子弹（3x3）
        u8g2_DrawBox(u8g2, bullet->x, bullet->y, 3, 3);
    }
}

static void draw_enemies(plane_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        if (!game->enemies[i].active) continue;

        enemy_t *enemy = &game->enemies[i];

        // 根据类型绘制不同sprite
        switch (enemy->type) {
            case ENEMY_TYPE_SMALL:
                draw_bitmap(u8g2, enemy->x, enemy->y, sprite_enemy_small, 7, 6);
                break;

            case ENEMY_TYPE_MEDIUM:
                draw_bitmap(u8g2, enemy->x, enemy->y, sprite_enemy_medium, 9, 8);
                break;

            case ENEMY_TYPE_HEAVY:
                draw_bitmap(u8g2, enemy->x, enemy->y, sprite_enemy_heavy, 11, 10);
                break;

            case ENEMY_TYPE_FAST:
                draw_bitmap(u8g2, enemy->x, enemy->y, sprite_enemy_fast, 6, 5);
                break;

            default:
                break;
        }

        // 调试：显示敌机血量（可选）
        // if (enemy->hp > 1) {
        //     u8g2_SetFont(u8g2, u8g2_font_micro_tr);
        //     char hp_buf[4];
        //     sprintf(hp_buf, "%d", enemy->hp);
        //     u8g2_DrawStr(u8g2, enemy->x, enemy->y - 2, hp_buf);
        // }
    }
}

static void draw_boss(plane_game_t *game, u8g2_t *u8g2)
{
    if (!game->boss.active) {
        return;  // Boss未激活，不绘制
    }

    // 1. 绘制Boss sprite（16x16）
    draw_bitmap(u8g2, game->boss.x, game->boss.y, sprite_boss, 16, 16);

    // 2. 绘制Boss血条（屏幕顶部）
    // 血条位置：屏幕顶部居中
    int16_t bar_x = 34;       // 居中位置（(128 - 60) / 2）
    int16_t bar_y = 2;        // 顶部偏移2像素
    uint8_t bar_max_width = 60;  // 血条最大宽度
    uint8_t bar_height = 4;   // 血条高度

    // 计算当前血量对应的宽度
    uint8_t bar_current_width = (uint8_t)((game->boss.hp * bar_max_width) / game->boss.max_hp);

    // 绘制血条边框（空心矩形）
    u8g2_DrawFrame(u8g2, bar_x - 1, bar_y - 1, bar_max_width + 2, bar_height + 2);

    // 绘制血条填充（实心矩形）
    if (bar_current_width > 0) {
        u8g2_DrawBox(u8g2, bar_x, bar_y, bar_current_width, bar_height);
    }

    // 3. 绘制"BOSS"文字标签（血条左侧）
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(u8g2, 2, bar_y + bar_height + 1, "BOSS");
}

static void draw_powerups(plane_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < MAX_POWERUPS; i++) {
        if (!game->powerups[i].active) continue;

        powerup_t *powerup = &game->powerups[i];

        // 根据类型绘制不同sprite
        switch (powerup->type) {
            case POWERUP_WEAPON:
                draw_bitmap(u8g2, powerup->x, powerup->y, sprite_powerup_weapon, 8, 8);
                break;

            case POWERUP_SHIELD:
                draw_bitmap(u8g2, powerup->x, powerup->y, sprite_powerup_shield, 8, 8);
                break;

            case POWERUP_BOMB:
                draw_bitmap(u8g2, powerup->x, powerup->y, sprite_powerup_bomb, 8, 8);
                break;

            default:
                break;
        }
    }
}

static void draw_explosions(plane_game_t *game, u8g2_t *u8g2)
{
    for (uint8_t i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!game->explosions[i].active) continue;

        explosion_t *exp = &game->explosions[i];

        // 根据当前帧绘制不同sprite
        const uint8_t *sprite = NULL;
        switch (exp->frame) {
            case 0:
                sprite = sprite_explosion_frame1;
                break;
            case 1:
                sprite = sprite_explosion_frame2;
                break;
            case 2:
                sprite = sprite_explosion_frame3;
                break;
            default:
                continue;  // 无效帧，跳过
        }

        // 绘制爆炸（8x8）
        if (sprite != NULL) {
            draw_bitmap(u8g2, exp->x, exp->y, sprite, 8, 8);
        }
    }
}

static void draw_ui(plane_game_t *game, u8g2_t *u8g2)
{
    // UI显示（带难度等级）
    char buf[32];

    // 设置字体
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);

    // 绘制当前分数（左下角）
    sprintf(buf, "S:%u", (unsigned int)game->score);
    u8g2_DrawStr(u8g2, 2, 63, buf);

    // 绘制生命值（中左下角）
    sprintf(buf, "HP:%u", (unsigned int)game->player_hp);
    u8g2_DrawStr(u8g2, 45, 63, buf);

    // 绘制武器等级（中右下角）
    sprintf(buf, "W:%u", (unsigned int)game->weapon_level);
    u8g2_DrawStr(u8g2, 80, 63, buf);

    // 绘制难度等级（右下角）
    // 格式：Lv:2（难度等级2）
    sprintf(buf, "Lv:%u", (unsigned int)game->difficulty_level);
    u8g2_DrawStr(u8g2, 100, 63, buf);
}

static void draw_game_over(plane_game_t *game, u8g2_t *u8g2)
{
    char buf[32];

    // 绘制"GAME OVER"标题
    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
    u8g2_DrawStr(u8g2, 30, 15, "GAME OVER");

    // 绘制最终分数
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    sprintf(buf, "Score: %u", (unsigned int)game->score);
    u8g2_DrawStr(u8g2, 30, 28, buf);

    // 绘制最高分（如果刷新了）
    if (game->score == game->high_score && game->score > 0) {
        u8g2_DrawStr(u8g2, 25, 38, "NEW RECORD!");
    } else {
        sprintf(buf, "Best: %u", (unsigned int)game->high_score);
        u8g2_DrawStr(u8g2, 30, 38, buf);
    }

    // 绘制击败Boss数和难度等级
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    sprintf(buf, "Boss:%u  Lv:%u", (unsigned int)game->boss_count,
            (unsigned int)game->difficulty_level);
    u8g2_DrawStr(u8g2, 25, 48, buf);

    // 绘制重新开始提示
    u8g2_DrawStr(u8g2, 20, 58, "Press A to Retry");
}

static void draw_bitmap(u8g2_t *u8g2, int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h)
{
    // 计算每行需要多少字节（向上取整）
    uint8_t bytes_per_row = (w + 7) / 8;

    for (uint8_t row = 0; row < h; row++) {
        // 遍历当前行的所有像素
        for (uint8_t col = 0; col < w; col++) {
            // 计算当前像素在bitmap中的位置
            uint8_t byte_index = row * bytes_per_row + (col / 8);
            uint8_t bit_index = 7 - (col % 8);

            // 检查对应的bit是否为1
            if (bitmap[byte_index] & (1 << bit_index)) {
                u8g2_DrawPixel(u8g2, x + col, y + row);
            }
        }
    }
}
