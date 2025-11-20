#include "test.hpp"
#include "gpio.h"


namespace my_test {

    static void disable_caches_and_barrier() {
        // 禁用 I/D cache（根据你的 CMSIS 版本，函数名可能为 SCB_DisableDCache/ICache）
        SCB_DisableDCache();
        SCB_DisableICache();
        __DSB();
        __ISB();
    }

    static void enable_caches_and_barrier() {
        __DSB();
        __ISB();
        SCB_EnableICache();
        SCB_EnableDCache();
        __DSB();
        __ISB();
    }

    // 返回 true 表示通过，false 表示失败
    bool test_sdram_full_and_alias() {
        volatile uint32_t *pSDRAM = reinterpret_cast<volatile uint32_t *>(0xC0000000UL);
        constexpr uint32_t SDRAM_BYTES = 32UL * 1024UL * 1024UL;            // 32 MB
        constexpr uint32_t SDRAM_SIZE_WORDS = SDRAM_BYTES / sizeof(uint32_t); // 32MB / 4
        constexpr uint32_t WORDS_PER_ROW = 512U; // 列数 512 * 4 bytes -> 2048 bytes -> 512 words
        constexpr uint32_t STRIDE = WORDS_PER_ROW; // 用于检测第9列位(位索引从0计)
        constexpr uint32_t QUICK_TEST_BLOCKS = 4096; // 限制快速别名检测的样本数量（可根据需求调整）
        uint32_t errors = 0;

        // 1) 先禁用 cache，确保所有读写都到 SDRAM
        disable_caches_and_barrier();

        // ---- 模式 1: 写 0xAAAAAAAA 并校验 ----
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            pSDRAM[i] = 0xAAAAAAAAu;
        }
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            uint32_t v = pSDRAM[i];
            if (v != 0xAAAAAAAAu) {
                ++errors;
                if (errors == 1) {
                    // 发现第一个错误，快速指示（可替换为日志）
                }
            }
        }
        if (errors) {
            // 早返回，恢复 cache 前点亮失败指示
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); // LED off 或按你的板级反向
            enable_caches_and_barrier();
            return false;
        }

        // ---- 模式 2: 写 0x55555555 并校验 ----
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            pSDRAM[i] = 0x55555555u;
        }
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            uint32_t v = pSDRAM[i];
            if (v != 0x55555555u) {
                ++errors;
                if (errors == 1) {}
            }
        }
        if (errors) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            enable_caches_and_barrier();
            return false;
        }

        // ---- 模式 3: 地址作为值的严格测试（全部空间） ----
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            pSDRAM[i] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&pSDRAM[i]));
        }
        for (uint32_t i = 0; i < SDRAM_SIZE_WORDS; ++i) {
            uint32_t expected = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&pSDRAM[i]));
            uint32_t v = pSDRAM[i];
            if (v != expected) {
                ++errors;
                if (errors == 1) {}
            }
        }
        if (errors) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            enable_caches_and_barrier();
            return false;
        }

        // ---- 额外：列别名（stride）快速检测用于判断第9列是否生效 ----
        // 原理：如果第9列被当作列位，那么 base 和 base+STRIDE 指向不同物理列（应保持不同数据）。
        // 为速度只测试前面一段样本（QUICK_TEST_BLOCKS），避免全区双写耗时太长。
        constexpr uint32_t samples = (QUICK_TEST_BLOCKS < (SDRAM_SIZE_WORDS - STRIDE)) ? QUICK_TEST_BLOCKS : (SDRAM_SIZE_WORDS - STRIDE);
        // 写入两个不同模式到 base 和 base+stride
        for (uint32_t i = 0; i < samples; ++i) {
            pSDRAM[i] = 0xA5A50000u ^ i;
            pSDRAM[i + STRIDE] = 0x5A5A0000u ^ i;
        }
        // 读取并比对
        for (uint32_t i = 0; i < samples; ++i) {
            uint32_t a = pSDRAM[i];
            uint32_t b = pSDRAM[i + STRIDE];
            if (a == b) {
                // 发现 alias（高度怀疑第9列未被当作列位）
                ++errors;
                break;
            }
        }

        // 恢复 cache
        enable_caches_and_barrier();

        if (errors == 0) {
            // 所有测试通过，点亮 LED（按你板子的点亮逻辑调整）
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            return true;
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            return false;
        }
    }

    // 对外接口，保留 void 返回并点亮 LED
    void test_sdram() {
        bool ok = test_sdram_full_and_alias();
        (void)ok; // 若需进一步处理，依据 ok 做日志或复位
    }

} 