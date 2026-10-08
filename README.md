# STM32N6 External Memory Driver Suite
### C Driver Suite for Infineon, ISSI & Micron External Flash and PSRAM Memories

[![CI Test Suite](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml/badge.svg)](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-STM32N6%20(Cortex--M55)-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32n6-series.html)
[![Standard](https://img.shields.io/badge/C%20Standard-C11%20%2F%20Cube--BSP-green.svg)](https://www.st.com)

A C driver suite for the **STM32N6** (Cortex-M55) that connects external Flash and RAM devices from **Infineon Technologies**, **ISSI** and **Micron Technology** to the **XSPI1 / XSPI2 / XSPI3** controllers (1-line SPI, 4-line quad SPI/QPI, 8-line DTR and HyperBus™) and to the **FMC** (16-bit asynchronous). It follows the STM32Cube BSP layout: one component driver per memory family, plus a unified manager (`ExtMem_*` API) that detects the device, configures the controller and exposes read, write, erase, memory-mapped mode, reset and power-down operations.

> [!IMPORTANT]
> **Validation status.** The drivers are implemented from the vendor datasheets and verified with a host test suite against a mock of the STM32CubeN6 HAL, and they cross-compile against the real HAL for Cortex-M55. **They have not yet been run on STM32N6 hardware or on physical memory devices.** Treat the suite as a reference implementation and validate it on your board before using it in a product. See [Testing and Validation](#-testing-and-validation).

---

## 📋 Table of Contents
- [Scope](#-scope)
- [Key Capabilities](#-key-capabilities)
- [Device Support](#-device-support)
- [Repository Structure](#-repository-structure)
- [Quick Integration into STM32CubeIDE](#-quick-integration-into-stm32cubeide)
- [Code Examples](#-code-examples)
- [Testing and Validation](#-testing-and-validation)
- [Known Limitations](#-known-limitations)
- [Development Status and Roadmap](#-development-status-and-roadmap)
- [Contributing](#-contributing)
- [Detailed Documentation](#-detailed-documentation)
- [Disclaimer & Limitation of Liability](#-disclaimer--limitation-of-liability)
- [License](#-license)

---

## 🎯 Scope

- **What it is**: an independent, device-specific driver collection with its own manager API, written for the STM32N6 XSPI and FMC peripherals. Each memory family has a driver that issues the command sequences, latencies and register settings given in its datasheet; a device table records the identification bytes, geometry, die count and clock limit of every listed part.
- **What it is not**: it is not based on, and is not a drop-in replacement for, STMicroelectronics' [External Memory Manager middleware](https://github.com/STMicroelectronics/stm32-mw-extmem-mgr) (generic SFDP-driven NOR, PSRAM and SD support for STM32Cube). Interoperability with that middleware has not been evaluated. The project is not affiliated with or endorsed by STMicroelectronics, Infineon, ISSI or Micron.
- **Intended use**: a starting point and reference for engineers bringing up these memories on STM32N6, and an example of a driver structure where each protocol detail is traceable to a datasheet and every line is exercised by host tests.

---

## 🚀 Key Capabilities

- **Device detection** (`ExtMem_Init()` / `ExtMem_AutoDetect()`), in this order:
  1. a part forced through `Config.ForcedPartNumber` or `Config.ForcedDeviceType`;
  2. the JEDEC ID (9Fh) looked up in the device table;
  3. a JEDEC JESD216 SFDP fallback (basic flash parameter table) for unlisted densities of the three quad NOR vendors only (ISSI, Micron MT25Q, Infineon);
  4. the HyperBus ID0 register for HyperRAM (manufacturer and row/column geometry give the capacity).

  HyperFlash is not probed (its identification goes through the CFI address space, which the probe does not implement) and must be forced. When nothing answers, `ExtMem_Init()` returns `EXTMEM_NOT_SUPPORTED` instead of guessing a device; there is no claim of universal compatibility.
- **Controller setup**: XSPIM port routing, VDDIO domain, clock divider (probing at up to 50 MHz, then the configured clock, refused if above the part maximum), `DEVSIZE` from the detected capacity (RM0486), chip-select boundary on stacked SEMPER and IS66WVH64M8 parts, and the CS# refresh counter for self-refreshing RAMs.
- **Protocol handling per family**: 8D-8D-8D entry and exit, latency and dummy-cycle programming, 3/4-byte addressing, quad enable, status and error-flag polling with timeouts taken from the datasheet maxima, per-die configuration and polling on stacked parts.
- **Memory-mapped mode** (`ExtMem_EnableMemoryMapped()`): configures the XSPI read (and, for RAMs, write) commands so the device appears at `0x90000000` (XSPI1), `0x70000000` (XSPI2), `0x80000000` (XSPI3) or `0x60000000` (FMC). This is the read path needed for execute-in-place; the MPU regions, linker sections and boot flow that running code from external Flash requires are left to the application (see the [integration guide](Docs/STM32CubeIDE_Integration_Guide.md)) and executing code from external memory has not been demonstrated.
- **Cache maintenance**: when the D-cache is enabled, the manager cleans and invalidates the affected lines (`SCB_CleanInvalidateDCache_by_Addr`) for reads and RAM writes performed through the memory-mapped window. Cache coherence of DMA buffers remains the caller's responsibility.
- **Error handling**: parameter and address-range checks against the detected capacity (`EXTMEM_INVALID_PARAM`), propagation of every HAL error, program/erase failure flags reported and cleared, polling timeouts (`EXTMEM_TIMEOUT` / `EXTMEM_ERROR`). There is no retry or recovery logic beyond that.
- **DMA**: `ExtMem_ReadDMA()` / `ExtMem_WriteDMA()` use the XSPI DMA path for Infineon HyperRAM and ISSI OctalRAM and fall back to blocking transfers for the other types; the DMA channels must be linked to the XSPI handle by the application.

---

## 📊 Device Support

"Supported" means: a driver implements the datasheet protocol, the part is in the device table with its limits, and the host test suite brings it up through `ExtMem_Init()` and exercises read, write, erase, memory-mapped mode, reset and power down against the emulated device. It does **not** mean the part has been tested on hardware.

| Manufacturer | Memory Family | Bus Interface | Max Clock | Primary Mode | Component Driver |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Infineon** | SEMPER™ Octal NOR (`S28HS/S28HL` 256T to 02GT) | XSPI1 / XSPI2 | 200 MHz (HS-T) / 166 MHz (HL-T) | 8D-8D-8D (Octal DTR) | `s28hs512t` |
| **Infineon** | HyperFlash™ (`S26KS/S26KL`) | XSPI1 / XSPI2 | 166 MHz | HyperBus™ | `s26ks512s` |
| **Infineon** | HyperRAM™ (`S27KS0641/S27KL0641`, `S70KS1281/S70KL1281`, `S80KS2562`) | XSPI1 / XSPI2 | 200 MHz | HyperBus™ DDR | `s27ks0641` |
| **Infineon** | SEMPER™ / FL-L Quad (`S25HL512T`, `S25HS512T`, `S25FL256L`) | XSPI1 / XSPI2 / XSPI3 | 118 MHz | 1-4-4 Quad SPI | `s25hl512t` |
| **ISSI** | Octal NOR Flash (`IS25LX/IS25WX` 064 to 512) | XSPI1 / XSPI2 | 200 MHz (WX) / 133 MHz (LX) | 8D-8D-8D (Octal DTR) | `is25lx256` |
| **ISSI** | HyperFlash™ (`IS26KS/IS26KL`) | XSPI1 / XSPI2 | 166 MHz | HyperBus™ | `s26ks512s` |
| **ISSI** | Quad NOR Flash (`IS25LP/WP` 080 to 512, `IS25LE/WE128`, `IS25LQ032B`) | XSPI1 / XSPI2 / XSPI3 | 104 to 133 MHz (per part) | 1-4-4 Quad SPI | `is25lp256` |
| **ISSI** | Parallel NOR Flash (`IS29GL032/064/128/256`) | FMC (16-bit) | 110 ns access | 16-bit Parallel CFI NOR | `is29gl_fmc` |
| **ISSI** | OctalRAM (`IS66WVO/IS67WVO`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D OPI (XSPI Macronix RAM mode) | `is66wvo32m8` |
| **ISSI** | HyperRAM™ (`IS66WVH8M8/16M8/64M8`) | XSPI1 / XSPI2 | 166 MHz (200 MHz IS66WVH64M8) | HyperBus™ DDR | `is66wvh16m8` |
| **ISSI** | Quad SPI PSRAM (`IS66WVS/IS67WVS`) | XSPI1 / XSPI2 / XSPI3 | 104 MHz | SPI / QPI | `is66wvs16m8` |
| **ISSI** | Serial SRAM (`IS62WVS/IS65WVS`) | XSPI1 / XSPI2 / XSPI3 | 20 MHz | SPI / SQI (1-1-1 / 4-4-4) | `is62wvs` |
| **ISSI / Infineon** | Asynchronous PSRAM / SRAM (`IS66WV51216`, `IS66WVE1M16/2M16/4M16`, `CY62167EV30`) | FMC (16-bit) | 70 ns access | 16-bit Parallel SRAM/PSRAM | `is66wv_fmc` |
| **Micron** | Xccela™ Octal NOR (`MT35XU/MT35XL` 256 to 02G) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `mt35xu512a` |
| **Micron** | MT25Q Quad NOR (`MT25QU/MT25QL` 032 to 01G) | XSPI1 / XSPI2 / XSPI3 | 125 MHz | 1-4-4 Quad SPI | `mt25qu512a` |
| **Micron** | Parallel NOR Flash (`MT28EW128/256/512/01G`) | FMC (16-bit) | 110 ns access | 16-bit Parallel CFI NOR | `is29gl_fmc` |

"Max Clock" is the highest bus clock the driver accepts with the latency it programs, as derived from the datasheet tables; it is not a measured result. All 106 parts, with density, voltage, die count and clock limit, are listed in [Docs/Supported_Memories_Matrix.md](Docs/Supported_Memories_Matrix.md); the datasheets used are listed in [Docs/Project_Context.md](Docs/Project_Context.md).

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
│   ├── extmem_demo.c                # Identification, write/read-back and memory-mapped read-back
│   └── extmem_benchmark.c           # Read/write throughput measurement (MB/s) for a board
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
│   ├── Supported_Memories_Matrix.md      # Every part of the device table with its limits
│   ├── Hardware_Design_and_Pinout.md     # STM32N6 pinout, PCB layout guidelines, VDDIO domains
│   └── Project_Context.md                # Architecture, conventions, status, datasheet sources
├── .github/
│   └── workflows/
│       └── ci.yml                   # CI: host tests, coverage, examples, Cortex-M55 cross-compile
├── Makefile                         # test, coverage, examples and target-check targets
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

The snippets below show the API; they have been compile-checked as part of `Examples/`, not run on hardware.

### 1. Initialization with Auto-Discovery
```c
#include "stm32n6_extmem.h"

ExtMem_HandleTypeDef hextmem = {0};

void Memory_Setup(void)
{
  /* XSPI1, clock divider 2 (400 MHz XSPI kernel clock / 2 = 200 MHz), VDDIO domain at 1.8 V */
  hextmem.Config.Bus            = EXTMEM_BUS_XSPI1;
  hextmem.Config.ClockPrescaler = 2;
  hextmem.Config.Force1V8       = true;

  /* Detection: forced part, then JEDEC ID, then SFDP (quad NOR vendors), then HyperBus ID0 */
  if (ExtMem_Init(&hextmem) == EXTMEM_OK)
  {
    printf("Detected memory: %s (%lu MB)\r\n",
           ExtMem_GetDeviceName(&hextmem),
           (unsigned long)(hextmem.Geometry.TotalSizeBytes / (1024U * 1024U)));
  }
}
```

### 2. Memory-Mapped Mode
```c
/* Map the device at the XSPI1 window (0x90000000) */
if (ExtMem_EnableMemoryMapped(&hextmem) == EXTMEM_OK)
{
  /* Reads are plain loads through the XSPI memory-mapped window */
  const volatile uint32_t *pExt = (const volatile uint32_t *)hextmem.MemoryMappedBase;
  printf("First word: 0x%08lX\r\n", (unsigned long)pExt[0]);
}
```
Running code from this window also needs an MPU region with the right attributes, linker sections and a boot flow that sets the memory up first; see the [integration guide](Docs/STM32CubeIDE_Integration_Guide.md).

### 3. RAM Read & Write
```c
uint8_t txData[1024];
uint8_t rxData[1024];

memset(txData, 0xA5, sizeof(txData));

/* Indirect transfers; the bus clock is the one configured in hextmem.Config */
if (ExtMem_Write(&hextmem, 0x00000000, txData, sizeof(txData)) == EXTMEM_OK &&
    ExtMem_Read(&hextmem, 0x00000000, rxData, sizeof(rxData)) == EXTMEM_OK)
{
  /* compare txData and rxData */
}
```

`Examples/extmem_demo.c` (identification, write/read-back, memory-mapped read-back) and `Examples/extmem_benchmark.c` (throughput measured with `HAL_GetTick()`) are intended to run on a board; no measured results are published yet.

---

## 🧪 Testing and Validation

### Validation status

| Level | Status | Evidence |
| :--- | :--- | :--- |
| Implementation against datasheets | Done | Every driver checked against the full datasheet of its family; sources and remaining gaps in [Docs/Project_Context.md](Docs/Project_Context.md) |
| Host compilation, warnings as errors | Done | `make test` (`-Wall -Wextra -Werror -std=c11`) |
| Host unit tests on a HAL mock | Done | 21 test groups, all passing |
| Line and branch coverage of the driver sources | Measured at 100% / 100% | `make coverage` (gcov); the CI gate enforces 100% of lines, branches are reported |
| Cross-compilation against the real STM32CubeN6 HAL (Cortex-M55) | Done in CI | `make target-check`, compile only, no link or run |
| Static analysis (MISRA, cppcheck, etc.) | Not done | |
| Execution on STM32N6 hardware | **Not done** | |
| Tests with physical memory devices | **Not done** | |
| Throughput benchmarks | **Not done** | `Examples/extmem_benchmark.c` provided, no results yet |
| Temperature / voltage characterization | **Not done** | |

### Host test harness

The host build replaces the STM32CubeN6 HAL with a mock (`Tests/mock_hal.c`) whose constants follow `stm32n6xx_hal_xspi.h` / `stm32n6xx_ll_fmc.h`. The mock:

- logs every HAL call, so each test checks the exact instruction, address, alternate byte, dummy and data phases (lines, width, STR/DTR, DQS) sent for every command, and flags HAL parameter combinations the real HAL would reject;
- emulates the memories at command level: opcode decoding (including the 8D-8D-8D instruction extension), write enable latch, 4-byte address mode, status / configuration / volatile registers, NOR program (bits only cleared) and erase, HyperBus register space, stacked-die status, and the HyperFlash and parallel NOR command state machines;
- injects a failure into any HAL call: every driver function is run once per HAL call with that call failing, and must report the error.

The manager test brings up every device of the table through `ExtMem_Init()` and runs program / erase / read, DMA, memory-mapped mode, reset, power down and deinit on it.

These tests show that the drivers issue the command sequences the datasheets describe and handle every error path the HAL can report. Because the device emulation is written from the same datasheets, they cannot reveal a misreading of a datasheet, and they say nothing about signal integrity, timing margins, delay-block tuning or silicon errata.

### Running the checks
```bash
make test          # host build and 21 test groups
make coverage      # gcov line + branch coverage of the 16 driver sources (100% of lines required)
make examples      # the examples must keep compiling against the driver API
make target-check  # cross-compile for Cortex-M55 against the STM32CubeN6 HAL (needs arm-none-eabi-gcc
                   # and the HAL, device and CMSIS repositories next to this one, as in CI)
```

The CI workflow runs all four targets on every push and pull request to `main`.

---

## ⚙️ Known Limitations

- **Clocks**: `Config.ClockPrescaler` is the XSPI clock divider (memory clock = XSPI kernel clock / divider). Probing and mode switching run at up to `EXTMEM_INIT_MAX_CLOCK_HZ` (50 MHz); `ExtMem_Init()` refuses a configured clock above the part maximum.
- **SEMPER Octal 8D-8D-8D commands** repeat the opcode in the second instruction byte (for example `EEh EEh` for the read), as stated by the SFDP and the transaction tables of the SEMPER Octal datasheets.
- **S28HS02GT / S28HL02GT (dual die)**: implemented from datasheet 002-23755. Each die has its own registers at its base address + 0x800000, status is polled on the die that runs the operation, chip erase is one addressed erase (61h) per die, and a write disable follows every program or erase so no die keeps WRPGEN set. Only the GZ speed grade (models 25/35) of the S28HS02GT runs 200 MHz DDR: the JEDEC ID does not identify the grade, so the database limits the part to 166 MHz.
- **SEMPER erase**: the factory sector map is uniform 256 KB; the 4 KB erase is only executed by parts configured for hybrid sectors.
- **SEMPER Octal in 1S-1S-1S** reads with Read Fast 0Bh (0Ch is not implemented) at the current address length: the driver keeps the part in 4-byte mode, also after a software reset.
- **S25Hx-T Quad I/O reads** use the factory memory latency (8 cycles plus 2 mode cycles), valid up to 118 MHz. SEMPER Quad has no quad page program: it programs in 1S-1S-1S (12h); the S25FL-L uses Quad Page Program (34h).
- **S25FL-L** shares the S25Hx-T driver: failure flags are read from SR2 and cleared with CLSR. A plain timeout on an S25FL-L whose SR1 protection bits (SEC / TBPROT) are set is reported as an error instead of a timeout.
- **ISSI quad NOR**: the volatile Read Register is set to 11 dummy cycles; the database limits every part to the QUAD I/O FAST READ frequency its datasheet gives for 11 cycles (104 MHz on IS25WP256D/032D/016D, 112 MHz on IS25WP512M, 117 MHz on IS25LP512M, 133 MHz otherwise). IS25LQ/WQ have no Read Register: their fixed mode byte plus 4 dummy cycles run up to 104 MHz.
- **MT25Q** keeps the factory 10 dummy cycles, which limit QUAD I/O FAST READ to 125 MHz.
- **Stacked Micron parts** (MT25QU01G, MT35XU01G, MT35XU02G): successive flag status reads return the status of each die, so a program or erase completes only when every die reports ready.
- **Dual-die HyperRAM**: S70KS1281 / S70KL1281 get CR0 on both dice (CA35 selects the die) and enter deep power down on both. IS66WVH64M8 gets CR0 on both dice (CA37 selects the die) with the fixed latency the datasheet requires, bursts restart at the 32 MByte die boundary, and deep power down is not available.
- **ISSI IS66WVO OctalRAM** uses the XSPI "Macronix RAM" mode with fixed latency (STM32N6 erratum ES0620). On early STM32N6 silicon this mode only decodes 13 row address bits, so only the first 8 MBytes of larger OctalRAM parts are reachable (see ES0620).
- **Self-refreshing RAMs** (HyperRAM, OctalRAM, quad PSRAM): CS# is released at least every `EXTMEM_PSRAM_MAX_CS_LOW_NS` (1 us, valid up to 105/125 degC).
- **Documentation sources**: S25Hx-T was checked against the Japanese edition of its datasheet (the English one could not be downloaded); IS66WVH8M8/16M8 against a partly unreadable PDF plus the IS66WVH64M8 datasheet of the same family; smaller densities of a family against the datasheet of a sibling density where the protocol is shared. IS29GL512 and IS25WQ032 were removed because no datasheet was available.
- **Speed grades**: where the JEDEC ID cannot tell speed grades apart (S28HS02GT FP vs GZ), the table uses the slower grade.

## 🧭 Development Status and Roadmap

The driver code and the host tests are complete for the parts in the device table. The next steps need hardware:

1. Bring-up on an STM32N6 board (for example STM32N6570-DK): identification, program/erase, memory-mapped read-back and CRC for each memory family available.
2. XSPI delay-block / sample-shift tuning for 166 to 200 MHz DTR operation.
3. Measured throughput with `Examples/extmem_benchmark.c`.
4. Execute-in-place demonstration (MPU, linker sections, boot sequence).

## 🤝 Contributing

Issues and pull requests are welcome, especially hardware test reports: please state the board, memory part number and date code, bus clock and what was tested. Code changes must keep `make test`, `make coverage`, `make examples` and `make target-check` passing, and protocol changes should cite the datasheet section they rely on.

## 📚 Detailed Documentation

- [STM32CubeIDE Integration Guide](Docs/STM32CubeIDE_Integration_Guide.md)
- [Supported Memories Matrix](Docs/Supported_Memories_Matrix.md)
- [Hardware Design & High-Speed PCB Routing Guidelines](Docs/Hardware_Design_and_Pinout.md)
- [Project Context (architecture, conventions, status, sources)](Docs/Project_Context.md)

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
