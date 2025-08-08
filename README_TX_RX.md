# 光耦传感器 SPL06 TX/RX 分离式通信库

这是一个用于低速光耦传感器 SPL06 的分离式通信库，将发送和接收功能分别封装为 TX 和 RX 两个独立的类。

## 文件结构

```
├── opticalcoupler_tx.h         # TX设备头文件
├── opticalcoupler_tx.cpp       # TX设备实现文件
├── opticalcoupler_rx.h         # RX设备头文件
├── opticalcoupler_rx.cpp       # RX设备实现文件
├── example_tx_device.cpp       # TX设备使用示例
├── example_rx_device.cpp       # RX设备使用示例
└── README_TX_RX.md             # 说明文档
```

## TX 设备（发送端）

### 特性
- **纯发送功能**：只负责数据发送，不包含接收逻辑
- **简单易用**：直接调用 IO 翻转实现发送
- **轻量级**：代码简洁，资源占用少

### 使用方法

```cpp
#include "opticalcoupler_tx.h"

// 创建TX对象
OpticalcouplerTx txDevice('T', 5);  // 使用GPIO 5

// 初始化
if (!txDevice.init()) {
    // 初始化失败处理
    return -1;
}

// 发送数据
uint8_t data[] = {0x55, 0xAA, 0x12, 0x34};
txDevice.sendData(data, 4);
```

### TX 设备 BSP 接口要求

```cpp
// GPIO 控制
void bspGpioTigger(short gpio, bool level);

// 延时函数
void bspUsDelay(uint32_t us);
```

## RX 设备（接收端）

### 特性
- **EXTI 中断接收**：使用外部中断实现高效接收
- **状态机解析**：自动解析帧格式和数据位
- **缓冲区管理**：自动管理接收缓冲区

### 使用方法

```cpp
#include "opticalcoupler_rx.h"

// 创建RX对象
OpticalcouplerRx rxDevice('R', 0);  // 使用GPIO 0，对应EXTI0

// 初始化
if (!rxDevice.init()) {
    // 初始化失败处理
    return -1;
}

// 检查接收数据
if (rxDevice.hasData()) {
    uint8_t recvBuffer[64];
    char result = rxDevice.recvData(recvBuffer, 64);
    if (result > 0) {
        // 处理接收到的数据
    }
}
```

### RX 设备 BSP 接口要求

```cpp
// GPIO 读取
bool bspGpioRead(short gpio);

// EXTI 中断设置
void bspExtiSetInterrupt(short gpio, bool enable, bool rising, bool falling);

// 系统时间（微秒）
uint32_t bspGetSystemTimeUs();
```

## 中断配置

### TX 设备
TX 设备不需要中断，只需要 GPIO 输出功能。

### RX 设备
RX 设备需要在 MCU 中断向量表中注册 EXTI 中断处理函数：

```cpp
// 根据使用的GPIO选择对应的EXTI中断处理函数
extern "C" void EXTI0_IRQHandler(void)  // GPIO 0
{
    if (g_opticalCouplerRx) {
        g_opticalCouplerRx->extiInterruptHandler();
    }
}

extern "C" void EXTI1_IRQHandler(void)  // GPIO 1
{
    // 处理EXTI1中断
}

// ... 其他EXTI中断处理函数
```

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

## 使用示例

### TX 设备示例

```cpp
// 周期性发送（每秒10次）
void periodicSendExample()
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    
    while (1) {
        txDevice.sendData(data, 4);
        bspMsDelay(100);  // 延时100ms
    }
}

// 定时器中断发送
extern "C" void TIMER_IRQHandler(void)
{
    static uint32_t counter = 0;
    counter++;
    
    if (counter >= 100) {  // 每100ms发送一次
        counter = 0;
        uint8_t data[] = {0xAA, 0x55, 0x01, 0x02};
        txDevice.sendData(data, 4);
    }
}
```

### RX 设备示例

```cpp
// 周期性检查接收数据
void periodicReceiveExample()
{
    while (1) {
        if (rxDevice.hasData()) {
            uint8_t recvBuffer[64];
            char result = rxDevice.recvData(recvBuffer, 64);
            if (result > 0) {
                // 处理接收到的数据
                processReceivedData(recvBuffer, result);
            }
        }
        bspMsDelay(10);  // 10ms检查一次
    }
}

// 处理接收到的数据
void processReceivedData(uint8_t* data, short dataSize)
{
    if (data[0] == 0x55 && data[1] == 0xAA) {
        printf("接收到传感器数据\n");
    } else if (data[0] == 0xAA && data[1] == 0x55) {
        printf("接收到状态信息\n");
    }
}
```

## 错误码说明

### TX 设备
- `0`：发送成功
- `-1`：参数错误或未初始化
- `-2`：数据太大（超过最大帧大小）

### RX 设备
- `>0`：接收到的字节数
- `-1`：参数错误或未初始化
- `-2`：没有完整数据

## 注意事项

1. **GPIO 配置**：
   - TX 设备：GPIO 配置为输出模式
   - RX 设备：GPIO 配置为输入模式，启用上拉电阻

2. **中断优先级**：
   - RX 设备的 EXTI 中断优先级应设置合适
   - 避免中断嵌套影响时序精度

3. **时序精度**：
   - 确保 `bspUsDelay()` 函数的精度
   - 建议使用硬件定时器实现微秒级延时

4. **缓冲区管理**：
   - 及时调用 `recvData()` 读取数据
   - 避免缓冲区溢出

5. **单向通信**：
   - TX 和 RX 设备独立工作
   - 如需双向通信需要额外的协议设计

## 移植说明

1. **实现 BSP 接口函数**：
   - 根据实际 MCU 平台实现相应的 BSP 函数
   - 确保时序精度和中断响应速度

2. **配置中断向量表**：
   - 根据使用的 GPIO 配置对应的 EXTI 中断
   - 设置合适的中断优先级

3. **调整时序参数**：
   - 根据硬件特性调整延时参数
   - 测试通信可靠性

4. **测试验证**：
   - 使用示波器验证时序
   - 测试数据传输的准确性

## 版本信息

- 版本：2.0.0
- 作者：AI Assistant
- 日期：2024
- 支持：低速光耦传感器 SPL06
- 特性：TX/RX 分离式设计 