#ifdef BUILD_APPLICATION
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


int main(void) {
    // set up MPU before initializing HAL
    // call_mpu_config();
    // SCB_EnableICache();
    // SCB_EnableDCache();
    // 重新设置VTOR（虽然bootloader已设置，但为了保险）
    // SCB->VTOR = 0x90000000;

    HAL_Init();
    SystemClock_Config();

    // Initialize all configured peripherals
    // MX_GPIO_Init();
    MX_FMC_Init();
    bsp::sdram::init_sequence(&hsdram1);
    MX_RTC_Init();
    MX_USART1_UART_Init();
    printf("[T.T] ==================================================================================\n");
    printf("[APP] Application started from QSPI Flash!\n");
    printf("[DEG] USART1 init complete\n");
    // QSPI 已配置为XIP模式,禁止再次初始化和读写测试
    // test functions, e.g., test sdram
    printf("[DEG] all peripherals setup\n");
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    printf("[DEG] sysclk frequency: %lu Hz\n", (unsigned long)sysclk);
    printf("[DEG] running sdram test...\n");
    my_test::test_sdram();
    printf("[DEG] sdram test passed\n");
    bsp_setup::RTCDateTime rtc_date_time = bsp_setup::rtc_setup();
    printf("[DEG] rtc setup at: 20%02d-%02d-%02d %02d:%02d:%02d\n", 
        rtc_date_time.date.Year,
        rtc_date_time.date.Month,
        rtc_date_time.date.Date,
        rtc_date_time.time.Hours,
        rtc_date_time.time.Minutes,
        rtc_date_time.time.Seconds
    );

    while (1) {
        printf("[APP] Application running...\n");
        HAL_Delay(20000);
    }
}

#endif // BUILD_APPLICATION
