#include "stm32h7xx_hal.h"

void SysTick_Handler(void) {
    HAL_IncTick();
}

/**
 * @brief RGB 颜色设置辅助函数
 * @param r, g, b: 传入 1 代表点亮该通道，传入 0 代表熄灭
 */
void Set_RGB_Color(uint8_t r, uint8_t g, uint8_t b) {
    // 硬件避坑：开漏输出逻辑反转！
    // 传入 1 时：引脚输出低电平 (RESET)，LED 导通点亮
    // 传入 0 时：引脚输出高电平 (SET)，呈高阻态，LED 熄灭
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, r ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, g ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, b ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

int main(void) {
    // 1. 初始化
    HAL_Init();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // 2. 配置引脚：PE3 (红), PE4 (绿), PE5 (蓝)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // 必须开漏
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    // 初始状态：全灭
    Set_RGB_Color(0, 0, 0);

    // 3. 超级循环：10步色彩秀
    while (1) {
        // --- 7种基础物理混色 ---
        Set_RGB_Color(1, 0, 0); HAL_Delay(500); // 1. 纯红
        Set_RGB_Color(0, 1, 0); HAL_Delay(500); // 2. 纯绿
        Set_RGB_Color(0, 0, 1); HAL_Delay(500); // 3. 纯蓝
        
        Set_RGB_Color(1, 1, 0); HAL_Delay(500); // 4. 黄色 (红+绿)
        Set_RGB_Color(0, 1, 1); HAL_Delay(500); // 5. 青色 (绿+蓝)
        Set_RGB_Color(1, 0, 1); HAL_Delay(500); // 6. 洋红 (红+蓝)
        
        Set_RGB_Color(1, 1, 1); HAL_Delay(500); // 7. 白色 (红+绿+蓝全开)
        
        // --- 特殊状态 ---
        Set_RGB_Color(0, 0, 0); HAL_Delay(500); // 8. 熄灭 (黑)
        
        Set_RGB_Color(1, 0, 0); HAL_Delay(500); // 9. 警灯：红
        Set_RGB_Color(0, 0, 1); HAL_Delay(500); // 10. 警灯：蓝
    }
}