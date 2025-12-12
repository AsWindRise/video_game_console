#include "snake_game.h"

// -----------------------------------------------------------------------------
// 私有函数声明
// -----------------------------------------------------------------------------

static void generate_food(snake_game_t *game);
static bool is_food_on_snake(snake_game_t *game, uint8_t x, uint8_t y);

// -----------------------------------------------------------------------------
// 公共函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化贪吃蛇游戏
 */
void snake_game_init(snake_game_t *game)
{
	// 保存回调函数和活动状态（不被清空）
	snake_exit_callback_t saved_callback = game->exit_callback;
	uint8_t saved_active = game->is_active;

	// 清空结构体
	memset(game, 0, sizeof(snake_game_t));

	// 恢复回调和活动状态
	game->exit_callback = saved_callback;
	game->is_active = saved_active;

	// 初始化蛇（中心位置，长度3，向右移动）
	game->length = 3;
	game->body_x[0] = 8;  // 头部
	game->body_y[0] = 4;
	game->body_x[1] = 7;  // 身体
	game->body_y[1] = 4;
	game->body_x[2] = 6;  // 尾部
	game->body_y[2] = 4;

	// 初始化方向（方案A：两个方向保持一致）
	game->direction = DIR_RIGHT;
	game->next_direction = DIR_RIGHT;

	// 生成第一个食物
	generate_food(game);

	// 初始化游戏状态
	game->game_state = GAME_STATE_RUNNING;
	game->score = 0;
	game->last_speed_up_score = 0;

	// 初始化动态速度系统
	game->update_interval = SNAKE_SPEED_INITIAL;  // 初始速度：250ms
	game->last_update_time = HAL_GetTick();
}

/**
 * @brief 处理玩家输入（方案A：下一方向锁存机制）
 */
void snake_game_update_input(snake_game_t *game)
{
	// === 全局按键：B键返回菜单（所有状态下都可以） ===
	if (input_is_just_pressed(INPUT_BTN_B))
	{
		if (game->exit_callback != NULL)
		{
			game->exit_callback();  // 调用退出回调返回菜单
		}
		return;
	}

	// 只在游戏运行状态才处理输入
	if (game->game_state != GAME_STATE_RUNNING)
	{
		// 游戏结束时，按START键重新开始
		if (game->game_state == GAME_STATE_GAME_OVER)
		{
			if (input_is_just_pressed(INPUT_BTN_START))
			{
				snake_game_init(game);  // 重新初始化游戏
			}
		}
		// 暂停状态：按START键恢复游戏
		else if (game->game_state == GAME_STATE_PAUSED)
		{
			if (input_is_just_pressed(INPUT_BTN_START))
			{
				game->game_state = GAME_STATE_RUNNING;  // 恢复运行
			}
		}
		return;
	}

	// 检测UP键（防反向：当前向下时不能向上）
	if (input_is_just_pressed(INPUT_BTN_UP))
	{
		if (game->direction != DIR_DOWN)
		{
			game->next_direction = DIR_UP;  // 锁存到next_direction
		}
	}

	// 检测DOWN键（防反向：当前向上时不能向下）
	if (input_is_just_pressed(INPUT_BTN_DOWN))
	{
		if (game->direction != DIR_UP)
		{
			game->next_direction = DIR_DOWN;
		}
	}

	// 检测LEFT键（防反向：当前向右时不能向左）
	if (input_is_just_pressed(INPUT_BTN_LEFT))
	{
		if (game->direction != DIR_RIGHT)
		{
			game->next_direction = DIR_LEFT;
		}
	}

	// 检测RIGHT键（防反向：当前向左时不能向右）
	if (input_is_just_pressed(INPUT_BTN_RIGHT))
	{
		if (game->direction != DIR_LEFT)
		{
			game->next_direction = DIR_RIGHT;
		}
	}

	// 检测暂停键（START键）
	if (input_is_just_pressed(INPUT_BTN_START))
	{
		game->game_state = GAME_STATE_PAUSED;
	}
}

/**
 * @brief 更新游戏逻辑（100ms周期调用）
 */
