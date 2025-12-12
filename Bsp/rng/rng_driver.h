#ifndef __RNG_DRIVER_H__
#define __RNG_DRIVER_H__

#include "mydefine.h"

// -----------------------------------------------------------------------------
// RNG驱动接口（游戏专用随机数生成）
// -----------------------------------------------------------------------------

/**
 * @brief 初始化RNG硬件随机数生成器
 * @note  调用此函数前需确保HAL库已初始化完成
 * @retval 0: 成功, -1: 失败
 */
int8_t rng_init(void);

/**
 * @brief 获取32位硬件随机数
 * @note  阻塞式获取，超时时间10ms
 * @param random: 存储随机数的指针
 * @retval 0: 成功, -1: 超时或失败
 */
int8_t rng_get_random(uint32_t *random);

/**
 * @brief 获取指定范围内的随机数 [min, max]
 * @note  游戏中最常用的接口，用于生成道具位置、敌人刷新等
 * @param min: 最小值（包含）
 * @param max: 最大值（包含）
 * @retval 返回[min, max]范围内的随机整数，失败返回min
 */
uint32_t rng_get_random_range(uint32_t min, uint32_t max);

/**
 * @brief 获取单字节随机数 [0, 255]
 * @note  用于生成颜色、小范围随机值等场景
 * @retval 返回0-255的随机字节
 */
uint8_t rng_get_random_byte(void);

/**
 * @brief 获取随机布尔值
 * @note  用于随机事件判断（50%概率），例如：随机方向、随机是否出现等
 * @retval 0 或 1
 */
uint8_t rng_get_random_bool(void);

/**
 * @brief 获取随机概率判断结果
 * @note  用于按概率触发事件，例如：30%概率掉落道具
 * @param probability: 概率值 0-100 (表示百分比)
 * @retval 1: 触发(命中概率), 0: 未触发
 * @example if(rng_get_random_probability(30)) { // 30%概率掉落道具 }
 */
uint8_t rng_get_random_probability(uint8_t probability);


#endif // __RNG_DRIVER_H__
