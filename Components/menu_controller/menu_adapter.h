#ifndef __MENU_ADAPTER_H__
#define __MENU_ADAPTER_H__

#include "menu_core.h"
#include "menu_render.h"
#include "input_manager.h"
#include "u8g2.h"

// =============================================================================
// 菜单适配器 - 与现有项目集成
// 设计目标：
// 1. 自动绑定input_manager的输入
// 2. 自动绑定u8g2的渲染
// 3. 提供一键初始化函数
// =============================================================================

// -----------------------------------------------------------------------------
// API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 一键初始化菜单适配器
 * @param menu 菜单实例指针
 * @param u8g2 u8g2实例指针
 * @note  自动绑定input_manager的输入回调和u8g2的渲染回调
 * @note  使用默认渲染配置（用户可后续通过menu_render_get_config修改）
 * @note  使用示例：
 *        menu_adapter_init(&g_main_menu, u8g2_get_instance());
 */
void menu_adapter_init(menu_instance_t *menu, u8g2_t *u8g2);

/**
 * @brief 获取UP键状态（input_manager适配）
 * @return 1表示按下，0表示未按下
 * @note  内部函数，通常不需要用户直接调用
 */
uint8_t menu_adapter_get_up(void);

/**
 * @brief 获取DOWN键状态（input_manager适配）
 * @return 1表示按下，0表示未按下
 * @note  内部函数，通常不需要用户直接调用
 */
uint8_t menu_adapter_get_down(void);

/**
 * @brief 获取确认键状态（input_manager适配）
 * @return 1表示按下，0表示未按下
 * @note  内部函数，通常不需要用户直接调用
 */
uint8_t menu_adapter_get_confirm(void);

/**
 * @brief 获取返回键状态（input_manager适配）
 * @return 1表示按下，0表示未按下
 * @note  内部函数，通常不需要用户直接调用
 */
uint8_t menu_adapter_get_back(void);

#endif // __MENU_ADAPTER_H__
