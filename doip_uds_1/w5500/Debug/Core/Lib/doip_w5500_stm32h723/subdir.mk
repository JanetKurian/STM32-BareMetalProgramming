################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Lib/doip_w5500_stm32h723/doip.c \
../Core/Lib/doip_w5500_stm32h723/doip_parser.c \
../Core/Lib/doip_w5500_stm32h723/doip_tcp.c \
../Core/Lib/doip_w5500_stm32h723/doip_udp.c 

OBJS += \
./Core/Lib/doip_w5500_stm32h723/doip.o \
./Core/Lib/doip_w5500_stm32h723/doip_parser.o \
./Core/Lib/doip_w5500_stm32h723/doip_tcp.o \
./Core/Lib/doip_w5500_stm32h723/doip_udp.o 

C_DEPS += \
./Core/Lib/doip_w5500_stm32h723/doip.d \
./Core/Lib/doip_w5500_stm32h723/doip_parser.d \
./Core/Lib/doip_w5500_stm32h723/doip_tcp.d \
./Core/Lib/doip_w5500_stm32h723/doip_udp.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Lib/doip_w5500_stm32h723/%.o: ../Core/Lib/doip_w5500_stm32h723/%.c Core/Lib/doip_w5500_stm32h723/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet/W5500" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DNS" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DHCP" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"D:/w5500/w5500/Core/Lib/doip_w5500_stm32h723" -I"D:/w5500/w5500/Core/Lib/doip_uds" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

