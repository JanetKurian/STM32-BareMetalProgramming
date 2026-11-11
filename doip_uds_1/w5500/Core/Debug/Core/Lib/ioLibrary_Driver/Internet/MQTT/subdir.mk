################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTClient.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/mqtt_interface.c 

OBJS += \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTClient.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/mqtt_interface.o 

C_DEPS += \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTClient.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/mqtt_interface.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Lib/ioLibrary_Driver/Internet/MQTT/%.o: ../Core/Lib/ioLibrary_Driver/Internet/MQTT/%.c Core/Lib/ioLibrary_Driver/Internet/MQTT/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet/W5500" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DNS" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DHCP" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

