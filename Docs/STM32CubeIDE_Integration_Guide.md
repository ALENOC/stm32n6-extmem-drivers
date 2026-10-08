# STM32CubeIDE Integration Guide
## STM32N6 External Memory Driver Suite (Infineon, ISSI & Micron)

How to add the driver suite to an **STM32CubeIDE** project for the **STM32N6** series. The MPU
configuration (`Examples/extmem_mpu.c`) is compile-checked against the STM32CubeN6 HAL in CI; the linker
fragment and the `main.c` example are templates. None of it has been run on hardware yet: adapt base
addresses, sizes and buses to your board.

---

### Table of Contents
1. [Project Directory Structure](#1-project-directory-structure)
2. [STM32CubeMX Configuration (Pinout & Clocks)](#2-stm32cubemx-configuration-pinout--clocks)
3. [Include Paths Configuration in STM32CubeIDE](#3-include-paths-configuration-in-stm32cubeide)
4. [MPU and D-Cache Configuration for Cortex-M55](#4-mpu-and-d-cache-configuration-for-cortex-m55)
5. [Linker Script (.ld) Configuration for XIP](#5-linker-script-ld-configuration-for-xip)
6. [Application Code Example in main.c](#6-application-code-example-in-mainc)
7. [Troubleshooting & Best Practices](#7-troubleshooting--best-practices)

---

### 1. Project Directory Structure

Copy the `Drivers/BSP` directory into your STM32CubeIDE project root:

```text
MyProject_STM32N6/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   └── stm32n6xx_hal_conf.h
│   └── Src/
│       ├── main.c
│       └── stm32n6xx_hal_msp.c
├── Drivers/
│   ├── CMSIS/
│   ├── STM32N6xx_HAL_Driver/
│   └── BSP/
│       ├── Components/
│       │   ├── Common/              <-- extmem_common.h, sfdp.h, sfdp.c
│       │   ├── s28hs512t/           <-- Infineon SEMPER Octal Flash
│       │   ├── s26ks512s/           <-- Infineon HyperFlash
│       │   ├── s27ks0641/           <-- Infineon HyperRAM
│       │   ├── s25hl512t/           <-- Infineon Quad SPI Flash
│       │   ├── is25lx256/           <-- ISSI Octal Flash
│       │   ├── is25lp256/           <-- ISSI Quad SPI Flash
│       │   ├── is66wvo32m8/         <-- ISSI Octal PSRAM
│       │   ├── is66wvh16m8/         <-- ISSI HyperRAM PSRAM
│       │   ├── is66wvs16m8/         <-- ISSI Quad PSRAM
│       │   ├── is62wvs/             <-- ISSI Serial Static RAM
│       │   ├── is66wv_fmc/          <-- Parallel PSRAM / SRAM (FMC)
│       │   ├── is29gl_fmc/          <-- ISSI / Micron FMC Parallel NOR Flash
│       │   ├── mt35xu512a/          <-- Micron Xccela Octal NOR Flash
│       │   └── mt25qu512a/          <-- Micron MT25Q Quad SPI Flash
│       └── STM32N6_ExtMem/          <-- stm32n6_extmem.h/.c, stm32n6_extmem_conf.h
```

---

### 2. STM32CubeMX Configuration (Pinout & Clocks)

1. **XSPI peripheral and pins**:
   - Enable the XSPI instance wired to the memory and generate `HAL_XSPI_MspInit()` with the pins of the
     XSPIM port you use (pin reference in [Hardware_Design_and_Pinout.md](Hardware_Design_and_Pinout.md)).
   - Interface width: 8 data lines + DQS for octal NOR, OctalRAM and HyperBus parts, 4 data lines for quad
     NOR, quad PSRAM and serial SRAM.
   - `ExtMem_Init()` re-initialises the XSPI handle itself (memory type, size, clock divider, chip-select
     boundary) and routes the instance to the XSPIM port, so the CubeMX XSPI parameters other than the pins
     are not used.
2. **Clock**:
   - Configure the XSPI kernel clock in RCC. `Config.ClockPrescaler` is a divider of that clock (for example
     400 MHz / 2 = 200 MHz); `ExtMem_Init()` refuses a result above the maximum clock of the detected part.
3. **VDDIO domain**:
   - `ExtMem_Init()` enables the domain of the XSPIM port and sets its range from `Config.Force1V8`. The
     board must supply that domain at the same voltage, matching the memory.
4. **FMC memories**: generate the FMC pins in `HAL_SRAM_MspInit()` (the suite drives both parallel RAMs and parallel NOR through the HAL SRAM driver); the timings are
   computed by `ExtMem_Init()` from `Config.FmcClockHz` and the device access times.
### 3. Include Paths Configuration in STM32CubeIDE

In the STM32CubeIDE project properties:
1. Right-click the project -> **Properties** -> **C/C++ Build** -> **Settings**.
2. Under **Tool Settings** -> **MCU GCC Compiler** -> **Include paths**, add:
   - `../Drivers/BSP/STM32N6_ExtMem`
   - `../Drivers/BSP/Components/Common`
   - `../Drivers/BSP/Components/s28hs512t`
   - `../Drivers/BSP/Components/s26ks512s`
   - `../Drivers/BSP/Components/s27ks0641`
   - `../Drivers/BSP/Components/s25hl512t`
   - `../Drivers/BSP/Components/is25lx256`
   - `../Drivers/BSP/Components/is25lp256`
   - `../Drivers/BSP/Components/is66wvo32m8`
   - `../Drivers/BSP/Components/is66wvh16m8`
   - `../Drivers/BSP/Components/is66wvs16m8`
   - `../Drivers/BSP/Components/is62wvs`
   - `../Drivers/BSP/Components/is66wv_fmc`
   - `../Drivers/BSP/Components/is29gl_fmc`
   - `../Drivers/BSP/Components/mt35xu512a`
   - `../Drivers/BSP/Components/mt25qu512a`

---

### 4. MPU and D-Cache Configuration for Cortex-M55

The Cortex-M55 uses the ARMv8.1-M MPU (PMSAv8): regions are defined by a base and a limit address and
point to memory attribute entries configured with `HAL_MPU_ConfigMemoryAttributes()`.
[`Examples/extmem_mpu.c`](../Examples/extmem_mpu.c) contains a complete `ExtMem_MPU_Config()` that maps:

- an external Flash window as read-only, executable, write-back cacheable;
- an external RAM window as read-write, non-executable, write-back cacheable.

Base addresses and sizes in that file follow the STM32N6570-DK assignment (NOR on XSPI2 at `0x70000000`,
PSRAM on XSPI1 at `0x90000000`); change them to match your board. The file is compiled against the
STM32CubeN6 HAL in CI.

### 5. Linker Script (.ld) Configuration for XIP

The STM32N6 has no internal Flash: the boot ROM loads a first-stage bootloader into internal SRAM, which
must initialise the external memory before any code or data placed there is used. Running application code
from external Flash therefore depends on your boot flow (for example ST's FSBL templates); this suite only
provides the memory initialisation and memory-mapped mode, and execute-in-place has not been demonstrated
with it.

A template for the `MEMORY` and `SECTIONS` additions (non-secure aliases, DK bus assignment; adjust
addresses and sizes to your linker script and board):

```ld
MEMORY
{
  /* External Flash on XSPI2 (memory-mapped) */
  XSPI2_MEM  (rx)  : ORIGIN = 0x70000000, LENGTH = 128M
  /* External RAM on XSPI1 (memory-mapped) */
  XSPI1_RAM  (rw)  : ORIGIN = 0x90000000, LENGTH = 32M
}

SECTIONS
{
  /* Code and constants placed in external Flash */
  .extmem_text :
  {
    . = ALIGN(4);
    *(.extmem_text)
    *(.extmem_text*)
    *(.extmem_rodata)
    *(.extmem_rodata*)
    . = ALIGN(4);
  } > XSPI2_MEM

  /* Large buffers (frame buffers, NPU tensors) in external RAM, not initialised at startup */
  .extmem_ram (NOLOAD) :
  {
    . = ALIGN(32);
    *(.extmem_ram)
    *(.extmem_ram*)
    . = ALIGN(32);
  } > XSPI1_RAM
}
```

To assign functions or buffers to external memory in C source code:
```c
__attribute__((section(".extmem_text"))) void HeavyNeuralNetworkFunction(void) { ... }
__attribute__((section(".extmem_ram"))) uint8_t FrameBuffer[1920 * 1080 * 2];
```

---

### 6. Application Code Example in main.c

```c
#include "main.h"
#include "stm32n6_extmem.h"

void ExtMem_MPU_Config(void);   /* Examples/extmem_mpu.c */

ExtMem_HandleTypeDef hExtFlash = {0};
ExtMem_HandleTypeDef hExtRam   = {0};

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  ExtMem_MPU_Config();

  /* 1. External Flash on XSPI2 (for example S28HS, IS25WX or MT35XU), 1.8 V */
  hExtFlash.Config.Bus            = EXTMEM_BUS_XSPI2;
  hExtFlash.Config.ClockPrescaler = 2;    /* divider: 400 MHz XSPI kernel clock / 2 = 200 MHz */
  hExtFlash.Config.Force1V8       = true;
  if (ExtMem_Init(&hExtFlash) != EXTMEM_OK)
  {
    Error_Handler();
  }
  if (ExtMem_EnableMemoryMapped(&hExtFlash) != EXTMEM_OK)   /* reads through 0x70000000 */
  {
    Error_Handler();
  }

  /* 2. External RAM on XSPI1 (for example IS66WVO OctalRAM or a HyperRAM), 1.8 V */
  hExtRam.Config.Bus            = EXTMEM_BUS_XSPI1;
  hExtRam.Config.ClockPrescaler = 2;
  hExtRam.Config.Force1V8       = true;
  if (ExtMem_Init(&hExtRam) != EXTMEM_OK || ExtMem_EnableMemoryMapped(&hExtRam) != EXTMEM_OK)
  {
    Error_Handler();
  }

  /* Direct access to the RAM window at 0x90000000 */
  volatile uint32_t *pRam = (volatile uint32_t *)hExtRam.MemoryMappedBase;
  pRam[0] = 0xDEADBEEFU;

  while (1)
  {
  }
}
```

### 7. Troubleshooting & Best Practices

- **Wrong data at high clock**: the drivers program the latency and dummy cycles of each part for the
  clock limit in the device table. If memory-mapped reads are wrong at full speed, first check signal
  integrity and the XSPI delay-block / sample-shift settings (not tuned by the drivers), then retry at a
  lower clock divider.
- **DMA and cache**: `ExtMem_ReadDMA()` / `ExtMem_WriteDMA()` do not maintain the cache for your buffers.
  Clean the source buffer (`SCB_CleanDCache_by_Addr()`) before a DMA write and invalidate the destination
  (`SCB_InvalidateDCache_by_Addr()`) after a DMA read when the buffers are cacheable.
- **Detection fails** (`EXTMEM_NOT_SUPPORTED`): HyperFlash is never auto-detected and needs
  `Config.ForcedPartNumber` or `Config.ForcedDeviceType`; other parts must answer JEDEC ID, SFDP (quad NOR
  vendors only) or the HyperBus ID0 register at the probing clock.
- **VDDIO**: the board supply of the domain must match `Config.Force1V8`. Running a 1.8 V device at 3.3 V
  can cause permanent damage.
