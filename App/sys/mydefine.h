#ifndef __MYDEFINE_H__
#define __MYDEFINE_H__

/* ========== HAL 库头文件 ========== */
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "sdio.h"          //SDIO接口（TF卡）

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
#include "rng_driver.h"		 // 随机数生成器驱动
#include "gd25qxx.h"// flash驱动头文件

/* ========== 组件层头文件 ========== */
#include "ebtn.h"          //easy-button组件库头文件
#include "ringbuffer.h"    //环形缓存区组件库头文件
#include "event_queue.h"   //消息队列组件库头文件
#include "rocker.h"        //摇杆处理组件库头文件
#include "input_manager.h" //用户输入抽象层
#include "ball_physics.h"  //通用球物理模块（打砖块、乒乓球等游戏复用）
#include "u8g2.h"					 //u8g2图形组件库头文件
#include "u8g2_stm32_hal.h" //u8g2的STM32 HAL适配层
#include "menu_core.h"     //菜单控制器核心模块
#include "menu_builder.h"  //菜单构建器辅助工具
#include "menu_render.h"   //菜单渲染模块
#include "menu_adapter.h"  //菜单适配器（input_manager+u8g2）
#include "lfs.h"           //LittleFS文件系统核心
#include "lfs_port.h"      //LittleFS SPI Flash适配层
#include "fatfs.h"         //FATFS文件系统（SD卡）

/* ========== 应用层头文件 ========== */
// 输入应用层
#include "rocker_app.h"      //摇杆处理应用层头文件

// 游戏管理
#include "game_manager.h"    //游戏管理器（统一生命周期管理）

// 游戏实现
#include "snake_game.h"      //贪吃蛇游戏
#include "dino_game.h"       //恐龙跑酷游戏
#include "plane_game.h"      //打飞机游戏
#include "tetris_game.h"     //俄罗斯方块游戏
#include "breakout_game.h"   //打砖块游戏
#include "sokoban_game.h"    //推箱子游戏
#include "minesweeper_game.h"//扫雷游戏
#include "pacman_game.h"     //吃豆人游戏
#include "pong_game.h"       //乒乓球游戏

// 菜单系统
#include "main_menu.h"       //主菜单

// 系统装配
#include "system_assembly.h" //系统初始化和任务注册

/* ========== 其他 ========== */
//#include "sys_key_monitor.h" //ebtn库移植测试文件
//#include "test_rocker_adc.h" //摇杆底层测试头文件
//#include "test_u8g2.h"       //u8g2图形库测试文件
//#include "test_input_manager.h"//输入管理组件测试头文件
//#include "test_menu.h"         //菜单系统测试头文件
#include "test_littlefs.h"     //LittleFS文件系统测试头文件
#include "test_sdcard.h"       //SD卡(FATFS)测试头文件

/* ========== 句柄 ========== */


/* ========== 全局用户变量 ========== */



#endif
