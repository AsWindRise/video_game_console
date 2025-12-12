# 游戏掌机工程架构文档

## 1. 分层架构

```
┌─────────────────────────────────────────────────────────┐
│                     应用层 (App)                        │
│              游戏逻辑 / 菜单系统 / 状态机                │
├─────────────────────────────────────────────────────────┤
│                   组件层 (Components)                   │
│ input_manager | event_queue | rocker | ebtn | scheduler │
│      u8g2 (显示组件) | littlefs (文件系统组件)          │
├─────────────────────────────────────────────────────────┤
│                    驱动层 (Bsp)                         │
│  ebtn_driver | rocker_adc_driver | uart_driver | oled   │
│           gd25qxx (SPI Flash) | sdio (SD卡)             │
├─────────────────────────────────────────────────────────┤
│                    HAL层 (Core)                         │
│    GPIO | ADC | DMA | TIM | USART | I2C | SPI | SDIO    │
└─────────────────────────────────────────────────────────┘
```

## 2. 目录结构

```
console/
├── App/                  # 应用层
│   ├── sys/              # 系统入口和装配
│   ├── input/            # 输入相关应用层封装
│   │   └── rocker_app    # 摇杆应用层封装（初始化+任务）
│   ├── game/             # 游戏实现 ✅
│   │   ├── snake_game    # 贪吃蛇游戏（16×8网格，动态速度）
│   │   ├── dino_game     # 恐龙跑酷游戏（横版跑酷，30fps）
│   │   ├── plane_game    # 打飞机游戏（横向卷轴STG，无尽模式）
│   │   ├── tetris_game   # 俄罗斯方块（10×20网格，7种方块，Wall Kick）
│   │   ├── breakout_game # 打砖块（3关卡，3种砖块，球物理）
│   │   ├── sokoban_game  # 推箱子（10×8网格，3关卡，推箱逻辑）
│   │   ├── minesweeper_game # 扫雷（10×8网格，3难度，递归展开）
│   │   ├── pacman_game   # 吃豆人（16×8迷宫，幽灵AI，能量豆）
│   │   ├── pong_game     # 乒乓球（双挡板，AI对战，球物理）
│   │   └── game_manager  # 游戏管理器（统一生命周期管理，C多态）
│   └── menu/             # 菜单系统 ✅
│       └── main_menu     # 主菜单（游戏选择+设置）
├── Bsp/                  # 板级驱动
│   ├── key/              # 按键驱动 (ebtn_driver)
│   ├── adc/              # 摇杆ADC驱动
│   ├── rng/              # 随机数生成器驱动
│   ├── oled/             # OLED驱动 (ssd1306库,备用)
│   ├── flash/            # SPI Flash驱动 (W25Q64/GD25Q64) ✅
│   │   └── gd25qxx       # GD25Q系列驱动（兼容W25Q）
│   └── uart/             # 串口驱动
├── Components/           # 平台无关组件
│   ├── ebtn/             # 按键处理库
│   ├── rocker/           # 摇杆处理组件
│   ├── event_queue/      # 事件队列
│   ├── input_manager/    # 用户输入抽象层 ✅
│   ├── scheduler/        # 任务调度器
│   ├── ringbuffer/       # 环形缓冲区
│   ├── ball_physics/     # 通用球物理组件（Breakout/Pong复用）✅
│   ├── menu_controller/  # 菜单控制器（core/builder/render/adapter）✅
│   ├── littlefs/         # LittleFS文件系统 ✅
│   │   ├── lfs.c/h       # LittleFS核心库
│   │   ├── lfs_port.c/h  # STM32 SPI Flash适配层
│   │   └── lfs_util.h    # 工具宏定义
│   └── u8g2/             # u8g2图形库及STM32适配层
├── Core/                 # STM32 HAL配置
├── Drivers/              # STM32 HAL库
├── Test/                 # 测试代码
└── docs/                 # 工程文档
```

## 3. 核心组件接口

### 3.1 事件队列 (event_queue)

**统一事件结构：**
```c
typedef struct {
    uint16_t source_id;   // 事件源ID (按键ID或ROCKER_SOURCE_ID)
    uint8_t  event_type;  // 事件类型
    uint32_t data;        // 附加数据
} app_event_t;
```

**API：**
```c
void event_queue_init(void);
bool event_queue_push(app_event_t evt);
bool event_queue_pop(app_event_t *evt_out);
```

### 3.2 按键驱动 (ebtn_driver)

**按键ID定义：**
```c
typedef enum {
    BTN_SW1 = 0,      // PE0
    BTN_SW2,          // PE1
    BTN_SW3,          // PE2
    BTN_SW4,          // PE3
    BTN_SK,           // PE4 (摇杆按键)
    BTN_MAX_COUNT,
    BTN_COMBO_0 = 101, // SW1 + SW2
    BTN_COMBO_1,       // SW1 + SW3
    BTN_COMBO_2,       // SW2 + SW3
} button_id_t;
```

**事件类型 (ebtn_evt_t)：**
| 事件 | 说明 |
|------|------|
| EBTN_EVT_ONPRESS | 按下 |
| EBTN_EVT_ONRELEASE | 释放 |
| EBTN_EVT_ONCLICK | 单击（含连击计数） |
| EBTN_EVT_KEEPALIVE | 长按保持（周期触发） |

**API：**
```c
void ebtn_driver_init(void);
void ebtn_process_task(void);  // 10ms周期调用
```

### 3.3 摇杆组件 (rocker)

**方向枚举：**
```c
typedef enum {
    ROCKER_DIR_CENTER = 0,
    ROCKER_DIR_UP,
    ROCKER_DIR_UP_RIGHT,
    ROCKER_DIR_RIGHT,
    ROCKER_DIR_DOWN_RIGHT,
    ROCKER_DIR_DOWN,
    ROCKER_DIR_DOWN_LEFT,
    ROCKER_DIR_LEFT,
    ROCKER_DIR_UP_LEFT
} rocker_direction_t;
```

**事件类型：**
| 事件 | 说明 |
|------|------|
| ROCKER_EVT_DIR_ENTER | 进入某方向 |
| ROCKER_EVT_DIR_LEAVE | 离开某方向 |
| ROCKER_EVT_DIR_HOLD | 持续保持 |

**事件数据打包：**
```c
#define ROCKER_SOURCE_ID 0x0100
#define ROCKER_EVT_PACK_DATA(dir, mag)   ((uint32_t)(dir) | ((uint32_t)(mag) << 8))
#define ROCKER_EVT_UNPACK_DIR(data)      ((rocker_direction_t)((data) & 0xFF))
#define ROCKER_EVT_UNPACK_MAG(data)      ((uint8_t)(((data) >> 8) & 0xFF))
```

**API：**
```c
void rocker_init(const rocker_config_t *config);
void rocker_update(uint16_t raw_x, uint16_t raw_y);
rocker_state_t rocker_get_state(void);
void rocker_event_enable(bool enable);
void rocker_event_hold_enable(bool enable, uint32_t interval_ms);
```

### 3.4 显示组件 (u8g2)

**硬件配置：**
- 显示屏：SSD1306 OLED (128x64像素)
- 通信接口：I2C (hi2c1)
- I2C地址：0x3C (7位地址)
- 缓冲模式：全帧缓冲 (Full Buffer, 1KB RAM)

**适配层文件：**
```
Components/u8g2/
├── u8g2_stm32_hal.h      # STM32适配层头文件
├── u8g2_stm32_hal.c      # STM32适配层实现
└── u8g2/*                # u8g2库源码
```

**核心配置宏：**
```c
#define U8G2_I2C_HANDLE     hi2c1        // I2C句柄
#define U8G2_I2C_ADDRESS    (0x3C << 1)  // I2C设备地址
```

