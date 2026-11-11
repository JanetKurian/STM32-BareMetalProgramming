################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.c \
../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.c 

OBJS += \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.o \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.o 

C_DEPS += \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTFormat.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTPacket.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.d \
./Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.o: ../Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/%.c Core/Lib/ioLibrary_Driver/Internet/MQTT/MQTTPacket/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet/W5500" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DNS" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Internet/DHCP" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver" -I"D:/w5500/w5500/Core/Lib/ioLibrary_Driver/Ethernet" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

