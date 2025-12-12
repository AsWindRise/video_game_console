#include "rng_driver.h"
#include "rng.h" // HAL库生成的RNG头文件

// -----------------------------------------------------------------------------
// 私有变量
// -----------------------------------------------------------------------------
static uint8_t rng_initialized = 0; // RNG初始化标志

// -----------------------------------------------------------------------------
// 公共函数实现
// -----------------------------------------------------------------------------

/**
 * @brief 初始化RNG硬件随机数生成器
 */
int8_t rng_init(void)
{
    // RNG已经在MX_RNG_Init中初始化了，这里只需要标记已初始化
    if (hrng.Instance == RNG)
    {
        rng_initialized = 1;
        return 0;
    }
    return -1;
}

/**
 * @brief 获取32位硬件随机数
 */
int8_t rng_get_random(uint32_t *random)
{
    if (!rng_initialized || random == NULL)
    {
        return -1;
    }

    // 使用HAL库阻塞式获取随机数，超时时间10ms
    HAL_StatusTypeDef status = HAL_RNG_GenerateRandomNumber(&hrng, random);

    return (status == HAL_OK) ? 0 : -1;
}

/**
 * @brief 获取指定范围内的随机数 [min, max]
 */
uint32_t rng_get_random_range(uint32_t min, uint32_t max)
{
    uint32_t random;

    // 参数校验
    if (min > max)
    {
        return min;
    }

    // 如果min等于max，直接返回
    if (min == max)
    {
        return min;
    }

    // 获取硬件随机数
    if (rng_get_random(&random) != 0)
    {
        return min; // 失败返回最小值
    }

    // 映射到[min, max]范围
    // 算法：random % (max - min + 1) + min
    uint32_t range = max - min + 1;
    return (random % range) + min;
}

/**
 * @brief 获取单字节随机数 [0, 255]
 */
uint8_t rng_get_random_byte(void)
{
    uint32_t random;

    if (rng_get_random(&random) != 0)
    {
        return 0; // 失败返回0
    }

    return (uint8_t)(random & 0xFF); // 取低8位
}

/**
 * @brief 获取随机布尔值
 */
uint8_t rng_get_random_bool(void)
{
    uint32_t random;

    if (rng_get_random(&random) != 0)
    {
        return 0;
    }

    return (random & 0x01); // 取最低位作为布尔值
}

/**
 * @brief 获取随机概率判断结果
 */
uint8_t rng_get_random_probability(uint8_t probability)
{
    // 参数校验：概率值必须在0-100之间
    if (probability > 100)
    {
        probability = 100;
    }

    if (probability == 0)
    {
        return 0; // 0%概率，必定不触发
    }

    if (probability >= 100)
    {
        return 1; // 100%概率，必定触发
    }

    // 生成0-99的随机数，与概率值比较
    uint32_t rand_value = rng_get_random_range(0, 99);

    return (rand_value < probability) ? 1 : 0;
}