void snake_game_update_logic(snake_game_t *game)
{
	// 只在游戏运行状态才更新逻辑
	if (game->game_state != GAME_STATE_RUNNING)
	{
		return;
	}

	// ============ 关键！应用next_direction ============
	// 在移动前，将next_direction应用为当前direction
	game->direction = game->next_direction;

	// 计算新蛇头位置
	int8_t new_head_x = game->body_x[0];
	int8_t new_head_y = game->body_y[0];

	switch (game->direction)
	{
	case DIR_UP:
		new_head_y--;
		break;
	case DIR_DOWN:
		new_head_y++;
		break;
	case DIR_LEFT:
		new_head_x--;
		break;
	case DIR_RIGHT:
		new_head_x++;
		break;
	}

	// 检查碰撞（墙壁）
	if (new_head_x < 0 || new_head_x >= SNAKE_GRID_WIDTH ||
	    new_head_y < 0 || new_head_y >= SNAKE_GRID_HEIGHT)
	{
		game->game_state = GAME_STATE_GAME_OVER;
		return;
	}

	// 检查碰撞（自身）- 从索引1开始，跳过头部
	for (uint8_t i = 1; i < game->length; i++)
	{
		if (new_head_x == game->body_x[i] &&
		    new_head_y == game->body_y[i])
		{
			game->game_state = GAME_STATE_GAME_OVER;
			return;
		}
	}

	// 检查是否吃到食物
	bool ate_food = (new_head_x == game->food_x &&
	                 new_head_y == game->food_y);

	if (ate_food)
	{
		// 吃到食物：蛇身加长
		game->length++;
		game->score += 10;

		// 动态速度系统：检查是否该加速
		// 每增加SNAKE_SPEED_INTERVAL分（3个食物），速度加快一次
		if (game->score - game->last_speed_up_score >= SNAKE_SPEED_INTERVAL)
		{
			game->last_speed_up_score = game->score;  // 记录本次加速时的分数

			// 加速：减少更新间隔（最小不低于SNAKE_SPEED_MIN）
			if (game->update_interval > SNAKE_SPEED_MIN)
			{
				game->update_interval -= SNAKE_SPEED_DECREASE;  // 减少10ms
				if (game->update_interval < SNAKE_SPEED_MIN)
				{
					game->update_interval = SNAKE_SPEED_MIN;  // 限制最小值
				}
			}
		}

		// 生成新食物
		generate_food(game);
	}

	// 移动蛇身（从尾到头依次向前移动）
	if (!ate_food)
	{
		// 没吃到食物：尾部消失（所有身体向前移动）
		for (uint8_t i = game->length - 1; i > 0; i--)
		{
			game->body_x[i] = game->body_x[i - 1];
			game->body_y[i] = game->body_y[i - 1];
		}
	}
	else
	{
		// 吃到食物：长度已增加，所有身体向前移动（尾部不动）
		for (uint8_t i = game->length - 1; i > 0; i--)
		{
			game->body_x[i] = game->body_x[i - 1];
			game->body_y[i] = game->body_y[i - 1];
		}
	}

	// 更新蛇头位置
	game->body_x[0] = new_head_x;
	game->body_y[0] = new_head_y;
}

/**
 * @brief 渲染游戏画面
 */
