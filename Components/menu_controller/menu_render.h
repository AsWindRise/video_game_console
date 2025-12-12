#ifndef __MENU_RENDER_H__
#define __MENU_RENDER_H__

#include "menu_core.h"
#include "u8g2.h"

// =============================================================================
// 菜单渲染模块 - u8g2默认渲染器
// 设计目标：
// 1. 提供开箱即用的u8g2渲染器
// 2. 支持自定义渲染配置（字体、坐标、行高等）
// 3. 支持多种UI风格（简洁风格、详细风格等）
// =============================================================================

// -----------------------------------------------------------------------------
// 1. 渲染配置结构
// -----------------------------------------------------------------------------

/**
 * @brief 渲染配置结构体
 * @note  用于自定义菜单的显示风格
 */
typedef struct
{
    uint8_t        x_offset;       /*!< X起始坐标（像素） */
    uint8_t        y_offset;       /*!< Y起始坐标（像素） */
    uint8_t        line_height;    /*!< 行高（像素） */
    uint8_t        cursor_width;   /*!< 光标宽度（像素） */
    const uint8_t *font;           /*!< 字体指针（u8g2字体） */
    uint8_t        show_scrollbar; /*!< 是否显示滚动条（1=显示，0=不显示） */
    uint8_t        show_title;     /*!< 是否显示标题栏（1=显示，0=不显示） */
    const char    *title_text;     /*!< 标题文本（如"主菜单"） */
} menu_render_config_t;

// -----------------------------------------------------------------------------
// 2. 默认配置值
// -----------------------------------------------------------------------------
#define MENU_DEFAULT_X_OFFSET     0       // 默认X偏移
#define MENU_DEFAULT_Y_OFFSET     24      // 默认Y偏移（为标题栏和分隔线留足空间）
#define MENU_DEFAULT_LINE_HEIGHT  13      // 默认行高13像素
#define MENU_DEFAULT_CURSOR_WIDTH 2       // 默认光标宽度2像素
#define MENU_DEFAULT_SHOW_SCROLLBAR 1     // 默认显示滚动条
#define MENU_DEFAULT_SHOW_TITLE     1     // 默认显示标题栏

// -----------------------------------------------------------------------------
// 3. API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化u8g2渲染器
 * @param u8g2   u8g2实例指针
 * @param config 渲染配置（可为NULL，使用默认配置）
 * @note  在使用menu_render_u8g2_default前必须调用此函数
 */
void menu_render_u8g2_init(u8g2_t *u8g2, menu_render_config_t *config);

/**
 * @brief u8g2默认渲染函数
 * @param menu 菜单实例指针
 * @note  此函数会自动从menu->render_context获取u8g2实例
 * @note  渲染风格：简洁列表风格，带光标、滚动条、标题栏
 */
void menu_render_u8g2_default(menu_instance_t *menu);

/**
 * @brief 获取当前渲染配置
 * @return 当前渲染配置指针
 * @note  用于运行时修改渲染配置
 */
menu_render_config_t *menu_render_get_config(void);

/**
 * @brief 设置标题文本
 * @param title 标题文本字符串
 * @note  用于动态修改菜单标题
 */
void menu_render_set_title(const char *title);

#endif // __MENU_RENDER_H__
