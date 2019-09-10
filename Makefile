# Alun Jones - Changed to using teensy_loader_cli for loading code
# Matt Williams - Set c99 standard as well.

PROJECT=main

# main program + cpu startup code
OBJS = $(PROJECT).o startup.o

# additional module support
OBJS += motor.o pwm.o

# USB CDC support
OBJS += USB/usb_desc.o USB/usb_dev.o USB/usb_mem.o USB/usb_serial.o

CFLAGS = -std=c99 -Wall -fno-common -O3 -mthumb -mcpu=cortex-m4 -mfloat-abi=soft -specs=nosys.specs -I./USB -I.
ASFLAGS = -mcpu=cortex-m4
LDFLAGS  = -lm -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -specs=nosys.specs -nostartfiles -TMK20D10_flash.ld

CC = arm-none-eabi-gcc
STRIP = arm-none-eabi-strip
OBJCOPY = arm-none-eabi-objcopy

all:: $(PROJECT).hex

run: $(PROJECT).hex
	teensy_loader_cli -mmcu=mk20dx256  -v $(PROJECT).hex

$(PROJECT).hex: $(PROJECT).elf
	$(STRIP) $(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex

$(PROJECT).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(PROJECT).elf

clean:
	rm -f $(OBJS) $(PROJECT).hex $(PROJECT).elf

%.o : %.c %.h cmsis_gcc.h core_cm4.h core_cmFunc.h core_cmInstr.h core_cmSimd.h MK20D10.h
	$(CC) $(CFLAGS) -c $< -o $@