void snake_game_render(snake_game_t *game)
{
	u8g2_t *u8g2 = u8g2_get_instance();

	// 清空缓冲区
	u8g2_ClearBuffer(u8g2);

	// 根据游戏状态渲染不同画面
	switch (game->game_state)
	{
	case GAME_STATE_RUNNING:
	{
		// 绘制蛇身（每格8×8像素）
		for (uint8_t i = 0; i < game->length; i++)
		{
			u8g2_DrawBox(u8g2,
			             game->body_x[i] * SNAKE_PIXEL_SIZE,
			             game->body_y[i] * SNAKE_PIXEL_SIZE,
			             SNAKE_PIXEL_SIZE,
			             SNAKE_PIXEL_SIZE);
		}

		// 绘制食物（使用圆形区分）
		u8g2_DrawCircle(u8g2,
		                game->food_x * SNAKE_PIXEL_SIZE + SNAKE_PIXEL_SIZE / 2,
		                game->food_y * SNAKE_PIXEL_SIZE + SNAKE_PIXEL_SIZE / 2,
		                SNAKE_PIXEL_SIZE / 2 - 1,
		                U8G2_DRAW_ALL);

		// 绘制分数（右上角）
		char score_str[16];
		snprintf(score_str, sizeof(score_str), "%d", game->score);
		u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
		u8g2_DrawStr(u8g2, 128 - 24, 8, score_str);

		// 绘制速度指示器（左上角，小字体）
		char speed_str[16];
		uint16_t speed_level = (SNAKE_SPEED_INITIAL - game->update_interval) / SNAKE_SPEED_DECREASE;
		snprintf(speed_str, sizeof(speed_str), "Lv%d", speed_level);
		u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
		u8g2_DrawStr(u8g2, 0, 7, speed_str);
		break;
	}

	case GAME_STATE_PAUSED:
	{
		// 显示暂停信息
		u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
		u8g2_DrawStr(u8g2, 30, 30, "PAUSED");
		u8g2_DrawStr(u8g2, 10, 45, "Press START");
		break;
	}

	case GAME_STATE_GAME_OVER:
	{
		// 显示游戏结束信息
		u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
		u8g2_DrawStr(u8g2, 20, 20, "GAME OVER");

		// 显示最终分数
		char score_str[32];
		snprintf(score_str, sizeof(score_str), "Score: %d", game->score);
		u8g2_DrawStr(u8g2, 25, 35, score_str);

		// 显示重新开始提示
		u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
		u8g2_DrawStr(u8g2, 5, 55, "Press START to retry");
		break;
	}

	default:
		break;
	}

	// 发送缓冲区到屏幕
	u8g2_SendBuffer(u8g2);
}

/**
 * @brief 游戏主任务（集成完整游戏循环）
 */
void snake_game_task(snake_game_t *game)
{
	// 只在游戏活跃时才运行
	if (!game->is_active)
	{
		return;
	}

	uint32_t current_time = HAL_GetTick();

	// 1. 输入处理（每次调用都执行，实时响应）
	snake_game_update_input(game);

	// 2. 游戏逻辑更新（使用动态update_interval）
	if (current_time - game->last_update_time >= game->update_interval)
	{
		game->last_update_time = current_time;
		snake_game_update_logic(game);
	}

	// 3. 渲染输出（每次调用都执行）
	snake_game_render(game);
}

// -----------------------------------------------------------------------------
// 私有函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 生成新的食物位置
 * @note  使用rng_driver生成随机坐标，确保不与蛇身重叠
 */
static void generate_food(snake_game_t *game)
{
	uint8_t new_x, new_y;

	// 使用do-while循环确保食物不与蛇身重叠
	do
	{
		// 使用rng_get_random_range生成范围内随机数（更简洁）
		new_x = (uint8_t)rng_get_random_range(0, SNAKE_GRID_WIDTH - 1);
		new_y = (uint8_t)rng_get_random_range(0, SNAKE_GRID_HEIGHT - 1);
	}
	while (is_food_on_snake(game, new_x, new_y));

	game->food_x = new_x;
	game->food_y = new_y;
}

/**
 * @brief 检查指定坐标是否与蛇身重叠
 * @param game: 游戏状态结构体指针
 * @param x: 要检查的X坐标
 * @param y: 要检查的Y坐标
 * @retval true=重叠, false=不重叠
 */
static bool is_food_on_snake(snake_game_t *game, uint8_t x, uint8_t y)
{
	for (uint8_t i = 0; i < game->length; i++)
	{
		if (x == game->body_x[i] && y == game->body_y[i])
		{
			return true;  // 与蛇身重叠
		}
	}
	return false;  // 不重叠
}

// -----------------------------------------------------------------------------
// 菜单集成API实现
// -----------------------------------------------------------------------------

/**
 * @brief 激活游戏
 */
void snake_game_activate(snake_game_t *game)
{
	if (game == NULL)
	{
		return;
	}

	game->is_active = 1;
}

/**
 * @brief 停用游戏
 */
void snake_game_deactivate(snake_game_t *game)
{
	if (game == NULL)
	{
		return;
	}

	game->is_active = 0;
}

/**
 * @brief 设置游戏退出回调
 */
void snake_game_set_exit_callback(snake_game_t *game, snake_exit_callback_t callback)
{
	if (game == NULL)
	{
		return;
	}

	game->exit_callback = callback;
}
