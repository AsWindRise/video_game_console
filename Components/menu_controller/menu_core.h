#ifndef __MENU_CORE_H__
#define __MENU_CORE_H__

#include <stdint.h>
#include <stddef.h>

// =============================================================================
// 菜单控制器核心模块 - 多级嵌套菜单系统
// 设计目标：
// 1. 资源受限的嵌入式设备（双向链表，O(1)导航）
// 2. 完全解耦输入和渲染（回调机制）
// 3. 支持动态菜单项（禁用/隐藏/动态值）
// 4. 历史栈管理（记忆每层游标位置）
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 配置参数
// -----------------------------------------------------------------------------
#define MENU_MAX_DEPTH 10     // 最大菜单深度（防止栈溢出）
#define MENU_MAX_LABEL_LEN 32 // 菜单项文本最大长度

// -----------------------------------------------------------------------------
// 2. 菜单项状态枚举
// -----------------------------------------------------------------------------

/**
 * @brief 菜单项状态
 * @note  支持动态禁用和隐藏功能
 */
typedef enum
{
    MENU_ITEM_NORMAL = 0,   /*!< 正常状态（可选中） */
    MENU_ITEM_DISABLED = 1, /*!< 禁用状态（显示但不可选，灰色） */
    MENU_ITEM_HIDDEN = 2    /*!< 隐藏状态（完全不显示） */
} menu_item_state_t;

// -----------------------------------------------------------------------------
// 3. 菜单项类型枚举
// -----------------------------------------------------------------------------

/**
 * @brief 菜单项类型
 * @note  决定按下确认键时的行为
 */
typedef enum
{
    MENU_TYPE_SUBMENU = 0, /*!< 子菜单（进入下一级） */
    MENU_TYPE_ACTION = 1   /*!< 动作项（执行绑定函数） */
} menu_item_type_t;

// -----------------------------------------------------------------------------
// 4. 前向声明
// -----------------------------------------------------------------------------
typedef struct menu_item_t menu_item_t;
typedef struct menu_instance_t menu_instance_t;

// -----------------------------------------------------------------------------
// 5. 回调函数类型定义
// -----------------------------------------------------------------------------

/**
 * @brief 动作函数类型
 * @note  菜单项执行时调用的函数
 */
typedef void (*menu_action_fn)(void);

/**
 * @brief 动态值获取函数类型
 * @return 返回要显示的字符串（如"开"/"关"/"75%"）
 * @note  用于实时显示菜单项的状态值
 */
typedef const char *(*menu_value_fn)(void);

/**
 * @brief 动态状态获取函数类型
 * @return 返回菜单项的状态（正常/禁用/隐藏）
 * @note  用于动态控制菜单项的可见性和可选性
 */
typedef menu_item_state_t (*menu_state_fn)(void);

/**
 * @brief 进入菜单回调函数类型
 * @param menu 菜单实例指针
 * @note  当进入某个菜单项时调用（生命周期钩子）
 */
typedef void (*menu_enter_fn)(menu_instance_t *menu);

/**
 * @brief 退出菜单回调函数类型
 * @param menu 菜单实例指针
 * @note  当离开某个菜单项时调用（生命周期钩子）
 */
typedef void (*menu_exit_fn)(menu_instance_t *menu);

// -----------------------------------------------------------------------------
// 6. 菜单项结构（双向链表节点）
// -----------------------------------------------------------------------------

/**
 * @brief 菜单项结构体（双向链表节点）
 * @note  使用双向链表实现同级菜单项的串联，提供O(1)的UP/DOWN导航性能
 */
struct menu_item_t
{
    // === 基本属性 ===
    const char *label;     /*!< 菜单项文本（必填） */
    menu_item_type_t type; /*!< 菜单项类型（SUBMENU或ACTION） */

    // === 链表指针（双向链表） ===
    menu_item_t *next;     /*!< 下一项（同级，用于DOWN键） */
    menu_item_t *prev;     /*!< 上一项（同级，用于UP键，O(1)性能） */
    menu_item_t *sub_menu; /*!< 子菜单头指针（type=SUBMENU时有效） */

