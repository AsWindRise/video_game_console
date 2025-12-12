/**
 * @file ball_physics.c
 * @brief 通用球物理模块实现
 */

#include "ball_physics.h"
#include <math.h>

/* ======================== 基础函数 ======================== */

/**
 * @brief 初始化球
 */
void ball_init(ball_physics_t *ball, float x, float y, float vx, float vy, uint8_t radius)
{
    ball->x = x;
    ball->y = y;
    ball->vx = vx;
    ball->vy = vy;
    ball->radius = radius;
}

/**
 * @brief 更新球的位置
 */
void ball_update(ball_physics_t *ball)
{
    ball->x += ball->vx;
    ball->y += ball->vy;
}

/* ======================== 反弹函数 ======================== */

/**
 * @brief 水平反弹（X方向速度反转）
 */
void ball_reflect_horizontal(ball_physics_t *ball)
{
    ball->vx = -ball->vx;
}

/**
 * @brief 垂直反弹（Y方向速度反转）
 */
void ball_reflect_vertical(ball_physics_t *ball)
{
    ball->vy = -ball->vy;
}

/**
 * @brief 挡板反弹（带角度调整）
 * @note  根据击球位置调整反弹角度，增加游戏性
 */
void ball_reflect_paddle(ball_physics_t *ball, int16_t paddle_x, uint8_t paddle_width)
{
    // 计算击球位置（相对于挡板中心）
    float hit_pos = ball->x - paddle_x;  // -paddle_width/2 ~ +paddle_width/2

    // 归一化到 -1.0 ~ +1.0
    float normalized = (hit_pos / (paddle_width / 2.0f));

    // 限制范围
    if (normalized < -1.0f) normalized = -1.0f;
    if (normalized > 1.0f) normalized = 1.0f;

    // 计算速度大小（保持球速恒定）
    float speed = sqrtf(ball->vx * ball->vx + ball->vy * ball->vy);

    // 计算反弹角度（-60度 ~ +60度）
    // 中心击球：0度（垂直反弹）
    // 边缘击球：±60度（斜向反弹）
    float angle = normalized * 1.047f;  // 60度 = 1.047弧度

    // 设置新速度
    ball->vx = speed * sinf(angle);
    ball->vy = -fabsf(speed * cosf(angle));  // 确保向上反弹
}

/* ======================== 碰撞检测函数 ======================== */

/**
 * @brief 检测球与矩形的碰撞（AABB算法）
 */
uint8_t ball_collides_with_rect(ball_physics_t *ball, rect_t *rect)
{
    // 计算球的边界
    float ball_left = ball->x - ball->radius;
    float ball_right = ball->x + ball->radius;
    float ball_top = ball->y - ball->radius;
    float ball_bottom = ball->y + ball->radius;

    // 计算矩形的边界
    int16_t rect_left = rect->x;
    int16_t rect_right = rect->x + rect->width;
    int16_t rect_top = rect->y;
    int16_t rect_bottom = rect->y + rect->height;

    // AABB碰撞检测
    if (ball_right < rect_left || ball_left > rect_right ||
        ball_bottom < rect_top || ball_top > rect_bottom) {
        return 0;  // 未碰撞
    }

    return 1;  // 碰撞
}

/**
 * @brief 检测球与矩形的碰撞并返回碰撞位置（详细版）
 */
uint8_t ball_collides_with_rect_detailed(ball_physics_t *ball, rect_t *rect,
                                          uint8_t *hit_top, uint8_t *hit_bottom,
                                          uint8_t *hit_left, uint8_t *hit_right)
{
    // 清空输出参数
    *hit_top = 0;
    *hit_bottom = 0;
    *hit_left = 0;
    *hit_right = 0;

    // 先检测是否碰撞
    if (!ball_collides_with_rect(ball, rect)) {
        return 0;
    }

    // 计算球的边界
    float ball_left = ball->x - ball->radius;
    float ball_right = ball->x + ball->radius;
    float ball_top = ball->y - ball->radius;
    float ball_bottom = ball->y + ball->radius;

    // 计算矩形的边界
    int16_t rect_left = rect->x;
    int16_t rect_right = rect->x + rect->width;
    int16_t rect_top = rect->y;
    int16_t rect_bottom = rect->y + rect->height;

    // 计算穿透深度（用于判断从哪个方向碰撞）
    float penetration_top = ball_bottom - rect_top;
    float penetration_bottom = rect_bottom - ball_top;
    float penetration_left = ball_right - rect_left;
    float penetration_right = rect_right - ball_left;

    // 找出最小穿透深度（就是碰撞的那一面）
    float min_penetration = penetration_top;
    *hit_top = 1;

    if (penetration_bottom < min_penetration) {
        min_penetration = penetration_bottom;
        *hit_top = 0;
        *hit_bottom = 1;
    }

    if (penetration_left < min_penetration) {
        min_penetration = penetration_left;
        *hit_top = 0;
        *hit_bottom = 0;
        *hit_left = 1;
    }

    if (penetration_right < min_penetration) {
        *hit_top = 0;
        *hit_bottom = 0;
        *hit_left = 0;
        *hit_right = 1;
    }

    return 1;  // 碰撞
}
