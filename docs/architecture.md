# 游戏掌机工程架构文档

## 1. 分层架构

```
┌─────────────────────────────────────────────────────────┐
│                     应用层 (App)                        │
│              游戏逻辑 / 菜单系统 / 状态机                │
├─────────────────────────────────────────────────────────┤
│                   组件层 (Components)                   │
│ input_manager | event_queue | rocker | ebtn | scheduler │
│                u8g2 (显示组件)                          │
├─────────────────────────────────────────────────────────┤
│                    驱动层 (Bsp)                         │
│  ebtn_driver | rocker_adc_driver | uart_driver | oled   │
├─────────────────────────────────────────────────────────┤
│                    HAL层 (Core)                         │
│         GPIO | ADC | DMA | TIM | USART | I2C            │
└─────────────────────────────────────────────────────────┘
```

## 2. 目录结构

```
console/
├── App/sys/              # 应用入口和系统装配
├── Bsp/                  # 板级驱动
│   ├── key/              # 按键驱动 (ebtn_driver)
│   ├── adc/              # 摇杆ADC驱动
│   ├── oled/             # OLED驱动 (ssd1306库,备用)
│   └── uart/             # 串口驱动
├── Components/           # 平台无关组件
│   ├── ebtn/             # 按键处理库
│   ├── rocker/           # 摇杆处理组件
│   ├── event_queue/      # 事件队列
│   ├── input_manager/    # 用户输入抽象层 [待实现]
│   ├── scheduler/        # 任务调度器
│   ├── ringbuffer/       # 环形缓冲区
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

### 3.5 用户输入抽象层 (input_manager) [待实现]

**设计目标：**
- 统一按键和摇杆输入为游戏风格抽象
- 提供轮询接口（实时状态查询）
- 提供事件接口（回调通知）
- 支持输入映射配置

**逻辑输入定义：**
```c
typedef enum {
    INPUT_ACTION_NONE = 0,
    // 方向
    INPUT_ACTION_UP,
    INPUT_ACTION_DOWN,
    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,
    // 功能键
    INPUT_ACTION_CONFIRM,    // 确认 (默认: SW1/SK)
    INPUT_ACTION_CANCEL,     // 取消 (默认: SW2)
    INPUT_ACTION_MENU,       // 菜单 (默认: SW3)
    INPUT_ACTION_OPTION,     // 选项 (默认: SW4)
} input_action_t;
```

**预期API：**
```c
void input_manager_init(void);
void input_manager_process(void);           // 主循环调用
bool input_is_pressed(input_action_t act);  // 轮询：是否按下
bool input_is_just_pressed(input_action_t act);  // 轮询：刚按下
bool input_is_just_released(input_action_t act); // 轮询：刚释放
bool input_poll_event(input_event_t *evt);  // 弹出一个输入事件
```

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
4. rocker_adc_driver_init()// 摇杆ADC
5. u8g2_component_init()   // u8g2显示组件
6. test_u8g2_init()        // u8g2测试初始化
7. input_manager_init()    // 输入管理器 [待实现]

// system_assembly_register_tasks() 中注册周期任务：
1. scheduler_add_task(test_task, 10)
2. scheduler_add_task(ebtn_process_task, 10)
3. scheduler_add_task(sys_monitor_task, 10)
4. scheduler_add_task(test_u8g2_task, 50)
```

## 6. 任务周期

| 任务 | 周期 | 说明 |
|------|------|------|
| test_task | 10ms | 按键测试任务 |
| ebtn_process_task | 10ms | 按键状态扫描 |
| sys_monitor_task | 10ms | 系统监控任务 |
| test_u8g2_task | 50ms | u8g2显示测试 (自动循环5种测试模式) |
| rocker_update | 20ms | 摇杆数据更新 [待集成] |
| input_manager_process | 10ms | 输入抽象处理 [待实现] |

文档版本: v2.0 | 更新日期: 2025-12-05
