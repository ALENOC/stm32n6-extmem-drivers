# Project Context

Single entry point for anyone (or any tool) resuming work on this repository: what the project is,
how it is organized, which rules apply, what is done and what is still open.
It contains no credentials, personal data or machine-specific paths.

## 1. Project

External memory driver suite for the **STM32N6** (Cortex-M55): serial NOR flash, HyperFlash,
HyperRAM, OctalRAM, quad PSRAM and serial SRAM on **XSPI1/2/3**, plus parallel PSRAM/SRAM and NOR
flash on the **FMC**. Vendors: Infineon, ISSI, Micron.

- Language: C11, no dynamic allocation, STM32CubeN6 HAL (`stm32n6xx_hal_xspi`, `stm32n6xx_ll_fmc`).
- Target build: Arm GNU toolchain, `-mcpu=cortex-m55`.
- Host build: GCC with a HAL mock (`-DEXTMEM_UNIT_TEST`) for the test suite.

## 2. Architecture

```
Application
  └── STM32N6_ExtMem/stm32n6_extmem.c      Unified manager: bus/port/VDDIO/clock setup, detection,
                                           read/write/erase, memory-mapped mode, DMA, reset, power down
        ├── stm32n6_extmem_devices.c        Device table (IDs, geometry, clocks, latencies, die count)
        ├── Components/Common/sfdp.c        JESD216 parser (BFPT, SCCR and SCCR multi-chip maps)
        └── Components/<part>/<part>.c      One protocol driver per memory family
```

Detection order in `ExtMem_AutoDetect()`:
1. forced part number / forced type from `ExtMem_Config_t`;
2. JEDEC ID (0x9F) lookup in the device table;
3. SFDP, only for quad NOR vendors with a driver (ISSI, Micron, Infineon);
4. HyperBus ID0 register (HyperRAM), tried with 7 then 6 latency clocks;
5. nothing answered: `EXTMEM_NOT_SUPPORTED` (no device is ever guessed).

Clocking: `Config.ClockPrescaler` is a divider of the XSPI kernel clock. Probing runs at up to
`EXTMEM_INIT_MAX_CLOCK_HZ`, the configured clock is applied after the memory is set up, and a clock
above the part maximum is refused. Self-refreshing RAMs get the XSPI refresh counter so CS# never
stays LOW longer than `EXTMEM_PSRAM_MAX_CS_LOW_NS`.

## 3. Conventions

- Every driver function returns 0 on success; every HAL failure is propagated (the test suite checks
  this by failing each HAL call in turn).
- Protocol details must come from a vendor datasheet, an ST reference/errata document or the Linux
  spi-nor driver; the source is named in a comment next to the code that depends on it.
- `HAL_XSPI_SIZE_*` and `HAL_XSPI_BONDARYOF_*` names are in **bits** (code n = 2^(n+1) or 2^n bytes).
- Repository text: English, no en/em dashes, commits authored by the repository owner only.

## 4. Build and verification

| Command | Purpose |
|:---|:---|
| `make test` | Host test suite (21 groups, every database device exercised end to end) |
| `make coverage` | gcov line and branch coverage of every driver source, 100% required |
| `make examples` | Examples compile against the driver API |
| `make target-check` | Cross-compile drivers and examples for Cortex-M55 against the STM32CubeN6 HAL |
| `make static-analysis` | GCC `-fanalyzer`, cppcheck and clang-tidy (must report nothing), MISRA C:2012 addon report ([Static_Analysis.md](Static_Analysis.md)) |

CI (`.github/workflows/ci.yml`) runs all five on every push and pull request to `main`.
The host suite proves the drivers issue the documented command sequences and handle every error
path; it does not replace validation on hardware.

## 5. Status

Done:
- [x] Every driver checked against the full vendor datasheet of each family (October 2026 audit), see section 6
- [x] HAL mock aligned with the STM32CubeN6 headers, including HAL parameter and state checks
- [x] 100% line and branch coverage, cross-compilation against the real HAL in CI
- [x] Static analysis with GCC `-fanalyzer`, cppcheck and clang-tidy clean in CI; MISRA C:2012 addon findings
      reduced from 1132 to 671, the rest documented as deviations (not a compliance claim)
- [x] Stacked dice: MT25Q 1 Gb, MT35X 1/2 Gb (all dice polled), S28HS02GT / S28HL02GT (datasheet 002-23755),
      IS66WVH64M8 dual-die HyperRAM
- [x] Device table: part numbers checked against the datasheets and every clock limit matched to the
      latency the driver programs

Open (need hardware):
- [ ] Board validation of every memory family at its maximum clock (ID, program/erase, XIP, CRC)
- [ ] IS66WVO parts above 8 MBytes on early STM32N6 silicon (erratum ES0620, Macronix RAM mode)
- [ ] XSPI delay block / sample shifting tuning for 200 MHz DTR on a specific board

Removed: IS29GL512 and IS25WQ032 (no datasheet available to verify them).

## 6. Reference documents used

Vendor datasheets (full documents) used for the audit of each driver:
- Infineon: SEMPER Octal 002-18216 (S28HS/HL 512T/01GT) and 002-23755 (S28HS/HL02GT), SEMPER Quad 002-23660
  (S25HS/HL 256T/512T/01GT), FL-L 002-00124 (S25FL128L/256L), HyperFlash
  001-99198 (S26KS/KL), HyperRAM 001-97964 (dual-die protocol), 002-31337 (S80KS2562), S27KS0642/0643,
  CY62167EV30
- ISSI: IS25LX/WX 032/064, 128/256, 512M; IS25LP/WP 080D/016D/032D/064D/128F/256D/512M;
  IS25LE/WE128E; IS66/67WVO 8M8/16M8/32M8/64M8; IS66WVH8M8/16M8/64M8 (64M8 Rev. A1); IS66/67WVS4M8/16M8;
  IS62/65WVS 0648/1288/2568/5128; IS66WV51216; IS66WVE1M16/2M16/4M16; IS29GL032/064/128/256
- Micron: MT35XU512ABA, MT35XU02G; MT25QU128/256/512/01G, MT25QL256
- ST: RM0486 (STM32N6 reference manual), ES0620 (STM32N6 errata), STM32N6570-DK BSP, STM32CubeN6 HAL
- Linux kernel `drivers/mtd/spi-nor` (spansion.c, micron-st.c, sfdp.c), used as a cross-check only
