#ifdef BUILD_BOOTLOADER

#include <cstdio>
#include "main.h"
#include "gpio.h"
#include "fmc.h"
#include "sdram.hpp"
#include "test.hpp"
#include "usart.h"
#include "rtc.h"
#include "bsp_setup.hpp"
#include "quadspi.h"
#include "qspi_w25q256.hpp"

typedef void (*pFunction)(void);
#define APP_ADDRESS  0x90000000U  // QSPI Flash起始地址
void JumpToApplication(void);

/// @brief this is bootloader main function
int main(void) {
    // set up MPU before initializing HAL
    call_mpu_config();
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    SystemClock_Config();

    // Initialize all configured peripherals
    MX_GPIO_Init();
    MX_FMC_Init();
    MX_USART1_UART_Init();
    printf("[T.T] ==================================================================================\n");
    printf("[DEG] BOOTLOADER\n");
    printf("[DEG] USART1 init complete\n");
    MX_QUADSPI_Init();
    printf("[DEG] QSPI init complete\n");
    int8_t w25q256_status = QSPI_W25Qxx_Init();
    printf("[DEG] W25Q256 INIT RESULT: %d\n", w25q256_status);
    QSPI_W25Qxx_MemoryMappedMode();
    printf("[DEG] QSPI memory-mapped mode set\n");
    // test functions, e.g., test sdram
    printf("[DEG] all peripherals setup\n");

    HAL_Delay(500);  // 短暂延迟，确保外设稳定
    JumpToApplication();

    printf("[LOG] jump to application running passed!\n");
    while (1) {
        HAL_Delay(1);
    }
}

void JumpToApplication(void)
{
    uint32_t appStack = *(__IO uint32_t*)APP_ADDRESS;
    uint32_t appEntry = *(__IO uint32_t*)(APP_ADDRESS + 4);
    
    // 验证应用程序有效性
    bool isValidStackPointer = 
        ((appStack >= 0x20000000) && (appStack <= 0x20020000)) ||  // ✅ 改为 <=
        ((appStack >= 0x24000000) && (appStack <= 0x24080000)) ||
        ((appStack >= 0x30000000) && (appStack <= 0x30048000)) ||
        ((appStack >= 0x38000000) && (appStack <= 0x38010000));


    if (!isValidStackPointer) {
        printf("[ERR] Invalid stack pointer: 0x%08lX\n", appStack);
        return;
    }

    if ((appEntry < 0x90000000) || (appEntry >= 0x92000000)) {
        printf("[ERR] Invalid entry point: 0x%08lX\n", appEntry);
        return;
    }

    printf("[DEG] Jumping to application...\n");
    printf("[DEG] Stack: 0x%08lX, Entry: 0x%08lX\n", appStack, appEntry);

    // 关闭中断
    __disable_irq();

    // 去初始化外设
    HAL_UART_DeInit(&huart1);
    // HAL_QSPI_DeInit(&hqspi);
    // 如果使用了其他外设也需要去初始化

    // 禁用SysTick
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // 清除所有中断使能和挂起位
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 设置中断向量表偏移
    SCB->VTOR = APP_ADDRESS;
    
    // 设置主堆栈指针
    __set_MSP(appStack);
    
    // 启用中断
    __enable_irq();
    
    // 跳转到应用程序
    pFunction appMain = (pFunction)appEntry;
    appMain();
}

#endif // BUILD_BOOTLOADER
