#ifndef __MENU_BUILDER_H__
#define __MENU_BUILDER_H__

#include "menu_core.h"

// =============================================================================
// 菜单构建器 - 快速构建菜单的辅助工具
// 设计目标：
// 1. 提供宏定义简化菜单项声明
// 2. 提供函数自动链接菜单项为双向链表
// 3. 提供辅助函数简化父子菜单关系设置
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 辅助宏定义
// -----------------------------------------------------------------------------

/**
 * @brief 定义一个普通菜单项（子菜单类型）
 * @param name       变量名
 * @param label_text 显示文本
 * @note  使用示例：MENU_ITEM(menu_games, "游戏")
 */
#define MENU_ITEM(name, label_text)                                            \
    menu_item_t name = {.label      = label_text,                              \
                        .type       = MENU_TYPE_SUBMENU,                       \
                        .next       = NULL,                                    \
                        .prev       = NULL,                                    \
                        .sub_menu   = NULL,                                    \
                        .action     = NULL,                                    \
                        .get_value  = NULL,                                    \
                        .get_state  = NULL,                                    \
                        .on_enter   = NULL,                                    \
                        .on_exit    = NULL,                                    \
                        .icon       = NULL,                                    \
                        .user_data  = NULL}

/**
 * @brief 定义一个动作菜单项（执行函数类型）
 * @param name       变量名
 * @param label_text 显示文本
 * @param action_fn  动作函数指针
 * @note  使用示例：MENU_ACTION(menu_start, "开始游戏", action_start_game)
 */
#define MENU_ACTION(name, label_text, action_fn)                               \
    menu_item_t name = {.label      = label_text,                              \
                        .type       = MENU_TYPE_ACTION,                        \
                        .next       = NULL,                                    \
                        .prev       = NULL,                                    \
                        .sub_menu   = NULL,                                    \
                        .action     = action_fn,                               \
                        .get_value  = NULL,                                    \
                        .get_state  = NULL,                                    \
                        .on_enter   = NULL,                                    \
                        .on_exit    = NULL,                                    \
                        .icon       = NULL,                                    \
                        .user_data  = NULL}

/**
 * @brief 定义一个带动态值显示的动作菜单项
 * @param name       变量名
 * @param label_text 显示文本
 * @param action_fn  动作函数指针
 * @param value_fn   动态值获取函数指针
 * @note  使用示例：MENU_ACTION_WITH_VALUE(menu_vol, "音量", action_vol, get_vol)
 */
#define MENU_ACTION_WITH_VALUE(name, label_text, action_fn, value_fn)          \
    menu_item_t name = {.label      = label_text,                              \
                        .type       = MENU_TYPE_ACTION,                        \
                        .next       = NULL,                                    \
                        .prev       = NULL,                                    \
                        .sub_menu   = NULL,                                    \
                        .action     = action_fn,                               \
                        .get_value  = value_fn,                                \
                        .get_state  = NULL,                                    \
                        .on_enter   = NULL,                                    \
                        .on_exit    = NULL,                                    \
                        .icon       = NULL,                                    \
                        .user_data  = NULL}

/**
 * @brief 定义一个带动态状态的动作菜单项
 * @param name       变量名
 * @param label_text 显示文本
 * @param action_fn  动作函数指针
 * @param state_fn   动态状态获取函数指针
 * @note  使用示例：MENU_ACTION_WITH_STATE(menu_save, "继续游戏", action_continue, get_save_state)
 */
#define MENU_ACTION_WITH_STATE(name, label_text, action_fn, state_fn)          \
    menu_item_t name = {.label      = label_text,                              \
                        .type       = MENU_TYPE_ACTION,                        \
                        .next       = NULL,                                    \
                        .prev       = NULL,                                    \
                        .sub_menu   = NULL,                                    \
                        .action     = action_fn,                               \
                        .get_value  = NULL,                                    \
                        .get_state  = state_fn,                                \
                        .on_enter   = NULL,                                    \
                        .on_exit    = NULL,                                    \
                        .icon       = NULL,                                    \
                        .user_data  = NULL}

// -----------------------------------------------------------------------------
// 2. 构建器函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 将菜单项数组自动链接为双向链表
 * @param items 菜单项指针数组
 * @param count 数组元素个数
 * @note  自动设置每个菜单项的prev和next指针
 * @note  使用示例：
 *        menu_item_t *items[] = {&menu1, &menu2, &menu3};
 *        menu_link_items(items, 3);
 */
void menu_link_items(menu_item_t *items[], uint8_t count);

/**
 * @brief 设置父菜单项的子菜单
 * @param parent     父菜单项指针
 * @param child_head 子菜单头指针
 * @note  用于建立菜单的层级关系
 * @note  使用示例：menu_set_submenu(&menu_main_games, &menu_game_snake);
 */
void menu_set_submenu(menu_item_t *parent, menu_item_t *child_head);

/**
 * @brief 设置菜单项的动作函数
 * @param item   菜单项指针
 * @param action 动作函数指针
 * @note  用于动态修改菜单项的动作函数
 */
void menu_set_action(menu_item_t *item, menu_action_fn action);

/**
 * @brief 设置菜单项的动态值函数
 * @param item      菜单项指针
 * @param get_value 动态值获取函数指针
 * @note  用于动态修改菜单项的值显示函数
 */
void menu_set_value_fn(menu_item_t *item, menu_value_fn get_value);

/**
 * @brief 设置菜单项的动态状态函数
 * @param item      菜单项指针
 * @param get_state 动态状态获取函数指针
 * @note  用于动态修改菜单项的状态函数
 */
void menu_set_state_fn(menu_item_t *item, menu_state_fn get_state);

/**
 * @brief 设置菜单项的进入回调
 * @param item     菜单项指针
 * @param on_enter 进入回调函数指针
 * @note  用于设置菜单项的生命周期钩子
 */
void menu_set_enter_callback(menu_item_t *item, menu_enter_fn on_enter);

/**
 * @brief 设置菜单项的退出回调
 * @param item    菜单项指针
 * @param on_exit 退出回调函数指针
 * @note  用于设置菜单项的生命周期钩子
 */
void menu_set_exit_callback(menu_item_t *item, menu_exit_fn on_exit);

#endif // __MENU_BUILDER_H__
