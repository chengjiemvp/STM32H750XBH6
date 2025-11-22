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
    call_mpu_config();
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    SystemClock_Config();

    // Initialize all configured peripherals
    MX_GPIO_Init();
    MX_FMC_Init();
    bsp::sdram::init_sequence(&hsdram1);
    MX_RTC_Init();
    MX_USART1_UART_Init();
    printf("[T.T] ===================================================================\n");
    printf("[DEG] USART1 init complete\n");
    MX_QUADSPI_Init();
    printf("[DEG] QSPI init complete\n");
    int8_t w25q256_status = QSPI_W25Qxx_Init();
    printf("[DEG] W25Q256 INIT RESULT: %d\n", w25q256_status);
    printf("[DEG] Starting W25Q256 read/write test\n");
    QSPI_W25Qxx_Test();
    printf("[DEG] Starting W25Q256 read/write complete\n");
    
    // test functions, e.g., test sdram
    printf("[DEG] all peripherals setup\n");
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
        HAL_Delay(1);
    }
}
