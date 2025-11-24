cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# ==================== 工具链设置（必须在project()之前）====================
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

set(CMAKE_PROJECT_NAME STM32H750XBH6)
project(${CMAKE_PROJECT_NAME})
enable_language(C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# ==================== 清除工具链中的全局链接脚本 ====================
string(REGEX REPLACE "-T[ ]*\"[^\"]*\"" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
string(REGEX REPLACE "-T[ ]*[^ ]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# ==================== C++警告抑制 ====================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")

# ==================== 主目标: Application (默认) ====================
add_executable(${CMAKE_PROJECT_NAME})

# 添加STM32CubeMX配置
add_subdirectory(cmake/stm32cubemx)

# 获取并过滤源文件
get_target_property(BASE_SOURCES ${CMAKE_PROJECT_NAME} SOURCES)

# 共享的用户源文件
set(COMMON_USER_SOURCES
    Core/Src/sdram.cpp
    Core/Src/bsp_setup.cpp
    Core/Src/qspi_w25q256.cpp
    Core/Src/test.cpp
)

# 配置Application目标
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_USER_SOURCES}
    Core/Src/app.cpp              # Application入口
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    BUILD_APPLICATION              # Application宏定义
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T${CMAKE_SOURCE_DIR}/STM32H750XX_QSPI.ld    # QSPI链接脚本
    -u _printf_float
)

# ==================== 可选目标: Bootloader ====================
add_executable(bootloader)

set_target_properties(bootloader PROPERTIES
    EXCLUDE_FROM_ALL TRUE          # 不在默认构建中
)

target_sources(bootloader PRIVATE
    ${BASE_SOURCES}                # STM32CubeMX生成的源文件
    ${COMMON_USER_SOURCES}         # 共享的用户代码
    Core/Src/boot_loader.cpp       # Bootloader入口
)

target_compile_definitions(bootloader PRIVATE
    BUILD_BOOTLOADER               # Bootloader宏定义
)

target_link_options(bootloader PRIVATE
    -T${CMAKE_SOURCE_DIR}/STM32H750XX_FLASH.ld   # 内部Flash链接脚本
    -u _printf_float
)

target_link_libraries(bootloader
    stm32cubemx                    # 接口库
    STM32_Drivers                  # HAL驱动
)

# 移除libob.a依赖
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
