# STM32H750XBH6 项目

本项目基于 STM32H750XBH6，包含 Bootloader 和 QSPI XIP 应用，支持外部 SDRAM、RTC、USART 等外设。

## 目录结构
- Core/        主代码
- Drivers/     HAL/CMSIS 驱动
- cmake/       构建脚本
- build/       编译输出

## 编译方法
- 默认构建APP
- 单独命令构建bootloader
- 创建了task文件，可以在vscode中通过task分别构建和烧录APP和bootloader
    - STM32H750XX_FLASH.ld 是bootloader的链接器脚本
    - STM32H750XX_QSPI.ld 是APP的链接器脚本
- APP和bootloader在同一个工程中，无需双工程.