**适配层API：**
```c
// 初始化u8g2组件 (完整初始化流程)
int u8g2_component_init(void);

// 获取u8g2实例指针
u8g2_t* u8g2_get_instance(void);

// 清空显示缓冲区并发送到屏幕
void u8g2_clear_screen(void);

// 设置显示开关 (0=关闭省电, 1=开启)
void u8g2_set_display_on(uint8_t on);
```

**回调函数 (移植核心)：**
```c
// I2C硬件通信回调 (批量发送)
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg,
                         uint8_t arg_int, void *arg_ptr);

// GPIO和延迟回调 (毫秒/微秒/纳秒延迟)
uint8_t u8g2_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg,
                                  uint8_t arg_int, void *arg_ptr);
```

**关键实现细节：**
1. **I2C批量传输模式**：使用静态缓冲区累积数据，在`U8X8_MSG_BYTE_END_TRANSFER`时一次性发送，避免SSD1306时序问题
2. **延迟实现**：
   - 毫秒延迟：使用`HAL_Delay()`
   - 微秒延迟：使用空循环+`__NOP()`
   - 纳秒延迟：符号性实现（大多数OLED不需要）
3. **无Reset引脚**：I2C OLED通常只有4线(VCC/GND/SCL/SDA)，GPIO相关消息直接返回成功

**常用u8g2 API示例：**
```c
u8g2_t* u8g2 = u8g2_get_instance();

// 清空缓冲区
u8g2_ClearBuffer(u8g2);

// 绘制图形
u8g2_DrawCircle(u8g2, 64, 32, 20, U8G2_DRAW_ALL);
u8g2_DrawBox(u8g2, 10, 10, 30, 20);

// 显示文本
u8g2_SetFont(u8g2, u8g2_font_10x20_tf);
u8g2_DrawStr(u8g2, 10, 30, "Hello");

// 发送到屏幕
u8g2_SendBuffer(u8g2);
```

**性能参数：**
- 刷新速率：约30-60 FPS (取决于I2C速率)
- I2C速率：100kHz (标准模式) 或 400kHz (快速模式)
- 缓冲区大小：1024字节 (128x64/8)

### 3.5 用户输入抽象层 (input_manager) ✅

**设计思路：**
- 采用物理按键抽象而非逻辑功能抽象
- 统一按键和摇杆输入为统一按键枚举
- 提供轮询接口（实时状态查询）
- 支持边缘触发（just_pressed/just_released）
- 支持双击检测

**物理按键定义：**
```c
typedef enum {
    // 方向键（来自摇杆）
    INPUT_BTN_UP = 0,
    INPUT_BTN_DOWN,
    INPUT_BTN_LEFT,
    INPUT_BTN_RIGHT,

    // 功能键（来自物理按键）
    INPUT_BTN_A,        // 对应SW3
    INPUT_BTN_B,        // 对应SW4
    INPUT_BTN_X,        // 对应SW2
    INPUT_BTN_Y,        // 对应SW1
    INPUT_BTN_START,    // 对应SK（摇杆中心按键）

    INPUT_BTN_MAX
} input_button_t;
```

**按键映射关系：**
| 硬件按键 | 逻辑按键 | 说明 |
|---------|---------|------|
| SW1 | Y | 扩展功能键 |
| SW2 | X | 扩展功能键 |
| SW3 | A | 主要确认键 |
| SW4 | B | 取消/返回键 |
| SK | START | 摇杆中心按键（开始/暂停）|

**API：**
```c
void input_manager_init(void);
void input_manager_task(void);                       // 10ms周期调用
uint8_t input_is_pressed(input_button_t btn);        // 轮询：是否按下
uint8_t input_is_just_pressed(input_button_t btn);   // 轮询：刚按下
uint8_t input_is_just_released(input_button_t btn);  // 轮询：刚释放
uint8_t input_is_double_click(input_button_t btn);   // 轮询：双击检测
input_state_t input_get_state(input_button_t btn);   // 获取完整状态
uint8_t input_any_direction_pressed(void);           // 任意方向键按下
uint8_t input_any_button_pressed(void);              // 任意功能键按下
```

**测试程序：**
- `Test/test_input_manager.c/h` - 实时显示所有按键和摇杆状态
- 显示边缘触发信息（刚按下/刚释放）
- 显示双击检测信息

### 3.6 摇杆应用层封装 (rocker_app) ✅

**设计目标：**
- 封装摇杆相关的所有初始化逻辑
- 提供统一的摇杆处理任务
- 自动完成中心点校准

**文件位置：**
- `App/input/rocker_app.c/h`

**API：**
```c
// 初始化摇杆系统（ADC驱动+组件+事件使能）
void rocker_app_init(void);

// 摇杆处理任务（10ms周期调用）
void rocker_process_task(void);
```

**职责：**
1. 从ADC驱动获取原始数据（Bsp层）
2. 上电自动校准中心点（等待100ms后采样）
3. 调用摇杆组件更新状态（Components层）
4. 组件内部自动推送事件到event_queue

### 3.7 贪吃蛇游戏 (snake_game) ✅

**设计目标：**
- 验证input_manager抽象层的有效性
- 验证完整的游戏循环架构
- 提供可玩的原型游戏

**文件位置：**
- `App/game/snake_game.c/h`

**游戏参数：**
```c
网格大小：16×8 (128格子)
像素大小：8×8像素/格 (正方形格子)
屏幕分辨率：128×64像素
初始速度：250ms/格 (4格/秒)
最快速度：100ms/格 (10格/秒)
加速条件：每吃30分(3个食物)加速一次
加速幅度：减少10ms
速度等级：Lv0-Lv15 (左上角显示)
```

**核心数据结构：**
```c
typedef struct {
    // 蛇身数据
    uint8_t body_x[128];        // X坐标数组（索引0为头部）
    uint8_t body_y[128];        // Y坐标数组
    uint8_t length;             // 当前蛇长度

    // 方向控制（方案A：锁存机制）
    direction_t direction;      // 当前方向（本次移动使用）
    direction_t next_direction; // 下一个方向（下次移动使用）

    // 游戏状态
    game_state_t game_state;    // 运行/暂停/游戏结束
    uint16_t score;             // 分数
    uint16_t update_interval;   // 动态更新间隔（动态速度系统）
} snake_game_t;
```

**核心API：**
```c
void snake_game_init(snake_game_t *game);           // 初始化游戏
void snake_game_update_input(snake_game_t *game);   // 处理输入（10ms调用）
void snake_game_update_logic(snake_game_t *game);   // 更新逻辑（动态间隔）
void snake_game_render(snake_game_t *game);         // 渲染画面
void snake_game_task(snake_game_t *game);           // 主任务（集成上述三个）
```

**方案A：下一方向锁存机制**
```c
// 问题：输入采样周期(10ms) < 游戏更新周期(250ms)
// 解决：使用next_direction暂存输入，防止丢失

// 输入处理（10ms调用）
void snake_game_update_input(snake_game_t *game)
{
    if (input_is_just_pressed(INPUT_BTN_UP))
    {
        if (game->direction != DIR_DOWN)  // 防反向
        {
            game->next_direction = DIR_UP;  // 锁存
        }
    }
    // ... 其他方向
}

// 游戏逻辑（动态间隔调用）
void snake_game_update_logic(snake_game_t *game)
{
    // 关键！应用锁存的方向
    game->direction = game->next_direction;

    // 移动蛇、碰撞检测...
}
```

