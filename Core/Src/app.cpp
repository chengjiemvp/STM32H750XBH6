#include "main.h"
#include "gpio.h"
#include "fmc.h"
#include "sdram.hpp"

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
}