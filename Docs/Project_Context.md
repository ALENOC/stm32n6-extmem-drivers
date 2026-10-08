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

CI (`.github/workflows/ci.yml`) runs all four on every push and pull request to `main`.
The host suite proves the drivers issue the documented command sequences and handle every error
path; it does not replace validation on hardware.

## 5. Status

Done:
- [x] Protocol review of every driver against datasheets (SEMPER octal/quad, IS25LX/WX, MT35X,
      MT25Q, IS25LP, S25FL-L, HyperRAM, HyperFlash, OctalRAM, quad PSRAM, serial SRAM, FMC NOR/PSRAM)
- [x] HAL mock aligned with the STM32CubeN6 headers, including HAL parameter and state checks
- [x] 100% line and branch coverage, cross-compilation against the real HAL in CI
- [x] Stacked-die support (MT25Q 1 Gb, MT35X 1/2 Gb, S28HS02GT through SFDP)

Open (need hardware or documents not publicly available):
- [ ] Board validation of every memory family at its maximum clock (ID, program/erase, XIP, CRC)
- [ ] S28HS02GT checked against its own datasheet (restricted access at Infineon)
- [ ] IS66WVO parts above 8 MBytes on early STM32N6 silicon (erratum ES0620, Macronix RAM mode)
- [ ] XSPI delay block / sample shifting tuning for 200 MHz DTR on a specific board

## 6. Reference documents used

- ST: RM0486 (STM32N6 reference manual), ES0620 (STM32N6 errata), STM32N6570-DK BSP, STM32CubeN6 HAL
- Infineon: SEMPER Octal 002-18216, SEMPER Quad 002-23660 / 002-12345, SEMPER Quad DDP (2 Gb),
  S25FL128L/256L 002-00124, S27KL0642/S27KS0642 002-31332
- ISSI: IS25WX064/032, IS25LP256D/IS25WP256D, IS66/67WVO8M8, IS66/67WVS16M8, IS66WVH16M8,
  IS62/65WVS0648/1288/2568/5128
- Linux kernel `drivers/mtd/spi-nor` (spansion.c, micron-st.c, sfdp.c)