**动态速度系统：**
```c
// 吃到食物时检查加速
if (ate_food)
{
    game->score += 10;

    // 每增加30分加速一次
    if (game->score - game->last_speed_up_score >= 30)
    {
        game->last_speed_up_score = game->score;

        // 减少更新间隔（最小100ms）
        if (game->update_interval > 100)
        {
            game->update_interval -= 10;
        }
    }
}
```

**游戏特性：**
- ✅ 完整游戏状态机（运行/暂停/游戏结束）
- ✅ 碰撞检测（墙壁+自身）
- ✅ 随机食物生成（使用rng_driver，do-while避免重叠）
- ✅ 动态速度系统（越吃越快，Lv0-Lv15）
- ✅ 分数统计（右上角显示）
- ✅ 重新开始功能（游戏结束后按START）
- ✅ UI显示（蛇=方块，食物=圆形，速度等级=Lv数字）

**架构验证成果：**
- ✅ 游戏代码完全通过input_manager抽象，无直接HAL调用
- ✅ 事件驱动架构工作正常（ebtn/rocker → event_queue → input_manager → game）
- ✅ 方案A锁存机制有效防止输入丢失和反向移动
- ✅ 动态速度系统流畅，玩家反馈良好

### 3.8 恐龙跑酷游戏 (dino_game) ✅

**设计目标：**
- Chrome断网小游戏的高还原度复刻
- 验证30fps帧率控制和对象池管理
- 提供流畅的跑酷体验

**文件位置：**
- `App/game/dino_game.c/h`

**游戏参数：**
```c
屏幕分辨率：128×64像素
目标帧率：30 FPS (33ms/帧)
恐龙尺寸：12×11像素 (Chrome原版像素画)
恐龙位置：X=16 (固定), Y动态(跳跃)
地面Y坐标：50
初始速度：3.5像素/帧
最大速度：6.5像素/帧
加速条件：每150分加速一次
普通跳跃高度：20像素
长按跳跃高度：28像素
跳跃总时长：450ms
```

**核心数据结构：**
```c
typedef struct {
    // 游戏状态
    dino_game_state_t game_state;       // READY/RUNNING/GAME_OVER
    uint8_t is_active;                  // 活跃标志
    void (*exit_callback)(void);        // 退出回调

    // 恐龙状态
    dino_jump_state_t jump_state;       // IDLE/RISING/FALLING
    int16_t dino_y;                     // 恐龙Y坐标（脚底）
    uint32_t jump_start_time;           // 跳跃开始时间
    uint16_t current_jump_height;       // 当前跳跃高度（普通/高跳）
    uint8_t run_anim_frame;             // 跑步动画帧（0或1）

    // 障碍物系统（对象池）
    dino_obstacle_t obstacles[3];       // 最多3个障碍物
    uint32_t last_obstacle_time;        // 上次生成时间
    uint16_t next_obstacle_delay;       // 下次生成延迟

    // 云朵系统（装饰）
    dino_cloud_t clouds[3];             // 最多3朵云

    // 分数和速度
    uint32_t score;                     // 当前分数
    uint32_t high_score;                // 历史最高分
    float speed;                        // 当前速度（像素/帧）
} dino_game_t;
```

**核心API：**
```c
void dino_game_init(dino_game_t *game);
void dino_game_update_input(dino_game_t *game);     // 10ms调用
void dino_game_update_logic(dino_game_t *game);     // 处理跳跃物理
void dino_game_render(dino_game_t *game);           // 30fps渲染
void dino_game_task(dino_game_t *game);             // 主循环
```

**跳跃物理系统：**
```c
// 长按检测机制
if (按下A键) {
    记录按下时间
    开始跳跃（使用最小跳跃高度）
}
if (松开A键) {
    计算按压时长
    if (时长 >= 80ms) {
        升级为高跳（28像素）
    } else {
        保持普通跳跃（20像素）
    }
}

// 抛物线运动（使用正弦函数模拟）
float progress = (now - jump_start_time) / JUMP_DURATION;
if (progress <= 0.5f) {
    // 上升阶段：sin(0 → π/2)
    dino_y = ground_y - sin(progress * PI) * current_jump_height;
} else {
    // 下降阶段：sin(π/2 → π)
    dino_y = ground_y - sin(progress * PI) * current_jump_height;
}
```

**障碍物生成系统：**
```c
// 随机生成间隔（1000-2000ms）
next_delay = rng_get_random_range(1000, 2000);

// 对象池管理
for (i = 0; i < 3; i++) {
    if (!obstacles[i].active) {
        obstacles[i].active = 1;
        obstacles[i].x = SCREEN_WIDTH;
        obstacles[i].type = 随机类型;
        break;
    }
}

// 移动和销毁
obstacles[i].x -= speed;
if (obstacles[i].x < -width) {
    obstacles[i].active = 0;
}
```

**碰撞检测：**
```c
// AABB碰撞（矩形重叠检测）
// 恐龙碰撞箱收紧2像素，提升手感
bool collision = aabb_collision(
    dino_x + 2, dino_y + 2, DINO_WIDTH - 4, DINO_HEIGHT - 4,
    obstacle_x, obstacle_y, obstacle_width, obstacle_height
);
```

**游戏特性：**
- ✅ Chrome原版像素画还原（恐龙、仙人掌）
- ✅ 长按跳跃机制（80ms判定）
- ✅ 抛物线跳跃物理（正弦函数模拟）
- ✅ 跑步动画（2帧，150ms切换）
- ✅ 障碍物随机生成（对象池管理）
- ✅ 云朵背景装饰（慢速移动）
- ✅ 动态加速系统（每150分加速）
- ✅ 30fps稳定帧率控制
- ✅ 完整UI显示（分数、最高分、游戏提示）

### 3.9 打飞机游戏 (plane_game) ✅

**设计目标：**
- 横向卷轴弹幕射击游戏（类似雷电、沙罗曼蛇）
- 无尽模式，难度递增
- 验证复杂游戏逻辑和多对象管理

**文件位置：**
- `App/game/plane_game.c/h`

**游戏参数：**
```c
屏幕分辨率：128×64像素
目标帧率：30 FPS (33ms/帧)
玩家位置：X=10 (固定左侧), Y=8-48 (上下移动)
玩家尺寸：8×8像素
初始生命值：3条命
射击间隔：120ms
子弹速度：5.0像素/帧 (向右)
敌机子弹速度：-3.5像素/帧 (向左)
敌机生成间隔：700-1300ms (随难度缩短)
Boss出现条件：每500分
```

**核心数据结构：**
```c
typedef struct {
    // 核心状态管理
    plane_game_state_t game_state;      // READY/RUNNING/GAME_OVER
    uint8_t is_active;                  // 活跃标志
    void (*exit_callback)(void);        // 退出回调

    // 玩家
    int16_t player_x, player_y;         // 玩家位置
    uint8_t player_hp;                  // 生命值（3条命）
    uint8_t player_shield;              // 护盾（0=无，1=有）
    uint8_t weapon_level;               // 武器等级（1-3）

    // 对象池（使用active标志管理）
    bullet_t player_bullets[12];        // 玩家子弹
    enemy_t enemies[8];                 // 敌机
    bullet_t enemy_bullets[20];         // 敌机子弹
    powerup_t powerups[5];              // 道具（P/S/B）
    explosion_t explosions[8];          // 爆炸动画

    // Boss系统
    boss_t boss;                        // Boss数据
    uint8_t boss_warning;               // Boss警告标志
    uint32_t boss_warning_start_time;   // 警告开始时间

    // 无尽模式难度系统
    uint8_t difficulty_level;           // 难度等级（每击败1次Boss +1）
    uint8_t boss_count;                 // 已击败Boss数量

    // 游戏数据
    uint32_t score;                     // 当前分数
    uint32_t high_score;                // 历史最高分
} plane_game_t;
```

