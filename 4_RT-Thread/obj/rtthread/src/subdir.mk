################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rtthread/src/clock.c \
../rtthread/src/components.c \
../rtthread/src/cpu.c \
../rtthread/src/device.c \
../rtthread/src/idle.c \
../rtthread/src/ipc.c \
../rtthread/src/irq.c \
../rtthread/src/kservice.c \
../rtthread/src/mem.c \
../rtthread/src/memheap.c \
../rtthread/src/mempool.c \
../rtthread/src/object.c \
../rtthread/src/scheduler.c \
../rtthread/src/slab.c \
../rtthread/src/thread.c \
../rtthread/src/timer.c 

OBJS += \
./rtthread/src/clock.o \
./rtthread/src/components.o \
./rtthread/src/cpu.o \
./rtthread/src/device.o \
./rtthread/src/idle.o \
./rtthread/src/ipc.o \
./rtthread/src/irq.o \
./rtthread/src/kservice.o \
./rtthread/src/mem.o \
./rtthread/src/memheap.o \
./rtthread/src/mempool.o \
./rtthread/src/object.o \
./rtthread/src/scheduler.o \
./rtthread/src/slab.o \
./rtthread/src/thread.o \
./rtthread/src/timer.o 

C_DEPS += \
./rtthread/src/clock.d \
./rtthread/src/components.d \
./rtthread/src/cpu.d \
./rtthread/src/device.d \
./rtthread/src/idle.d \
./rtthread/src/ipc.d \
./rtthread/src/irq.d \
./rtthread/src/kservice.d \
./rtthread/src/mem.d \
./rtthread/src/memheap.d \
./rtthread/src/mempool.d \
./rtthread/src/object.d \
./rtthread/src/scheduler.d \
./rtthread/src/slab.d \
./rtthread/src/thread.d \
./rtthread/src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
rtthread/src/%.o: ../rtthread/src/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Debug" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Core" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\User" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Peripheral\inc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\drivers" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include\libc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v\common" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\src" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\misc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\serial" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\finsh" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

