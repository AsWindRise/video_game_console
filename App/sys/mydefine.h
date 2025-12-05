#ifndef __MYDEFINE_H__
#define __MYDEFINE_H__

/* ========== HAL 库头文件 ========== */
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* ========== C 标准头文件 ========== */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* ========== 核心调度器头文件 ========== */
#include "scheduler.h"

/* ========== 驱动层头文件 ========== */
#include "key_driver.h"
#include "ebtn_driver.h"       //easy-button驱动层头文件
#include "uart_driver.h"       //串口驱动层头文件
#include "rocker_adc_driver.h" //摇杆adc驱动层头文件
#include "ssd1306.h"  // oled驱动核心
#include "ssd1306_fonts.h" // oled字体文件

/* ========== 组件层头文件 ========== */
#include "ebtn.h"          //easy-button组件库头文件
#include "ringbuffer.h"    //环形缓存区组件库头文件
#include "event_queue.h"   //消息队列组件库头文件
#include "rocker.h"        //摇杆处理组件库头文件
#include "input_manager.h" //用户输入抽象层
#include "u8g2.h"					 //u8g2图形组件库头文件
#include "u8g2_stm32_hal.h" //u8g2的STM32 HAL适配层

/* ========== 应用层头文件 ========== */
#include "system_assembly.h" //应用的入口装配文件，包含系统初始化和所有应用任务的注册。

/* ========== 其他 ========== */
#include "sys_key_monitor.h" //ebtn库移植测试文件
#include "test_rocker_adc.h" //摇杆底层测试头文件
#include "test_u8g2.h"       //u8g2图形库测试文件

/* ========== 句柄 ========== */


/* ========== 全局用户变量 ========== */



#endif
