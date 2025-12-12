#include "system_assembly.h"
// #include "test_menu.h"  // 测试菜单（已禁用）

// -----------------------------------------------------------------------------
// 系统初始化和任务注册函数实现 (装配逻辑)
// -----------------------------------------------------------------------------

// 贪吃蛇游戏实例（全局静态变量）
static snake_game_t g_snake_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(snake_game, snake_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_snake_game, "Snake", snake_game);

// 恐龙跑酷游戏实例（全局静态变量）
static dino_game_t g_dino_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(dino_game, dino_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_dino_game, "Dino", dino_game);

// 打飞机游戏实例（全局静态变量）
static plane_game_t g_plane_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(plane_game, plane_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_plane_game, "Plane", plane_game);

// 俄罗斯方块游戏实例（全局静态变量）
static tetris_game_t g_tetris_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(tetris_game, tetris_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_tetris_game, "Tetris", tetris_game);

// 打砖块游戏实例（全局静态变量）
static breakout_game_t g_breakout_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(breakout_game, breakout_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_breakout_game, "Breakout", breakout_game);

// 推箱子游戏实例（全局静态变量）
static sokoban_game_t g_sokoban_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(sokoban_game, sokoban_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_sokoban_game, "Sokoban", sokoban_game);

// 扫雷游戏实例（全局静态变量）
static minesweeper_game_t g_minesweeper_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(minesweeper_game, minesweeper_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_minesweeper_game, "Minesweeper", minesweeper_game);

// 吃豆人游戏实例（全局静态变量）
static pacman_game_t g_pacman_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(pacman_game, pacman_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_pacman_game, "Pac-Man", pacman_game);

// 乒乓球游戏实例（全局静态变量）
static pong_game_t g_pong_game;

// 使用GAME_ADAPTER宏生成适配器函数
GAME_ADAPTER(pong_game, pong_game_t)

// 使用GAME_DESCRIPTOR宏生成游戏描述符
GAME_DESCRIPTOR(g_pong_game, "Pong", pong_game);


   void test_flash(void)
  {
      uint8_t write_buf[32] = "Hello W25Q64 Flash!";
      uint8_t read_buf[32] = {0};
      uint16_t id;
      uint8_t test_passed = 1;

      my_printf(&huart1, "\r\n========= Flash功能测试 =========\r\n");

      // 1. 初始化
      spi_flash_init();
      my_printf(&huart1, "[1] 初始化完成\r\n");

      // 2. 读取ID
      id = spi_flash_read_id();
      my_printf(&huart1, "[2] Flash ID: 0x%04X ", id);
      if (id == 0xEF17) {
          my_printf(&huart1, "(W25Q64)\r\n");
      } else if (id == 0xEF18) {
          my_printf(&huart1, "(W25Q128)\r\n");
      } else {
          my_printf(&huart1, "(未知)\r\n");
          test_passed = 0;
      }

      // 3. 擦除扇区0
      my_printf(&huart1, "[3] 擦除扇区0...");
      spi_flash_sector_erase(0);
      my_printf(&huart1, "完成\r\n");

      // 4. 读取擦除后的数据（应该全是0xFF）
      my_printf(&huart1, "[4] 验证擦除...");
      spi_flash_buffer_read(read_buf, 0, 32);
      uint8_t erase_ok = 1;
      for (int i = 0; i < 32; i++) {
          if (read_buf[i] != 0xFF) {
              erase_ok = 0;
              break;
          }
      }
      if (erase_ok) {
          my_printf(&huart1, "成功(全0xFF)\r\n");
      } else {
          my_printf(&huart1, "失败!\r\n");
          test_passed = 0;
      }

      // 5. 写入数据
      my_printf(&huart1, "[5] 写入数据: \"%s\"...", write_buf);
      spi_flash_page_write(write_buf, 0, sizeof(write_buf));
      my_printf(&huart1, "完成\r\n");

      // 6. 读取数据
      my_printf(&huart1, "[6] 读取数据...");
      memset(read_buf, 0, sizeof(read_buf));
      spi_flash_buffer_read(read_buf, 0, sizeof(read_buf));
      my_printf(&huart1, "完成\r\n");
      my_printf(&huart1, "    读取结果: \"%s\"\r\n", read_buf);

      // 7. 验证数据
      my_printf(&huart1, "[7] 验证数据...");
      if (memcmp(write_buf, read_buf, sizeof(write_buf)) == 0) {
          my_printf(&huart1, "成功!\r\n");
      } else {
          my_printf(&huart1, "失败!\r\n");
          test_passed = 0;
      }

      // 8. 测试不同地址写入
      my_printf(&huart1, "[8] 测试地址0x100写入...");
      uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
      spi_flash_page_write(test_data, 0x100, sizeof(test_data));
      memset(read_buf, 0, sizeof(read_buf));
      spi_flash_buffer_read(read_buf, 0x100, sizeof(test_data));

      uint8_t addr_test_ok = 1;
      for (int i = 0; i < sizeof(test_data); i++) {
          if (read_buf[i] != test_data[i]) {
              addr_test_ok = 0;
              break;
          }
      }
      if (addr_test_ok) {
          my_printf(&huart1, "成功\r\n");
      } else {
          my_printf(&huart1, "失败\r\n");
          test_passed = 0;
      }

      // 9. 清理测试数据
      my_printf(&huart1, "[9] 清理测试数据(擦除扇区0)...");
      spi_flash_sector_erase(0);
      my_printf(&huart1, "完成\r\n");

      // 总结
      my_printf(&huart1, "==================================\r\n");
      if (test_passed) {
          my_printf(&huart1, ">>> 所有测试通过! <<<\r\n");
      } else {
          my_printf(&huart1, ">>> 测试失败! <<<\r\n");
      }
      my_printf(&huart1, "==================================\r\n\r\n");
  }