**5种敌机类型：**
| 类型 | 生命值 | 速度 | 尺寸 | 分数 | 射击概率 | 特性 |
|-----|--------|------|------|------|----------|------|
| ENEMY_TYPE_SMALL | 1 HP | 2.0像素/帧 | 7x6 | 10分 | 3%/帧 | 小型战斗机，快速 |
| ENEMY_TYPE_MEDIUM | 2 HP | 1.5像素/帧 | 9x8 | 20分 | 5%/帧 | 中型轰炸机，中速 |
| ENEMY_TYPE_HEAVY | 3 HP | 1.0像素/帧 | 11x10 | 30分 | 8%/帧 | 重型装甲，频繁开火 |
| ENEMY_TYPE_FAST | 1 HP | 3.5像素/帧 | 6x5 | 15分 | 1%/帧 | 超快速，波浪移动 |
| ENEMY_TYPE_BOSS | 20+ HP | 0.8像素/帧 | 16x16 | 200分 | 特殊 | Boss，三阶段攻击 |

**3种道具系统：**
| 道具 | 效果 | 尺寸 | 掉落概率 | 持续时间 |
|-----|------|------|----------|----------|
| POWERUP_WEAPON (P) | 火力升级（Lv1→Lv2→Lv3） | 8x8 | 15% | 永久 |
| POWERUP_SHIELD (S) | 护盾（免死1次） | 8x8 | 10% | 直到被击中 |
| POWERUP_BOMB (B) | 清屏炸弹 | 8x8 | 5% | 立即使用 |

**武器系统：**
```c
// Lv1：单发（居中）
spawn_bullet(player_x + 8, player_y + 3);

// Lv2：双发（上下）
spawn_bullet(player_x + 8, player_y);
spawn_bullet(player_x + 8, player_y + 6);

// Lv3：三发（上中下）
spawn_bullet(player_x + 8, player_y);
spawn_bullet(player_x + 8, player_y + 3);
spawn_bullet(player_x + 8, player_y + 6);
```

**Boss三阶段攻击系统：**
```c
// 阶段1 (HP 20-15): 单发瞄准弹
if (boss.phase == 1) {
    // 向玩家方向发射子弹
    float dx = player_x - boss_x;
    float dy = player_y - boss_y;
    spawn_bullet_directional(boss_x, boss_y, dx, dy);
}

// 阶段2 (HP 14-8): 扇形弹幕
else if (boss.phase == 2) {
    // 发射5发扇形弹幕
    for (int i = -2; i <= 2; i++) {
        float angle = i * 0.3f;
        spawn_bullet_angle(boss_x, boss_y, angle);
    }
}

// 阶段3 (HP 7-1): 圆形弹幕（狂暴）
else {
    // 发射8发圆形弹幕（全方向）
    for (int i = 0; i < 8; i++) {
        float angle = (i / 8.0f) * 2 * PI;
        spawn_bullet_angle(boss_x, boss_y, angle);
    }
}
```

**无尽模式难度递增系统：**
```c
// 击败Boss后难度提升
if (boss.hp <= 0) {
    difficulty_level++;  // 难度等级+1
    boss_count++;        // Boss击败数+1
    boss.active = 0;     // Boss销毁，游戏继续
}

// 难度影响：
// 1. 敌机速度：1.0 + difficulty × 0.08 (最高1.8倍)
// 2. 敌机生成间隔：缩短 difficulty × 40ms (最快400ms)
// 3. 敌机类型概率：更多重型/快速敌机
// 4. Boss HP：初始20 + boss_count × 5 (最高50 HP)
```

**Boss警告系统：**
```c
// Boss出现前显示2秒警告
if (boss_warning) {
    uint32_t elapsed = now - boss_warning_start_time;
    if (elapsed < 2000) {
        // 每300ms闪烁一次
        if ((elapsed / 300) % 2 == 0) {
            显示"WARNING! BOSS INCOMING"
        }
    }
}
```

**爆炸动画系统：**
```c
// 3帧爆炸动画（50ms/帧）
typedef struct {
    uint8_t active;
    int16_t x, y;
    explosion_type_t type;  // SMALL/MEDIUM/LARGE
    uint8_t frame;          // 0-2
    uint32_t last_frame_time;
} explosion_t;

// 帧序列：完整爆炸 → 扩散 → 消散
```

**游戏特性：**
- ✅ 5种敌机（小型、中型、重型、快速、Boss）
- ✅ 3种道具（P/S/B道具）
- ✅ 3级武器系统（单发→双发→三发）
- ✅ Boss三阶段攻击（单发→扇形→圆形弹幕）
- ✅ 无尽模式（难度递增，Boss HP增长）
- ✅ Boss警告系统（闪烁文字提示）
- ✅ 爆炸动画（3帧动画，50ms/帧）
- ✅ 完整UI显示（分数、生命、武器等级、难度等级）
- ✅ 优化后的Game Over界面（分数、Boss数、难度等级）

### 3.10 游戏管理器 (game_manager) ✅

**设计目标：**
- 统一所有游戏的生命周期管理
- 简化新游戏的接入流程
- 封装游戏与菜单的场景切换逻辑

**文件位置：**
- `App/game/game_manager.c/h`

**核心设计思想：**
```c
// 类似C++虚函数表的多态实现
typedef struct {
    void (*init)(void *instance);
    void (*activate)(void *instance);
    void (*deactivate)(void *instance);
    void (*task)(void *instance);
    void (*set_exit_callback)(void *instance, void (*callback)(void));
} game_interface_t;

// 游戏描述符（描述一个游戏）
typedef struct {
    const char *name;           // 游戏名称（如"Snake"）
    void *instance;             // 游戏实例指针
    game_interface_t interface; // 接口实现（函数指针表）
} game_descriptor_t;
```

**核心API：**
```c
void game_manager_init(void);
int game_manager_register(const game_descriptor_t *descriptor);
int game_manager_start_game(const char *game_name);
void game_manager_exit_current_game(void);
void game_manager_task_all(void);
```

**辅助宏（简化游戏接入）：**
```c
// 1. 定义游戏适配器（自动生成适配函数）
GAME_ADAPTER(snake_game, snake_game_t)
// 展开为：
// static void snake_game_adapter_init(void *instance) {
//     snake_game_init((snake_game_t *)instance);
// }
// ... (其他4个适配函数)

// 2. 定义游戏描述符（自动生成描述符变量）
GAME_DESCRIPTOR(g_snake_game, "Snake", snake_game)
// 展开为：
// static const game_descriptor_t g_snake_game_descriptor = {
//     .name = "Snake",
//     .instance = (void *)&g_snake_game,
//     .interface = {
//         .init = snake_game_adapter_init,
//         .activate = snake_game_adapter_activate,
//         // ...
//     }
// };
```

**游戏接入流程（3步）：**
```c
// Step 1: 定义游戏实例
static snake_game_t g_snake_game;

// Step 2: 定义适配器和描述符
GAME_ADAPTER(snake_game, snake_game_t)
GAME_DESCRIPTOR(g_snake_game, "Snake", snake_game)

// Step 3: 注册到游戏管理器
game_manager_register(&g_snake_game_descriptor);
```

