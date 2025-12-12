#include "main_menu.h"

// =============================================================================
// 主菜单实现
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 全局变量
// -----------------------------------------------------------------------------
static menu_instance_t g_main_menu;    // 主菜单实例
static uint8_t         g_volume = 75;  // 音量设置（0-100）
static uint8_t         g_brightness = 80; // 亮度设置（0-100）
static uint8_t         g_sound_on = 1; // 音效开关（1=开，0=关）

// -----------------------------------------------------------------------------
// 2. 游戏启动动作函数（使用game_manager统一管理）
// -----------------------------------------------------------------------------

/**
 * @brief 启动贪吃蛇游戏
 * @note  通过game_manager启动游戏，所有场景切换逻辑由game_manager处理
 */
void action_start_snake(void)
{
    game_manager_start_game("Snake");
}

/**
 * @brief 启动恐龙跑酷游戏
 * @note  通过game_manager启动游戏
 */
void action_start_dino(void)
{
    game_manager_start_game("Dino");
}

/**
 * @brief 启动打飞机游戏
 * @note  通过game_manager启动游戏
 */
void action_start_plane(void)
{
    game_manager_start_game("Plane");
}

/**
 * @brief 启动俄罗斯方块游戏
 * @note  通过game_manager启动游戏
 */
void action_start_tetris(void)
{
    game_manager_start_game("Tetris");
}

/**
 * @brief 启动打砖块游戏
 * @note  通过game_manager启动游戏
 */
void action_start_breakout(void)
{
    game_manager_start_game("Breakout");
}

/**
 * @brief 启动乒乓球游戏
 * @note  通过game_manager启动游戏
 */
void action_start_pong(void)
{
    game_manager_start_game("Pong");
}

/**
 * @brief 启动推箱子游戏
 * @note  通过game_manager启动游戏
 */
void action_start_sokoban(void)
{
    game_manager_start_game("Sokoban");
}

/**
 * @brief 启动扫雷游戏
 * @note  通过game_manager启动游戏
 */
void action_start_minesweeper(void)
{
    game_manager_start_game("Minesweeper");
}

/**
 * @brief 启动吃豆人游戏
 * @note  通过game_manager启动游戏
 */
void action_start_pacman(void)
{
    game_manager_start_game("Pac-Man");
}

// -----------------------------------------------------------------------------
// 3. 设置调整动作函数
// -----------------------------------------------------------------------------

/**
 * @brief 调整音量
 * @note  每次按A键增加10%，超过100%则归零
 */
void action_adjust_volume(void)
{
    g_volume += 10;
    if (g_volume > 100)
    {
        g_volume = 0;
    }

    // TODO: 调用系统音量控制API
    // system_set_volume(g_volume);

    // 刷新菜单显示
    menu_force_refresh(&g_main_menu);
}

/**
 * @brief 切换音效开关
 */
void action_toggle_sound(void)
{
    g_sound_on = !g_sound_on;

    // TODO: 调用系统音效控制API
    // system_set_sound_enabled(g_sound_on);

    // 刷新菜单显示
    menu_force_refresh(&g_main_menu);
}

/**
 * @brief 调整亮度
 * @note  每次按A键增加10%，超过100%则归零
 */
void action_adjust_brightness(void)
{
    g_brightness += 10;
    if (g_brightness > 100)
    {
        g_brightness = 0;
    }

    // TODO: 调用系统亮度控制API
    // system_set_brightness(g_brightness);

    // 刷新菜单显示
    menu_force_refresh(&g_main_menu);
}

/**
 * @brief 重置所有设置为默认值
 */
void action_reset_settings(void)
{
    g_volume     = 75;
    g_brightness = 80;
    g_sound_on   = 1;

    // TODO: 调用系统重置API
    // system_reset_settings();

    // 刷新菜单显示
    menu_force_refresh(&g_main_menu);
}

// -----------------------------------------------------------------------------
// 4. 其他动作函数
// -----------------------------------------------------------------------------

/**
 * @brief 显示关于信息
 * @note  暂时不做任何操作
 */
void action_about(void)
{
    // TODO: 未来可以实现一个关于页面
    // 暂时不做任何操作
}

// -----------------------------------------------------------------------------
// 5. 动态值显示函数
// -----------------------------------------------------------------------------

/**
 * @brief 获取音量显示值
 * @return 格式化的音量字符串（如"75%"）
 */
const char *get_volume_value(void)
{
    static char buf[8];
    sprintf(buf, "%d%%", g_volume);
    return buf;
}

/**
 * @brief 获取音效开关显示值
 * @return "ON"或"OFF"
 */
const char *get_sound_value(void)
{
    return g_sound_on ? "ON" : "OFF";
}

