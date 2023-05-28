################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/drv_gpio.c \
../drivers/drv_usart.c 

OBJS += \
./drivers/drv_gpio.o \
./drivers/drv_usart.o 

C_DEPS += \
./drivers/drv_gpio.d \
./drivers/drv_usart.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Debug" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Core" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\User" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Peripheral\inc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\drivers" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include\libc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v\common" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\src" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\misc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\serial" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\finsh" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

