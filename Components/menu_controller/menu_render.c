#include "menu_render.h"
#include <string.h>

// =============================================================================
// 私有变量
// =============================================================================
static u8g2_t              *g_u8g2 = NULL;            // u8g2实例指针
static menu_render_config_t g_config;                 // 渲染配置
static uint8_t              g_initialized = 0;        // 初始化标志

// =============================================================================
// 私有函数声明
// =============================================================================
static void render_title_bar(menu_instance_t *menu);
static void render_menu_items(menu_instance_t *menu);
static void render_scrollbar(menu_instance_t *menu);
static menu_item_state_t get_item_state(menu_item_t *item);

// =============================================================================
// 公共API实现
// =============================================================================

/**
 * @brief 初始化u8g2渲染器
 */
void menu_render_u8g2_init(u8g2_t *u8g2, menu_render_config_t *config)
{
    if (u8g2 == NULL)
    {
        return;
    }

    g_u8g2 = u8g2;

    // 如果用户提供了配置，则使用用户配置
    if (config != NULL)
    {
        memcpy(&g_config, config, sizeof(menu_render_config_t));
    }
    else
    {
        // 使用默认配置
        g_config.x_offset       = MENU_DEFAULT_X_OFFSET;
        g_config.y_offset       = MENU_DEFAULT_Y_OFFSET;
        g_config.line_height    = MENU_DEFAULT_LINE_HEIGHT;
        g_config.cursor_width   = MENU_DEFAULT_CURSOR_WIDTH;
        g_config.font           = u8g2_font_6x10_tf; // 默认字体
        g_config.show_scrollbar = MENU_DEFAULT_SHOW_SCROLLBAR;
        g_config.show_title     = MENU_DEFAULT_SHOW_TITLE;
        g_config.title_text     = "MENU";
    }

    g_initialized = 1;
}

/**
 * @brief u8g2默认渲染函数
 */
void menu_render_u8g2_default(menu_instance_t *menu)
{
    if (!g_initialized || g_u8g2 == NULL || menu == NULL)
    {
        return;
    }

    // 清空缓冲区
    u8g2_ClearBuffer(g_u8g2);

    // 设置字体
    u8g2_SetFont(g_u8g2, g_config.font);

    // 渲染标题栏（如果启用）
    if (g_config.show_title)
    {
        render_title_bar(menu);
    }

    // 渲染菜单项列表
    render_menu_items(menu);

    // 渲染滚动条（如果启用）
    if (g_config.show_scrollbar && menu->total_items > menu->visible_lines)
    {
        render_scrollbar(menu);
    }

    // 发送到屏幕
    u8g2_SendBuffer(g_u8g2);
}

/**
 * @brief 获取当前渲染配置
 */
menu_render_config_t *menu_render_get_config(void)
{
    return &g_config;
}

/**
 * @brief 设置标题文本
 */
void menu_render_set_title(const char *title)
{
    if (title == NULL)
    {
        return;
    }

    g_config.title_text = title;
}

// =============================================================================
// 私有函数实现
// =============================================================================

/**
 * @brief 渲染标题栏
 */
static void render_title_bar(menu_instance_t *menu)
{
    if (g_u8g2 == NULL || menu == NULL)
    {
        return;
    }

    // 绘制标题文本（居中）
    uint8_t title_width = u8g2_GetStrWidth(g_u8g2, g_config.title_text);
    uint8_t title_x     = (u8g2_GetDisplayWidth(g_u8g2) - title_width) / 2;
    u8g2_DrawStr(g_u8g2, title_x, 10, g_config.title_text);

    // 绘制分隔线
    u8g2_DrawHLine(g_u8g2, 0, 11, u8g2_GetDisplayWidth(g_u8g2));
}

/**
 * @brief 渲染菜单项列表
 */
