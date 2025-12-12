#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "mydefine.h"

// =============================================================================
// 主菜单模块 - 游戏掌机的主界面
// 职责：
// 1. 提供游戏选择入口
// 2. 提供系统设置入口
// 3. 提供关于信息显示
// =============================================================================

// -----------------------------------------------------------------------------
// API函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 初始化主菜单
 * @note  在system_assembly_init中调用
 */
void main_menu_init(void);

/**
 * @brief 主菜单任务
 * @note  在调度器中注册，10ms周期调用
 */
void main_menu_task(void);

/**
 * @brief 激活主菜单
 * @note  从游戏返回时调用
 */
void main_menu_activate(void);

/**
 * @brief 暂停主菜单
 * @note  启动游戏时调用
 */
void main_menu_deactivate(void);

/**
 * @brief 强制刷新主菜单显示
 * @note  设置更改后调用
 */
void main_menu_refresh(void);

#endif // __MAIN_MENU_H__
