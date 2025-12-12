#include "test_menu.h"
#include "menu_core.h"
#include "menu_builder.h"
#include "menu_adapter.h"
#include "u8g2_stm32_hal.h"
#include <stdio.h>

// =============================================================================
// 测试菜单示例代码
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 全局变量
// -----------------------------------------------------------------------------
static menu_instance_t g_test_menu; // 菜单实例
static uint8_t         g_volume = 75; // 模拟音量设置
static uint8_t         g_sound_on = 1; // 模拟音效开关

// -----------------------------------------------------------------------------
// 2. 动作函数（游戏启动）
// -----------------------------------------------------------------------------
void action_start_snake(void)
{
    // TODO: 启动贪吃蛇游戏
    // snake_game_start();

    // 测试阶段：暂时只打印调试信息，不暂停菜单
    // 未来实现真实游戏时，应该：
    // 1. 暂停菜单：menu_deactivate(&g_test_menu);
    // 2. 启动游戏循环
    // 3. 游戏结束后重新激活菜单：menu_activate(&g_test_menu);
}

void action_start_tetris(void)
{
    // TODO: 启动俄罗斯方块游戏
    // tetris_game_start();

    // 测试阶段：暂时不暂停菜单
}

void action_start_pong(void)
{
    // TODO: 启动乒乓球游戏
    // pong_game_start();

    // 测试阶段：暂时不暂停菜单
}

// -----------------------------------------------------------------------------
// 3. 动作函数（设置）
// -----------------------------------------------------------------------------
void action_adjust_volume(void)
{
    // 简单模拟：每次调用增加10%音量
    g_volume += 10;
    if (g_volume > 100)
    {
        g_volume = 0;
    }

    // 强制刷新显示（更新动态值）
    menu_force_refresh(&g_test_menu);
}

void action_toggle_sound(void)
{
    // 切换音效开关
    g_sound_on = !g_sound_on;

    // 强制刷新显示
    menu_force_refresh(&g_test_menu);
}

void action_reset_settings(void)
{
    // 重置设置
    g_volume   = 75;
    g_sound_on = 1;

    // 强制刷新显示
    menu_force_refresh(&g_test_menu);
}

void action_about(void)
{
    // TODO: 显示关于界面
    // 测试阶段：暂时不暂停菜单，避免卡死
    // 未来可以实现一个About页面，按B键返回菜单
}

// -----------------------------------------------------------------------------
// 4. 动态值显示函数
// -----------------------------------------------------------------------------
const char *get_volume_value(void)
{
    static char buf[8];
    sprintf(buf, "%d%%", g_volume);
    return buf;
}

const char *get_sound_value(void)
{
    return g_sound_on ? "ON" : "OFF";
}

// -----------------------------------------------------------------------------
// 5. 菜单项定义
// -----------------------------------------------------------------------------

// === 游戏子菜单 ===
MENU_ACTION(menu_game_snake, "Snake", action_start_snake);
MENU_ACTION(menu_game_tetris, "Tetris", action_start_tetris);
MENU_ACTION(menu_game_pong, "Pong", action_start_pong);

// === 设置子菜单 ===
MENU_ACTION_WITH_VALUE(menu_set_volume, "Volume", action_adjust_volume,
                        get_volume_value);
MENU_ACTION_WITH_VALUE(menu_set_sound, "Sound", action_toggle_sound,
                        get_sound_value);
MENU_ACTION(menu_set_reset, "Reset", action_reset_settings);

// === 主菜单 ===
MENU_ITEM(menu_main_games, "Games");
MENU_ITEM(menu_main_settings, "Settings");
MENU_ACTION(menu_main_about, "About", action_about);

// -----------------------------------------------------------------------------
// 6. 初始化函数
// -----------------------------------------------------------------------------
void test_menu_init(void)
{
    // === 构建游戏子菜单 ===
    menu_item_t *game_items[] = {&menu_game_snake, &menu_game_tetris,
                                  &menu_game_pong};
    menu_link_items(game_items, 3);
    menu_set_submenu(&menu_main_games, &menu_game_snake);

    // === 构建设置子菜单 ===
    menu_item_t *setting_items[] = {&menu_set_volume, &menu_set_sound,
                                     &menu_set_reset};
    menu_link_items(setting_items, 3);
    menu_set_submenu(&menu_main_settings, &menu_set_volume);

    // === 构建主菜单 ===
    menu_item_t *main_items[] = {&menu_main_games, &menu_main_settings,
                                  &menu_main_about};
    menu_link_items(main_items, 3);

    // === 初始化菜单实例 ===
    menu_init(&g_test_menu, &menu_main_games);

    // === 使用适配器自动绑定输入和渲染 ===
    menu_adapter_init(&g_test_menu, u8g2_get_instance());

    // === 设置可见行数（根据屏幕大小调整） ===
    menu_set_visible_lines(&g_test_menu, 4); // 128x64屏幕显示4行

    // === 激活菜单 ===
    menu_activate(&g_test_menu);
}

// -----------------------------------------------------------------------------
// 7. 任务函数
// -----------------------------------------------------------------------------
void test_menu_task(void)
{
    // 调用菜单主任务（处理输入+渲染）
    menu_task(&g_test_menu);
}
