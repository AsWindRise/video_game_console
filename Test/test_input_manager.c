#include "test_input_manager.h"
#include "input_manager.h"
#include "u8g2_stm32_hal.h"
#include <stdio.h>
#include <string.h>

// -----------------------------------------------------------------------------
// 私有变量
// -----------------------------------------------------------------------------

// 按键名称表（用于显示）
static const char *btn_names[INPUT_BTN_MAX] = {
    "UP", "DN", "LF", "RT",  // 方向键
    "A", "B", "X", "Y",      // 功能键
    "ST"                     // START键
};

// -----------------------------------------------------------------------------
// 私有函数
// -----------------------------------------------------------------------------

/**
 * @brief 获取按键状态符号
 * @param btn: 按键枚举
 * @retval 状态符号字符串
 */
static const char *get_button_status_symbol(input_button_t btn)
{
    if (input_is_just_pressed(btn))
    {
        return "v"; // 刚按下（↓的替代符号）
    }
    else if (input_is_just_released(btn))
    {
        return "^"; // 刚释放（↑的替代符号）
    }
    else if (input_is_pressed(btn))
    {
        return "#"; // 持续按下（■的替代符号）
    }
    else
    {
        return "-"; // 未按下（□的替代符号）
    }
}

// -----------------------------------------------------------------------------
// 公共函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化输入测试
 */
void test_input_manager_init(void)
{
    // 无需额外初始化
}

/**
 * @brief 输入测试任务
 */
void test_input_manager_task(void)
{
    u8g2_t *u8g2 = u8g2_get_instance();

    u8g2_ClearBuffer(u8g2);

    // ======== 标题 ========
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 0, 10, "Input Test");
    u8g2_DrawHLine(u8g2, 0, 12, 128);

    // ======== 显示功能键状态（按硬件顺序：SW1~SW4 SK）========
    u8g2_SetFont(u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(u8g2, 0, 22, "Btn:");

    char btn_status[32];
    snprintf(btn_status, sizeof(btn_status), "%s %s %s %s %s",
             get_button_status_symbol(INPUT_BTN_Y),   // SW1
             get_button_status_symbol(INPUT_BTN_X),   // SW2
             get_button_status_symbol(INPUT_BTN_A),   // SW3
             get_button_status_symbol(INPUT_BTN_B),   // SW4
             get_button_status_symbol(INPUT_BTN_START)); // SK
    u8g2_DrawStr(u8g2, 30, 22, btn_status);

    // 显示按键名称（对应硬件：SW1 SW2 SW3 SW4 SK）
    u8g2_DrawStr(u8g2, 30, 30, "Y  X  A  B  ST");

    // ======== 显示方向键状态（UP DOWN LEFT RIGHT）========
    u8g2_DrawStr(u8g2, 0, 40, "Dir:");

    char dir_status[32];
    snprintf(dir_status, sizeof(dir_status), "%s %s %s %s",
             get_button_status_symbol(INPUT_BTN_UP),
             get_button_status_symbol(INPUT_BTN_DOWN),
             get_button_status_symbol(INPUT_BTN_LEFT),
             get_button_status_symbol(INPUT_BTN_RIGHT));
    u8g2_DrawStr(u8g2, 30, 40, dir_status);

    // 显示方向名称
    u8g2_DrawStr(u8g2, 30, 48, "U  D  L  R");

    // ======== 显示边缘触发信息 ========
    u8g2_DrawHLine(u8g2, 0, 50, 128);
    u8g2_DrawStr(u8g2, 0, 58, "Edge:");

    char edge_info[32] = "";
    uint8_t has_edge = 0;

    // 检测刚按下
    for (input_button_t i = INPUT_BTN_UP; i < INPUT_BTN_MAX; i++)
    {
        if (input_is_just_pressed(i))
        {
            strcat(edge_info, btn_names[i]);
            strcat(edge_info, "v ");
            has_edge = 1;
        }
        else if (input_is_just_released(i))
        {
            strcat(edge_info, btn_names[i]);
            strcat(edge_info, "^ ");
            has_edge = 1;
        }
    }

    if (!has_edge)
    {
        strcpy(edge_info, "NONE");
    }

    u8g2_DrawStr(u8g2, 30, 58, edge_info);

    // ======== 显示双击信息 ========
    u8g2_DrawStr(u8g2, 0, 64, "Dbl:");

    char dbl_info[32] = "";
    uint8_t has_double = 0;

    for (input_button_t i = INPUT_BTN_UP; i < INPUT_BTN_MAX; i++)
    {
        if (input_is_double_click(i))
        {
            strcat(dbl_info, btn_names[i]);
            strcat(dbl_info, " ");
            has_double = 1;
        }
    }

    if (!has_double)
    {
        strcpy(dbl_info, "NONE");
    }

    u8g2_DrawStr(u8g2, 30, 64, dbl_info);

    // ======== 显示图例 ========
    u8g2_SetFont(u8g2, u8g2_font_4x6_tf);
    u8g2_DrawStr(u8g2, 90, 64, "# v ^ -");

    u8g2_SendBuffer(u8g2);
}
