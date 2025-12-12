#include "menu_core.h"
#include <string.h>

// =============================================================================
// 私有辅助函数声明
// =============================================================================
static uint8_t menu_count_visible_items(menu_item_t *head);
static menu_item_t *menu_get_visible_item_at(menu_item_t *head, uint8_t index);
static menu_item_state_t menu_get_item_state(menu_item_t *item);
static void menu_update_scroll_window(menu_instance_t *menu);
static void menu_enter_level(menu_instance_t *menu, menu_item_t *new_head);

// =============================================================================
// 初始化与配置函数
// =============================================================================

/**
 * @brief 初始化菜单实例
 */
void menu_init(menu_instance_t *menu, menu_item_t *root_menu)
{
    if (menu == NULL || root_menu == NULL)
    {
        return;
    }

    // 清空结构体
    memset(menu, 0, sizeof(menu_instance_t));

    // 初始化当前状态
    menu->current_menu_head = root_menu;
    menu->current_selected  = root_menu;
    menu->current_index     = 0;
    menu->total_items       = menu_count_visible_items(root_menu);

    // 初始化显示窗口
    menu->display_offset = 0;
    menu->visible_lines  = 5; // 默认5行

    // 初始化历史栈
    menu->stack_depth = 0;

    // 初始化状态标志
    menu->is_active    = 0; // 默认未激活
    menu->need_refresh = 1; // 初始需要刷新

    // 输入去抖状态初始化
    menu->last_up      = 0;
    menu->last_down    = 0;
    menu->last_confirm = 0;
    menu->last_back    = 0;
}

/**
 * @brief 设置可见行数
 */
void menu_set_visible_lines(menu_instance_t *menu, uint8_t lines)
{
    if (menu == NULL || lines == 0)
    {
        return;
    }

    menu->visible_lines = lines;
    menu->need_refresh  = 1;
}

/**
 * @brief 设置输入回调函数
 */
void menu_set_input_callbacks(menu_instance_t *menu,
                               menu_input_fn    get_up,
                               menu_input_fn    get_down,
                               menu_input_fn    get_confirm,
                               menu_input_fn    get_back)
{
    if (menu == NULL)
    {
        return;
    }

    menu->get_up      = get_up;
    menu->get_down    = get_down;
    menu->get_confirm = get_confirm;
    menu->get_back    = get_back;
}

/**
 * @brief 设置渲染回调函数
 */
void menu_set_render_callback(menu_instance_t *menu,
                               menu_render_fn   render,
                               void            *render_context)
{
    if (menu == NULL)
    {
        return;
    }

    menu->render         = render;
    menu->render_context = render_context;
    menu->need_refresh   = 1;
}

// =============================================================================
// 运行时控制函数
// =============================================================================

/**
 * @brief 菜单主任务（处理输入+渲染）
 */
void menu_task(menu_instance_t *menu)
{
    if (menu == NULL || !menu->is_active)
    {
        return;
    }

    // 处理输入
    menu_handle_input(menu);

    // 渲染显示
    menu_render(menu);
}

/**
 * @brief 处理输入
 */
void menu_handle_input(menu_instance_t *menu)
{
    if (menu == NULL || !menu->is_active)
    {
        return;
    }

    // 检查回调函数是否已绑定
    if (menu->get_up == NULL || menu->get_down == NULL ||
        menu->get_confirm == NULL || menu->get_back == NULL)
    {
        return;
    }

    // 读取当前输入状态
    uint8_t up      = menu->get_up();
    uint8_t down    = menu->get_down();
    uint8_t confirm = menu->get_confirm();
    uint8_t back    = menu->get_back();

    // UP键：边缘触发（防止连续触发）
    if (up && !menu->last_up)
    {
        menu_navigate_up(menu);
    }

    // DOWN键：边缘触发
    if (down && !menu->last_down)
    {
        menu_navigate_down(menu);
    }

    // 确认键：边缘触发
    if (confirm && !menu->last_confirm)
    {
        menu_navigate_confirm(menu);
    }

    // 返回键：边缘触发
    if (back && !menu->last_back)
    {
        menu_navigate_back(menu);
    }

    // 更新上次状态（去抖）
    menu->last_up      = up;
    menu->last_down    = down;
    menu->last_confirm = confirm;
    menu->last_back    = back;
}

