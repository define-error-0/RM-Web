#include "opticalcoupler_rx.h"
#include <stdio.h>

// 全局RX对象
OpticalcouplerRx* g_opticalCouplerRx = nullptr;

// EXTI中断处理函数（需要在MCU的中断向量表中注册）
extern "C" void EXTI0_IRQHandler(void)
{
    if (g_opticalCouplerRx) {
        g_opticalCouplerRx->extiInterruptHandler();
    }
}

// 前向声明
void processReceivedData(uint8_t* data, short dataSize);

// 示例：接收数据
void receiveData()
{
    uint8_t recvBuffer[64];
    short bufferSize = sizeof(recvBuffer);
    
    // 检查是否有数据
    if (g_opticalCouplerRx->hasData()) {
        char result = g_opticalCouplerRx->recvData(recvBuffer, bufferSize);
        if (result > 0) {
            printf("RX: 接收到 %d 字节数据: ", result);
            for (int i = 0; i < result; i++) {
                printf("0x%02X ", recvBuffer[i]);
            }
            printf("\n");
            
            // 处理接收到的数据
            processReceivedData(recvBuffer, result);
        } else {
            printf("RX: 接收失败，错误码: %d\n", result);
        }
    } else {
        printf("RX: 没有接收到数据\n");
    }
}

// 示例：处理接收到的数据
void processReceivedData(uint8_t* data, short dataSize)
{
    if (dataSize < 2) {
        printf("RX: 数据长度不足\n");
        return;
    }
    
    // 根据数据头判断数据类型
    if (data[0] == 0x55 && data[1] == 0xAA) {
        printf("RX: 接收到传感器数据\n");
        // 处理传感器数据
        if (dataSize >= 6) {
            uint16_t sensorValue1 = (data[2] << 8) | data[3];
            uint16_t sensorValue2 = (data[4] << 8) | data[5];
            printf("RX: 传感器值1: %d, 传感器值2: %d\n", sensorValue1, sensorValue2);
        }
    } else if (data[0] == 0xAA && data[1] == 0x55) {
        printf("RX: 接收到状态信息\n");
        // 处理状态信息
        if (dataSize >= 5) {
            printf("RX: 状态码: 0x%02X, 参数1: 0x%02X, 参数2: 0x%02X\n", 
                   data[2], data[3], data[4]);
        }
    } else {
        printf("RX: 接收到未知类型数据\n");
    }
}

// 示例：周期性检查接收数据
void periodicReceiveExample()
{
    uint32_t counter = 0;
    
    printf("RX: 开始周期性检查接收数据\n");
    
    while (1) {
        // 检查接收数据
        receiveData();
        counter++;
        
        // 每100次检查打印一次状态
        if (counter % 100 == 0) {
            printf("RX: 已检查 %d 次，当前状态: %d\n", 
                   counter, g_opticalCouplerRx->getRecvState());
        }
        
        // 延时10ms检查一次
        // bspMsDelay(10); // 需要根据实际MCU平台实现
    }
}

// 示例：状态监控
void statusMonitorExample()
{
    printf("RX: 开始状态监控\n");
    
    while (1) {
        // 检查接收状态
        RecvState_t state = g_opticalCouplerRx->getRecvState();
        
        switch (state) {
            case RECV_IDLE:
                printf("RX: 状态 - 空闲\n");
                break;
            case RECV_START:
                printf("RX: 状态 - 接收起始位\n");
                break;
            case RECV_DATA:
                printf("RX: 状态 - 接收数据位\n");
                break;
            case RECV_END:
                printf("RX: 状态 - 接收结束位\n");
                break;
            default:
                printf("RX: 状态 - 未知\n");
                break;
        }
        
        // 检查是否有完整数据
        if (g_opticalCouplerRx->hasData()) {
            printf("RX: 有完整数据待处理\n");
            receiveData();
        }
        
        // 延时100ms检查一次状态
        // bspMsDelay(100); // 需要根据实际MCU平台实现
    }
}

// 主函数示例 - RX设备
int main()
{
    // 创建RX对象（假设使用GPIO 0，对应EXTI0）
    g_opticalCouplerRx = new OpticalcouplerRx('R', 0);
    
    // 初始化
    if (!g_opticalCouplerRx->init()) {
        printf("RX: 光耦初始化失败\n");
        return -1;
    }
    
    printf("RX: 光耦初始化成功\n");
    printf("RX: 等待接收数据...\n");
    
    // 开始周期性检查接收数据
    periodicReceiveExample();
    
    return 0;
}

// 其他EXTI中断处理函数（如果需要多个GPIO）
extern "C" void EXTI1_IRQHandler(void)
{
    // 处理其他EXTI中断
}

extern "C" void EXTI2_IRQHandler(void)
{
    // 处理其他EXTI中断
}

extern "C" void EXTI3_IRQHandler(void)
{
    // 处理其他EXTI中断
}

extern "C" void EXTI4_IRQHandler(void)
{
    // 处理其他EXTI中断
}

// EXTI9_5中断处理函数（EXTI5-EXTI9）
extern "C" void EXTI9_5_IRQHandler(void)
{
    // 处理EXTI5-EXTI9中断
}

// EXTI15_10中断处理函数（EXTI10-EXTI15）
extern "C" void EXTI15_10_IRQHandler(void)
{
    // 处理EXTI10-EXTI15中断
} 