**场景切换流程（自动化）：**
```c
// 从菜单启动游戏
int game_manager_start_game(const char *game_name) {
    // 1. 查找游戏描述符
    descriptor = find_game(game_name);

    // 2. 停用菜单
    main_menu_deactivate();

    // 3. 清空输入状态和事件队列
    input_manager_clear();
    event_queue_clear();

    // 4. 初始化游戏
    descriptor->interface.init(descriptor->instance);

    // 5. 设置退出回调
    descriptor->interface.set_exit_callback(
        descriptor->instance,
        game_manager_exit_current_game
    );

    // 6. 激活游戏
    descriptor->interface.activate(descriptor->instance);

    current_game = descriptor;
    return 0;
}

// 从游戏退出到菜单
void game_manager_exit_current_game(void) {
    // 1. 停用游戏
    current_game->interface.deactivate(current_game->instance);

    // 2. 清空输入状态和事件队列
    input_manager_clear();
    event_queue_clear();

    // 3. 激活菜单
    main_menu_activate();

    current_game = NULL;
}
```

**任务调度：**
```c
// 调用所有游戏的任务函数（游戏自己检查is_active）
void game_manager_task_all(void) {
    for (i = 0; i < registered_game_count; i++) {
        games[i]->interface.task(games[i]->instance);
    }
}
```

**架构优势：**
- ✅ 统一接口，所有游戏遵循相同的生命周期
- ✅ 简化接入，新游戏只需3步即可接入
- ✅ 解耦合，游戏不需要知道菜单的存在
- ✅ 集中管理，场景切换逻辑封装在一处
- ✅ 可扩展，支持动态注册多个游戏

### 3.11 主菜单 (main_menu) ✅

**设计目标：**
- 提供友好的游戏选择界面
- 支持游戏启动和设置调整
- 使用menu_controller组件实现

**文件位置：**
- `App/menu/main_menu.c/h`

**菜单结构：**
```
主菜单 (Main Menu)
├── Games （游戏）✅ 9款经典游戏
│   ├── Snake        → action_start_snake()
│   ├── Dino Run     → action_start_dino()
│   ├── Plane War    → action_start_plane()
│   ├── Tetris       → action_start_tetris()
│   ├── Breakout     → action_start_breakout()
│   ├── Sokoban      → action_start_sokoban()
│   ├── Minesweeper  → action_start_minesweeper()
│   ├── Pac-Man      → action_start_pacman()
│   └── Pong         → action_start_pong()
├── Settings （设置）
│   ├── Volume [75%] → action_adjust_volume()
│   ├── Sound [ON]   → action_toggle_sound()
│   ├── Brightness [80%] → action_adjust_brightness()
│   └── Reset        → action_reset_settings()
└── About （关于）   → action_about()
```

**核心API：**
```c
void main_menu_init(void);          // 初始化菜单
void main_menu_task(void);          // 主任务（10ms周期）
void main_menu_activate(void);      // 激活菜单
void main_menu_deactivate(void);    // 停用菜单
void main_menu_refresh(void);       // 强制刷新显示
```

**游戏启动流程（通过game_manager）：**
```c
void action_start_snake(void) { game_manager_start_game("Snake"); }
void action_start_dino(void) { game_manager_start_game("Dino"); }
void action_start_plane(void) { game_manager_start_game("Plane"); }
void action_start_tetris(void) { game_manager_start_game("Tetris"); }
void action_start_breakout(void) { game_manager_start_game("Breakout"); }
void action_start_sokoban(void) { game_manager_start_game("Sokoban"); }
void action_start_minesweeper(void) { game_manager_start_game("Minesweeper"); }
void action_start_pacman(void) { game_manager_start_game("Pac-Man"); }
void action_start_pong(void) { game_manager_start_game("Pong"); }
```

**动态值显示：**
```c
// 音量显示（0-100%）
const char *get_volume_value(void) {
    static char buf[8];
    sprintf(buf, "%d%%", g_volume);
    return buf;
}

// 音效开关显示（ON/OFF）
const char *get_sound_value(void) {
    return g_sound_on ? "ON" : "OFF";
}

// 亮度显示（0-100%）
const char *get_brightness_value(void) {
    static char buf[8];
    sprintf(buf, "%d%%", g_brightness);
    return buf;
}
```

**菜单定义宏（使用menu_builder）：**
```c
// 游戏子菜单项（9款游戏）
MENU_ACTION(menu_game_snake, "Snake", action_start_snake);
MENU_ACTION(menu_game_dino, "Dino Run", action_start_dino);
MENU_ACTION(menu_game_plane, "Plane War", action_start_plane);
MENU_ACTION(menu_game_tetris, "Tetris", action_start_tetris);
MENU_ACTION(menu_game_breakout, "Breakout", action_start_breakout);
MENU_ACTION(menu_game_sokoban, "Sokoban", action_start_sokoban);
MENU_ACTION(menu_game_minesweeper, "Minesweeper", action_start_minesweeper);
MENU_ACTION(menu_game_pacman, "Pac-Man", action_start_pacman);
MENU_ACTION(menu_game_pong, "Pong", action_start_pong);

// 设置子菜单项（带动态值显示）
MENU_ACTION_WITH_VALUE(menu_set_volume, "Volume",
                        action_adjust_volume, get_volume_value);
MENU_ACTION_WITH_VALUE(menu_set_sound, "Sound",
                        action_toggle_sound, get_sound_value);

// 主菜单项
MENU_ITEM(menu_main_games, "Games");
MENU_ITEM(menu_main_settings, "Settings");
MENU_ACTION(menu_main_about, "About", action_about);
```

**菜单初始化：**
```c
void main_menu_init(void) {
    // 1. 构建游戏子菜单（9款游戏）
    menu_item_t *game_items[] = {
        &menu_game_snake, &menu_game_dino, &menu_game_plane,
        &menu_game_tetris, &menu_game_breakout, &menu_game_sokoban,
        &menu_game_minesweeper, &menu_game_pacman, &menu_game_pong
    };
    menu_link_items(game_items, 9);
    menu_set_submenu(&menu_main_games, &menu_game_snake);

    // 2. 构建设置子菜单
    menu_item_t *setting_items[] = {
        &menu_set_volume, &menu_set_sound, &menu_set_brightness
    };
    menu_link_items(setting_items, 3);
    menu_set_submenu(&menu_main_settings, &menu_set_volume);

    // 3. 构建主菜单
    menu_item_t *main_items[] = {
        &menu_main_games, &menu_main_settings, &menu_main_about
    };
    menu_link_items(main_items, 3);

    // 4. 初始化菜单实例
    menu_init(&g_main_menu, &menu_main_games);

    // 5. 绑定输入和渲染
    menu_adapter_init(&g_main_menu, u8g2_get_instance());

    // 6. 设置可见行数
    menu_set_visible_lines(&g_main_menu, 3);

    // 7. 激活菜单
    menu_activate(&g_main_menu);
}
```

**菜单特性：**
- ✅ 多级菜单导航（主菜单→子菜单）
- ✅ 动态值显示（音量、亮度百分比）
- ✅ 统一游戏启动接口（通过game_manager）
- ✅ 设置即时生效（调整后立即刷新显示）
- ✅ 128x64屏幕优化（显示3行菜单项）
- ✅ 9款游戏集成（动作/射击/益智/街机全覆盖）

### 3.12 通用球物理组件 (ball_physics) ✅

**设计目标：**
- 提供可复用的球物理系统
- 支持碰撞检测和反弹计算
- 被Breakout和Pong复用

**文件位置：**
- `Components/ball_physics/ball_physics.c/h`

**核心数据结构：**
```c
// 球的物理状态
typedef struct {
    float x;            // 球中心X坐标
    float y;            // 球中心Y坐标
    float vx;           // X方向速度（像素/帧）
    float vy;           // Y方向速度（像素/帧）
    uint8_t radius;     // 球半径（像素）
} ball_physics_t;

// 矩形边界框（用于碰撞检测）
typedef struct {
    int16_t x;          // 矩形左上角X坐标
    int16_t y;          // 矩形左上角Y坐标
    uint8_t width;      // 矩形宽度
    uint8_t height;     // 矩形高度
} rect_t;
```

