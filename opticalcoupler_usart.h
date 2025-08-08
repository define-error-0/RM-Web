#ifndef OPTICALCOUPLER_USART_H
#define OPTICALCOUPLER_USART_H

#include <stdint.h>
#include <stdbool.h>

// 光耦通信协议常量
#define FRAME_START_HIGH_US    150     // 帧起始高电平时间(μs)
#define FRAME_END_LOW_US       100     // 帧结束低电平时间(μs)
#define BIT_INTERVAL_US        30      // 位间隔时间(μs)
#define BIT_0_HIGH_US          40      // 逻辑0高电平时间(μs)
#define BIT_1_HIGH_US          70      // 逻辑1高电平时间(μs)
#define FRAME_TIMEOUT_MS       50      // 帧超时时间(ms)
#define MAX_FRAME_SIZE         64      // 最大帧大小

// 接收状态枚举
typedef enum {
    RECV_IDLE = 0,            // 空闲状态
    RECV_START,               // 接收起始位
    RECV_DATA,                // 接收数据位
    RECV_END                  // 接收结束位
} RecvState_t;

// 接收缓冲区结构
typedef struct {
    uint8_t buffer[MAX_FRAME_SIZE];
    uint16_t index;
    uint16_t size;
    uint32_t lastEdgeTime;    // 上次边沿时间
    RecvState_t state;
    bool frameComplete;
} RecvBuffer_t;

class OpticalcouplerUsart {
private:
    char cmd;
    short Gpio;
    RecvBuffer_t recvBuffer;
    bool isInitialized;
    
    // 私有方法
    void resetRecvBuffer();
    void handleGpioInterrupt();
    uint32_t getCurrentTimeUs();
    bool detectBitValue(uint32_t highDuration);

public:
    OpticalcouplerUsart(char cmd, short Gpio);
    ~OpticalcouplerUsart();
    
    // 初始化函数
    bool init();
    void deinit();
    
    // 发送函数
    char sendData(uint8_t* data, short dataSize);
    
    // 接收函数
    char recvData(uint8_t* data, short dataSize);
    bool hasData();
    void clearBuffer();
    
    // 中断处理函数（需要在外部GPIO中断中调用）
    void gpioInterruptHandler();
    
    // 状态查询
    bool isReady();
    RecvState_t getRecvState();
};

#endif // OPTICALCOUPLER_USART_H 