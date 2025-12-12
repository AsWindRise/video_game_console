/**
 ******************************************************************************
 * @file    u8g2_stm32_hal.c
 * @brief   u8g2图形库的STM32 HAL适配层实现
 * @author  老王
 * @note    这个文件实现u8g2库与STM32 HAL库之间的适配
 *          包含I2C通信和GPIO/延迟回调函数的具体实现
 *
 *          这个适配层是u8g2移植的核心部分
 *          没有这个文件u8g2库无法在STM32上正常运行
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "u8g2_stm32_hal.h"

/* Private defines -----------------------------------------------------------*/
/* 无需私有定义 */

/* Private variables ---------------------------------------------------------*/
/* 无需私有变量 */

/* Exported variables --------------------------------------------------------*/
/**
 * @brief 全局u8g2实例
 * @note  应用层通过u8g2_get_instance()获取这个实例
 */
u8g2_t g_u8g2;

/* Private function prototypes -----------------------------------------------*/
/* 无需私有函数 */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief u8g2的I2C硬件通信回调函数
 * @note  这个函数处理u8g2库的所有I2C通信请求
 *        参考教程:使用缓冲区批量发送,而不是每次都发送
 */
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    /* 静态缓冲区:u8g2最多发送32字节 */
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    uint8_t *data;

    switch(msg)
    {
        case U8X8_MSG_BYTE_INIT:
            /* I2C已经初始化,不需要额外操作 */
            break;

        case U8X8_MSG_BYTE_SET_DC:
            /* I2C模式不需要DC引脚 */
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            /* 开始传输:重置缓冲区索引 */
            buf_idx = 0;
            break;

        case U8X8_MSG_BYTE_SEND:
            /* 将数据存入缓冲区,而不是立即发送 */
            data = (uint8_t *)arg_ptr;
            while(arg_int > 0)
            {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            /* 结束传输:一次性发送缓冲区的所有数据 */
            if(HAL_I2C_Master_Transmit(&U8G2_I2C_HANDLE,
                                       u8x8_GetI2CAddress(u8x8),
                                       buffer,
                                       buf_idx,
                                       HAL_MAX_DELAY) != HAL_OK)
            {
                return 0;
            }
            break;

        default:
            return 0;
    }

    return 1;
}

/**
 * @brief u8g2的GPIO和延迟回调函数
 * @note  这个函数处理u8g2库的所有GPIO和延迟请求
 *        主要用于延迟操作,某些OLED初始化需要精确的延迟
 *
 *        这个函数非常重要,延迟不准确可能导致OLED初始化失败
 */
uint8_t u8g2_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            /*
             * 【毫秒延迟】
             * arg_int: 延迟的毫秒数
             *
             * HAL_Delay()是STM32 HAL库提供的毫秒延迟函数
             * 基于SysTick定时器实现,精度1ms
             */
            HAL_Delay(arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            /*
             * 【10微秒延迟】
             * arg_int: 延迟的次数,每次10微秒
             * 总延迟 = arg_int * 10 微秒
             *
             * 注意:HAL_Delay()只能延迟毫秒,无法延迟微秒
             * 这里用循环空转的方式实现微秒延迟(不太精确)
             *
             * 如果需要精确的微秒延迟,可以用以下方法:
             * 1. 使用DWT(Data Watchpoint and Trace)计数器
             * 2. 使用定时器(TIM)
             * 3. 使用汇编NOP指令精确延迟
             *
             * 对于大多数OLED来说,这个粗略的延迟足够了
             */
            for(uint16_t i = 0; i < arg_int; i++)
            {
                /*
                 * 这个循环延迟大约10微秒(取决于CPU频率)
                 * STM32F4@168MHz: 大约需要1680个时钟周期
                 *
                 * 注意:这个延迟不太精确,但对大多数OLED来说足够了
                 * 如果OLED初始化有问题,可以考虑使用DWT实现精确延迟
                 */
                for(volatile uint32_t j = 0; j < 168; j++)
                {
                    __NOP(); /* 空操作,防止编译器优化掉循环 */
                }
            }
            break;

        case U8X8_MSG_DELAY_100NANO:
            /*
             * 【100纳秒延迟】
             * arg_int: 延迟的次数,每次100纳秒
             * 总延迟 = arg_int * 100 纳秒
             *
             * 注意:100纳秒非常短,STM32@168MHz一个时钟周期约6ns
             * 这么短的延迟用软件实现精度不够,这里使用NOP指令象征性延迟
             *
             * 幸运的是大多数OLED不需要这么精确的纳秒级延迟
             */
            __NOP(); /* 空操作指令,象征性延迟 */
            break;

        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            /*
             * 【初始化GPIO】
             * 这个消息在u8g2_InitDisplay()时被调用
             *
             * 如果你的OLED有Reset引脚,需要在这里初始化
             * 我们用的是I2C的OLED,一般只有4根线(VCC/GND/SCL/SDA)
             * 没有Reset引脚,所以这里不需要做任何事情
             *
             * 如果你的OLED有Reset引脚,参考代码:
             * GPIO_InitTypeDef GPIO_InitStruct = {0};
             * GPIO_InitStruct.Pin = OLED_RESET_PIN;
             * GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
             * GPIO_InitStruct.Pull = GPIO_NOPULL;
             * GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
             * HAL_GPIO_Init(OLED_RESET_PORT, &GPIO_InitStruct);
             */
            break;

        case U8X8_MSG_GPIO_RESET:
            /*
             * 【控制Reset引脚】
             * arg_int: 1=高电平, 0=低电平
             *
             * 如果你的OLED有Reset引脚,需要在这里控制
             * Reset引脚用于硬件复位OLED控制器
             *
             * 我们没有Reset引脚,所以这里不需要做任何事情
             *
             * 如果你的OLED有Reset引脚,参考代码:
             * HAL_GPIO_WritePin(OLED_RESET_PORT, OLED_RESET_PIN,
             *                   arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
             */
            break;

        case U8X8_MSG_GPIO_I2C_CLOCK:
        case U8X8_MSG_GPIO_I2C_DATA:
            /*
             * 【控制I2C时钟和数据引脚】
             * 这两个消息用于软件模拟I2C(bit-banging)
             *
             * 我们用的是硬件I2C,不需要软件模拟
             * 所以这里不需要做任何事情
             *
             * 如果你要用软件I2C,需要在这里实现GPIO翻转
             */
            break;

        default:
            /* 未知消息,返回0表示不支持 */
            return 0;
    }

    /* 返回1表示操作成功 */
    return 1;
}

