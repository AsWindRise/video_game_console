/**
 ******************************************************************************
 * @file    u8g2_stm32_hal.h
 * @brief   u8g2图形库的STM32 HAL适配层头文件
 * @author  老王
 * @note    这个文件提供u8g2库与STM32 HAL库之间的适配接口
 *          包含I2C通信和GPIO/延迟回调函数的声明
 ******************************************************************************
 */

#ifndef __U8G2_STM32_HAL_H__
#define __U8G2_STM32_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "u8g2.h"
#include "main.h"
#include "i2c.h"

/* Exported defines ----------------------------------------------------------*/
/**
 * @brief u8g2使用的I2C句柄
 * @note  这个宏定义了u8g2库使用的I2C外设,默认使用I2C1
 *        如果你用的是其他I2C外设(如I2C2),请修改这个宏
 */
#define U8G2_I2C_HANDLE     hi2c1

/**
 * @brief SSD1306的I2C从机地址
 * @note  SSD1306的7位地址是0x3C,写入时需要左移1位变成0x78
 *        这个地址是OLED模块的标准地址,一般不需要修改
 */
#define U8G2_I2C_ADDRESS    (0x3C << 1)

/* Exported types ------------------------------------------------------------*/
/* 无需导出类型 */

/* Exported variables --------------------------------------------------------*/
/**
 * @brief 全局u8g2实例
 * @note  这个全局变量在u8g2_stm32_hal.c中定义,应用层可以直接使用
 *        但建议通过u8g2_get_instance()函数获取,更符合封装原则
 */
extern u8g2_t g_u8g2;

/* Exported functions --------------------------------------------------------*/

/* ========== 底层回调函数 ========== */

/**
 * @brief u8g2的I2C硬件通信回调函数
 * @param u8x8: u8x8结构体指针(u8g2的底层结构)
 * @param msg: 消息类型(初始化、发送、开始、结束等)
 * @param arg_int: 整数参数(如发送字节数)
 * @param arg_ptr: 指针参数(如数据缓冲区指针)
 * @return 1表示成功,0表示失败
 *
 * @note 这个函数被u8g2库调用,用于处理所有I2C通信操作
 *       包括初始化、开始传输、发送数据、结束传输等
 *       这是u8g2移植的核心函数之一!
 */
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/**
 * @brief u8g2的GPIO和延迟回调函数
 * @param u8x8: u8x8结构体指针
 * @param msg: 消息类型(延迟、GPIO控制等)
 * @param arg_int: 整数参数(如延迟时间)
 * @param arg_ptr: 指针参数(一般不用)
 * @return 1表示成功,0表示失败
 *
 * @note 这个函数被u8g2库调用,用于处理延迟和GPIO操作
 *       包括毫秒延迟、微秒延迟、Reset引脚控制等
 *       这是u8g2移植的核心函数之二!
 */
uint8_t u8g2_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/* ========== 高层封装接口 ========== */

/**
 * @brief u8g2组件初始化函数
 * @return 0表示成功, -1表示失败
 *
 * @note 这个函数封装了u8g2的完整初始化流程:
 *       1. Setup - 设置显示驱动和回调函数
 *       2. InitDisplay - 初始化显示屏硬件
 *       3. SetPowerSave - 开启显示
 *       4. ClearDisplay - 清空显示内容
 *
 *       应用层只需要调用这一个函数即可完成u8g2初始化
 */
int u8g2_component_init(void);

/**
 * @brief 获取u8g2实例指针
 * @return u8g2实例指针
 *
 * @note 应用层通过这个函数获取u8g2实例,而不是直接访问全局变量
 *       这样更符合封装原则,便于后期维护
 */
u8g2_t* u8g2_get_instance(void);

/**
 * @brief 清空显示缓冲区并发送到屏幕
 *
 * @note 这是一个便捷函数,等价于:
 *       u8g2_ClearBuffer() + u8g2_SendBuffer()
 */
void u8g2_clear_screen(void);

/**
 * @brief 设置显示开关
 * @param on: 1=开启显示, 0=关闭显示(省电模式)
 *
 * @note 关闭显示后屏幕会黑屏,但u8g2缓冲区内容不会丢失
 */
void u8g2_set_display_on(uint8_t on);

#ifdef __cplusplus
}
#endif

#endif /* __U8G2_STM32_HAL_H__ */
