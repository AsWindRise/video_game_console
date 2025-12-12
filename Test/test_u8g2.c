/**
 ******************************************************************************
 * @file    test_u8g2.c
 * @brief   u8g2图形库测试代码实现
 * @author  老王
 * @note    这个文件实现了u8g2图形库的各种测试功能
 *          包括基本图形、文本、动画等测试
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "test_u8g2.h"

/* Private defines -----------------------------------------------------------*/
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

/* Private variables ---------------------------------------------------------*/
static u8g2_test_mode_t current_test_mode = U8G2_TEST_BASIC_SHAPES;
static uint32_t animation_counter = 0;
static uint32_t frame_counter = 0;
static uint32_t last_fps_time = 0;
static float current_fps = 0.0f;

/* Private function prototypes -----------------------------------------------*/
/* 无需私有函数声明 */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief u8g2测试初始化函数
 */
void test_u8g2_init(void)
{
    current_test_mode = U8G2_TEST_BASIC_SHAPES;
    animation_counter = 0;
    frame_counter = 0;
    last_fps_time = HAL_GetTick();
    current_fps = 0.0f;
}

/**
 * @brief u8g2测试任务(周期性调用)
 */
void test_u8g2_task(void)
{
    static uint32_t last_mode_switch_time = 0;
    uint32_t current_time = HAL_GetTick();

    // 每3秒自动切换测试模式
    if(current_time - last_mode_switch_time >= 3000)
    {
        test_u8g2_next_mode();
        last_mode_switch_time = current_time;
        animation_counter = 0;  // 重置动画计数器
    }

    // 根据当前测试模式执行不同的测试
    switch(current_test_mode)
    {
        case U8G2_TEST_BASIC_SHAPES:
            test_u8g2_basic_shapes();
            break;

        case U8G2_TEST_TEXT:
            test_u8g2_text_display();
            break;

        case U8G2_TEST_ANIMATION:
            test_u8g2_animation();
            break;

        case U8G2_TEST_MENU:
            test_u8g2_menu_ui();
            break;

        case U8G2_TEST_MIXED:
            test_u8g2_mixed_display();
            break;

        default:
            current_test_mode = U8G2_TEST_BASIC_SHAPES;
            break;
    }

    // 更新动画计数器
    animation_counter++;

    // 计算FPS(每秒更新一次)
    frame_counter++;
    if(current_time - last_fps_time >= 1000)
    {
        current_fps = (float)frame_counter / ((current_time - last_fps_time) / 1000.0f);
        frame_counter = 0;
        last_fps_time = current_time;
    }
}

/**
 * @brief 切换测试模式
 */
void test_u8g2_next_mode(void)
{
    current_test_mode++;
    if(current_test_mode >= U8G2_TEST_MODE_MAX)
    {
        current_test_mode = U8G2_TEST_BASIC_SHAPES;
    }
    animation_counter = 0;  // 重置动画计数器
}

/**
 * @brief 获取当前测试模式
 */
u8g2_test_mode_t test_u8g2_get_mode(void)
{
    return current_test_mode;
}

/* ========== 各种测试函数实现 ========== */

/**
 * @brief 基本图形测试
 * @note  测试点、线、矩形、圆形、三角形等基本图形
 */
void test_u8g2_basic_shapes(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    // 清空缓冲区
    u8g2_ClearBuffer(u8g2);

    // 绘制标题
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 30, 10, "Shapes Test");

    // 绘制点
    u8g2_DrawPixel(u8g2, 10, 20);
    u8g2_DrawPixel(u8g2, 12, 20);
    u8g2_DrawPixel(u8g2, 14, 20);

    // 绘制线
    u8g2_DrawLine(u8g2, 10, 25, 40, 25);
    u8g2_DrawLine(u8g2, 10, 27, 40, 40);

    // 绘制矩形(空心)
    u8g2_DrawFrame(u8g2, 50, 15, 20, 15);

    // 绘制矩形(实心)
    u8g2_DrawBox(u8g2, 75, 15, 20, 15);

    // 绘制圆形(空心)
    u8g2_DrawCircle(u8g2, 60, 45, 10, U8G2_DRAW_ALL);

    // 绘制实心圆
    u8g2_DrawDisc(u8g2, 85, 45, 8, U8G2_DRAW_ALL);

    // 绘制圆角矩形
    u8g2_DrawRFrame(u8g2, 100, 15, 25, 20, 5);

    // 绘制三角形
    u8g2_DrawTriangle(u8g2, 110, 50, 120, 40, 130, 50);

    // 发送到屏幕
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 文本显示测试
 * @note  测试不同字体、不同大小的文本显示
 */
void test_u8g2_text_display(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    // 清空缓冲区
    u8g2_ClearBuffer(u8g2);

    // 小字体
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 5, 10, "6x10 Font");

    // 中等字体
    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
    u8g2_DrawStr(u8g2, 5, 25, "7x13 Font");

    // 大字体
    u8g2_SetFont(u8g2, u8g2_font_10x20_tf);
    u8g2_DrawStr(u8g2, 5, 45, "10x20");

    // 显示数字
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %lu", animation_counter);
    u8g2_DrawStr(u8g2, 5, 60, buf);

    // 发送到屏幕
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 动画测试
 * @note  测试弹跳球、进度条等动画效果
 */