**核心API：**
```c
// 初始化球
void ball_init(ball_physics_t *ball, float x, float y,
               float vx, float vy, uint8_t radius);

// 更新球的位置
void ball_update(ball_physics_t *ball);

// 水平反弹（碰到左右墙壁）
void ball_reflect_horizontal(ball_physics_t *ball);

// 垂直反弹（碰到上下墙壁）
void ball_reflect_vertical(ball_physics_t *ball);

// 挡板反弹（带角度调整）
void ball_reflect_paddle(ball_physics_t *ball, int16_t paddle_x, uint8_t paddle_width);

// 检测球与矩形的碰撞
uint8_t ball_collides_with_rect(ball_physics_t *ball, rect_t *rect);

// 检测球与矩形的碰撞（详细版，返回碰撞位置）
uint8_t ball_collides_with_rect_detailed(ball_physics_t *ball, rect_t *rect,
                                          uint8_t *hit_top, uint8_t *hit_bottom,
                                          uint8_t *hit_left, uint8_t *hit_right);
```

**复用场景：**
- **Breakout（打砖块）**：球vs挡板、球vs砖块、球vs墙壁
- **Pong（乒乓球）**：球vs左挡板、球vs右挡板、球vs上下墙

**设计优势：**
- ✅ 单一职责：只负责球的物理运动和碰撞
- ✅ 平台无关：不依赖具体游戏逻辑
- ✅ 易复用：2款游戏共享同一套物理代码
- ✅ 易测试：物理计算可独立测试

### 3.13-3.18 新增游戏架构说明

#### 3.13 俄罗斯方块 (tetris_game) ✅

**核心特性：**
- 10×20网格（6×6像素格子）
- 7种方块（I/O/T/S/Z/J/L）
- Wall Kick旋转系统
- DAS连续移动机制
- 消行动画
- 等级递增系统

#### 3.14 打砖块 (breakout_game) ✅

**核心特性：**
- 复用ball_physics组件
- 3种砖块类型（普通1HP/坚固2HP/不可破坏）
- 3个关卡（基础/中等/困难）
- 连击系统（连续击砖加分）
- 挡板角度控制

#### 3.15 推箱子 (sokoban_game) ✅

**核心特性：**
- 10×8网格（网格模型）
- 3个关卡（2/3/4箱子）
- 箱子推动状态转换（BOX ↔ BOX_ON_TARGET）
- 步数统计
- 撤销功能（A键重新开始当前关）

#### 3.16 扫雷 (minesweeper_game) ✅

**核心特性：**
- 10×8网格，3个难度
- 首次点击安全保护（周围8格无雷）
- 递归展开算法（flood fill）
- 旗帜标记系统（Y键标记）
- 时间记录

#### 3.17 吃豆人 (pacman_game) ✅

**核心特性：**
- 16×8迷宫网格
- 方向预输入（转弯缓冲）
- 张嘴闭嘴动画（方向感知）
- 幽灵追踪AI（优先追距离远的轴）
- 能量豆反转机制（5秒无敌）

#### 3.18 乒乓球 (pong_game) ✅

**核心特性：**
- 复用ball_physics组件
- 玩家vs AI双挡板对战
- AI追踪算法（追踪球Y坐标，反应延迟）
- 挡板角度控制（击球位置影响反弹）
- 发球权系统（得分方发球）
- 先到11分获胜

## 4. 数据流

```
硬件输入                                      硬件输出
    │                                            ▲
    ▼                                            │
┌─────────────────┐     ┌─────────────────┐     │
│  ebtn_driver    │     │ rocker_adc_drv  │     │
│  (GPIO轮询)     │     │   (ADC+DMA)     │     │
└────────┬────────┘     └────────┬────────┘     │
         │                       │               │
         ▼                       ▼               │
┌─────────────────┐     ┌─────────────────┐     │
│     ebtn        │     │     rocker      │     │
│  (去抖/连击)    │     │ (滤波/死区/方向) │       │
└────────┬────────┘     └────────┬────────┘     │
         │                       │               │
         └───────────┬───────────┘               │
                     ▼                           │
              ┌─────────────┐                    │
              │ event_queue │                    │
              │  (统一事件)  │                    │
              └──────┬──────┘                    │
                     ▼                           │
              ┌─────────────┐                    │
              │input_manager│  ← 待实现          │
              │ (逻辑抽象)  │                    │
              └──────┬──────┘                    │
                     ▼                           │
                 应用层 ─────────────────────────┤
                  │                              │
                  └─► u8g2 (显示) ───────────────┘
                      (I2C → SSD1306 OLED)
```

## 5. 初始化顺序

```c
// system_assembly_init() 中调用顺序：
1. scheduler_init()        // 调度器先初始化
2. ebtn_driver_init()      // 按键驱动
3. event_queue_init()      // 事件队列
4. rocker_app_init()       // 摇杆应用层（包含ADC驱动+组件+事件使能）
5. u8g2_component_init()   // u8g2显示组件
6. rng_init()              // 随机数生成器驱动
7. input_manager_init()    // 输入管理器
8. game_manager_init()     // 游戏管理器初始化
9. main_menu_init()        // 主菜单初始化（最后初始化，激活菜单）

// 游戏注册（在game_manager_init之后，main_menu_init之前）：
game_manager_register(&g_snake_game_descriptor);        // 注册贪吃蛇游戏
game_manager_register(&g_dino_game_descriptor);         // 注册恐龙跑酷游戏
game_manager_register(&g_plane_game_descriptor);        // 注册打飞机游戏
game_manager_register(&g_tetris_game_descriptor);       // 注册俄罗斯方块
game_manager_register(&g_breakout_game_descriptor);     // 注册打砖块
game_manager_register(&g_sokoban_game_descriptor);      // 注册推箱子
game_manager_register(&g_minesweeper_game_descriptor);  // 注册扫雷
game_manager_register(&g_pacman_game_descriptor);       // 注册吃豆人
game_manager_register(&g_pong_game_descriptor);         // 注册乒乓球

// system_assembly_register_tasks() 中注册周期任务：
1. scheduler_add_task(ebtn_process_task, 10)       // 按键扫描
2. scheduler_add_task(rocker_process_task, 10)     // 摇杆处理
3. scheduler_add_task(input_manager_task, 10)      // 输入管理器
4. scheduler_add_task(game_manager_task_all, 10)   // 所有游戏任务（统一调度）
5. scheduler_add_task(main_menu_task, 10)          // 主菜单任务
```

## 6. 任务周期

| 任务 | 周期 | 说明 |
|------|------|------|
| ebtn_process_task | 10ms | 按键状态扫描（GPIO轮询+去抖） |
| rocker_process_task | 10ms | 摇杆数据采集、校准、组件更新 |
| input_manager_task | 10ms | 输入事件处理、状态更新（统一输入抽象） |
| game_manager_task_all | 10ms | 游戏任务统一调度（内部调用所有9个游戏的task） |
| └─ snake_game_task | 10ms | 贪吃蛇游戏任务（动态速度控制） |
| └─ dino_game_task | 10ms | 恐龙跑酷游戏任务（30fps帧率控制） |
| └─ plane_game_task | 10ms | 打飞机游戏任务（30fps帧率控制） |
| └─ tetris_game_task | 10ms | 俄罗斯方块游戏任务 |
| └─ breakout_game_task | 10ms | 打砖块游戏任务 |
| └─ sokoban_game_task | 10ms | 推箱子游戏任务 |
| └─ minesweeper_game_task | 10ms | 扫雷游戏任务 |
| └─ pacman_game_task | 10ms | 吃豆人游戏任务 |
| └─ pong_game_task | 10ms | 乒乓球游戏任务 |
| main_menu_task | 10ms | 主菜单任务（输入+渲染） |