static void render_menu_items(menu_instance_t *menu)
{
    if (g_u8g2 == NULL || menu == NULL)
    {
        return;
    }

    menu_item_t *item         = menu->current_menu_head;
    uint8_t      y            = g_config.y_offset;
    uint8_t      visible_idx  = 0; // 可见项索引
    uint8_t      render_count = 0; // 已渲染项计数

    // 遍历链表找到显示窗口的起始项
    while (item != NULL && visible_idx < menu->display_offset)
    {
        menu_item_state_t state = get_item_state(item);
        if (state != MENU_ITEM_HIDDEN)
        {
            visible_idx++;
        }
        item = item->next;
    }

    // 渲染可见窗口内的菜单项
    while (item != NULL && render_count < menu->visible_lines)
    {
        menu_item_state_t state = get_item_state(item);

        // 跳过隐藏项
        if (state == MENU_ITEM_HIDDEN)
        {
            item = item->next;
            continue;
        }

        // 判断是否为当前选中项
        uint8_t is_selected = (item == menu->current_selected) ? 1 : 0;

        // 绘制光标（反色高亮）
        if (is_selected)
        {
            uint8_t box_height = g_config.line_height;
            // 高亮框位置：y是文本基线，文字向上约10像素，向下约2像素
            // 所以高亮框从 y-10 开始，高度为 line_height
            u8g2_SetDrawColor(g_u8g2, 1);
            u8g2_DrawBox(g_u8g2, g_config.x_offset, y - 10,
                         u8g2_GetDisplayWidth(g_u8g2) - 10, box_height);
            u8g2_SetDrawColor(g_u8g2, 0); // 反色文字
        }

        // 绘制菜单项文本
        if (item->label != NULL)
        {
            // 处理禁用项（显示但不可选）
            if (state == MENU_ITEM_DISABLED)
            {
                // 简单处理：正常显示（后续可以改为灰色或斜体）
            }

            u8g2_DrawStr(g_u8g2, g_config.x_offset + 2, y, item->label);

            // 绘制动态值（如"音量：75%"）
            if (item->get_value != NULL)
            {
                const char *value = item->get_value();
                if (value != NULL)
                {
                    uint8_t value_x = u8g2_GetDisplayWidth(g_u8g2) - 30;
                    u8g2_DrawStr(g_u8g2, value_x, y, value);
                }
            }

            // 绘制子菜单标记（">"）
            if (item->type == MENU_TYPE_SUBMENU && item->sub_menu != NULL)
            {
                uint8_t arrow_x = u8g2_GetDisplayWidth(g_u8g2) - 14;
                u8g2_DrawStr(g_u8g2, arrow_x, y, ">");
            }
        }

        // 恢复绘制颜色
        u8g2_SetDrawColor(g_u8g2, 1);

        // 更新Y坐标和计数
        y += g_config.line_height;
        render_count++;
        item = item->next;
    }
}

/**
 * @brief 渲染滚动条
 */
static void render_scrollbar(menu_instance_t *menu)
{
    if (g_u8g2 == NULL || menu == NULL || menu->total_items == 0)
    {
        return;
    }

    uint8_t display_height = u8g2_GetDisplayHeight(g_u8g2);
    uint8_t scrollbar_x    = u8g2_GetDisplayWidth(g_u8g2) - 2;
    uint8_t scrollbar_y    = g_config.y_offset;
    uint8_t scrollbar_h    = display_height - g_config.y_offset;

    // 计算滚动条滑块的位置和高度
    uint8_t thumb_h = (scrollbar_h * menu->visible_lines) / menu->total_items;
    if (thumb_h < 4)
    {
        thumb_h = 4; // 最小高度4像素
    }

    uint8_t thumb_y =
        scrollbar_y +
        (scrollbar_h - thumb_h) * menu->display_offset / menu->total_items;

    // 绘制滚动条背景（边框）
    u8g2_DrawVLine(g_u8g2, scrollbar_x, scrollbar_y, scrollbar_h);

    // 绘制滚动条滑块（实心）
    u8g2_DrawBox(g_u8g2, scrollbar_x - 1, thumb_y, 3, thumb_h);
}

/**
 * @brief 获取菜单项状态（支持动态状态）
 */
static menu_item_state_t get_item_state(menu_item_t *item)
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
