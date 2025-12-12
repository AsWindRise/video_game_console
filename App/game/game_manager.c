#include "game_manager.h"

// =============================================================================
// 游戏管理器实现
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 内部数据结构
// -----------------------------------------------------------------------------

#define MAX_GAMES 12  /*!< 最大支持游戏数量（预留扩展空间） */

/**
 * @brief 游戏管理器状态
 */
typedef struct {
    const game_descriptor_t *registry[MAX_GAMES];  /*!< 游戏注册表 */
    uint8_t game_count;                            /*!< 已注册游戏数量 */
    const game_descriptor_t *current_game;         /*!< 当前运行的游戏 */
} game_manager_t;

// 游戏管理器全局实例
static game_manager_t g_game_manager;

// -----------------------------------------------------------------------------
// 2. 内部函数（游戏退出统一回调）
// -----------------------------------------------------------------------------

/**
 * @brief 游戏退出统一回调（由具体游戏调用）
 * @note  所有游戏的退出回调都会指向这个函数
 */
static void game_exit_callback(void)
{
    game_manager_exit_current_game();
}

// -----------------------------------------------------------------------------
// 3. API函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化游戏管理器
 */
void game_manager_init(void)
{
    memset(&g_game_manager, 0, sizeof(game_manager_t));
    g_game_manager.game_count = 0;
    g_game_manager.current_game = NULL;
}

/**
 * @brief 注册游戏
 * @param descriptor: 游戏描述符指针
 * @return 0=成功，-1=注册表已满
 */
int game_manager_register(const game_descriptor_t *descriptor)
{
    // 检查注册表是否已满
    if (g_game_manager.game_count >= MAX_GAMES)
    {
        return -1;  // 注册表已满
    }

    // 注册游戏
    g_game_manager.registry[g_game_manager.game_count] = descriptor;
    g_game_manager.game_count++;

    return 0;
}

/**
 * @brief 启动游戏（根据游戏名称）
 * @param game_name: 游戏名称（如"Snake"）
 * @return 0=成功，-1=游戏未找到
 */
int game_manager_start_game(const char *game_name)
{
    // 查找游戏
    const game_descriptor_t *game = NULL;
    for (uint8_t i = 0; i < g_game_manager.game_count; i++)
    {
        if (strcmp(g_game_manager.registry[i]->name, game_name) == 0)
        {
            game = g_game_manager.registry[i];
            break;
        }
    }

    // 游戏未找到
    if (game == NULL)
    {
        return -1;
    }

    // === 场景切换流程 ===

    // 1. 停用菜单
    main_menu_deactivate();

    // 2. 清空输入状态和事件队列（避免菜单残留事件影响游戏）
    input_manager_clear();
    event_queue_clear();

    // 3. 初始化游戏
    if (game->interface.init != NULL)
    {
        game->interface.init(game->instance);
    }

    // 4. 设置退出回调
    if (game->interface.set_exit_callback != NULL)
    {
        game->interface.set_exit_callback(game->instance, game_exit_callback);
    }

    // 5. 激活游戏
    if (game->interface.activate != NULL)
    {
        game->interface.activate(game->instance);
    }

    // 6. 记录当前游戏
    g_game_manager.current_game = game;

    return 0;
}

/**
 * @brief 退出当前游戏
 */
void game_manager_exit_current_game(void)
{
    // 检查是否有游戏在运行
    if (g_game_manager.current_game == NULL)
    {
        return;
    }

    const game_descriptor_t *game = g_game_manager.current_game;

    // === 场景切换流程 ===

    // 1. 停用游戏
    if (game->interface.deactivate != NULL)
    {
        game->interface.deactivate(game->instance);
    }

    // 2. 清空输入状态和事件队列（避免游戏残留事件影响菜单）
    input_manager_clear();
    event_queue_clear();

    // 3. 激活菜单
    main_menu_activate();

    // 4. 清除当前游戏记录
    g_game_manager.current_game = NULL;
}

/**
 * @brief 获取当前运行的游戏
 * @return 当前游戏描述符指针，NULL表示无游戏运行
 */
const game_descriptor_t* game_manager_get_current_game(void)
{
    return g_game_manager.current_game;
}

/**
 * @brief 调用所有游戏的任务函数
 * @note  在调度器中注册，10ms周期调用
 */
void game_manager_task_all(void)
{
    // 遍历所有注册的游戏，调用它们的task函数
    // 游戏内部会检查is_active标志，决定是否执行逻辑
    for (uint8_t i = 0; i < g_game_manager.game_count; i++)
    {
        const game_descriptor_t *game = g_game_manager.registry[i];
        if (game->interface.task != NULL)
        {
            game->interface.task(game->instance);
        }
    }
}
