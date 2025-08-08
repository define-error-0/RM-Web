#include "opticalcoupler_tx.h"

// 假设的BSP接口（需要根据实际MCU平台调整）
extern "C" {
    void bspGpioTigger(short gpio, bool level);
    void bspUsDelay(uint32_t us);
}

OpticalcouplerTx::OpticalcouplerTx(char cmd, short Gpio)
{
    this->cmd = cmd;
    this->Gpio = Gpio;
    this->isInitialized = false;
}

OpticalcouplerTx::~OpticalcouplerTx()
{
    deinit();
}

bool OpticalcouplerTx::init()
{
    if (isInitialized) return true;
    
    // 初始化GPIO为输出模式
    bspGpioTigger(this->Gpio, 0);  // 初始状态为低电平
    
    isInitialized = true;
    return true;
}

void OpticalcouplerTx::deinit()
{
    if (!isInitialized) return;
    
    // 设置GPIO为低电平
    bspGpioTigger(this->Gpio, 0);
    
    isInitialized = false;
}

char OpticalcouplerTx::sendData(uint8_t* data, short dataSize)
{
    if (!isInitialized || dataSize <= 0 || data == nullptr) {
        return -1;
    }

    if (dataSize > MAX_FRAME_SIZE) {
        return -2; // 数据太大
    }

    // 帧起始标记（150μs高电平）
    bspGpioTigger(this->Gpio, 1);
    bspUsDelay(FRAME_START_HIGH_US);
    bspGpioTigger(this->Gpio, 0);
    bspUsDelay(BIT_INTERVAL_US);

    // 发送数据位
    for (int i = 0; i < dataSize; i++) {
        uint8_t byte = data[i];
        for (int bit = 0; bit < 8; bit++) { // LSB优先
            bool bitVal = (byte >> bit) & 0x01;
            
            // 发送数据位：0=40μs高电平, 1=70μs高电平
            bspGpioTigger(this->Gpio, 1);
            bspUsDelay(bitVal ? BIT_1_HIGH_US : BIT_0_HIGH_US);
            bspGpioTigger(this->Gpio, 0);
            bspUsDelay(BIT_INTERVAL_US);
        }
    }

    // 帧结束（100μs低电平）
    bspGpioTigger(this->Gpio, 0);
    bspUsDelay(FRAME_END_LOW_US);
    
    return 0;
}

bool OpticalcouplerTx::isReady()
{
    return isInitialized;
} 