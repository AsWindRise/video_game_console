                                                                #ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "mydefine.h"

// =============================================================================
// 游戏管理器 - 统一游戏接口和生命周期管理
// 设计目标：
// 1. 定义统一的游戏接口，所有游戏都遵循相同的生命周期
// 2. 提供游戏注册机制，方便添加新游戏
// 3. 封装游戏启动/退出逻辑，避免代码散落各处
// 4. 协调游戏与菜单的场景切换
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 游戏接口定义
// -----------------------------------------------------------------------------

/**
 * @brief 统一游戏接口（所有游戏必须实现这些函数）
 * @note  使用函数指针实现多态，类似C++的虚函数表
 */
typedef struct {
    /**
     * @brief 初始化游戏
     * @param instance: 游戏实例指针（void*，由具体游戏强制转换）
     * @note  设置游戏初始状态，生成初始元素（如食物）
     */
    void (*init)(void *instance);

    /**
     * @brief 激活游戏
     * @param instance: 游戏实例指针
     * @note  从菜单进入游戏时调用，使游戏开始运行
     */
    void (*activate)(void *instance);

    /**
     * @brief 停用游戏
     * @param instance: 游戏实例指针
     * @note  返回菜单时调用，使游戏停止运行
     */
    void (*deactivate)(void *instance);

    /**
     * @brief 游戏任务函数
     * @param instance: 游戏实例指针
     * @note  10ms周期调用，处理输入+逻辑+渲染
     */
    void (*task)(void *instance);

    /**
     * @brief 设置退出回调
     * @param instance: 游戏实例指针
     * @param callback: 退出回调函数指针（void (*callback)(void)）
     * @note  游戏内按退出键时调用此回调返回菜单
     */
    void (*set_exit_callback)(void *instance, void (*callback)(void));
} game_interface_t;

// -----------------------------------------------------------------------------
// 2. 游戏描述符
// -----------------------------------------------------------------------------

/**
 * @brief 游戏描述符（描述一个游戏的元信息和接口）
 */
typedef struct {
    const char *name;           /*!< 游戏名称（如"Snake"、"Tetris"）*/
    void *instance;             /*!< 游戏实例指针（指向具体游戏的状态结构体）*/
    game_interface_t interface; /*!< 游戏接口实现（函数指针表）*/
} game_descriptor_t;

// -----------------------------------------------------------------------------
// 3. API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化游戏管理器
 * @note  在system_assembly_init中调用，必须在所有游戏注册之前调用
 */
void game_manager_init(void);

/**
 * @brief 注册游戏
 * @param descriptor: 游戏描述符指针
 * @return 0=成功，-1=注册表已满
 * @note  应在system_assembly_init中调用，注册所有可用游戏
 */
int game_manager_register(const game_descriptor_t *descriptor);

/**
 * @brief 启动游戏（根据游戏名称）
 * @param game_name: 游戏名称（如"Snake"）
 * @return 0=成功，-1=游戏未找到
 * @note  从菜单启动游戏时调用，会自动：
 *        1. 停用菜单
 *        2. 清空输入状态和事件队列
 *        3. 初始化游戏
 *        4. 设置退出回调
 *        5. 激活游戏
 */
int game_manager_start_game(const char *game_name);

/**
 * @brief 退出当前游戏
 * @note  游戏内部按退出键时调用，会自动：
 *        1. 停用游戏
 *        2. 清空输入状态和事件队列
 *        3. 激活菜单
 */
void game_manager_exit_current_game(void);

/**
 * @brief 获取当前运行的游戏
 * @return 当前游戏描述符指针，NULL表示无游戏运行
 */
const game_descriptor_t* game_manager_get_current_game(void);

/**
 * @brief 调用所有游戏的任务函数
 * @note  在调度器中注册，10ms周期调用
 *        内部会调用所有注册游戏的task函数，游戏自己检查is_active决定是否执行
 */
void game_manager_task_all(void);

// -----------------------------------------------------------------------------
// 4. 辅助宏定义（简化游戏注册代码）
// -----------------------------------------------------------------------------

/**
 * @brief 定义游戏适配器宏（自动生成适配函数）
 * @param game_name: 游戏名称前缀（如snake_game）
 * @param game_type: 游戏类型（如snake_game_t）
 * @note  使用示例：
 *        GAME_ADAPTER(snake_game, snake_game_t)
 *        会生成：
 *        - snake_game_adapter_init()
 *        - snake_game_adapter_activate()
 *        - snake_game_adapter_deactivate()
 *        - snake_game_adapter_task()
 *        - snake_game_adapter_set_exit_callback()
 */
#define GAME_ADAPTER(game_name, game_type)                                     \
    static void game_name##_adapter_init(void *instance)                       \
    {                                                                          \
        game_name##_init((game_type *)instance);                               \
    }                                                                          \
    static void game_name##_adapter_activate(void *instance)                   \
    {                                                                          \
        game_name##_activate((game_type *)instance);                           \
    }                                                                          \
    static void game_name##_adapter_deactivate(void *instance)                 \
    {                                                                          \
        game_name##_deactivate((game_type *)instance);                         \
    }                                                                          \
    static void game_name##_adapter_task(void *instance)                       \
    {                                                                          \
        game_name##_task((game_type *)instance);                               \
    }                                                                          \
    static void game_name##_adapter_set_exit_callback(void *instance,          \
                                                       void (*callback)(void)) \
    {                                                                          \
        game_name##_set_exit_callback((game_type *)instance,                   \
                                      (void (*)(void))callback);               \
    }

/**
 * @brief 定义游戏描述符宏（简化注册代码）
 * @param game_var: 游戏实例变量
 * @param game_name_str: 游戏名称字符串（如"Snake"）
 * @param game_prefix: 游戏函数前缀（如snake_game）
 * @note  使用示例：
 *        static snake_game_t g_snake_game;
 *        GAME_ADAPTER(snake_game, snake_game_t)
 *        GAME_DESCRIPTOR(g_snake_game, "Snake", snake_game)
 *        会生成一个game_descriptor_t结构体变量：g_snake_game_descriptor
 */
#define GAME_DESCRIPTOR(game_var, game_name_str, game_prefix)                 \
    static const game_descriptor_t game_var##_descriptor = {                   \
        .name = game_name_str,                                                 \
        .instance = (void *)&game_var,                                         \
        .interface = {                                                         \
            .init = game_prefix##_adapter_init,                                \
            .activate = game_prefix##_adapter_activate,                        \
            .deactivate = game_prefix##_adapter_deactivate,                    \
            .task = game_prefix##_adapter_task,                                \
            .set_exit_callback = game_prefix##_adapter_set_exit_callback,      \
        }                                                                      \
    }

#endif // __GAME_MANAGER_H__
