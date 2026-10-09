################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CAN_SPI.c \
../Core/Src/Data_Transmission_functional_unit.c \
../Core/Src/Diagnostic_Communication_Management_functional_unit.c \
../Core/Src/MCP2515.c \
../Core/Src/Stored_Data_Transmission_functional_unit.c \
../Core/Src/Upload_Download_functional_unit.c \
../Core/Src/main.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/uds_services.c 

OBJS += \
./Core/Src/CAN_SPI.o \
./Core/Src/Data_Transmission_functional_unit.o \
./Core/Src/Diagnostic_Communication_Management_functional_unit.o \
./Core/Src/MCP2515.o \
./Core/Src/Stored_Data_Transmission_functional_unit.o \
./Core/Src/Upload_Download_functional_unit.o \
./Core/Src/main.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/uds_services.o 

C_DEPS += \
./Core/Src/CAN_SPI.d \
./Core/Src/Data_Transmission_functional_unit.d \
./Core/Src/Diagnostic_Communication_Management_functional_unit.d \
./Core/Src/MCP2515.d \
./Core/Src/Stored_Data_Transmission_functional_unit.d \
./Core/Src/Upload_Download_functional_unit.d \
./Core/Src/main.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/uds_services.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CAN_SPI.cyclo ./Core/Src/CAN_SPI.d ./Core/Src/CAN_SPI.o ./Core/Src/CAN_SPI.su ./Core/Src/Data_Transmission_functional_unit.cyclo ./Core/Src/Data_Transmission_functional_unit.d ./Core/Src/Data_Transmission_functional_unit.o ./Core/Src/Data_Transmission_functional_unit.su ./Core/Src/Diagnostic_Communication_Management_functional_unit.cyclo ./Core/Src/Diagnostic_Communication_Management_functional_unit.d ./Core/Src/Diagnostic_Communication_Management_functional_unit.o ./Core/Src/Diagnostic_Communication_Management_functional_unit.su ./Core/Src/MCP2515.cyclo ./Core/Src/MCP2515.d ./Core/Src/MCP2515.o ./Core/Src/MCP2515.su ./Core/Src/Stored_Data_Transmission_functional_unit.cyclo ./Core/Src/Stored_Data_Transmission_functional_unit.d ./Core/Src/Stored_Data_Transmission_functional_unit.o ./Core/Src/Stored_Data_Transmission_functional_unit.su ./Core/Src/Upload_Download_functional_unit.cyclo ./Core/Src/Upload_Download_functional_unit.d ./Core/Src/Upload_Download_functional_unit.o ./Core/Src/Upload_Download_functional_unit.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/uds_services.cyclo ./Core/Src/uds_services.d ./Core/Src/uds_services.o ./Core/Src/uds_services.su

.PHONY: clean-Core-2f-Src