    // === 回调函数 ===
    menu_action_fn action;   /*!< 动作函数（type=ACTION时有效） */
    menu_value_fn get_value; /*!< 动态值获取函数（可选，显示"标签：值"） */
    menu_state_fn get_state; /*!< 动态状态获取函数（可选，默认NORMAL） */

    // === 生命周期钩子（可选） ===
    menu_enter_fn on_enter; /*!< 进入此菜单项时的回调 */
    menu_exit_fn on_exit;   /*!< 离开此菜单项时的回调 */

    // === 扩展属性 ===
    const uint8_t *icon; /*!< 图标位图指针（可选，未来扩展） */
    void *user_data;     /*!< 用户自定义数据指针（可选） */
};

// -----------------------------------------------------------------------------
// 7. 输入回调类型
// -----------------------------------------------------------------------------

/**
 * @brief 输入回调函数类型
 * @return 返回1表示按键按下，返回0表示未按下
 * @note  用于解耦input_manager，支持自定义输入源
 */
typedef uint8_t (*menu_input_fn)(void);

// -----------------------------------------------------------------------------
// 8. 渲染回调类型
// -----------------------------------------------------------------------------

/**
 * @brief 渲染回调函数类型
 * @param menu 菜单实例指针
 * @note  用于解耦u8g2，支持自定义渲染引擎和UI风格
 */
typedef void (*menu_render_fn)(menu_instance_t *menu);

// -----------------------------------------------------------------------------
// 9. 菜单实例结构
// -----------------------------------------------------------------------------

/**
 * @brief 菜单实例结构体
 * @note  管理菜单的导航状态、历史栈、输入和渲染
 */
struct menu_instance_t
{
    // === 当前状态 ===
    menu_item_t *current_menu_head; /*!< 当前菜单的头指针 */
    menu_item_t *current_selected;  /*!< 当前选中的菜单项指针 */
    uint8_t current_index;          /*!< 当前选中索引（0-based，排除隐藏项） */
    uint8_t total_items;            /*!< 当前层级菜单项总数（排除隐藏项） */

    // === 显示窗口（滚动管理） ===
    uint8_t display_offset; /*!< 滚动窗口起始索引 */
    uint8_t visible_lines;  /*!< 屏幕可显示行数（默认5） */

    // === 历史栈（LIFO，用于返回功能） ===
    menu_item_t *history_menu[MENU_MAX_DEPTH];   /*!< 父菜单头指针栈 */
    menu_item_t *history_cursor[MENU_MAX_DEPTH]; /*!< 父菜单选中项栈（记忆位置） */
    uint8_t stack_depth;                         /*!< 当前栈深度 */

    // === 输入回调（用户绑定） ===
    menu_input_fn get_up;      /*!< 获取UP键状态的回调函数 */
    menu_input_fn get_down;    /*!< 获取DOWN键状态的回调函数 */
    menu_input_fn get_confirm; /*!< 获取确认键状态的回调函数 */
    menu_input_fn get_back;    /*!< 获取返回键状态的回调函数 */

    // === 渲染回调（用户绑定） ===
    menu_render_fn render; /*!< 自定义渲染函数 */
    void *render_context;  /*!< 渲染上下文（如u8g2指针） */

    // === 状态标志 ===
    uint8_t is_active;    /*!< 菜单是否激活（1=激活，0=暂停） */
    uint8_t need_refresh; /*!< 是否需要刷新显示（脏标记） */

    // === 输入去抖（防止连续触发） ===
    uint8_t last_up;      /*!< 上次UP键状态 */
    uint8_t last_down;    /*!< 上次DOWN键状态 */
    uint8_t last_confirm; /*!< 上次确认键状态 */
    uint8_t last_back;    /*!< 上次返回键状态 */
};

// =============================================================================
// API 函数声明
// =============================================================================

// -----------------------------------------------------------------------------
// 初始化与配置
// -----------------------------------------------------------------------------

/**
 * @brief 初始化菜单实例
 * @param menu      菜单实例指针
 * @param root_menu 根菜单的头指针
 * @note  初始化后需要调用 menu_set_input_callbacks 和 menu_set_render_callback
 */
void menu_init(menu_instance_t *menu, menu_item_t *root_menu);