void test_u8g2_animation(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    // 清空缓冲区
    u8g2_ClearBuffer(u8g2);

    // 绘制标题
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 30, 10, "Animation");

    // 弹跳球动画(简化版,使用线性运动代替正弦波)
    int ball_x = 14 + (animation_counter % 100);
    int ball_y = 32;
    u8g2_DrawDisc(u8g2, ball_x, ball_y, 5, U8G2_DRAW_ALL);

    // 进度条动画
    int progress = (animation_counter % 100);
    u8g2_DrawFrame(u8g2, 10, 50, 108, 10);
    u8g2_DrawBox(u8g2, 11, 51, progress, 8);

    // 显示进度百分比
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", progress);
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 54, 48, buf);

    // 发送到屏幕
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 菜单UI测试
 * @note  测试简单的菜单界面显示
 */
void test_u8g2_menu_ui(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    // 清空缓冲区
    u8g2_ClearBuffer(u8g2);

    // 绘制菜单标题栏
    u8g2_DrawBox(u8g2, 0, 0, 128, 12);
    u8g2_SetDrawColor(u8g2, 0);  // 反色
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 40, 10, "Main Menu");
    u8g2_SetDrawColor(u8g2, 1);  // 恢复正常

    // 绘制菜单项
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);

    // 选中项(根据动画计数器循环选择)
    int selected = (animation_counter / 20) % 4;

    // 菜单项1
    if(selected == 0) {
        u8g2_DrawBox(u8g2, 5, 17, 118, 10);
        u8g2_SetDrawColor(u8g2, 0);
    }
    u8g2_DrawStr(u8g2, 10, 25, "> Start Game");
    u8g2_SetDrawColor(u8g2, 1);

    // 菜单项2
    if(selected == 1) {
        u8g2_DrawBox(u8g2, 5, 29, 118, 10);
        u8g2_SetDrawColor(u8g2, 0);
    }
    u8g2_DrawStr(u8g2, 10, 37, "> Settings");
    u8g2_SetDrawColor(u8g2, 1);

    // 菜单项3
    if(selected == 2) {
        u8g2_DrawBox(u8g2, 5, 41, 118, 10);
        u8g2_SetDrawColor(u8g2, 0);
    }
    u8g2_DrawStr(u8g2, 10, 49, "> About");
    u8g2_SetDrawColor(u8g2, 1);

    // 菜单项4
    if(selected == 3) {
        u8g2_DrawBox(u8g2, 5, 53, 118, 10);
        u8g2_SetDrawColor(u8g2, 0);
    }
    u8g2_DrawStr(u8g2, 10, 61, "> Exit");
    u8g2_SetDrawColor(u8g2, 1);

    // 发送到屏幕
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 混合显示测试
 * @note  测试图形+文本+数据的混合显示
 */
void test_u8g2_mixed_display(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    // 清空缓冲区
    u8g2_ClearBuffer(u8g2);

    // 绘制标题
    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
    u8g2_DrawStr(u8g2, 20, 12, "Game Console");

    // 绘制分隔线
    u8g2_DrawLine(u8g2, 0, 15, 127, 15);

    // 显示系统信息
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);

    // CPU图标 + 温度(模拟数据)
    u8g2_DrawBox(u8g2, 5, 20, 6, 6);
    char buf[32];
    snprintf(buf, sizeof(buf), "CPU:45C");
    u8g2_DrawStr(u8g2, 15, 26, buf);

    // 内存图标 + 使用率(模拟数据)
    u8g2_DrawFrame(u8g2, 5, 32, 6, 6);
    snprintf(buf, sizeof(buf), "RAM:60%%");
    u8g2_DrawStr(u8g2, 15, 38, buf);

    // 电池图标 + 电量(动态变化)
    int battery = 100 - (animation_counter % 100);
    u8g2_DrawFrame(u8g2, 5, 44, 10, 6);
    u8g2_DrawBox(u8g2, 15, 46, 2, 2);  // 电池头
    int battery_width = (battery * 8) / 100;
    u8g2_DrawBox(u8g2, 6, 45, battery_width, 4);
    snprintf(buf, sizeof(buf), "BAT:%d%%", battery);
    u8g2_DrawStr(u8g2, 20, 50, buf);

    // FPS显示
    snprintf(buf, sizeof(buf), "FPS:%.1f", current_fps);
    u8g2_DrawStr(u8g2, 5, 62, buf);

    // 绘制一个旋转的图标(简化版,使用循环位置)
    int icon_x = 100;
    int icon_y = 40;
    int offset = (animation_counter % 20) - 10;

    // 绘制旋转的十字(简化为动态位置)
    u8g2_DrawLine(u8g2, icon_x - 10, icon_y + offset, icon_x + 10, icon_y - offset);
    u8g2_DrawLine(u8g2, icon_x - 10, icon_y - offset, icon_x + 10, icon_y + offset);

    // 发送到屏幕
    u8g2_SendBuffer(u8g2);
}

/**
 * @brief 性能测试
 * @note  测试u8g2的刷新性能和帧率
 */
void test_u8g2_performance(void)
{
    u8g2_t* u8g2 = u8g2_get_instance();

    uint32_t start_time = HAL_GetTick();

    // 执行1000次清屏和发送操作
    for(int i = 0; i < 1000; i++)
    {
        u8g2_ClearBuffer(u8g2);
        u8g2_SendBuffer(u8g2);
    }

    uint32_t elapsed_time = HAL_GetTick() - start_time;

    // 显示测试结果
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 20, 20, "Performance Test");

    char buf[64];
    snprintf(buf, sizeof(buf), "1000 frames:");
    u8g2_DrawStr(u8g2, 10, 35, buf);

    snprintf(buf, sizeof(buf), "%lums", elapsed_time);
    u8g2_DrawStr(u8g2, 10, 50, buf);

    u8g2_SendBuffer(u8g2);

    // 等待一段时间显示结果
    HAL_Delay(3000);
}

/* Private functions ---------------------------------------------------------*/
/* 无私有函数 */
