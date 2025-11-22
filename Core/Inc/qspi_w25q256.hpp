#pragma  once
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <cinttypes>

// 错误码（类型化，避免隐式混用）
enum class W25QxxResult : std::int32_t {
    Ok               =  0,  // 通信正常
    InitError        = -1,  // 初始化错误
    WriteEnableFail  = -2,  // 写使能错误
    AutopollingFail  = -3,  // 轮询等待错误，无响应
    EraseFail        = -4,  // 擦除错误
    TransmitFail     = -5,  // 传输错误
    MemoryMappedFail = -6   // 内存映射模式错误
};

// 方便与旧接口互操作（若仍需要整型常量）
inline constexpr std::int32_t QSPI_W25Qxx_OK = static_cast<std::int32_t>(W25QxxResult::Ok);

// 命令（类型安全枚举）
enum class W25QxxCmd : uint8_t {
    EnableReset           = 0x66, // 使能复位
    ResetDevice           = 0x99, // 复位器件
    JedecID               = 0x9F, // JEDEC ID
    WriteEnable           = 0x06, // 写使能
    SectorErase4K         = 0x21, // 扇区擦除（4K）
    BlockErase64K         = 0xDC, // 块擦除（64K）
    ChipErase             = 0xC7, // 整片擦除
    QuadInputPageProgram  = 0x34, // 1-1-4 模式页编程
    FastReadQuadIO        = 0xEC, // 1-4-4 模式快速读取
    ReadStatusReg1        = 0x05  // 读状态寄存器1
};

// 若需要直接作为字节传参，提供 constexpr 别名
inline constexpr uint8_t W25Qxx_CMD_EnableReset          = static_cast<uint8_t>(W25QxxCmd::EnableReset);
inline constexpr uint8_t W25Qxx_CMD_ResetDevice          = static_cast<uint8_t>(W25QxxCmd::ResetDevice);
inline constexpr uint8_t W25Qxx_CMD_JedecID              = static_cast<uint8_t>(W25QxxCmd::JedecID);
inline constexpr uint8_t W25Qxx_CMD_WriteEnable          = static_cast<uint8_t>(W25QxxCmd::WriteEnable);
inline constexpr uint8_t W25Qxx_CMD_SectorErase          = static_cast<uint8_t>(W25QxxCmd::SectorErase4K);
inline constexpr uint8_t W25Qxx_CMD_BlockErase_64K       = static_cast<uint8_t>(W25QxxCmd::BlockErase64K);
inline constexpr uint8_t W25Qxx_CMD_ChipErase            = static_cast<uint8_t>(W25QxxCmd::ChipErase);
inline constexpr uint8_t W25Qxx_CMD_QuadInputPageProgram = static_cast<uint8_t>(W25QxxCmd::QuadInputPageProgram);
inline constexpr uint8_t W25Qxx_CMD_FastReadQuad_IO      = static_cast<uint8_t>(W25QxxCmd::FastReadQuadIO);
inline constexpr uint8_t W25Qxx_CMD_ReadStatus_REG1      = static_cast<uint8_t>(W25QxxCmd::ReadStatusReg1);

// 状态寄存器位
inline constexpr uint8_t W25Qxx_Status_REG1_BUSY = 0x01; // SR1 bit0: BUSY
inline constexpr uint8_t W25Qxx_Status_REG1_WEL  = 0x02; // SR1 bit1: WEL

// 常量尺寸
inline constexpr std::size_t W25Qxx_PageSize  = 256u;
inline constexpr std::size_t W25Qxx_FlashSize = 0x0200'0000u; // 32 MB

// JEDEC ID（组合值，存为 32-bit）
inline constexpr uint32_t W25Qxx_FLASH_ID = 0x00'EF40'19u; // 0xEF 40 19

// 内存映射基址（类型安全）
inline constexpr std::uintptr_t W25Qxx_MemAddr = 0x9000'0000u;

// 超时（使用 chrono 表示，单位明确）
inline constexpr std::chrono::seconds W25Qxx_ChipErase_TIMEOUT_MAX{400}; // 400 s

int8_t	 QSPI_W25Qxx_Init(void);						                                       // W25Qxx初始化
int8_t 	 QSPI_W25Qxx_Reset(void);					                                           // 复位器件
uint32_t QSPI_W25Qxx_ReadID(void);					                                           // 读取器件ID
int8_t	 QSPI_W25Qxx_Test(void);						                                       // Flash读写测试
int8_t 	 QSPI_W25Qxx_MemoryMappedMode(void);		                                           // 进入内存映射模式
int8_t 	 QSPI_W25Qxx_SectorErase(uint32_t SectorAddress);		                               // 扇区擦除，4K字节， 参考擦除时间 45ms
int8_t 	 QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress);	                               // 块擦除，64K字节，时间约150ms，实际建议64K擦除，擦除最快
int8_t 	 QSPI_W25Qxx_ChipErase (void);                                                         // 整片擦除，参考擦除时间 20S
int8_t	 QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite); // 按页写入，最大256字节
int8_t	 QSPI_W25Qxx_WriteBuffer(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);		   // 写入数据，最大不能超过flash芯片的大小
int8_t 	 QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);  // 读取数据，最大不能超过flash芯片的大小
int8_t   QSPI_W25Qxx_AutoPollingMemReady(void);
int8_t   QSPI_W25Qxx_WriteEnable(void);