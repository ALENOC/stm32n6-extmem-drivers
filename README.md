# STM32N6 External Memory Driver Suite
### C Driver Suite for Infineon, ISSI & Micron External Flash and PSRAM Memories

[![CI Test Suite](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml/badge.svg)](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-STM32N6%20(Cortex--M55)-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32n6-series.html)
[![Standard](https://img.shields.io/badge/C%20Standard-C11%20%2F%20Cube--BSP-green.svg)](https://www.st.com)

A production-grade, modular C driver suite compliant with STMicroelectronics **STM32Cube BSP** architectural standards, designed to interface the **STM32N6** microcontroller family (ARM Cortex-M55 core + Neural-ART NPU) with the complete spectrum of high-speed external memories from **Infineon Technologies**, **Integrated Silicon Solution, Inc. (ISSI)**, and **Micron Technology**.

Supports **XSPI1**, **XSPI2**, **XSPI3** (Single, Quad, Octal DTR up to 200 MHz, HyperBus™) and **FMC** (16-bit asynchronous parallel bus) interfaces.

---

## 📋 Table of Contents
- [Key Features](#-key-features)
- [Supported Devices Matrix](#-supported-devices-matrix)
- [Repository Structure](#-repository-structure)
- [Quick Integration into STM32CubeIDE](#-quick-integration-into-stm32cubeide)
- [Code Examples](#-code-examples)
  - [1. Initialization with Auto-Discovery](#1-initialization-with-auto-discovery)
  - [2. Execute-In-Place (XIP Memory-Mapped Mode)](#2-execute-in-place-xip-memory-mapped-mode)
  - [3. High-Throughput PSRAM Read & Write](#3-high-throughput-psram-read--write)
- [Unit Test Suite & Host Simulation](#-unit-test-suite--host-simulation)
- [Detailed Documentation](#-detailed-documentation)
- [Disclaimer & Limitation of Liability](#-disclaimer--limitation-of-liability)
- [License](#-license)

---

## 🚀 Key Features

- **Comprehensive Infineon Technologies Coverage**:
  - **SEMPER™ Octal NOR Flash** (`S28HS512T`, `S28HL512T`, `S28HS256T`, `S28HL256T`, `S28HS01GT`): xSPI Profile 1.0 protocol (8D-8D-8D DDR up to 200 MHz / 400 MB/s).
  - **HyperFlash™** (`S26KS512S`, `S26KL512S`, `S26KS256S`, `S26KL256S`, `S26KS128S`, `S26KL128S`): native Cypress HyperBus™ interface at 1.8V / 3.0V.
  - **HyperRAM™** (`S27KS0641`, `S27KL0641`, `S27KS128`, `S27KL128`, `S27KS256`, `S27KL256`, `S27KS512`, `S27HS/HL` 2.0 and 3.0 series).
  - **SEMPER™ / FL Quad SPI Flash** (`S25HL512T`, `S25HS512T`, `S25FL256L`, `S25FL128L`, `S25FL512S`).
- **Comprehensive ISSI (Integrated Silicon Solution Inc.) Coverage**:
  - **Octal NOR Flash** (`IS25LX064`, `IS25WX064`, `IS25LX128`, `IS25WX128`, `IS25LX256`, `IS25WX256`, `IS25LX512`, `IS25WX512`): xSPI Profile 1.0/2.0 DTR mode up to 200 MHz.
  - **HyperFlash™ NOR Flash** (`IS26KS128S`, `IS26KL128S`, `IS26KS256S`, `IS26KL256S`, `IS26KS512S`, `IS26KL512S`): 1.8V / 3.0V HyperBus™ interface up to 166 MHz.
  - **Quad SPI NOR Flash** (`IS25LP` and `IS25WP` from 2Mb up to 512Mb: `020/040/080/016/032/064/128/256/512`, plus `IS25LE/WE` ultra-low power and `IS25LQ/WQ` series).
  - **Parallel NOR Flash via FMC** (`IS29GL512`, `IS29GL256`, `IS29GL128`, `IS29GL064`, `IS29GL032`): 16-bit Parallel CFI NOR Flash.
  - **Octal PSRAM xSPI Profile 2.0** (`IS66WVO8M8`, `IS66WVO16M8`, `IS66WVO32M8`, `IS66WVO64M8` and automotive `IS67WVO` series).
  - **HyperRAM™ PSRAM** (`IS66WVH8M8`, `IS66WVH16M8`, `IS66WVH32M8` and automotive `IS67WVH` series).
  - **Quad SPI PSRAM** (`IS66WVS1M8`, `IS66WVS2M8`, `IS66WVS4M8`, `IS66WVS16M8`).
  - **Serial Static RAM** (`IS62WVS / IS65WVS 0648 / 1288 / 2568 / 5128` SPI/SDI/SQI).
  - **Parallel Asynchronous PSRAM/SRAM via FMC** (`IS66WV / IS67WV 51216 / 102416 / 204816 / 409616` 16-bit).
- **Comprehensive Micron Technology Coverage**:
  - **Xccela™ Octal NOR Flash** (`MT35XU02G`, `MT35XU01G`, `MT35XL01G`, `MT35XU512A`, `MT35XL512A`, `MT35XU256A`, `MT35XL256A`): xSPI Profile 1.0 (8D-8D-8D DDR up to 200 MHz / 400 MB/s with DQS, Flag Status Register polling, 32-bit addressing).
  - **MT25Q / N25Q Quad SPI NOR Flash** (`MT25QU` 1.8V & `MT25QL` 3.0V from 32Mb to 1Gb: `032/064/128/256/512/01G`): high-speed 1-4-4 Quad I/O up to 133 MHz with 4-byte address enter/exit support.
  - **Parallel NOR Flash via FMC Bank 1** (`MT28EW128`, `MT28EW256`, `MT28EW512`, `MT28EW01G`): 16-bit Asynchronous Parallel CFI NOR Flash.
- **Auto-Discovery and Automatic Recognition**:
  - JEDEC ID (0x9F) lookup in the chip database, then a JEDEC JESD216 SFDP fallback for unlisted densities of the supported quad NOR vendors (ISSI, Micron MT25Q, Infineon).
  - HyperRAM detection through the HyperBus ID0 register (manufacturer and row/column geometry give the capacity).
  - HyperFlash has no register space: select it with `Config.ForcedDeviceType` or `Config.ForcedPartNumber`.
  - When nothing answers, `ExtMem_Init()` returns `EXTMEM_NOT_SUPPORTED` instead of guessing a device.
- **Execute-In-Place (XIP) Memory-Mapped Mode**:
  - Single-call transition into memory-mapped address space: `0x90000000` (XSPI1), `0x70000000` (XSPI2), `0x60000000` (FMC).
- **Multi-Density Shared Driver Architecture**:
  - Memory chips within the same family sharing command opcodes and registers (e.g. `IS25LP` 32Mb to 512Mb, `MT25QU` 32Mb to 1Gb, `S28HS` 256Mb to 2Gb, `IS66WVO` 64Mb to 512Mb) use a unified component driver.
  - **Dynamic Capacity & DEVSIZE Calculation**: Automatically maps memory density to the STM32N6 `XSPI_DCR1.DEVSIZE` register according to RM0486 ($2^{(\text{DEVSIZE} + 1)}$ bytes).
  - **24-bit vs 32-bit Address Switching**: Seamlessly toggles 3-byte addressing for $\le 16\text{ MB}$ ($128\text{ Mb}$) and 4-byte addressing for $> 16\text{ MB}$ (with opcode switching or 4-byte address mode commands `0xB7`/`0xE9`).
  - **Strict Boundary Protection**: All read, write, and erase operations strictly enforce address range validation against the verified chip geometry, protecting against memory corruption.
- **ARM Cortex-M55 D-Cache Management**:
  - Built-in cache clean and invalidate routines (`SCB_CleanInvalidateDCache_by_Addr`) compliant with ARMv8.1-M architecture to guarantee cache coherence with DMA and hardware peripherals.

---

## 📊 Supported Devices Matrix

| Manufacturer | Memory Family | Bus Interface | Max Clock | Primary Mode | Component Driver |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Infineon** | SEMPER™ Octal NOR (`S28HS/S28HL`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `s28hs512t` |
| **Infineon** | HyperFlash™ (`S26KS/S26KL`) | XSPI1 / XSPI2 | 166 MHz | HyperBus™ | `s26ks512s` |
| **Infineon** | HyperRAM™ (`S27KS/S27KL/S27HS/S27HL`) | XSPI1 / XSPI2 | 200 MHz | HyperBus™ DDR | `s27ks0641` |
| **Infineon** | SEMPER™ / FL Quad (`S25HL/S25HS/S25FL`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `s25hl512t` |
| **ISSI** | Octal NOR Flash (`IS25LX/IS25WX`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `is25lx256` |
| **ISSI** | HyperFlash™ (`IS26KS/IS26KL`) | XSPI1 / XSPI2 | 166 MHz | HyperBus™ | `s26ks512s` |
| **ISSI** | Quad NOR Flash (`IS25LP/IS25WP/IS25LE/WE/LQ/WQ`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `is25lp256` |
| **ISSI** | Parallel NOR Flash (`IS29GL032/064/128/256/512`) | FMC (16-bit) | 110 ns | 16-bit Parallel CFI NOR | `is29gl_fmc` |
| **ISSI** | Octal PSRAM (`IS66WVO/IS67WVO`) | XSPI1 / XSPI2 | 200 MHz | xSPI Profile 2.0 (8D-8D-8D) | `is66wvo32m8` |
| **ISSI** | HyperRAM™ PSRAM (`IS66WVH/IS67WVH`) | XSPI1 / XSPI2 | 200 MHz | HyperBus™ DDR | `is66wvh16m8` |
| **ISSI** | Quad SPI PSRAM (`IS66WVS/IS67WVS`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `is66wvs16m8` |
| **ISSI** | Serial Static RAM (`IS62WVS/IS65WVS`) | XSPI1 / XSPI2 / XSPI3 | 45 MHz | SPI / SQI (1-1-1 / 4-4-4) | `is62wvs` |
| **ISSI/IFX** | Parallel Asynch PSRAM (`IS66WV/CY62`) | FMC (16-bit) | Asynchronous (~10-55ns) | 16-bit Parallel SRAM/PSRAM | `is66wv_fmc` |
| **Micron** | Xccela™ Octal NOR (`MT35XU/MT35XL`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `mt35xu512a` |
| **Micron** | MT25Q Quad NOR (`MT25QU/MT25QL`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `mt25qu512a` |
| **Micron** | Parallel NOR Flash (`MT28EW`) | FMC (16-bit) | 100 MHz (70ns) | 16-bit Parallel CFI NOR | `is29gl_fmc` |

*For the comprehensive matrix including ordering part numbers, packages, and voltage options, refer to [Docs/Supported_Memories_Matrix.md](Docs/Supported_Memories_Matrix.md).*

---

## 📁 Repository Structure

```text
stm32n6-extmem-drivers/
├── Drivers/
│   └── BSP/
│       ├── Components/
│       │   ├── Common/              # extmem_common.h, sfdp.h, sfdp.c
│       │   ├── s28hs512t/           # Infineon SEMPER Octal Flash Driver
│       │   ├── s26ks512s/           # Infineon HyperFlash Driver
│       │   ├── s27ks0641/           # Infineon HyperRAM Driver
│       │   ├── s25hl512t/           # Infineon Quad SPI Flash Driver
│       │   ├── is25lx256/           # ISSI Octal Flash Driver
│       │   ├── is25lp256/           # ISSI Quad SPI Flash Driver
│       │   ├── is66wvo32m8/         # ISSI Octal PSRAM Driver
│       │   ├── is66wvh16m8/         # ISSI HyperRAM PSRAM Driver
│       │   ├── is66wvs16m8/         # ISSI Quad PSRAM Driver
│       │   ├── is62wvs/             # ISSI Serial Static RAM Driver
│       │   ├── is66wv_fmc/          # FMC Parallel PSRAM Driver
│       │   ├── is29gl_fmc/          # ISSI / Micron FMC Parallel NOR Flash Driver
│       │   ├── mt35xu512a/          # Micron Xccela Octal NOR Flash Driver
│       │   └── mt25qu512a/          # Micron MT25Q Quad SPI Flash Driver
│       └── STM32N6_ExtMem/          # High-Level Unified Manager
│           ├── stm32n6_extmem.h
│           ├── stm32n6_extmem.c
│           ├── stm32n6_extmem_conf.h
│           ├── stm32n6_extmem_conf_template.h
│           ├── stm32n6_extmem_devices.h
│           └── stm32n6_extmem_devices.c     # Device database (defined once)
├── Examples/
│   ├── extmem_demo.c                # Hardware diagnostics, self-test & XIP execution
│   └── extmem_benchmark.c           # Read/Write throughput benchmark in MB/s
├── Tests/
│   ├── mock_hal.h / mock_hal.c      # STM32N6 HAL mock with command log, fault injection and device emulation
│   ├── test_common.h / .c           # Protocol assertions (command phases, sequences, fault sweeps)
│   ├── test_sfdp_db.c               # SFDP parser and device database checks
│   ├── test_octal_nor.c             # S28Hx-T, IS25LX/WX, MT35X
│   ├── test_quad_nor.c              # S25Hx-T / S25FL-L, IS25LP/WP, MT25Q
│   ├── test_hyperbus.c              # HyperFlash and HyperRAM
│   ├── test_ram.c                   # Octal / quad PSRAM and serial SRAM
│   ├── test_fmc.c                   # FMC PSRAM/SRAM and parallel NOR
│   ├── test_manager.c               # Unified manager, every database device end to end
│   ├── coverage_report.py           # gcov summary used by `make coverage`
│   ├── extmem_unit_tests.h          # Unit test declarations
│   └── main_test.c                  # Host test runner for Linux/macOS/Windows
├── Docs/
│   ├── STM32CubeIDE_Integration_Guide.md # Step-by-step CubeIDE setup & Linker guide
│   ├── Supported_Memories_Matrix.md      # In-depth memory comparison matrix
│   └── Hardware_Design_and_Pinout.md     # STM32N6 pinout, 200MHz PCB layout, VDDIO domains
├── .github/
│   └── workflows/
│       └── ci.yml                   # Continuous Integration with GitHub Actions
├── Makefile                         # Host test suite compilation and execution
├── LICENSE                          # MIT License and Disclaimer of Liability
└── README.md
```

---

## 🛠 Quick Integration into STM32CubeIDE

1. **Copy Driver Folders**:
   Copy `Drivers/BSP/` into your STM32CubeIDE project root under `Drivers/BSP/`.
2. **Add Include Paths**:
   Navigate to *Project Properties -> C/C++ Build -> Settings -> Tool Settings -> MCU GCC Compiler -> Include paths* and add:
   - `../Drivers/BSP/STM32N6_ExtMem`
   - `../Drivers/BSP/Components/Common`
   - `../Drivers/BSP/Components/s28hs512t` (along with any other required component directories)
3. **Configure Settings**:
   Copy `stm32n6_extmem_conf_template.h` to `stm32n6_extmem_conf.h` and tune clock prescalers and timeout thresholds.
   For FMC memories, set `Config.FmcClockHz` to the FMC kernel clock when it differs from HCLK: the asynchronous timings (ADDSET, DATAST, BUSTURN) are computed from it and the device access times.
   Pin muxing stays in `HAL_XSPI_MspInit()` / `HAL_SRAM_MspInit()` of your project (CubeMX generated); `ExtMem_Init()` enables the VDDIO domain, clocks and the XSPI I/O manager.
4. **Linker Script and MPU Setup**:
   Refer to [Docs/STM32CubeIDE_Integration_Guide.md](Docs/STM32CubeIDE_Integration_Guide.md) for configuring Cortex-M55 MPU regions and linker sections (`.extmem_text` for XIP firmware and `.extmem_ram` for framebuffers/NPU tensors).

---

## 💻 Code Examples

### 1. Initialization with Auto-Discovery
```c
#include "stm32n6_extmem.h"

ExtMem_HandleTypeDef hextmem = {0};

void Memory_Setup(void)
{
  /* Configure XSPI1 bus, prescaler 2 (200 MHz), VDDIO power domain at 1.8V */
  hextmem.Config.Bus            = EXTMEM_BUS_XSPI1;
  hextmem.Config.ClockPrescaler = 2;
  hextmem.Config.Force1V8       = true;

  /* Automatic chip detection via SFDP / JEDEC ID / HyperBus ID */
  if (ExtMem_Init(&hextmem) == EXTMEM_OK)
  {
    printf("Detected memory: %s (%ld MB)\r\n", 
           ExtMem_GetDeviceName(&hextmem), 
           hextmem.Geometry.TotalSizeBytes / (1024 * 1024));
  }
}
```

### 2. Execute-In-Place (XIP Memory-Mapped Mode)
```c
/* Transition into XIP mode at base address 0x90000000 */
if (ExtMem_EnableMemoryMapped(&hextmem) == EXTMEM_OK)
{
  /* Direct CPU pointer dereferencing without software transaction overhead */
  const uint32_t *pExternalCode = (const uint32_t *)hextmem.MemoryMappedBase;
  printf("First vector table entry: 0x%08lX\r\n", pExternalCode[0]);
}
```

### 3. High-Throughput PSRAM Read & Write
```c
uint8_t txData[1024];
uint8_t rxData[1024];

/* Initialize buffer with test payload */
memset(txData, 0xA5, sizeof(txData));

/* Write at 200 MHz DDR (Octal DTR or HyperRAM) */
ExtMem_Write(&hextmem, 0x00000000, txData, sizeof(txData));

/* Read back */
ExtMem_Read(&hextmem, 0x00000000, rxData, sizeof(rxData));
```

---

## 🧪 Unit Test Suite & Host Simulation

The host test harness replaces the STM32CubeN6 HAL with a mock (`Tests/mock_hal.c`) whose constants follow `stm32n6xx_hal_xspi.h` / `stm32n6xx_ll_fmc.h`. The mock:

- logs every HAL call, so each test checks the exact instruction, address, alternate byte, dummy and data phases (lines, width, STR/DTR, DQS) sent for every command;
- emulates the memories at command level: opcode decoding (including the 8D-8D-8D instruction extension), write enable latch, 4-byte address mode, status / configuration / volatile registers, NOR program (bits only cleared) and erase, HyperBus register space, and the HyperFlash and parallel NOR command state machines;
- injects a failure into any HAL call: every driver function is run once per HAL call with that call failing, and must report the error.

The manager test brings up **every device of the database** through `ExtMem_Init()` and runs program / erase / read, DMA, memory-mapped mode, reset, power down and deinit on it.

### Running Tests Locally
```bash
make test        # 21 test groups
make coverage    # gcov line + branch coverage of every driver source (100% required)
make examples    # the examples must keep compiling against the driver API
```

The CI workflow runs all three targets on every push and pull request.

> [!NOTE]
> The host suite proves the drivers send the command sequences documented by the memory vendors and handle every error path. It does not replace validation on real hardware: signal integrity, timing margins and silicon errata can only be checked on a board.

---

## ⚙️ Known Limitations

- **S28Hx02GT (2 Gb SEMPER Octal)** is a dual-die part that needs per-die register access; it is not in the device table.
- **S25Hx-T Quad I/O reads** use the factory memory latency (8 cycles plus 2 mode cycles), valid up to 118 MHz.
- **S25FL-L** shares the S25Hx-T driver; its failure flags live in SR2 and are reported as a timeout, not cleared.
- **HyperRAM** is configured for 6 latency clocks (up to 166 MHz). 200 MHz parts need a higher latency code.
- **ISSI quad NOR**: the volatile Read Register is set to 11 dummy cycles (1-4-4 up to 139 MHz). Parts without a Read Register keep the factory 6 cycles (81 MHz).
- **ISSI IS66WVO octal PSRAM** is driven with the AP Memory style OPI command set (0x00 / 0x80 / 0x40 / 0xC0). Check it against the datasheet of the exact part you use.

## 📚 Detailed Documentation

- [STM32CubeIDE Integration Guide](Docs/STM32CubeIDE_Integration_Guide.md)
- [Supported Memories Matrix](Docs/Supported_Memories_Matrix.md)
- [Hardware Design & High-Speed PCB Routing Guidelines](Docs/Hardware_Design_and_Pinout.md)

---

## ⚠️ Disclaimer & Limitation of Liability

> [!CAUTION]
> **COMMUNITY PROJECT PROVIDED FOR EDUCATIONAL AND DEMONSTRATION PURPOSES ONLY**
> 
> This repository, source code, drivers, and associated documentation are distributed **strictly for demonstration, reference, and educational purposes for the open embedded engineering community**.
> 
> 1. **No Warranty**: This software is provided "AS IS", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, or non-infringement.
> 2. **Complete Exclusion of Liability**: In no event shall the author ([ALENOC](https://github.com/ALENOC)), maintainers, or contributors be liable for any direct, indirect, incidental, special, exemplary, punitive, or consequential damages (including, but not limited to, damage to STM32N6 microcontrollers, external memories, PCB hardware, loss of data, business interruption, or equipment failure) arising in any way out of the use of or inability to use this software.
> 3. **Mandatory Hardware Verification**: It is the sole responsibility of the user or system integrator to independently verify electrical compatibility (1.8V vs 3.3V operating domains, logic levels, VDDIO rails), high-speed setup and hold timings up to 200 MHz, official datasheets from STMicroelectronics, Infineon Technologies, ISSI, and Micron Technology, and to perform thorough validation and safety testing prior to incorporating any code into prototypes, commercial hardware, or production firmware.

---

## 📄 License

Released under the open-source [MIT License](LICENSE).  
Copyright (c) 2026 Alessandro Nocentini (ALENOC) & Community Contributors.
