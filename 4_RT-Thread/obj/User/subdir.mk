################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v30x_it.c \
../User/main.c \
../User/system_ch32v30x.c 

OBJS += \
./User/ch32v30x_it.o \
./User/main.o \
./User/system_ch32v30x.o 

C_DEPS += \
./User/ch32v30x_it.d \
./User/main.d \
./User/system_ch32v30x.d 


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Debug" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Core" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\User" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Peripheral\inc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\drivers" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include\libc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v\common" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\src" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\misc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\serial" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\finsh" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

