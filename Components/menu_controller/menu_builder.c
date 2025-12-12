#include "menu_builder.h"

// =============================================================================
// 菜单构建器实现
// =============================================================================

/**
 * @brief 将菜单项数组自动链接为双向链表
 */
void menu_link_items(menu_item_t *items[], uint8_t count)
{
    if (items == NULL || count == 0)
    {
        return;
    }

    // 遍历数组，设置prev和next指针
    for (uint8_t i = 0; i < count; i++)
    {
        if (items[i] == NULL)
        {
            continue;
        }

        // 设置prev指针（指向前一个元素）
        if (i > 0)
        {
            items[i]->prev = items[i - 1];
        }
        else
        {
            items[i]->prev = NULL; // 第一个元素prev为NULL
        }

        // 设置next指针（指向后一个元素）
        if (i < count - 1)
        {
            items[i]->next = items[i + 1];
        }
        else
        {
            items[i]->next = NULL; // 最后一个元素next为NULL
        }
    }
}

/**
 * @brief 设置父菜单项的子菜单
 */
void menu_set_submenu(menu_item_t *parent, menu_item_t *child_head)
{
    if (parent == NULL || child_head == NULL)
    {
        return;
    }

    parent->type     = MENU_TYPE_SUBMENU;
    parent->sub_menu = child_head;
}

/**
 * @brief 设置菜单项的动作函数
 */
void menu_set_action(menu_item_t *item, menu_action_fn action)
{
    if (item == NULL)
    {
        return;
    }

    item->type   = MENU_TYPE_ACTION;
    item->action = action;
}

/**
 * @brief 设置菜单项的动态值函数
 */
void menu_set_value_fn(menu_item_t *item, menu_value_fn get_value)
{
    if (item == NULL)
    {
        return;
    }

    item->get_value = get_value;
}

/**
 * @brief 设置菜单项的动态状态函数
 */
void menu_set_state_fn(menu_item_t *item, menu_state_fn get_state)
{
    if (item == NULL)
    {
        return;
    }

    item->get_state = get_state;
}

/**
 * @brief 设置菜单项的进入回调
 */
void menu_set_enter_callback(menu_item_t *item, menu_enter_fn on_enter)
{
    if (item == NULL)
    {
        return;
    }

    item->on_enter = on_enter;
}

/**
 * @brief 设置菜单项的退出回调
 */
void menu_set_exit_callback(menu_item_t *item, menu_exit_fn on_exit)
{
    if (item == NULL)
    {
        return;
    }

    item->on_exit = on_exit;
}
