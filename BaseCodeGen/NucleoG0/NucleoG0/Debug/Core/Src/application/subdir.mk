################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/application/application.c 

OBJS += \
./Core/Src/application/application.o 

C_DEPS += \
./Core/Src/application/application.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/application/application.o: ../Core/Src/application/application.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32G070xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Drivers/CMSIS/Include -I../Core/Src/button -I../Core/Inc -I../Core/Src/application -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Core/Src/led -I../Core/Src/debugConsole -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/Src/application/application.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

