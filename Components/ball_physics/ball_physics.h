/**
 * @file ball_physics.h
 * @brief 通用球物理模块 - 可复用于打砖块、乒乓球等游戏
 * @note  提供球的运动、反弹、碰撞检测等通用功能
 */

#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H

#include <stdint.h>

/* ======================== 数据结构定义 ======================== */

/**
 * @brief 球的物理状态
 */
typedef struct {
    float x;            // 球中心X坐标
    float y;            // 球中心Y坐标
    float vx;           // X方向速度（像素/帧）
    float vy;           // Y方向速度（像素/帧）
    uint8_t radius;     // 球半径（像素）
} ball_physics_t;

/**
 * @brief 矩形边界框（用于碰撞检测）
 */
typedef struct {
    int16_t x;          // 矩形左上角X坐标
    int16_t y;          // 矩形左上角Y坐标
    uint8_t width;      // 矩形宽度
    uint8_t height;     // 矩形高度
} rect_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief 初始化球
 * @param ball 球指针
 * @param x 初始X坐标
 * @param y 初始Y坐标
 * @param vx 初始X速度
 * @param vy 初始Y速度
 * @param radius 球半径
 */
void ball_init(ball_physics_t *ball, float x, float y, float vx, float vy, uint8_t radius);

/**
 * @brief 更新球的位置
 * @param ball 球指针
 */
void ball_update(ball_physics_t *ball);

/**
 * @brief 水平反弹（碰到左右墙壁）
 * @param ball 球指针
 */
void ball_reflect_horizontal(ball_physics_t *ball);

/**
 * @brief 垂直反弹（碰到上下墙壁）
 * @param ball 球指针
 */
void ball_reflect_vertical(ball_physics_t *ball);

/**
 * @brief 挡板反弹（带角度调整）
 * @param ball 球指针
 * @param paddle_x 挡板中心X坐标
 * @param paddle_width 挡板宽度
 * @note  根据击球位置调整反弹角度：
 *        - 中心击球：垂直反弹
 *        - 边缘击球：斜向反弹（增加游戏性）
 */
void ball_reflect_paddle(ball_physics_t *ball, int16_t paddle_x, uint8_t paddle_width);

/**
 * @brief 检测球与矩形的碰撞
 * @param ball 球指针
 * @param rect 矩形指针
 * @return 1=碰撞，0=未碰撞
 */
uint8_t ball_collides_with_rect(ball_physics_t *ball, rect_t *rect);

/**
 * @brief 检测球与矩形的碰撞并返回碰撞位置
 * @param ball 球指针
 * @param rect 矩形指针
 * @param hit_top 输出参数：是否碰到矩形顶部
 * @param hit_bottom 输出参数：是否碰到矩形底部
 * @param hit_left 输出参数：是否碰到矩形左侧
 * @param hit_right 输出参数：是否碰到矩形右侧
 * @return 1=碰撞，0=未碰撞
 */
uint8_t ball_collides_with_rect_detailed(ball_physics_t *ball, rect_t *rect,
                                          uint8_t *hit_top, uint8_t *hit_bottom,
                                          uint8_t *hit_left, uint8_t *hit_right);

#endif // BALL_PHYSICS_H
