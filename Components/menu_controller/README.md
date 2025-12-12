# 菜单控制器 (Menu Controller)

## 📖 概述

这是一个专为**资源受限的嵌入式设备**设计的多级菜单系统，基于u8g2图形库开发，支持完整的导航、动作绑定和动态显示功能。

### 核心特性
- ✅ **双向链表结构** - O(1)复杂度的UP/DOWN导航性能
- ✅ **多级嵌套菜单** - 支持最多10层菜单深度（可配置）
- ✅ **循环滚动** - 列表首尾自动循环（Wrap-Around）
- ✅ **可见窗口管理** - 自动滚动显示，支持大菜单
- ✅ **动作函数绑定** - 支持函数指针回调
- ✅ **动态值显示** - 实时显示状态值（如"音量：75%"）
- ✅ **动态状态管理** - 支持菜单项禁用/隐藏
- ✅ **历史栈管理** - 记忆每层游标位置
- ✅ **输入解耦** - 自动适配input_manager
- ✅ **渲染解耦** - 自动适配u8g2显示库

### 资源占用
- **RAM占用**: 约2.2KB（50个菜单项）
- **Flash占用**: 约6.5KB（代码）
- **CPU占用**: 约5%（10ms任务周期）

---

## 📁 文件结构

```
Components/menu_controller/
├── menu_core.h          # 核心数据结构和API声明
├── menu_core.c          # 核心导航逻辑实现
├── menu_builder.h       # 构建器辅助宏和函数
├── menu_builder.c       # 构建器实现
├── menu_render.h        # 渲染接口
├── menu_render.c        # u8g2默认渲染器实现
├── menu_adapter.h       # input_manager适配器接口
├── menu_adapter.c       # 适配器实现
└── README.md            # 本文档
```

---

## 🚀 快速开始

### 第1步：添加文件到Keil项目

在Keil MDK项目中添加以下源文件：

**添加到`Components/menu_controller`组：**
- `Components/menu_controller/menu_core.c`
- `Components/menu_controller/menu_builder.c`
- `Components/menu_controller/menu_render.c`
- `Components/menu_controller/menu_adapter.c`

**添加头文件包含路径：**
```
Options for Target -> C/C++ -> Include Paths
添加：Components/menu_controller
```

### 第2步：最简单的使用示例

```c
// ==================== 文件：App/menu/main_menu.c ====================
#include "menu_core.h"
#include "menu_builder.h"
#include "menu_adapter.h"
#include "u8g2_stm32_hal.h"

// === 定义动作函数 ===
void action_start_game(void) {
    // 启动游戏逻辑
}

void action_settings(void) {
    // 打开设置界面
}

// === 定义菜单项 ===
MENU_ACTION(menu_start, "开始游戏", action_start_game);
MENU_ACTION(menu_settings, "设置", action_settings);
MENU_ACTION(menu_exit, "退出", NULL);

// === 初始化菜单 ===
menu_instance_t g_main_menu;

void main_menu_init(void) {
    // 链接菜单项为双向链表
    menu_item_t *items[] = {&menu_start, &menu_settings, &menu_exit};
    menu_link_items(items, 3);

    // 初始化菜单实例
    menu_init(&g_main_menu, &menu_start);

    // 使用适配器（自动绑定input_manager和u8g2）
    menu_adapter_init(&g_main_menu, u8g2_get_instance());

    // 激活菜单
    menu_activate(&g_main_menu);
}

// === 在主循环中调用 ===
void main_loop(void) {
    while (1) {
        scheduler_run();
        menu_task(&g_main_menu);  // 10ms调用一次
    }
}
```

### 第3步：带子菜单的复杂示例

```c
// === 游戏子菜单 ===
MENU_ACTION(menu_game_snake, "贪吃蛇", action_snake);
MENU_ACTION(menu_game_tetris, "俄罗斯方块", action_tetris);
MENU_ACTION(menu_game_pong, "乒乓球", action_pong);

// === 设置子菜单 ===
MENU_ACTION(menu_set_volume, "音量", action_volume);
MENU_ACTION(menu_set_brightness, "亮度", action_brightness);

// === 主菜单 ===
MENU_ITEM(menu_main_games, "游戏");
MENU_ITEM(menu_main_settings, "设置");

void main_menu_init(void) {
    // 构建游戏子菜单
    menu_item_t *game_items[] = {&menu_game_snake, &menu_game_tetris, &menu_game_pong};
    menu_link_items(game_items, 3);
    menu_set_submenu(&menu_main_games, &menu_game_snake);

    // 构建设置子菜单
    menu_item_t *set_items[] = {&menu_set_volume, &menu_set_brightness};
    menu_link_items(set_items, 2);
    menu_set_submenu(&menu_main_settings, &menu_set_volume);

    // 构建主菜单
    menu_item_t *main_items[] = {&menu_main_games, &menu_main_settings};
    menu_link_items(main_items, 2);

    // 初始化菜单实例
    menu_init(&g_main_menu, &menu_main_games);
    menu_adapter_init(&g_main_menu, u8g2_get_instance());
    menu_activate(&g_main_menu);
}
```

