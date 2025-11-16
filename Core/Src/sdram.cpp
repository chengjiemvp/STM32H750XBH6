#include "sdram.hpp"


namespace bsp {
    namespace sdram {
        HAL_StatusTypeDef init_sequence(SDRAM_HandleTypeDef *hsdram) {
            if (hsdram == nullptr) return HAL_ERROR;

            // 超时时间（ms）——可根据需要调整
            constexpr uint32_t SDRAM_CMD_TIMEOUT = 1000U;

            FMC_SDRAM_CommandTypeDef Command = {0};

            // ---- 1) Power-up delay: ensure CKE/clock stable (>=100us) ----
            // 如果你需要更小精度延时，可用纳秒级等待；用 HAL_Delay(1) 足够安全
            HAL_Delay(1); // >=100us

            // ---- 2) PRECHARGE ALL (PALL) ----
            Command.CommandMode = FMC_SDRAM_CMD_PALL;
            Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
            Command.AutoRefreshNumber = 1;
            Command.ModeRegisterDefinition = 0;
            if (HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_CMD_TIMEOUT) != HAL_OK) return HAL_ERROR;

            // 等待 tRP（以毫秒为单位简单保守延时）
            // 你的 SdramTiming.RPDelay 已设置为 3 cycles；为安全起见这里延时 1 ms
            HAL_Delay(1);

            // ---- 3) AUTO REFRESH x2 ----
            Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
            Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
            Command.AutoRefreshNumber = 2; // 2 次
            Command.ModeRegisterDefinition = 0;
            if (HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_CMD_TIMEOUT) != HAL_OK) return HAL_ERROR;

            // tRFC / 刷新恢复时间，保守延时
            HAL_Delay(1);

            // ---- 4) LOAD MODE REGISTER ----
            // Mode Register for IS42S32800J-6TLI, using:
            //  - Burst Length = 8
            //  - Burst Type = Sequential
            //  - CAS Latency = 3
            //  - Operating Mode = Standard
            //  - Write Burst Mode = Single
            //
            // 以下宏与 HAL 示例风格一致，最终 ModeRegisterDefinition 为 16-bit 值。
            constexpr uint32_t SDRAM_MODEREG_BURST_LENGTH_8        = 0x0003U;
            constexpr uint32_t SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL = 0x0000U;
            constexpr uint32_t SDRAM_MODEREG_CAS_LATENCY_3         = 0x0030U;
            constexpr uint32_t SDRAM_MODEREG_OPERATING_MODE_STANDARD = 0x0000U;
            constexpr uint32_t SDRAM_MODEREG_WRITEBURST_MODE_SINGLE = 0x0200U;

            uint32_t modeReg =
                SDRAM_MODEREG_BURST_LENGTH_8 |
                SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                SDRAM_MODEREG_CAS_LATENCY_3 |
                SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

            Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
            Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
            Command.AutoRefreshNumber = 1;
            Command.ModeRegisterDefinition = modeReg;
            if (HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_CMD_TIMEOUT) != HAL_OK) return HAL_ERROR;

            // tMRD = 2 clock cycles; 保守延时 1 ms
            HAL_Delay(1);

            // ---- 5) 完成：SDRAM 进入正常运行模式 ----
            return HAL_OK;
        }
    }
}