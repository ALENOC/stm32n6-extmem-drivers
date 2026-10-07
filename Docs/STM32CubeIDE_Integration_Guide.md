# Guida Completa all'Integrazione in STM32CubeIDE
## STM32N6 External Memory Driver Suite (Infineon & ISSI)

Questa guida illustra passo dopo passo come integrare la libreria in qualsiasi progetto **STM32CubeIDE** per microcontrollori della serie **STM32N6** (STM32N657, STM32N647, STM32N655, ecc.).

---

### Indice
1. [Struttura delle Directory nel Progetto](#1-struttura-delle-directory-nel-progetto)
2. [Configurazione di STM32CubeMX (Pinout & Clocks)](#2-configurazione-di-stm32cubemx-pinout--clocks)
3. [Configurazione di Include Paths in STM32CubeIDE](#3-configurazione-di-include-paths-in-stm32cubeide)
4. [Configurazione MPU e D-Cache per Cortex-M55](#4-configurazione-mpu-e-d-cache-per-cortex-m55)
5. [Configurazione del Linker Script (.ld) per XIP](#5-configurazione-del-linker-script-ld-per-xip)
6. [Esempio di Codice in main.c](#6-esempio-di-codice-in-mainc)
7. [Troubleshooting & Best Practices](#7-troubleshooting--best-practices)

---

### 1. Struttura delle Directory nel Progetto

Copiare la cartella `Drivers/BSP` all'interno del proprio workspace STM32CubeIDE:

```text
MioProgetto_STM32N6/
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
│       │   └── is66wv_fmc/          <-- Parallel PSRAM (FMC)
│       └── STM32N6_ExtMem/          <-- stm32n6_extmem.h/.c, stm32n6_extmem_conf.h
```

---

### 2. Configurazione di STM32CubeMX (Pinout & Clocks)

1. **Abilitare la periferica XSPI (XSPI1, XSPI2 o XSPI3)**:
   - Selezionare **XSPI1** (o XSPI2/3) in CubeMX.
   - **Mode**:
     - Per memorie Octal (S28HS, IS25LX, IS66WVO): *Octal Mode* (8 linee I/O + DQS + CLK + NCS).
     - Per memorie HyperBus (S26KS HyperFlash, S27KS HyperRAM, IS66WVH): *HyperBus Mode*.
     - Per memorie Quad (S25HL, IS25LP, IS66WVS): *Quad Mode*.
2. **Clock Configuration**:
   - Assicurarsi che la frequenza sorgente `XSPI_CLK` sia impostata a 400 MHz (o 200/266 MHz).
   - Con prescaler `2`, la memoria lavorerà a 200 MHz DDR (400 MB/s).
3. **Power / VDDIO Range**:
   - Per frequenze superiori a 133 MHz, è mandatorio impostare il dominio `VDDIO` a **1.8V**.
   - In CubeMX: *System Core* -> *PWR* -> *VDDIO2 Range* = **1.8V**.

---

### 3. Configurazione di Include Paths in STM32CubeIDE

Nel menu di STM32CubeIDE:
1. Fare clic destro sul progetto -> **Properties** -> **C/C++ Build** -> **Settings**.
2. Sotto **Tool Settings** -> **MCU GCC Compiler** -> **Include paths**, aggiungere:
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
   - `../Drivers/BSP/Components/is66wv_fmc`

---

### 4. Configurazione MPU e D-Cache per Cortex-M55

L'Arm Cortex-M55 dispone di cache I-Cache e D-Cache (32 KB ciascuna). Per massimizzare le prestazioni ed evitare inconsistenze di memoria durante trasferimenti DMA o polling:

```c
void MPU_Config_ExtMem(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disabilita temporaneamente la MPU */
  HAL_MPU_Disable();

  /* Regione XSPI1: Base 0x90000000, Dimensione 64MB */
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

  /* Abilita MPU con gestione dei fault */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  /* Abilita I-Cache e D-Cache */
  SCB_EnableICache();
  SCB_EnableDCache();
}
```

---

### 5. Configurazione del Linker Script (.ld) per XIP

Nel linker script (`STM32N657xx_FLASH.ld`), aggiungere la regione della memoria esterna nel blocco `MEMORY`:

```ld
MEMORY
{
  ITCM_RAM   (rwx) : ORIGIN = 0x00000000, LENGTH = 64K
  DTCM_RAM   (rw)  : ORIGIN = 0x20000000, LENGTH = 128K
  AXI_SRAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 1024K
  /* Memoria Esterna Flash su XSPI1 (Memory-Mapped) */
  XSPI1_MEM  (rx)  : ORIGIN = 0x90000000, LENGTH = 64M
  /* Memoria Esterna PSRAM su XSPI2 (Memory-Mapped) */
  XSPI2_RAM  (rwx) : ORIGIN = 0x70000000, LENGTH = 32M
}

SECTIONS
{
  /* Sezione per codice ed asset eseguiti direttamente da Flash Esterna (XIP) */
  .extmem_text :
  {
    . = ALIGN(4);
    *(.extmem_text)
    *(.extmem_text*)
    *(.extmem_rodata)
    *(.extmem_rodata*)
    . = ALIGN(4);
  } > XSPI1_MEM

  /* Sezione per grandi buffer grafici, modelli AI o heap su PSRAM esterna */
  .extmem_ram (NOLOAD) :
  {
    . = ALIGN(32);
    *(.extmem_ram)
    *(.extmem_ram*)
    . = ALIGN(32);
  } > XSPI2_RAM
}
```

Per collocare funzioni o variabili in memoria esterna nel codice C:
```c
__attribute__((section(".extmem_text"))) void HeavyNeuralNetworkFunction(void) { ... }
__attribute__((section(".extmem_ram"))) uint8_t FrameBuffer[1920 * 1080 * 2];
```

---

### 6. Esempio di Codice in main.c

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

  /* 1. Inizializzazione Flash Esterna (es. Infineon S28HS512T o ISSI IS25LX256 su XSPI1) */
  hExtFlash.Config.Bus            = EXTMEM_BUS_XSPI1;
  hExtFlash.Config.ClockPrescaler = 2;    /* 200 MHz */
  hExtFlash.Config.Force1V8       = true; /* 1.8V */
  if (ExtMem_Init(&hExtFlash) != EXTMEM_OK)
  {
    Error_Handler();
  }

  /* Abilita Memory Mapped Mode per XIP immediato */
  ExtMem_EnableMemoryMapped(&hExtFlash);

  /* 2. Inizializzazione PSRAM Esterna (es. ISSI IS66WVO o Infineon HyperRAM su XSPI2) */
  hExtRam.Config.Bus            = EXTMEM_BUS_XSPI2;
  hExtRam.Config.ClockPrescaler = 2;    /* 200 MHz */
  hExtRam.Config.Force1V8       = true;
  if (ExtMem_Init(&hExtRam) != EXTMEM_OK)
  {
    Error_Handler();
  }
  ExtMem_EnableMemoryMapped(&hExtRam);

  /* Accesso diretto da puntatore in RAM Esterna a 0x70000000 */
  volatile uint32_t *pPsram = (volatile uint32_t *)hExtRam.MemoryMappedBase;
  pPsram[0] = 0xDEADBEEF;

  while (1)
  {
    // Ciclo applicativo
  }
}
```