**说明：**
- 所有游戏任务通过`game_manager_task_all`统一调度
- 游戏内部使用`is_active`标志决定是否执行
- 活跃游戏：执行完整游戏循环（输入+逻辑+渲染）
- 非活跃游戏：立即返回，不消耗CPU时间
- 30fps游戏（dino/plane）内部使用帧时间控制，实际渲染周期为33ms

## 7. 场景切换机制 ✅

### 7.1 设计目标

实现游戏与菜单之间的无缝切换，确保：
- 场景切换时输入状态干净（无残留事件）
- 任务始终运行，通过活跃标志控制前台场景
- 简洁的回调机制，避免紧耦合

### 7.2 核心机制

**活跃状态控制：**
```c
// 游戏和菜单都有is_active标志
typedef struct {
    uint8_t is_active;  // 1=活跃(前台), 0=停止(后台)
    // ...
} snake_game_t;

// 任务中检查活跃状态
void snake_game_task(snake_game_t *game) {
    if (!game->is_active) return;  // 非活跃直接返回
    // 执行游戏逻辑...
}
```

**状态清空函数：**
```c
// event_queue清空
void event_queue_clear(void) {
    __disable_irq();
    rt_ringbuffer_reset(&rb_event_queue);
    __enable_irq();
}

// input_manager清空
void input_manager_clear(void) {
    for (uint8_t i = 0; i < INPUT_BTN_MAX; i++) {
        btn_pressed[i] = 0;
        btn_just_pressed[i] = 0;
        btn_just_released[i] = 0;
    }
}
```

### 7.3 切换流程

**菜单 → 游戏：**
```c
void action_start_snake(void) {
    // 1. 停用菜单
    main_menu_deactivate();

    // 2. 清空输入状态（避免菜单残留事件）
    input_manager_clear();
    event_queue_clear();

    // 3. 初始化游戏
    snake_game_init(game);
    snake_game_set_exit_callback(game, on_snake_exit);

    // 4. 激活游戏
    snake_game_activate(game);
}
```

**游戏 → 菜单：**
```c
static void on_snake_exit(void) {
    // 1. 停用游戏
    snake_game_deactivate(game);

    // 2. 清空输入状态（避免游戏残留事件）
    input_manager_clear();
    event_queue_clear();

    // 3. 激活菜单
    main_menu_activate();
}
```

### 7.4 退出回调机制

**游戏端：**
```c
typedef struct {
    // 退出回调（按B键时调用）
    void (*exit_callback)(void);
} snake_game_t;

// 游戏检测退出键
void snake_game_update_input(snake_game_t *game) {
    if (input_is_just_pressed(INPUT_BTN_B)) {
        if (game->exit_callback != NULL) {
            game->exit_callback();  // 调用回调返回菜单
        }
        return;
    }
}
```

**菜单端：**
```c
// 设置退出回调
snake_game_set_exit_callback(game, on_snake_exit);
```

### 7.5 任务并发模型

```
调度器 (10ms周期)
    ↓
├─► ebtn_process_task       (始终运行)
├─► rocker_process_task     (始终运行)
├─► input_manager_task      (始终运行)
├─► snake_game_task         (is_active控制)
│   └─► if (!is_active) return;
└─► main_menu_task          (is_active控制)
    └─► if (!is_active) return;

场景切换：通过is_active标志控制谁在前台
菜单活跃 → 游戏活跃 → 菜单活跃 → ...
```

### 7.6 关键设计原则

1. ✅ **双向清空**：菜单→游戏、游戏→菜单都要清空输入状态
2. ✅ **时机正确**：在场景停用后、新场景激活前清空
3. ✅ **解耦合**：游戏不知道菜单存在，通过回调返回
4. ✅ **简洁性**：不使用复杂的状态机，活跃标志足够
5. ✅ **并发安全**：清空函数使用中断保护

---

---

## 8. 已实现游戏总览

### 游戏库（9款经典游戏）

| 游戏 | 类型 | 网格/分辨率 | 核心技术 | 复用组件 |
|------|------|------------|----------|----------|
| **Snake** | 动作 | 16×8 | 动态速度、方向锁存 | rng_driver |
| **Dino Run** | 动作 | 128×64 | 30fps、长按跳跃、对象池 | rng_driver |
| **Plane War** | 射击 | 128×64 | 30fps、5种敌机、Boss战、无尽模式 | rng_driver |
| **Tetris** | 益智 | 10×20 | 7种方块、Wall Kick、DAS、消行动画 | rng_driver |
| **Breakout** | 动作 | 128×64 | 3关卡、3种砖块、连击系统 | ball_physics, rng_driver |
| **Sokoban** | 益智 | 10×8 | 3关卡、推箱逻辑、状态转换 | - |
| **Minesweeper** | 益智 | 10×8 | 3难度、递归展开、首次安全 | rng_driver |
| **Pac-Man** | 街机 | 16×8 | 迷宫、幽灵AI、能量豆、动画 | rng_driver |
| **Pong** | 街机 | 128×64 | AI对战、挡板角度、发球权 | ball_physics |

### 通用组件复用统计

**ball_physics组件：**
- ✅ Breakout（打砖块）
- ✅ Pong（乒乓球）

**rng_driver随机数组件：**
- ✅ Snake（食物生成）
- ✅ Dino Run（障碍物生成）
- ✅ Plane War（敌机生成、道具掉落）
- ✅ Tetris（方块随机）
- ✅ Breakout（无随机，但依赖组件）
- ✅ Minesweeper（地雷布置）
- ✅ Pac-Man（幽灵随机移动）

**设计模式应用：**
- ✅ **多态模式**：game_manager统一管理9款游戏
- ✅ **对象池模式**：Dino/Plane/Breakout的对象管理
- ✅ **组件复用模式**：ball_physics被2款游戏复用
- ✅ **状态机模式**：所有游戏的状态管理

---

## 9. 存储系统架构 ✅

### 9.1 双存储系统概述

项目采用**双存储系统**设计，充分利用两种存储介质的特性：

```
┌────────────────────────────────────────────────────────────┐
│                    存储系统架构                             │
├────────────────────────────────────────────────────────────┤
│                  存储抽象层 (待实现)                        │
│         storage_manager | config_manager | save_manager    │
├─────────────────────────┬──────────────────────────────────┤
│   LittleFS (内部Flash)  │       FATFS (SD卡)               │
│   - 掉电安全            │       - 大容量                   │
│   - 磨损均衡            │       - 电脑可读                 │
│   - 系统配置            │       - 游戏存档/资源            │
├─────────────────────────┼──────────────────────────────────┤
│   lfs_port适配层        │       bsp_driver_sd              │
│   (Components/littlefs) │       (Middlewares/FATFS)        │
├─────────────────────────┼──────────────────────────────────┤
│   gd25qxx SPI Flash驱动 │       HAL_SD SDIO驱动            │
│   (Bsp/flash)           │       (Core/sdio.c)              │
├─────────────────────────┴──────────────────────────────────┤
│                    硬件层                                   │
│   W25Q64 (SPI1接口, 8MB)  │  SD卡 (SDIO 4位+DMA, 30GB)     │
└────────────────────────────────────────────────────────────┘
```

### 9.2 存储介质对比

