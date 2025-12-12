#include "menu_adapter.h"

// =============================================================================
// 菜单适配器实现
// =============================================================================

/**
 * @brief 一键初始化菜单适配器
 */
void menu_adapter_init(menu_instance_t *menu, u8g2_t *u8g2)
{
    if (menu == NULL || u8g2 == NULL)
    {
        return;
    }

    // 初始化u8g2渲染器（使用默认配置）
    menu_render_u8g2_init(u8g2, NULL);

    // 绑定输入回调（input_manager适配）
    menu_set_input_callbacks(menu, menu_adapter_get_up, menu_adapter_get_down,
                              menu_adapter_get_confirm,
                              menu_adapter_get_back);

    // 绑定渲染回调（u8g2渲染器）
    menu_set_render_callback(menu, menu_render_u8g2_default, u8g2);
}

// =============================================================================
// 输入适配函数实现
// =============================================================================

/**
 * @brief 获取UP键状态（映射到INPUT_BTN_UP）
 * @note  使用input_is_pressed而非input_is_just_pressed，
 *        因为menu_core内部已经实现了边缘检测机制
 */
uint8_t menu_adapter_get_up(void)
{
    return input_is_pressed(INPUT_BTN_UP);
}

/**
 * @brief 获取DOWN键状态（映射到INPUT_BTN_DOWN）
 * @note  使用input_is_pressed而非input_is_just_pressed
 */
uint8_t menu_adapter_get_down(void)
{
    return input_is_pressed(INPUT_BTN_DOWN);
}

/**
 * @brief 获取确认键状态（映射到INPUT_BTN_A）
 * @note  使用input_is_pressed而非input_is_just_pressed
 */
uint8_t menu_adapter_get_confirm(void)
{
    return input_is_pressed(INPUT_BTN_A);
}

/**
 * @brief 获取返回键状态（映射到INPUT_BTN_B）
 * @note  使用input_is_pressed而非input_is_just_pressed
 */
uint8_t menu_adapter_get_back(void)
{
    return input_is_pressed(INPUT_BTN_B);
}
