#ifndef OPTICALCOUPLER_TX_H
#define OPTICALCOUPLER_TX_H

#include <stdint.h>
#include <stdbool.h>

// 光耦通信协议常量
#define FRAME_START_HIGH_US    150     // 帧起始高电平时间(μs)
#define FRAME_END_LOW_US       100     // 帧结束低电平时间(μs)
#define BIT_INTERVAL_US        30      // 位间隔时间(μs)
#define BIT_0_HIGH_US          40      // 逻辑0高电平时间(μs)
#define BIT_1_HIGH_US          70      // 逻辑1高电平时间(μs)
#define MAX_FRAME_SIZE         64      // 最大帧大小

class OpticalcouplerTx {
private:
    char cmd;
    short Gpio;
    bool isInitialized;
    
public:
    OpticalcouplerTx(char cmd, short Gpio);
    ~OpticalcouplerTx();
    
    // 初始化函数
    bool init();
    void deinit();
    
    // 发送函数
    char sendData(uint8_t* data, short dataSize);
    
    // 状态查询
    bool isReady();
};

#endif // OPTICALCOUPLER_TX_H 