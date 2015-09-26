################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pecan-lpc82x.axf

# Tool invocations
pecan-lpc82x.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"/home/sven/LPCXpresso/workspace/lpc_chip_82x/Debug" -L"/home/sven/LPCXpresso/workspace/CMSIS_DSPLIB_CM0/lib" -Xlinker -Map="pecan-lpc82x.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "pecan-lpc82x_Debug.ld" -o "pecan-lpc82x.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) pecan-lpc82x.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "pecan-lpc82x.axf"; # arm-none-eabi-objcopy -v -O binary "pecan-lpc82x.axf" "pecan-lpc82x.bin" ; # checksum -p LPC824 -d "pecan-lpc82x.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
