################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Si446x.c \
../src/adc.c \
../src/aprs.c \
../src/ax25.c \
../src/base64.c \
../src/cr_startup_lpc82x.c \
../src/crp.c \
../src/glcdfont.c \
../src/gps.c \
../src/log.c \
../src/main.c \
../src/modem.c \
../src/mtb.c \
../src/pecan-lpc82x.c \
../src/sysinit.c \
../src/time.c \
../src/uart.c 

S_SRCS += \
../src/aeabi_romdiv_patch.s 

OBJS += \
./src/Si446x.o \
./src/adc.o \
./src/aeabi_romdiv_patch.o \
./src/aprs.o \
./src/ax25.o \
./src/base64.o \
./src/cr_startup_lpc82x.o \
./src/crp.o \
./src/glcdfont.o \
./src/gps.o \
./src/log.o \
./src/main.o \
./src/modem.o \
./src/mtb.o \
./src/pecan-lpc82x.o \
./src/sysinit.o \
./src/time.o \
./src/uart.o 

C_DEPS += \
./src/Si446x.d \
./src/adc.d \
./src/aprs.d \
./src/ax25.d \
./src/base64.d \
./src/cr_startup_lpc82x.d \
./src/crp.d \
./src/glcdfont.d \
./src/gps.d \
./src/log.d \
./src/main.d \
./src/modem.d \
./src/mtb.d \
./src/pecan-lpc82x.d \
./src/sysinit.d \
./src/time.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DDEBUG -D__CODE_RED -DCORE_M0PLUS -D__MTB_BUFFER_SIZE=256 -D__USE_ROMDIVIDE -D__USE_LPCOPEN -DNO_BOARD_LIB -D__USE_CMSIS_DSPLIB=CMSIS_DSPLIB_CM0 -DCR_INTEGER_PRINTF -DCR_PRINTF_CHAR -D__LPC82X__ -D__REDLIB__ -I"/home/sven/LPCXpresso/workspace/lpc_chip_82x/inc" -I"/home/sven/LPCXpresso/workspace/CMSIS_DSPLIB_CM0/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU Assembler'
	arm-none-eabi-gcc -c -x assembler-with-cpp -DDEBUG -D__CODE_RED -DCORE_M0PLUS -D__USE_ROMDIVIDE -D__USE_LPCOPEN -DNO_BOARD_LIB -D__USE_CMSIS_DSPLIB=CMSIS_DSPLIB_CM0 -D__LPC82X__ -D__REDLIB__ -I"/home/sven/LPCXpresso/workspace/lpc_chip_82x/inc" -I"/home/sven/LPCXpresso/workspace/CMSIS_DSPLIB_CM0/inc" -g3 -mcpu=cortex-m0 -mthumb -specs=redlib.specs -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


