################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/debugConsole/debugConsole.c 

OBJS += \
./Core/Src/debugConsole/debugConsole.o 

C_DEPS += \
./Core/Src/debugConsole/debugConsole.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/debugConsole/debugConsole.o: ../Core/Src/debugConsole/debugConsole.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32G070xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Core/Src/application -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Core/Src/led -I../Core/Src/button -I../Core/Src/debugConsole -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/debugConsole/debugConsole.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