| 特性 | LittleFS (SPI Flash) | FATFS (SD卡) |
|------|---------------------|--------------|
| **容量** | 8MB | 30GB+ |
| **读取速度** | ~200KB/s | ~4.9MB/s |
| **写入速度** | ~100KB/s | ~2.1MB/s |
| **掉电安全** | ✅ 强（日志结构） | ⚠️ 一般 |
| **磨损均衡** | ✅ 内置 | ❌ 无 |
| **电脑兼容** | ❌ 需专用工具 | ✅ 直接读写 |
| **适用场景** | 系统配置、关键数据 | 游戏存档、资源文件 |

### 9.3 存储分工策略

**LittleFS（内部Flash）存储：**
- 系统全局配置（音量、亮度、语言）
- 玩家账号索引
- 关键校验数据

**FATFS（SD卡）存储：**
- 玩家独立存档
- 游戏高分记录
- 资源文件（字体、图片）
- 日志文件

### 9.4 LittleFS适配层接口

**文件位置：** `Components/littlefs/lfs_port.c/h`

**硬件配置：**
```c
#define LFS_FLASH_BLOCK_SIZE     4096    // 块大小（4KB）
#define LFS_FLASH_BLOCK_COUNT    2048    // 块数量（8MB / 4KB）
#define LFS_FLASH_READ_SIZE      256     // 最小读取单位
#define LFS_FLASH_PROG_SIZE      256     // 最小写入单位
#define LFS_FLASH_CACHE_SIZE     256     // 缓存大小
#define LFS_FLASH_LOOKAHEAD_SIZE 16      // 预读大小
```

**核心API：**
```c
// 初始化并挂载LittleFS（自动格式化）
int lfs_port_init(void);

// 卸载LittleFS
int lfs_port_deinit(void);

// 获取LittleFS实例指针
lfs_t* lfs_port_get_instance(void);

// 文件操作
int lfs_port_file_open(const char *path, int flags);
int lfs_port_file_close(int fd);
int lfs_port_file_read(int fd, void *buffer, uint32_t size);
int lfs_port_file_write(int fd, const void *buffer, uint32_t size);

// 目录操作
int lfs_port_mkdir(const char *path);
int lfs_port_remove(const char *path);
int lfs_port_stat(const char *path, struct lfs_info *info);
```

**Flash回调实现：**
```c
// 读取回调（调用gd25qxx_read_buffer）
static int lfs_flash_read(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, void *buffer, lfs_size_t size);

// 写入回调（调用gd25qxx_write_buffer）
static int lfs_flash_prog(const struct lfs_config *c, lfs_block_t block,
                          lfs_off_t off, const void *buffer, lfs_size_t size);

// 擦除回调（调用gd25qxx_erase_sector）
static int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block);

// 同步回调（SPI Flash无需同步，直接返回成功）
static int lfs_flash_sync(const struct lfs_config *c);
```

### 9.5 FATFS接口

**文件位置：** `Middlewares/Third_Party/FatFs/`

**硬件配置（SDIO）：**
```c
// SDIO引脚配置
PC8  - SDIO_D0
PC9  - SDIO_D1
PC10 - SDIO_D2
PC11 - SDIO_D3
PC12 - SDIO_CK
PD2  - SDIO_CMD
PD3  - SD_DETECT (卡检测)

// SDIO参数
总线宽度: 4位
时钟分频: 2 (48MHz / 2 = 24MHz)
DMA: DMA2_Stream3, Channel4
```

**核心API：**
```c
// 挂载/卸载
FRESULT f_mount(FATFS *fs, const TCHAR *path, BYTE opt);

// 文件操作
FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode);
FRESULT f_close(FIL *fp);
FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br);
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT f_sync(FIL *fp);
FRESULT f_lseek(FIL *fp, FSIZE_t ofs);

// 目录操作
FRESULT f_mkdir(const TCHAR *path);
FRESULT f_opendir(DIR *dp, const TCHAR *path);
FRESULT f_readdir(DIR *dp, FILINFO *fno);
FRESULT f_closedir(DIR *dp);
FRESULT f_unlink(const TCHAR *path);
FRESULT f_rename(const TCHAR *path_old, const TCHAR *path_new);

// 容量查询
FRESULT f_getfree(const TCHAR *path, DWORD *nclst, FATFS **fatfs);
```

**文件打开模式：**
| 模式 | 说明 |
|------|------|
| FA_READ | 只读 |
| FA_WRITE | 只写 |
| FA_OPEN_EXISTING | 打开已存在文件（默认） |
| FA_CREATE_NEW | 创建新文件（已存在则失败） |
| FA_CREATE_ALWAYS | 创建新文件（已存在则覆盖） |
| FA_OPEN_ALWAYS | 打开文件（不存在则创建） |
| FA_OPEN_APPEND | 打开并移到文件末尾 |

### 9.6 SPI Flash驱动接口

**文件位置：** `Bsp/flash/gd25qxx.c/h`

**硬件配置：**
```c
// SPI引脚
PA4  - SPI_CS (片选)
PA5  - SPI_SCK (时钟)
PA6  - SPI_MISO (主入从出)
PA7  - SPI_MOSI (主出从入)

// Flash芯片：W25Q64 (8MB, 128个64KB块, 2048个4KB扇区)
```

**核心API：**
```c
// 初始化和ID读取
void gd25qxx_init(void);
uint32_t gd25qxx_read_id(void);  // 返回0xEF17表示W25Q64

// 读取
void gd25qxx_read_buffer(uint8_t *buffer, uint32_t addr, uint32_t size);

// 写入（自动处理跨页）
void gd25qxx_write_buffer(uint8_t *buffer, uint32_t addr, uint32_t size);

// 擦除
void gd25qxx_erase_sector(uint32_t sector_addr);  // 4KB扇区擦除
void gd25qxx_erase_block(uint32_t block_addr);    // 64KB块擦除
void gd25qxx_erase_chip(void);                     // 全片擦除
```

### 9.7 存储系统测试

**LittleFS测试：** `Test/test_littlefs.c/h`
```c
int test_lfs_run_all(void);      // 运行所有测试
int test_lfs_file_write(void);   // 文件写入测试
int test_lfs_file_read(void);    // 文件读取测试
int test_lfs_directory(void);    // 目录操作测试
int test_lfs_game_save(void);    // 游戏存档模拟测试
```

**FATFS测试：** `Test/test_sdcard.c/h`
```c
int test_sdcard_run_all(void);      // 运行基础测试
int test_sdcard_run_advanced(void); // 运行高级测试

// 基础测试
int test_sdcard_detect(void);    // SD卡检测
int test_sdcard_mount(void);     // FATFS挂载
int test_sdcard_write(void);     // 文件写入
int test_sdcard_read(void);      // 文件读取
int test_sdcard_append(void);    // 文件追加
int test_sdcard_directory(void); // 目录操作
int test_sdcard_capacity(void);  // 容量查询
int test_sdcard_game_save(void); // 游戏存档模拟

// 高级测试
int test_sdcard_write_speed(uint32_t size_kb);  // 写入速度测试
int test_sdcard_read_speed(uint32_t size_kb);   // 读取速度测试
int test_sdcard_nested_dirs(void);              // 嵌套目录测试
int test_sdcard_batch_files(uint32_t count);    // 批量文件测试
int test_sdcard_rename(void);                   // 重命名测试
```

### 9.8 存储系统初始化顺序

```c
// system_assembly_init() 中的存储系统初始化：
1. gd25qxx_init()           // SPI Flash驱动初始化
2. lfs_port_init()          // LittleFS挂载（自动格式化）
3. MX_FATFS_Init()          // FATFS驱动链接（CubeMX生成）
4. f_mount(&SDFatFS, ...)   // SD卡挂载（可选，运行时挂载）
```

---

文档版本: v6.0 | 更新日期: 2025-12-12 | 新增：存储系统架构（LittleFS+FATFS）