/**
 * @brief 渲染菜单
 */
void menu_render(menu_instance_t *menu)
{
    if (menu == NULL || !menu->is_active)
    {
        return;
    }

    // 检查是否需要刷新
    if (!menu->need_refresh)
    {
        return;
    }

    // 调用用户绑定的渲染函数
    if (menu->render != NULL)
    {
        menu->render(menu);
    }

    // 清除刷新标志
    menu->need_refresh = 0;
}

/**
 * @brief 激活菜单
 */
void menu_activate(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    menu->is_active    = 1;
    menu->need_refresh = 1;
}

/**
 * @brief 暂停菜单
 */
void menu_deactivate(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    menu->is_active = 0;
}

/**
 * @brief 强制刷新显示
 */
void menu_force_refresh(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    menu->need_refresh = 1;
}

// =============================================================================
// 导航控制函数
// =============================================================================

/**
 * @brief 向上导航
 */
void menu_navigate_up(menu_instance_t *menu)
{
    if (menu == NULL || menu->current_selected == NULL)
    {
        return;
    }

    menu_item_t *prev_item = menu->current_selected->prev;

    // 如果有前一项，则移动到前一项
    if (prev_item != NULL)
    {
        // 检查前一项的状态（跳过隐藏项）
        menu_item_state_t state = menu_get_item_state(prev_item);
        while (prev_item != NULL && state == MENU_ITEM_HIDDEN)
        {
            prev_item = prev_item->prev;
            if (prev_item != NULL)
            {
                state = menu_get_item_state(prev_item);
            }
        }

        // 如果找到可见项，则移动
        if (prev_item != NULL)
        {
            // 调用离开回调
            if (menu->current_selected->on_exit != NULL)
            {
                menu->current_selected->on_exit(menu);
            }

            menu->current_selected = prev_item;
            if (menu->current_index > 0)
            {
                menu->current_index--;
            }

            // 调用进入回调
            if (menu->current_selected->on_enter != NULL)
            {
                menu->current_selected->on_enter(menu);
            }

            menu_update_scroll_window(menu);
            menu->need_refresh = 1;
            return;
        }
    }

    // 循环到末尾（Wrap-Around）
    menu_item_t *last_item = menu->current_menu_head;
    while (last_item->next != NULL)
    {
        last_item = last_item->next;
    }

    // 从末尾向前找第一个可见项
    while (last_item != NULL)
    {
        menu_item_state_t state = menu_get_item_state(last_item);
        if (state != MENU_ITEM_HIDDEN)
        {
            // 调用离开回调
            if (menu->current_selected->on_exit != NULL)
            {
                menu->current_selected->on_exit(menu);
            }

            menu->current_selected = last_item;
            menu->current_index    = menu->total_items - 1;

            // 调用进入回调
            if (menu->current_selected->on_enter != NULL)
            {
                menu->current_selected->on_enter(menu);
            }

            menu_update_scroll_window(menu);
            menu->need_refresh = 1;
            return;
        }
        last_item = last_item->prev;
    }
}

/**
 * @brief 向下导航
 */
