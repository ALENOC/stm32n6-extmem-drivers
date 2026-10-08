# Complete STM32CubeIDE Integration Guide
## STM32N6 External Memory Driver Suite (Infineon, ISSI & Micron)

This step-by-step guide explains how to integrate the external memory driver suite into any **STM32CubeIDE** project targeting the **STM32N6** microcontroller family (STM32N657, STM32N647, STM32N655, etc.).

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

1. **Enable XSPI Peripherals (XSPI1, XSPI2, or XSPI3)**:
   - Select **XSPI1** (or XSPI2/3) in CubeMX.
   - **Mode**:
     - For Octal memories (Infineon S28HS, ISSI IS25LX, ISSI IS66WVO, Micron MT35XU): *Octal Mode* (8 I/O lines + DQS + CLK + NCS).
     - For HyperBus memories (Infineon S26KS HyperFlash, S27KS HyperRAM, ISSI IS66WVH): *HyperBus Mode*.
     - For Quad memories (Infineon S25HL, ISSI IS25LP, ISSI IS66WVS, Micron MT25QU): *Quad Mode*.
2. **Clock Configuration**:
   - Ensure the `XSPI_CLK` source clock is configured to 400 MHz (or 200/266 MHz).
   - With prescaler `2`, the memory operates at 200 MHz DDR (400 MB/s).
3. **Power / VDDIO Range**:
   - For clock frequencies above 133 MHz, the `VDDIO` power domain **must be set to 1.8V**.
   - In CubeMX: *System Core* -> *PWR* -> *VDDIO2 Range* = **1.8V**.

---

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

The ARM Cortex-M55 features 32 KB I-Cache and 32 KB D-Cache. To maximize throughput while avoiding cache incoherency during DMA transfers or register polling:

```c
void MPU_Config_ExtMem(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Temporarily disable MPU */
  HAL_MPU_Disable();

  /* XSPI1 Region: Base 0x90000000, Size 64MB */
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress      = 0x90000000U;
  MPU_InitStruct.Size             = MPU_REGION_SIZE_64MB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable MPU with default fault handling */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  /* Enable Instruction and Data Caches */
  SCB_EnableICache();
  SCB_EnableDCache();
}
```

---

### 5. Linker Script (.ld) Configuration for XIP

In your linker script (`STM32N657xx_FLASH.ld`), add the external memory regions inside the `MEMORY` block:

```ld
MEMORY
{
  ITCM_RAM   (rwx) : ORIGIN = 0x00000000, LENGTH = 64K
  DTCM_RAM   (rw)  : ORIGIN = 0x20000000, LENGTH = 128K
  AXI_SRAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 1024K
  /* External Flash on XSPI1 (Memory-Mapped) */
  XSPI1_MEM  (rx)  : ORIGIN = 0x90000000, LENGTH = 64M
  /* External PSRAM on XSPI2 (Memory-Mapped) */
  XSPI2_RAM  (rwx) : ORIGIN = 0x70000000, LENGTH = 32M
}

SECTIONS
{
  /* Section for code and assets executed directly from External Flash (XIP) */
  .extmem_text :
  {
    . = ALIGN(4);
    *(.extmem_text)
    *(.extmem_text*)
    *(.extmem_rodata)
    *(.extmem_rodata*)
    . = ALIGN(4);
  } > XSPI1_MEM

  /* Section for large framebuffers, AI tensors, or heap in External PSRAM */
  .extmem_ram (NOLOAD) :
  {
    . = ALIGN(32);
    *(.extmem_ram)
    *(.extmem_ram*)
    . = ALIGN(32);
  } > XSPI2_RAM
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

ExtMem_HandleTypeDef hExtFlash;
ExtMem_HandleTypeDef hExtRam;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MPU_Config_ExtMem();

  /* 1. External Flash Initialization (e.g. Infineon S28HS, ISSI IS25LX, or Micron MT35XU on XSPI1) */
  hExtFlash.Config.Bus            = EXTMEM_BUS_XSPI1;
  hExtFlash.Config.ClockPrescaler = 2;    /* divider: 400 MHz XSPI kernel clock / 2 = 200 MHz */
  hExtFlash.Config.Force1V8       = true; /* 1.8V */
  if (ExtMem_Init(&hExtFlash) != EXTMEM_OK)
  {
    Error_Handler();
  }

  /* Enable Memory-Mapped mode for direct CPU execution (XIP) */
  ExtMem_EnableMemoryMapped(&hExtFlash);

  /* 2. External PSRAM Initialization (e.g. ISSI IS66WVO or Infineon HyperRAM on XSPI2) */
  hExtRam.Config.Bus            = EXTMEM_BUS_XSPI2;
  hExtRam.Config.ClockPrescaler = 2;    /* divider: 400 MHz XSPI kernel clock / 2 = 200 MHz */
  hExtRam.Config.Force1V8       = true;
  if (ExtMem_Init(&hExtRam) != EXTMEM_OK)
  {
    Error_Handler();
  }
  ExtMem_EnableMemoryMapped(&hExtRam);

  /* Direct pointer access to external PSRAM at 0x70000000 */
  volatile uint32_t *pPsram = (volatile uint32_t *)hExtRam.MemoryMappedBase;
  pPsram[0] = 0xDEADBEEF;

  while (1)
  {
    /* Main application loop */
  }
}
```

---

### 7. Troubleshooting & Best Practices

- **Dummy Cycles Mismatch**: If reading in memory-mapped mode returns corrupted data or phase-shifted bytes, verify that the dummy cycles configured in `stm32n6_extmem_conf.h` or discovered via SFDP match the memory's volatile configuration register.
- **Cache Invalidation**: Whenever performing DMA transfers from external memory into internal SRAM, invoke `SCB_InvalidateDCache_by_Addr()` on the target buffer to avoid stale cache hits.
- **VDDIO Voltage Rails**: Ensure the hardware VDDIO supply rail matches the configuration register (`PWR_VDDIO_RANGE_1V8` for 1.8V vs `PWR_VDDIO_RANGE_3V3` for 3.3V). Running a 1.8V device at 3.3V can cause permanent damage.
