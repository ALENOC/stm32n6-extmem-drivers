# STM32N6 External Memory Driver Suite Makefile
# Host-based automated unit test runner

CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -O2 -DEXTMEM_UNIT_TEST
# Designated initializers in the test command specs override defaults on purpose
TEST_CFLAGS = -Wno-override-init
TARGET = extmem_test_runner

INCLUDES = \
	-I Tests \
	-I Drivers/BSP/Components/Common \
	-I Drivers/BSP/STM32N6_ExtMem \
	-I Drivers/BSP/Components/s28hs512t \
	-I Drivers/BSP/Components/s26ks512s \
	-I Drivers/BSP/Components/s27ks0641 \
	-I Drivers/BSP/Components/s25hl512t \
	-I Drivers/BSP/Components/is25lx256 \
	-I Drivers/BSP/Components/is25lp256 \
	-I Drivers/BSP/Components/is66wvo32m8 \
	-I Drivers/BSP/Components/is66wvh16m8 \
	-I Drivers/BSP/Components/is66wvs16m8 \
	-I Drivers/BSP/Components/is62wvs \
	-I Drivers/BSP/Components/is66wv_fmc \
	-I Drivers/BSP/Components/is29gl_fmc \
	-I Drivers/BSP/Components/mt35xu512a \
	-I Drivers/BSP/Components/mt25qu512a

TEST_SRCS = \
	Tests/mock_hal.c \
	Tests/test_common.c \
	Tests/test_sfdp_db.c \
	Tests/test_octal_nor.c \
	Tests/test_quad_nor.c \
	Tests/test_hyperbus.c \
	Tests/test_ram.c \
	Tests/test_fmc.c \
	Tests/test_manager.c \
	Tests/main_test.c

DRIVER_SRCS = \
	Drivers/BSP/Components/Common/sfdp.c \
	Drivers/BSP/Components/s28hs512t/s28hs512t.c \
	Drivers/BSP/Components/s26ks512s/s26ks512s.c \
	Drivers/BSP/Components/s27ks0641/s27ks0641.c \
	Drivers/BSP/Components/s25hl512t/s25hl512t.c \
	Drivers/BSP/Components/is25lx256/is25lx256.c \
	Drivers/BSP/Components/is25lp256/is25lp256.c \
	Drivers/BSP/Components/is66wvo32m8/is66wvo32m8.c \
	Drivers/BSP/Components/is66wvh16m8/is66wvh16m8.c \
	Drivers/BSP/Components/is66wvs16m8/is66wvs16m8.c \
	Drivers/BSP/Components/is62wvs/is62wvs.c \
	Drivers/BSP/Components/is66wv_fmc/is66wv_fmc.c \
	Drivers/BSP/Components/is29gl_fmc/is29gl_fmc.c \
	Drivers/BSP/Components/mt35xu512a/mt35xu512a.c \
	Drivers/BSP/Components/mt25qu512a/mt25qu512a.c \
	Drivers/BSP/STM32N6_ExtMem/stm32n6_extmem_devices.c \
	Drivers/BSP/STM32N6_ExtMem/stm32n6_extmem.c

SRCS = $(TEST_SRCS) $(DRIVER_SRCS)

# Sources with executable code (the device table is data only)
COVERAGE_SRCS = $(filter-out %_devices.c,$(DRIVER_SRCS))
OBJS = $(SRCS:.c=.o)

# Cross-compilation against the real STM32CubeN6 HAL (see target-check)
ARM_CC          ?= arm-none-eabi-gcc
STM32N6_HAL_DIR ?= ../stm32n6xx-hal-driver
STM32N6_DEV_DIR ?= ../cmsis-device-n6
CMSIS_CORE_DIR  ?= ../CMSIS_6/CMSIS/Core/Include
TARGET_BUILD    = build/target
TARGET_CFLAGS   = -mcpu=cortex-m55 -mthumb -mfloat-abi=hard -std=c11 -O2 -Wall -Wextra -Werror \
                  -DSTM32N657xx -DUSE_HAL_DRIVER
DRIVER_INCLUDES = $(patsubst %,-I %,$(filter-out -I Tests,$(INCLUDES)))

.PHONY: all test coverage examples target-check clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)

Tests/%.o: Tests/%.c
	$(CC) $(CFLAGS) $(TEST_CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TARGET)
	@echo "Running STM32N6 External Memory Driver Suite Tests..."
	./$(TARGET)

# Line and branch coverage of the driver sources (requires gcov)
coverage: clean
	$(MAKE) CFLAGS="-Wall -Wextra -Werror -std=c11 -O0 -DEXTMEM_UNIT_TEST --coverage" LDFLAGS="--coverage" $(TARGET)
	./$(TARGET)
	@for f in $(COVERAGE_SRCS); do gcov -b -o $$(dirname $$f) $$f > /dev/null; done
	python3 Tests/coverage_report.py $(COVERAGE_SRCS)

# The examples must keep compiling against the driver API
examples:
	$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only Examples/extmem_demo.c Examples/extmem_benchmark.c

# Every driver and example must compile for Cortex-M55 against the real HAL headers
target-check:
	@mkdir -p $(TARGET_BUILD)
	sed -e 's|/\*#define HAL_XSPI_MODULE_ENABLED *\*/|#define HAL_XSPI_MODULE_ENABLED|' \
	    -e 's|/\*#define HAL_SRAM_MODULE_ENABLED *\*/|#define HAL_SRAM_MODULE_ENABLED|' \
	    $(STM32N6_HAL_DIR)/Inc/stm32n6xx_hal_conf_template.h > $(TARGET_BUILD)/stm32n6xx_hal_conf.h
	@for f in $(DRIVER_SRCS) Examples/extmem_demo.c Examples/extmem_benchmark.c; do \
	  echo "  ARM_CC $$f"; \
	  $(ARM_CC) $(TARGET_CFLAGS) -I $(TARGET_BUILD) -I $(STM32N6_HAL_DIR)/Inc -I $(STM32N6_DEV_DIR)/Include \
	    -I $(CMSIS_CORE_DIR) $(DRIVER_INCLUDES) -c $$f -o $(TARGET_BUILD)/$$(basename $$f .c).o || exit 1; \
	done

clean:
	rm -f $(OBJS) $(TARGET) *.gcov $(SRCS:.c=.gcda) $(SRCS:.c=.gcno)
	rm -rf build