/**
 * @brief 设置可见行数
 * @param menu  菜单实例指针
 * @param lines 屏幕可显示的行数（如5行）
 * @note  用于滚动窗口管理，默认5行
 */
void menu_set_visible_lines(menu_instance_t *menu, uint8_t lines);

/**
 * @brief 设置输入回调函数
 * @param menu         菜单实例指针
 * @param get_up       获取UP键状态的函数
 * @param get_down     获取DOWN键状态的函数
 * @param get_confirm  获取确认键状态的函数
 * @param get_back     获取返回键状态的函数
 * @note  每个函数返回1表示按下，返回0表示未按下
 */
void menu_set_input_callbacks(menu_instance_t *menu,
                              menu_input_fn get_up,
                              menu_input_fn get_down,
                              menu_input_fn get_confirm,
                              menu_input_fn get_back);

/**
 * @brief 设置渲染回调函数
 * @param menu           菜单实例指针
 * @param render         自定义渲染函数
 * @param render_context 渲染上下文（如u8g2指针）
 * @note  渲染函数会在菜单状态改变时自动调用
 */
void menu_set_render_callback(menu_instance_t *menu,
                              menu_render_fn render,
                              void *render_context);

// -----------------------------------------------------------------------------
// 运行时控制
// -----------------------------------------------------------------------------

/**
 * @brief 菜单主任务（处理输入+渲染）
 * @param menu 菜单实例指针
 * @note  在主循环中调用，建议10ms周期
 */
void menu_task(menu_instance_t *menu);

/**
 * @brief 处理输入（仅输入处理）
 * @param menu 菜单实例指针
 * @note  如果需要分离输入和渲染，可单独调用此函数
 */
void menu_handle_input(menu_instance_t *menu);

/**
 * @brief 渲染菜单（仅渲染）
 * @param menu 菜单实例指针
 * @note  如果需要分离输入和渲染，可单独调用此函数
 */
void menu_render(menu_instance_t *menu);

/**
 * @brief 激活菜单
 * @param menu 菜单实例指针
 * @note  激活后菜单才会处理输入和渲染
 */
void menu_activate(menu_instance_t *menu);

/**
 * @brief 暂停菜单
 * @param menu 菜单实例指针
 * @note  暂停后菜单不会处理输入和渲染
 */
void menu_deactivate(menu_instance_t *menu);

/**
 * @brief 强制刷新显示
 * @param menu 菜单实例指针
 * @note  用于动态值更新后手动触发刷新
 */
void menu_force_refresh(menu_instance_t *menu);

// -----------------------------------------------------------------------------
// 导航控制（可手动调用）
// -----------------------------------------------------------------------------

/**
 * @brief 向上导航
 * @param menu 菜单实例指针
 * @note  手动调用，通常由menu_handle_input内部调用
 */
void menu_navigate_up(menu_instance_t *menu);

/**
 * @brief 向下导航
 * @param menu 菜单实例指针
 * @note  手动调用，通常由menu_handle_input内部调用
 */
void menu_navigate_down(menu_instance_t *menu);

/**
 * @brief 确认/进入
 * @param menu 菜单实例指针
 * @note  手动调用，通常由menu_handle_input内部调用
 */
void menu_navigate_confirm(menu_instance_t *menu);

/**
 * @brief 返回/退出
 * @param menu 菜单实例指针
 * @note  手动调用，通常由menu_handle_input内部调用
 */
void menu_navigate_back(menu_instance_t *menu);

// -----------------------------------------------------------------------------
// 查询接口
// -----------------------------------------------------------------------------

/**
 * @brief 获取当前选中的菜单项
 * @param menu 菜单实例指针
 * @return 当前选中的菜单项指针
 */
menu_item_t *menu_get_current_item(menu_instance_t *menu);

/**
 * @brief 获取当前菜单深度
 * @param menu 菜单实例指针
 * @return 当前菜单深度（0表示根菜单）
 */
uint8_t menu_get_depth(menu_instance_t *menu);

/**
 * @brief 判断是否在根菜单
 * @param menu 菜单实例指针
 * @return 1表示在根菜单，0表示在子菜单
 */
uint8_t menu_is_at_root(menu_instance_t *menu);

#endif // __MENU_CORE_H__
