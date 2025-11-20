#include "main.h"
#include "gpio.h"
#include "fmc.h"
#include "sdram.hpp"
#include "test.hpp"

volatile uint32_t sink;
void cpu_stress_single_core() {
    for (;;) {
        // 简单但不可优化的整型运算
        uint32_t a = 0xA5A5A5A5;
        for (int i = 0; i < 1000; ++i) {
        a = (a << 1) ^ (a >> 3) ^ (a * 2654435761u);
        }
        sink ^= a;
    }
}

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
    // test functions, e.g., test sdram
    my_test::test_sdram();

    cpu_stress_single_core();

    while (1) {
        HAL_Delay(1);
    }
}