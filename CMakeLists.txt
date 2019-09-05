cmake_minimum_required(VERSION 3.14)
project(GCodeParser C)

set(CMAKE_C_STANDARD 99)

include_directories(USB)

add_executable(GCodeParser
        main.c
        cmsis_gcc.h
        core_cm4.h
        core_cmFunc.h
        core_cmInstr.h
        core_cmSimd.h
        MK20D10.h
        MK20D10_features.h
        motor.c
        motor.h
        pwm.c
        pwm.h
        startup.c
        syscalls.c
        syscalls.h
        USB/usb_desc.c
        USB/usb_desc.h
        USB/usb_dev.c
        USB/usb_dev.h
        USB/usb_mem.c
        USB/usb_mem.h
        USB/usb_serial.c
        USB/usb_serial.h
        )
