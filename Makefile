# STM32N6 External Memory Driver Suite Makefile
# Host-based automated unit test runner

CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -O2 -DEXTMEM_UNIT_TEST
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
	-I Drivers/BSP/Components/is66wv_fmc

SRCS = \
	Tests/mock_hal.c \
	Tests/extmem_unit_tests.c \
	Tests/main_test.c \
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
	Drivers/BSP/Components/is66wv_fmc/is66wv_fmc.c \
	Drivers/BSP/STM32N6_ExtMem/stm32n6_extmem.c

OBJS = $(SRCS:.c=.o)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TARGET)
	@echo "Running STM32N6 External Memory Driver Suite Tests..."
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