void menu_navigate_down(menu_instance_t *menu)
{
    if (menu == NULL || menu->current_selected == NULL)
    {
        return;
    }

    menu_item_t *next_item = menu->current_selected->next;

    // 如果有下一项，则移动到下一项
    if (next_item != NULL)
    {
        // 检查下一项的状态（跳过隐藏项）
        menu_item_state_t state = menu_get_item_state(next_item);
        while (next_item != NULL && state == MENU_ITEM_HIDDEN)
        {
            next_item = next_item->next;
            if (next_item != NULL)
            {
                state = menu_get_item_state(next_item);
            }
        }

        // 如果找到可见项，则移动
        if (next_item != NULL)
        {
            // 调用离开回调
            if (menu->current_selected->on_exit != NULL)
            {
                menu->current_selected->on_exit(menu);
            }

            menu->current_selected = next_item;
            menu->current_index++;

            // 调用进入回调
            if (menu->current_selected->on_enter != NULL)
            {
                menu->current_selected->on_enter(menu);
            }

            menu_update_scroll_window(menu);
            menu->need_refresh = 1;
            return;
        }
    }

    // 循环到头部（Wrap-Around）
    menu_item_t *first_item = menu->current_menu_head;
    while (first_item != NULL)
    {
        menu_item_state_t state = menu_get_item_state(first_item);
        if (state != MENU_ITEM_HIDDEN)
        {
            // 调用离开回调
            if (menu->current_selected->on_exit != NULL)
            {
                menu->current_selected->on_exit(menu);
            }

            menu->current_selected = first_item;
            menu->current_index    = 0;

            // 调用进入回调
            if (menu->current_selected->on_enter != NULL)
            {
                menu->current_selected->on_enter(menu);
            }

            menu_update_scroll_window(menu);
            menu->need_refresh = 1;
            return;
        }
        first_item = first_item->next;
    }
}

/**
 * @brief 确认/进入
 */
void menu_navigate_confirm(menu_instance_t *menu)
{
    if (menu == NULL || menu->current_selected == NULL)
    {
        return;
    }

    // 检查菜单项状态（禁用项不响应）
    menu_item_state_t state = menu_get_item_state(menu->current_selected);
    if (state == MENU_ITEM_DISABLED || state == MENU_ITEM_HIDDEN)
    {
        return;
    }

    // 优先检查是否有子菜单
    if (menu->current_selected->type == MENU_TYPE_SUBMENU &&
        menu->current_selected->sub_menu != NULL)
    {
        // 检查栈深度（防止溢出）
        if (menu->stack_depth >= MENU_MAX_DEPTH)
        {
            return; // 达到最大深度，不允许继续深入
        }

        // 保存当前状态到历史栈
        menu->history_menu[menu->stack_depth]   = menu->current_menu_head;
        menu->history_cursor[menu->stack_depth] = menu->current_selected;
        menu->stack_depth++;

        // 进入子菜单
        menu_enter_level(menu, menu->current_selected->sub_menu);
        menu->need_refresh = 1;
        return;
    }

    // 否则，执行动作函数
    if (menu->current_selected->type == MENU_TYPE_ACTION &&
        menu->current_selected->action != NULL)
    {
        menu->current_selected->action();
        menu->need_refresh = 1; // 动作执行后可能需要刷新显示
    }
}

/**
 * @brief 返回/退出
 */
void menu_navigate_back(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    // 检查是否在根菜单（栈为空）
    if (menu->stack_depth == 0)
    {
        return; // 已在根菜单，无法再返回
    }

    // 从历史栈恢复上一层菜单
    menu->stack_depth--;
    menu_item_t *parent_menu   = menu->history_menu[menu->stack_depth];
    menu_item_t *parent_cursor = menu->history_cursor[menu->stack_depth];

    // 恢复菜单状态
    menu->current_menu_head = parent_menu;
    menu->current_selected  = parent_cursor;

    // 重新计算索引和总数
    menu->total_items   = menu_count_visible_items(parent_menu);
    menu->current_index = 0;

    // 找到当前选中项的索引
    menu_item_t *temp = parent_menu;
    while (temp != NULL && temp != parent_cursor)
    {
        menu_item_state_t state = menu_get_item_state(temp);
        if (state != MENU_ITEM_HIDDEN)
        {
            menu->current_index++;
        }
        temp = temp->next;
    }

    // 更新滚动窗口
    menu_update_scroll_window(menu);
    menu->need_refresh = 1;
}

