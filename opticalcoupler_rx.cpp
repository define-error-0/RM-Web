#include "opticalcoupler_rx.h"
#include <string.h>

// 假设的BSP接口（需要根据实际MCU平台调整）
extern "C" {
    bool bspGpioRead(short gpio);
    void bspExtiSetInterrupt(short gpio, bool enable, bool rising, bool falling);
    uint32_t bspGetSystemTimeUs();
}

OpticalcouplerRx::OpticalcouplerRx(char cmd, short Gpio)
{
    this->cmd = cmd;
    this->Gpio = Gpio;
    this->isInitialized = false;
    resetRecvBuffer();
}

OpticalcouplerRx::~OpticalcouplerRx()
{
    deinit();
}

bool OpticalcouplerRx::init()
{
    if (isInitialized) return true;
    
    // 设置EXTI中断（上升沿和下降沿都触发）
    bspExtiSetInterrupt(this->Gpio, true, true, true);
    
    resetRecvBuffer();
    isInitialized = true;
    return true;
}

void OpticalcouplerRx::deinit()
{
    if (!isInitialized) return;
    
    // 关闭EXTI中断
    bspExtiSetInterrupt(this->Gpio, false, false, false);
    
    isInitialized = false;
}

void OpticalcouplerRx::resetRecvBuffer()
{
    memset(&recvBuffer, 0, sizeof(RecvBuffer_t));
    recvBuffer.state = RECV_IDLE;
    recvBuffer.frameComplete = false;
}

uint32_t OpticalcouplerRx::getCurrentTimeUs()
{
    return bspGetSystemTimeUs();
}

bool OpticalcouplerRx::detectBitValue(uint32_t highDuration)
{
    // 根据高电平持续时间判断位值
    // 40μs为逻辑0，70μs为逻辑1，取中间值55μs作为判断阈值
    return (highDuration >= 55);
}

void OpticalcouplerRx::handleGpioInterrupt()
{
    uint32_t currentTime = getCurrentTimeUs();
    bool gpioLevel = bspGpioRead(this->Gpio);
    
    if (gpioLevel) {
        // 上升沿 - 记录时间
        recvBuffer.lastEdgeTime = currentTime;
    } else {
        // 下降沿 - 计算高电平持续时间
        uint32_t highDuration = currentTime - recvBuffer.lastEdgeTime;
        
        switch (recvBuffer.state) {
            case RECV_IDLE:
                // 检测帧起始（150μs高电平）
                if (highDuration >= FRAME_START_HIGH_US - 20 && 
                    highDuration <= FRAME_START_HIGH_US + 20) {
                    recvBuffer.state = RECV_DATA;
                    recvBuffer.index = 0;
                    recvBuffer.size = 0;
                }
                break;
                
            case RECV_DATA:
                // 检测数据位
                if (highDuration >= BIT_0_HIGH_US - 15 && 
                    highDuration <= BIT_1_HIGH_US + 15) {
                    
                    bool bitVal = detectBitValue(highDuration);
                    
                    // 将位值存储到当前字节
                    if (recvBuffer.index < MAX_FRAME_SIZE) {
                        uint16_t byteIndex = recvBuffer.size / 8;
                        uint8_t bitIndex = recvBuffer.size % 8;
                        
                        if (bitIndex == 0) {
                            recvBuffer.buffer[byteIndex] = 0;
                        }
                        
                        if (bitVal) {
                            recvBuffer.buffer[byteIndex] |= (1 << bitIndex);
                        }
                        
                        recvBuffer.size++;
                        
                        // 检查是否接收到完整字节
                        if (bitIndex == 7) {
                            recvBuffer.index++;
                        }
                    }
                } else if (highDuration >= FRAME_END_LOW_US - 20) {
                    // 检测到帧结束
                    recvBuffer.state = RECV_END;
                    recvBuffer.frameComplete = true;
                } else {
                    // 无效的脉冲，重置接收状态
                    resetRecvBuffer();
                }
                break;
                
            case RECV_END:
                // 帧结束后的处理
                resetRecvBuffer();
                break;
                
            default:
                resetRecvBuffer();
                break;
        }
    }
}

void OpticalcouplerRx::extiInterruptHandler()
{
    if (isInitialized) {
        handleGpioInterrupt();
    }
}

char OpticalcouplerRx::recvData(uint8_t* data, short dataSize)
{
    if (!isInitialized || dataSize <= 0 || data == nullptr) {
        return -1;
    }

    if (!recvBuffer.frameComplete) {
        return -2; // 没有完整数据
    }

    // 计算接收到的字节数
    uint16_t receivedBytes = recvBuffer.size / 8;
    if (receivedBytes > dataSize) {
        receivedBytes = dataSize;
    }

    // 复制数据
    memcpy(data, recvBuffer.buffer, receivedBytes);
    
    // 清除缓冲区
    clearBuffer();
    
    return receivedBytes;
}

bool OpticalcouplerRx::hasData()
{
    return recvBuffer.frameComplete;
}

void OpticalcouplerRx::clearBuffer()
{
    resetRecvBuffer();
}

bool OpticalcouplerRx::isReady()
{
    return isInitialized;
}

RecvState_t OpticalcouplerRx::getRecvState()
{
    return recvBuffer.state;
} 