### 第4步：动态值显示（状态显示）

```c
// === 动态值函数 ===
const char* get_volume_value(void) {
    static char buf[8];
    uint8_t vol = system_get_volume();
    sprintf(buf, "%d%%", vol);
    return buf;
}

const char* get_sound_value(void) {
    return system_get_sound_enabled() ? "ON" : "OFF";
}

// === 带动态值的菜单项 ===
MENU_ACTION_WITH_VALUE(menu_volume, "音量", action_adjust_volume, get_volume_value);
MENU_ACTION_WITH_VALUE(menu_sound, "音效", action_toggle_sound, get_sound_value);

// 显示效果：
// > 音量  75%
//   音效  ON
```

### 第5步：动态禁用/隐藏菜单项

```c
// === 动态状态函数 ===
menu_item_state_t get_continue_state(void) {
    // 如果没有存档，则隐藏"继续游戏"
    return save_file_exists() ? MENU_ITEM_NORMAL : MENU_ITEM_HIDDEN;
}

menu_item_state_t get_online_state(void) {
    // 如果网络未连接，则禁用"在线对战"
    return network_is_connected() ? MENU_ITEM_NORMAL : MENU_ITEM_DISABLED;
}

// === 带动态状态的菜单项 ===
MENU_ACTION_WITH_STATE(menu_continue, "继续游戏", action_continue, get_continue_state);
MENU_ACTION_WITH_STATE(menu_online, "在线对战", action_online, get_online_state);
```

---

## 📚 API参考

### 初始化与配置

| 函数 | 说明 |
|------|------|
| `menu_init()` | 初始化菜单实例 |
| `menu_adapter_init()` | 一键绑定输入和渲染（推荐） |
| `menu_set_input_callbacks()` | 自定义输入回调 |
| `menu_set_render_callback()` | 自定义渲染回调 |
| `menu_set_visible_lines()` | 设置可见行数 |

### 运行时控制

| 函数 | 说明 |
|------|------|
| `menu_task()` | 主任务（输入+渲染） |
| `menu_activate()` | 激活菜单 |
| `menu_deactivate()` | 暂停菜单 |
| `menu_force_refresh()` | 强制刷新显示 |

### 构建器辅助

| 宏/函数 | 说明 |
|---------|------|
| `MENU_ITEM()` | 定义子菜单项 |
| `MENU_ACTION()` | 定义动作项 |
| `MENU_ACTION_WITH_VALUE()` | 定义带动态值的项 |
| `MENU_ACTION_WITH_STATE()` | 定义带动态状态的项 |
| `menu_link_items()` | 链接菜单项为双向链表 |
| `menu_set_submenu()` | 设置子菜单 |

### 查询接口

| 函数 | 说明 |
|------|------|
| `menu_get_current_item()` | 获取当前选中项 |
| `menu_get_depth()` | 获取当前菜单深度 |
| `menu_is_at_root()` | 判断是否在根菜单 |

---

## 🎨 自定义渲染

```c
// 自定义渲染函数
void my_custom_render(menu_instance_t *menu) {
    u8g2_t *u8g2 = (u8g2_t*)menu->render_context;

    u8g2_ClearBuffer(u8g2);
    // ... 自定义绘制逻辑 ...
    u8g2_SendBuffer(u8g2);
}

// 绑定自定义渲染
menu_set_render_callback(&g_main_menu, my_custom_render, u8g2_get_instance());
```

---

## ⚙️ 配置选项

在`menu_core.h`中可修改：

```c
#define MENU_MAX_DEPTH      10      // 最大菜单深度
#define MENU_MAX_LABEL_LEN  32      // 菜单项文本最大长度
```

在`menu_render.h`中可修改：

```c
#define MENU_DEFAULT_LINE_HEIGHT  12      // 默认行高
#define MENU_DEFAULT_SHOW_SCROLLBAR 1     // 默认显示滚动条
#define MENU_DEFAULT_SHOW_TITLE     1     // 默认显示标题栏
```

---

## 🐛 测试程序

参考`Test/test_menu.c`，包含完整的测试示例：
- 多级菜单（主菜单→游戏→设置）
- 动作函数绑定
- 动态值显示
- 完整导航测试

---

## 📖 设计文档

详细设计思路和架构说明，请参考项目根目录下的：
- `docs/architecture.md` - 系统架构文档
- `docs/progress.md` - 开发进度文档

---

## 🙏 致谢

本菜单系统遵循SOLID、KISS、DRY原则设计，专为STM32游戏掌机项目打造。

---

**版本**: v1.0
**最后更新**: 2025-12-10
**作者**: 老王（LaowangEngineer）
