################################################################################
# MRS Version: {"version":"1.8.5","date":"2023/05/22"}
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../Startup/startup_ch32v30x.S 

OBJS += \
./Startup/startup_ch32v30x.o 

S_UPPER_DEPS += \
./Startup/startup_ch32v30x.d 


# Each subdirectory must supply rules for building sources it contributes
Startup/%.o: ../Startup/%.S
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -x assembler-with-cpp -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\Startup" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\drivers" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\include\libc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\libcpu\risc-v\common" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\src" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\include" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\misc" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\drivers\serial" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread\components\finsh" -I"D:\MounRiver\Projects\CH32V307-Beginner\4_RT-Thread\rtthread" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