/**
 * @brief 获取亮度显示值
 * @return 格式化的亮度字符串（如"80%"）
 */
const char *get_brightness_value(void)
{
    static char buf[8];
    sprintf(buf, "%d%%", g_brightness);
    return buf;
}

// -----------------------------------------------------------------------------
// 6. 菜单项定义
// -----------------------------------------------------------------------------

// === 游戏子菜单 ===
MENU_ACTION(menu_game_snake, "Snake", action_start_snake);
MENU_ACTION(menu_game_dino, "Dino Run", action_start_dino);
MENU_ACTION(menu_game_plane, "Plane War", action_start_plane);
MENU_ACTION(menu_game_tetris, "Tetris", action_start_tetris);
MENU_ACTION(menu_game_breakout, "Breakout", action_start_breakout);
MENU_ACTION(menu_game_sokoban, "Sokoban", action_start_sokoban);
MENU_ACTION(menu_game_minesweeper, "Minesweeper", action_start_minesweeper);
MENU_ACTION(menu_game_pacman, "Pac-Man", action_start_pacman);
MENU_ACTION(menu_game_pong, "Pong", action_start_pong);

// === 设置子菜单 ===
MENU_ACTION_WITH_VALUE(menu_set_volume, "Volume", action_adjust_volume,
                        get_volume_value);
MENU_ACTION_WITH_VALUE(menu_set_sound, "Sound", action_toggle_sound,
                        get_sound_value);
MENU_ACTION_WITH_VALUE(menu_set_brightness, "Brightness",
                        action_adjust_brightness, get_brightness_value);
MENU_ACTION(menu_set_reset, "Reset", action_reset_settings);

// === 主菜单 ===
MENU_ITEM(menu_main_games, "Games");
MENU_ITEM(menu_main_settings, "Settings");
MENU_ACTION(menu_main_about, "About", action_about);

// -----------------------------------------------------------------------------
// 7. 初始化函数
// -----------------------------------------------------------------------------

/**
 * @brief 初始化主菜单
 */
void main_menu_init(void)
{
    // === 构建游戏子菜单 ===
    menu_item_t *game_items[] = {&menu_game_snake, &menu_game_dino,
                                  &menu_game_plane, &menu_game_tetris,
                                  &menu_game_breakout, &menu_game_sokoban,
                                  &menu_game_minesweeper, &menu_game_pacman,
                                  &menu_game_pong};
    menu_link_items(game_items, 9);
    menu_set_submenu(&menu_main_games, &menu_game_snake);

    // === 构建设置子菜单 ===
    menu_item_t *setting_items[] = {&menu_set_volume, &menu_set_sound,
                                     &menu_set_brightness, &menu_set_reset};
    menu_link_items(setting_items, 4);
    menu_set_submenu(&menu_main_settings, &menu_set_volume);

    // === 构建主菜单 ===
    menu_item_t *main_items[] = {&menu_main_games, &menu_main_settings,
                                  &menu_main_about};
    menu_link_items(main_items, 3);

    // === 初始化菜单实例 ===
    menu_init(&g_main_menu, &menu_main_games);

    // === 使用适配器自动绑定输入和渲染 ===
    menu_adapter_init(&g_main_menu, u8g2_get_instance());

    // === 设置可见行数（根据屏幕大小调整） ===
    // 屏幕64像素 - 标题12像素 = 52像素可用，行高13像素，可显示3-4行
    menu_set_visible_lines(&g_main_menu, 3); // 128x64屏幕显示3行

    // === 激活菜单 ===
    menu_activate(&g_main_menu);
}

// -----------------------------------------------------------------------------
// 8. 任务函数
// -----------------------------------------------------------------------------

/**
 * @brief 主菜单任务
 * @note  10ms周期调用
 */
void main_menu_task(void)
{
    menu_task(&g_main_menu);
}

// -----------------------------------------------------------------------------
// 9. 控制接口
// -----------------------------------------------------------------------------

/**
 * @brief 激活主菜单
 * @note  从游戏返回时调用，激活后再次清空输入状态，防止残留输入导致游标乱动
 */
void main_menu_activate(void)
{
    menu_activate(&g_main_menu);

    // 【关键】激活后再次清空输入状态和事件队列
    // 原因：game_manager清空后，驱动任务（ebtn/rocker）又产生了新事件
    // 这里再清空一次，确保菜单不会读到残留的输入状态
    input_manager_clear();
    event_queue_clear();
}

/**
 * @brief 暂停主菜单
 */
void main_menu_deactivate(void)
{
    menu_deactivate(&g_main_menu);
}

/**
 * @brief 强制刷新主菜单显示
 */
void main_menu_refresh(void)
{
    menu_force_refresh(&g_main_menu);
}