/**
 * @brief 系统的主要初始化函数。
 */
void system_assembly_init(void)
{
	// 系统各组件初始化
	scheduler_init();
	ebtn_driver_init();
	event_queue_init();
	rocker_app_init();           // 摇杆应用层初始化（包含ADC驱动+组件+事件使能）
//	test_rocker_adc_init();

	// 初始化u8g2显示组件
	u8g2_component_init();

	// 初始化u8g2测试
//	test_u8g2_init();

	//随机数生成器驱动初始化
	rng_init();

	// 初始化输入管理器
	input_manager_init();

	// 初始化游戏管理器
	game_manager_init();

	// 注册贪吃蛇游戏（使用game_manager统一管理）
	// 游戏初始状态为非活跃，由菜单通过game_manager启动
	memset(&g_snake_game, 0, sizeof(snake_game_t));  // 预初始化为非活跃状态
	snake_game_deactivate(&g_snake_game);
	game_manager_register(&g_snake_game_descriptor);

	// 注册恐龙跑酷游戏
	memset(&g_dino_game, 0, sizeof(dino_game_t));
	dino_game_deactivate(&g_dino_game);
	game_manager_register(&g_dino_game_descriptor);

	// 注册打飞机游戏
	memset(&g_plane_game, 0, sizeof(plane_game_t));
	plane_game_deactivate(&g_plane_game);
	game_manager_register(&g_plane_game_descriptor);

	// 注册俄罗斯方块游戏
	memset(&g_tetris_game, 0, sizeof(tetris_game_t));
	tetris_game_deactivate(&g_tetris_game);
	game_manager_register(&g_tetris_game_descriptor);

	// 注册打砖块游戏
	memset(&g_breakout_game, 0, sizeof(breakout_game_t));
	breakout_game_deactivate(&g_breakout_game);
	game_manager_register(&g_breakout_game_descriptor);

	// 注册推箱子游戏
	memset(&g_sokoban_game, 0, sizeof(sokoban_game_t));
	sokoban_game_deactivate(&g_sokoban_game);
	game_manager_register(&g_sokoban_game_descriptor);

	// 注册扫雷游戏
	memset(&g_minesweeper_game, 0, sizeof(minesweeper_game_t));
	minesweeper_game_deactivate(&g_minesweeper_game);
	game_manager_register(&g_minesweeper_game_descriptor);

	// 注册吃豆人游戏
	memset(&g_pacman_game, 0, sizeof(pacman_game_t));
	pacman_game_deactivate(&g_pacman_game);
	game_manager_register(&g_pacman_game_descriptor);

	// 注册乒乓球游戏
	memset(&g_pong_game, 0, sizeof(pong_game_t));
	pong_game_deactivate(&g_pong_game);
	game_manager_register(&g_pong_game_descriptor);

	// 初始化主菜单
	main_menu_init();
	test_flash();  // Temporarily disabled - conflicts with LittleFS

	test_littlefs_init();
	test_littlefs_run_all();
	
	test_sdcard_init();
  test_sdcard_run_all();
  test_sdcard_run_advanced();
}

/**
 * @brief 应用任务注册函数。
 * 职责：将所有应用层任务注册到调度器中。
 */
void system_assembly_register_tasks(void)
{
	scheduler_add_task(ebtn_process_task, 10);       // ebtn按键处理任务
	scheduler_add_task(rocker_process_task, 10);     // 摇杆处理任务
	scheduler_add_task(input_manager_task, 10);      // 输入管理器任务
	scheduler_add_task(game_manager_task_all, 10);   // 游戏管理器任务（调用所有注册游戏的task）
	scheduler_add_task(main_menu_task, 10);          // 主菜单任务
}

 