/* ========== 高层封装接口实现 ========== */

/**
 * @brief u8g2组件初始化函数
 * @note  封装了u8g2的完整初始化流程,应用层只需调用这一个函数
 */
int u8g2_component_init(void)
{
    /*
     * 步骤1: Setup - 设置u8g2结构体
     * 参数说明:
     * - &g_u8g2: u8g2实例指针
     * - U8G2_R0: 显示方向(0度旋转,可选U8G2_R1/R2/R3表示90/180/270度)
     * - u8x8_byte_hw_i2c: I2C通信回调函数
     * - u8g2_gpio_and_delay_stm32: GPIO和延迟回调函数
     *
     * 函数名说明:
     * - ssd1306: 显示驱动芯片型号
     * - i2c: 通信方式(I2C)
     * - 128x64: 分辨率
     * - noname: 通用型号(不是特定厂商定制)
     * - f: Full buffer(全帧缓冲,需要1KB RAM,显示效果最好)
     *      也可以用_1()/_2()表示1页/2页缓冲(省RAM但需要分页刷新)
     */
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&g_u8g2,
                                           U8G2_R0,
                                           u8x8_byte_hw_i2c,
                                           u8g2_gpio_and_delay_stm32);

    /*
     * 步骤2: InitDisplay - 初始化显示屏硬件
     * 这个函数会发送SSD1306的初始化命令序列到OLED
     * 包括时钟频率、MUX比率、显示偏移、电荷泵等设置
     */
    u8g2_InitDisplay(&g_u8g2);

    /*
     * 步骤3: SetPowerSave - 设置电源模式
     * 参数: 0=正常显示, 1=睡眠模式(关闭显示)
     * 初始化后默认是睡眠模式,需要手动开启显示
     */
    u8g2_SetPowerSave(&g_u8g2, 0);

    /*
     * 步骤3.5: SetContrast - 设置对比度
     * 参数: 0-255, 255是最高对比度
     * 注意:有些OLED默认对比度很低,需要手动设置高一点
     */
    u8g2_SetContrast(&g_u8g2, 255);

    /*
     * 步骤4: ClearDisplay - 清空显示内容
     * 这个函数会清空OLED的GDRAM(显示存储器)
     * 确保屏幕上没有垃圾数据
     */
    u8g2_ClearDisplay(&g_u8g2);

    /* 初始化成功 */
    return 0;
}

/**
 * @brief 获取u8g2实例指针
 * @note  应用层通过这个函数获取u8g2实例,符合封装原则
 */
u8g2_t* u8g2_get_instance(void)
{
    return &g_u8g2;
}

/**
 * @brief 清空显示缓冲区并发送到屏幕
 * @note  这是一个便捷函数,适用于需要完全清空屏幕的场景
 */
void u8g2_clear_screen(void)
{
    u8g2_ClearBuffer(&g_u8g2);  /* 清空RAM缓冲区 */
    u8g2_SendBuffer(&g_u8g2);   /* 发送到OLED */
}

/**
 * @brief 设置显示开关
 * @param on: 1=开启, 0=关闭(省电模式)
 */
void u8g2_set_display_on(uint8_t on)
{
    u8g2_SetPowerSave(&g_u8g2, on ? 0 : 1);
}

/* Private functions ---------------------------------------------------------*/
/* 无私有函数 */

/**
 * ============================================================================
 * 【u8g2 STM32移植说明】
 *
 * 1. 这个文件实现了u8g2库需要的两个核心回调函数:
 *    - u8x8_byte_hw_i2c: I2C通信回调
 *    - u8g2_gpio_and_delay_stm32: GPIO和延迟回调
 *
 * 2. 这两个函数是u8g2库与STM32硬件之间的桥梁,缺一不可
 *
 * 3. I2C通信使用STM32 HAL库的HAL_I2C_Master_Transmit()函数
 *    这个函数会自动处理I2C的START、地址、数据、STOP信号
 *
 * 4. 延迟功能使用HAL_Delay()和空循环实现
 *    如果需要精确的微秒延迟,建议使用DWT或定时器
 *
 * 5. 如果OLED有Reset引脚,需要在GPIO相关的case中添加代码
 *
 * 6. 如果使用软件I2C,需要实现GPIO_I2C_CLOCK和GPIO_I2C_DATA的控制
 *
 * 有了这个适配层文件,u8g2库就能正常运行在STM32平台上了
 * ============================================================================
 */
