################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/ftpc.c 

OBJS += \
./Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/ftpc.o 

C_DEPS += \
./Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/ftpc.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/%.o: ../Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/%.c Drivers/Lib/ioLibrary_Driver/Internet/FTPClient/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32H7xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