// =============================================================================
// 查询接口函数
// =============================================================================

/**
 * @brief 获取当前选中的菜单项
 */
menu_item_t *menu_get_current_item(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return NULL;
    }

    return menu->current_selected;
}

/**
 * @brief 获取当前菜单深度
 */
uint8_t menu_get_depth(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return 0;
    }

    return menu->stack_depth;
}

/**
 * @brief 判断是否在根菜单
 */
uint8_t menu_is_at_root(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return 1;
    }

    return (menu->stack_depth == 0) ? 1 : 0;
}

// =============================================================================
// 私有辅助函数实现
// =============================================================================

/**
 * @brief 计算可见菜单项数量（排除隐藏项）
 */
static uint8_t menu_count_visible_items(menu_item_t *head)
{
    uint8_t     count = 0;
    menu_item_t *temp = head;

    while (temp != NULL)
    {
        menu_item_state_t state = menu_get_item_state(temp);
        if (state != MENU_ITEM_HIDDEN)
        {
            count++;
        }
        temp = temp->next;
    }

    return count;
}

/**
 * @brief 获取第N个可见菜单项（排除隐藏项）
 */
static menu_item_t *menu_get_visible_item_at(menu_item_t *head, uint8_t index)
{
    uint8_t     count = 0;
    menu_item_t *temp = head;

    while (temp != NULL)
    {
        menu_item_state_t state = menu_get_item_state(temp);
        if (state != MENU_ITEM_HIDDEN)
        {
            if (count == index)
            {
                return temp;
            }
            count++;
        }
        temp = temp->next;
    }

    return NULL;
}

/**
 * @brief 获取菜单项状态（支持动态状态）
 */
static menu_item_state_t menu_get_item_state(menu_item_t *item)
{
    if (item == NULL)
    {
        return MENU_ITEM_HIDDEN;
    }

    // 如果有动态状态函数，则调用
    if (item->get_state != NULL)
    {
        return item->get_state();
    }

    // 否则返回默认状态
    return MENU_ITEM_NORMAL;
}

/**
 * @brief 更新滚动窗口
 */
static void menu_update_scroll_window(menu_instance_t *menu)
{
    if (menu == NULL)
    {
        return;
    }

    // 如果总项数小于等于可见行数，则不需要滚动
    if (menu->total_items <= menu->visible_lines)
    {
        menu->display_offset = 0;
        return;
    }

    // 如果选中项在窗口上方，则向上滚动
    if (menu->current_index < menu->display_offset)
    {
        menu->display_offset = menu->current_index;
    }

    // 如果选中项在窗口下方，则向下滚动
    if (menu->current_index >= menu->display_offset + menu->visible_lines)
    {
        menu->display_offset = menu->current_index - menu->visible_lines + 1;
    }
}

/**
 * @brief 进入新的菜单层级
 */
static void menu_enter_level(menu_instance_t *menu, menu_item_t *new_head)
{
    if (menu == NULL || new_head == NULL)
    {
        return;
    }

    // 更新菜单头指针
    menu->current_menu_head = new_head;

    // 找到第一个可见项作为默认选中项
    menu->current_selected = new_head;
    menu_item_t *temp      = new_head;
    while (temp != NULL)
    {
        menu_item_state_t state = menu_get_item_state(temp);
        if (state != MENU_ITEM_HIDDEN)
        {
            menu->current_selected = temp;
            break;
        }
        temp = temp->next;
    }

    // 重置索引和滚动窗口
    menu->current_index  = 0;
    menu->display_offset = 0;
    menu->total_items    = menu_count_visible_items(new_head);

    // 调用进入回调
    if (menu->current_selected != NULL && menu->current_selected->on_enter != NULL)
    {
        menu->current_selected->on_enter(menu);
    }
}
