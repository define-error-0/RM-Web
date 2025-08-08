#include "opticalcoupler_tx.h"
#include <stdio.h>

// 全局TX对象
OpticalcouplerTx* g_opticalCouplerTx = nullptr;

// 示例：发送传感器数据
void sendSensorData()
{
    uint8_t sensorData[] = {0x55, 0xAA, 0x12, 0x34, 0x56, 0x78};
    short dataSize = sizeof(sensorData);
    
    printf("TX: 发送传感器数据: ");
    for (int i = 0; i < dataSize; i++) {
        printf("0x%02X ", sensorData[i]);
    }
    printf("\n");
    
    char result = g_opticalCouplerTx->sendData(sensorData, dataSize);
    if (result == 0) {
        printf("TX: 发送成功\n");
    } else {
        printf("TX: 发送失败，错误码: %d\n", result);
    }
}

// 示例：发送状态信息
void sendStatusInfo()
{
    uint8_t statusData[] = {0xAA, 0x55, 0x01, 0x02, 0x03};
    short dataSize = sizeof(statusData);
    
    printf("TX: 发送状态信息: ");
    for (int i = 0; i < dataSize; i++) {
        printf("0x%02X ", statusData[i]);
    }
    printf("\n");
    
    char result = g_opticalCouplerTx->sendData(statusData, dataSize);
    if (result == 0) {
        printf("TX: 发送成功\n");
    } else {
        printf("TX: 发送失败，错误码: %d\n", result);
    }
}

// 示例：周期性发送（每秒10次）
void periodicSendExample()
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    short dataSize = sizeof(data);
    uint32_t counter = 0;
    
    printf("TX: 开始周期性发送（每秒10次）\n");
    
    while (1) {
        // 发送数据
        g_opticalCouplerTx->sendData(data, dataSize);
        counter++;
        
        printf("TX: 第 %d 次发送完成\n", counter);
        
        // 延时100ms（每秒10次）
        // bspMsDelay(100); // 需要根据实际MCU平台实现
    }
}

// 示例：定时器中断发送
void timerSendExample()
{
    // 这个函数在定时器中断中调用
    static uint32_t counter = 0;
    counter++;
    
    // 每100ms发送一次（假设定时器中断为1ms）
    if (counter >= 100) {
        counter = 0;
        
        if (g_opticalCouplerTx && g_opticalCouplerTx->isReady()) {
            uint8_t data[] = {0xAA, 0x55, 0x01, 0x02};
            g_opticalCouplerTx->sendData(data, 4);
        }
    }
}

// 主函数示例 - TX设备
int main()
{
    // 创建TX对象（假设使用GPIO 5）
    g_opticalCouplerTx = new OpticalcouplerTx('T', 5);
    
    // 初始化
    if (!g_opticalCouplerTx->init()) {
        printf("TX: 光耦初始化失败\n");
        return -1;
    }
    
    printf("TX: 光耦初始化成功\n");
    
    // 发送测试数据
    sendSensorData();
    
    // 延时一段时间
    // bspMsDelay(1000);
    
    // 发送状态信息
    sendStatusInfo();
    
    // 开始周期性发送
    periodicSendExample();
    
    return 0;
}

// 定时器中断处理函数（可选）
extern "C" void TIMER_IRQHandler(void)
{
    timerSendExample();
} 