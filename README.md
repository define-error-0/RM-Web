# 光耦传感器 SPL06 通信库

这是一个用于低速光耦传感器 SPL06 的 C++ 通信库，支持单向通信，每秒可传输 10 次数据。

## 特性

- **单向通信**：支持发送和接收功能
- **中断接收**：使用 GPIO 中断实现高效的数据接收
- **单总线发送**：类似单总线协议的发送方式
- **低速优化**：专为低速光耦传感器设计
- **帧格式**：包含起始位、数据位和结束位的完整帧格式

## 通信协议

### 帧格式
```
[起始位] [数据位0] [数据位1] ... [数据位N] [结束位]
```

### 时序规范
- **帧起始**：150μs 高电平
- **数据位0**：40μs 高电平 + 30μs 低电平
- **数据位1**：70μs 高电平 + 30μs 低电平
- **帧结束**：100μs 低电平
- **传输速率**：每秒 10 次（每 100ms 一次）

## 文件结构

```
├── opticalcoupler_usart.h    # 头文件
├── opticalcoupler_usart.cpp  # 实现文件
├── example_usage.cpp         # 使用示例
└── README.md                 # 说明文档
```

## 使用方法

### 1. 基本初始化

```cpp
#include "opticalcoupler_usart.h"

// 创建光耦对象（使用 GPIO 5）
OpticalcouplerUsart opticalCoupler('O', 5);

// 初始化
if (!opticalCoupler.init()) {
    // 初始化失败处理
    return -1;
}
```

### 2. 发送数据

```cpp
uint8_t data[] = {0x55, 0xAA, 0x12, 0x34};
short dataSize = sizeof(data);

char result = opticalCoupler.sendData(data, dataSize);
if (result == 0) {
    printf("发送成功\n");
} else {
    printf("发送失败，错误码: %d\n", result);
}
```

### 3. 接收数据

```cpp
uint8_t recvBuffer[64];
short bufferSize = sizeof(recvBuffer);

// 检查是否有数据
if (opticalCoupler.hasData()) {
    char result = opticalCoupler.recvData(recvBuffer, bufferSize);
    if (result > 0) {
        printf("接收到 %d 字节数据\n", result);
    }
}
```

### 4. 中断处理

需要在 MCU 的中断向量表中注册 GPIO 中断处理函数：

```cpp
// GPIO 中断处理函数
extern "C" void GPIO_IRQHandler(void)
{
    opticalCoupler.gpioInterruptHandler();
}
```

## BSP 接口要求

该库需要以下 BSP 函数支持：

```cpp
// GPIO 控制
void bspGpioTigger(short gpio, bool level);
bool bspGpioRead(short gpio);

// 延时函数
void bspUsDelay(uint32_t us);

// GPIO 中断设置
void bspGpioSetInterrupt(short gpio, bool enable, bool rising, bool falling);

// 系统时间（微秒）
uint32_t bspGetSystemTimeUs();
```

## 错误码说明

- `0`：操作成功
- `-1`：参数错误或未初始化
- `-2`：没有完整数据（接收时）

## 注意事项

1. **时序精度**：确保 `bspUsDelay()` 函数的精度，建议使用硬件定时器
2. **中断优先级**：GPIO 中断优先级应设置合适，避免影响其他功能
3. **缓冲区大小**：最大帧大小限制为 64 字节，可根据需要调整
4. **单向通信**：该库设计为单向通信，如需双向通信需要额外的协议设计
5. **低速特性**：专为低速光耦设计，不适合高速数据传输

## 示例应用

### 周期性通信

```cpp
// 每 100ms 发送一次数据（每秒 10 次）
while (1) {
    uint8_t sensorData[] = {0x01, 0x02, 0x03, 0x04};
    opticalCoupler.sendData(sensorData, 4);
    
    // 检查接收数据
    if (opticalCoupler.hasData()) {
        // 处理接收到的数据
    }
    
    bspMsDelay(100); // 延时 100ms
}
```

### 定时器中断发送

```cpp
// 定时器中断处理函数
extern "C" void TIMER_IRQHandler(void)
{
    static uint32_t counter = 0;
    counter++;
    
    // 每 100ms 发送一次
    if (counter >= 100) {
        counter = 0;
        uint8_t data[] = {0xAA, 0x55, 0x01, 0x02};
        opticalCoupler.sendData(data, 4);
    }
}
```

## 移植说明

1. 根据实际 MCU 平台实现 BSP 接口函数
2. 调整 GPIO 中断配置
3. 根据硬件特性调整时序参数
4. 测试通信可靠性

## 版本信息

- 版本：1.0.0
- 作者：AI Assistant
- 日期：2024
- 支持：低速光耦传感器 SPL06 