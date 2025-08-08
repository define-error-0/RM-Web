#include "opticalcoupler_usart.h"
#include <stdio.h>

// 全局光耦对象
OpticalcouplerUsart* g_opticalCoupler = nullptr;

// GPIO中断处理函数（需要在MCU的中断向量表中注册）
extern "C" void GPIO_IRQHandler(void)
{
    if (g_opticalCoupler) {
        g_opticalCoupler->gpioInterruptHandler();
    }
}

// 示例：发送数据
void sendExample()
{
    uint8_t data[] = {0x55, 0xAA, 0x12, 0x34};
    short dataSize = sizeof(data);
    
    printf("发送数据: ");
    for (int i = 0; i < dataSize; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");
    
    char result = g_opticalCoupler->sendData(data, dataSize);
    if (result == 0) {
        printf("发送成功\n");
    } else {
        printf("发送失败，错误码: %d\n", result);
    }
}

// 示例：接收数据
void receiveExample()
{
    uint8_t recvBuffer[64];
    short bufferSize = sizeof(recvBuffer);
    
    // 检查是否有数据
    if (g_opticalCoupler->hasData()) {
        char result = g_opticalCoupler->recvData(recvBuffer, bufferSize);
        if (result > 0) {
            printf("接收到 %d 字节数据: ", result);
            for (int i = 0; i < result; i++) {
                printf("0x%02X ", recvBuffer[i]);
            }
            printf("\n");
        } else {
            printf("接收失败，错误码: %d\n", result);
        }
    } else {
        printf("没有接收到数据\n");
    }
}

// 示例：周期性通信（每秒10次）
void periodicCommunicationExample()
{
    uint8_t sensorData[] = {0x01, 0x02, 0x03, 0x04};
    short dataSize = sizeof(sensorData);
    
    // 每100ms发送一次数据（每秒10次）
    while (1) {
        // 发送数据
        g_opticalCoupler->sendData(sensorData, dataSize);
        
        // 检查接收数据
        if (g_opticalCoupler->hasData()) {
            receiveExample();
        }
        
        // 延时100ms
        // bspMsDelay(100); // 需要根据实际MCU平台实现
    }
}

// 主函数示例
int main()
{
    // 创建光耦对象（假设使用GPIO 5）
    g_opticalCoupler = new OpticalcouplerUsart('O', 5);
    
    // 初始化
    if (!g_opticalCoupler->init()) {
        printf("光耦初始化失败\n");
        return -1;
    }
    
    printf("光耦初始化成功\n");
    
    // 使用示例
    sendExample();
    
    // 等待接收数据
    while (1) {
        receiveExample();
        // bspMsDelay(10); // 10ms检查一次
    }
    
    return 0;
}

// 定时器中断处理函数（可选，用于周期性发送）
extern "C" void TIMER_IRQHandler(void)
{
    static uint32_t counter = 0;
    counter++;
    
    // 每100ms（假设定时器中断为1ms）
    if (counter >= 100) {
        counter = 0;
        if (g_opticalCoupler && g_opticalCoupler->isReady()) {
            uint8_t data[] = {0xAA, 0x55, 0x01, 0x02};
            g_opticalCoupler->sendData(data, 4);
        }
    }
} 