##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.6.0] date: [Sun Jan 19 20:06:40 GMT 2020]
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = OpenCodi


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build
SDK_DIR = ../../STM32CubeL4-master
DRIVERS_DIR = $(SDK_DIR)/Drivers

######################################
# source
######################################
# C sources
C_SOURCES =  \
Src/main.c \
Src/stm32l4xx_it.c \
Src/stm32l4xx_hal_msp.c \
Src/stm32l4xx_hal_timebase_tim.c \
Src/display_driver.c \
Src/ts_driver.c \
Src/syscalls.c \
Src/system_stm32l4xx.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gfxmmu.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dsi.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_ltdc.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_sram.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_ospi.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_fmc.c \
$(DRIVERS_DIR)/BSP/STM32L4R9I_EVAL/stm32l4r9i_eval.c \
$(DRIVERS_DIR)/BSP/STM32L4R9I_EVAL/stm32l4r9i_eval_sram.c \
$(DRIVERS_DIR)/BSP/STM32L4R9I_EVAL/stm32l4r9i_eval_ospi_nor.c \
$(DRIVERS_DIR)/BSP/Components/ft3x67/ft3x67.c \
lvgl/src/lv_core/lv_debug.c \
lvgl/src/lv_core/lv_disp.c \
lvgl/src/lv_core/lv_group.c \
lvgl/src/lv_core/lv_indev.c \
lvgl/src/lv_core/lv_obj.c \
lvgl/src/lv_core/lv_refr.c \
lvgl/src/lv_core/lv_style.c \
lvgl/src/lv_draw/lv_draw.c \
lvgl/src/lv_draw/lv_draw_arc.c \
lvgl/src/lv_draw/lv_draw_basic.c \
lvgl/src/lv_draw/lv_draw_img.c \
lvgl/src/lv_draw/lv_draw_label.c \
lvgl/src/lv_draw/lv_draw_line.c \
lvgl/src/lv_draw/lv_draw_rect.c \
lvgl/src/lv_draw/lv_draw_triangle.c \
lvgl/src/lv_draw/lv_img_cache.c \
lvgl/src/lv_draw/lv_img_decoder.c \
lvgl/src/lv_font/lv_font.c \
lvgl/src/lv_font/lv_font_fmt_txt.c \
lvgl/src/lv_font/lv_font_roboto_16.c \
lvgl/src/lv_hal/lv_hal_disp.c \
lvgl/src/lv_hal/lv_hal_indev.c \
lvgl/src/lv_hal/lv_hal_tick.c \
lvgl/src/lv_misc/lv_anim.c \
lvgl/src/lv_misc/lv_area.c \
lvgl/src/lv_misc/lv_async.c \
lvgl/src/lv_misc/lv_bidi.c \
lvgl/src/lv_misc/lv_circ.c \
lvgl/src/lv_misc/lv_color.c \
lvgl/src/lv_misc/lv_fs.c \
lvgl/src/lv_misc/lv_gc.c \
lvgl/src/lv_misc/lv_ll.c \
lvgl/src/lv_misc/lv_log.c \
lvgl/src/lv_misc/lv_math.c \
lvgl/src/lv_misc/lv_mem.c \
lvgl/src/lv_misc/lv_printf.c \
lvgl/src/lv_misc/lv_task.c \
lvgl/src/lv_misc/lv_templ.c \
lvgl/src/lv_misc/lv_txt.c \
lvgl/src/lv_misc/lv_utils.c \
lvgl/src/lv_objx/lv_arc.c \
lvgl/src/lv_objx/lv_bar.c \
lvgl/src/lv_objx/lv_btn.c \
lvgl/src/lv_objx/lv_btnm.c \
lvgl/src/lv_objx/lv_calendar.c \
lvgl/src/lv_objx/lv_canvas.c \
lvgl/src/lv_objx/lv_cb.c \
lvgl/src/lv_objx/lv_chart.c \
lvgl/src/lv_objx/lv_cont.c \
lvgl/src/lv_objx/lv_cpicker.c \
lvgl/src/lv_objx/lv_ddlist.c \
lvgl/src/lv_objx/lv_gauge.c \
lvgl/src/lv_objx/lv_img.c \
lvgl/src/lv_objx/lv_imgbtn.c \
lvgl/src/lv_objx/lv_kb.c \
lvgl/src/lv_objx/lv_label.c \
lvgl/src/lv_objx/lv_led.c \
lvgl/src/lv_objx/lv_line.c \
lvgl/src/lv_objx/lv_list.c \
lvgl/src/lv_objx/lv_lmeter.c \
lvgl/src/lv_objx/lv_mbox.c \
lvgl/src/lv_objx/lv_objx_templ.c \
lvgl/src/lv_objx/lv_page.c \
lvgl/src/lv_objx/lv_preload.c \
lvgl/src/lv_objx/lv_roller.c \
lvgl/src/lv_objx/lv_slider.c \
lvgl/src/lv_objx/lv_spinbox.c \
lvgl/src/lv_objx/lv_sw.c \
lvgl/src/lv_objx/lv_ta.c \
lvgl/src/lv_objx/lv_table.c \
lvgl/src/lv_objx/lv_tabview.c \
lvgl/src/lv_objx/lv_tileview.c \
lvgl/src/lv_objx/lv_win.c \
lvgl/src/lv_themes/lv_theme.c \
lvgl/src/lv_themes/lv_theme_default.c

# ASM sources
ASM_SOURCES =  \
startup_stm32l4r9xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L4R9xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IInc \
-Ilvgl \
-I$(DRIVERS_DIR)/BSP/Components \
-I$(DRIVERS_DIR)/BSP/STM32L4R9I_EVAL \
-I$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Inc \
-I$(DRIVERS_DIR)/STM32L4xx_HAL_Driver/Inc/Legacy \
-I$(DRIVERS_DIR)/CMSIS/Device/ST/STM32L4xx/Include \
-I$(DRIVERS_DIR)/CMSIS/Include


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32L4R9AIIx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***