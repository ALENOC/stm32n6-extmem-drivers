# Supported Memories Matrix

One row per entry of the device database (`Drivers/BSP/STM32N6_ExtMem/stm32n6_extmem_devices.c`).
"Max clock" is the highest bus clock the driver accepts for the part with the latency it programs;
`ExtMem_Init()` refuses a faster configured clock. Values come from the vendor datasheets listed in
[Project_Context.md](Project_Context.md).

| Vendor | Part | Density | Technology | Interface | Voltage | Max clock | Driver |
|:---|:---|:---|:---|:---|:---|:---|:---|
| Infineon | `S28HS512T` | 512 Mb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `s28hs512t` |
| Infineon | `S28HL512T` | 512 Mb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 166 MHz | `s28hs512t` |
| Infineon | `S28HS256T` | 256 Mb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `s28hs512t` |
| Infineon | `S28HL256T` | 256 Mb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 166 MHz | `s28hs512t` |
| Infineon | `S28HS01GT` | 1 Gb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `s28hs512t` |
| Infineon | `S28HL01GT` | 1 Gb | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 166 MHz | `s28hs512t` |
| Infineon | `S28HS02GT` | 2 Gb, 2 dice | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 166 MHz | `s28hs512t` |
| Infineon | `S28HL02GT` | 2 Gb, 2 dice | SEMPER™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `s28hs512t` |
| Infineon | `S26KS512S` | 512 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| Infineon | `S26KL512S` | 512 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 3.0 V | 100 MHz | `s26ks512s` |
| Infineon | `S26KS256S` | 256 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| Infineon | `S26KS128S` | 128 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| Infineon | `S27KS0641` | 64 Mb | HyperRAM™ | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s27ks0641` |
| Infineon | `S27KL0641` | 64 Mb | HyperRAM™ | HyperBus™ DDR, RWDS | 3.0 V | 100 MHz | `s27ks0641` |
| Infineon | `S70KS1281` | 128 Mb, 2 dice | HyperRAM™ | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s27ks0641` |
| Infineon | `S70KL1281` | 128 Mb, 2 dice | HyperRAM™ | HyperBus™ DDR, RWDS | 3.0 V | 100 MHz | `s27ks0641` |
| Infineon | `S80KS2562` | 256 Mb | HyperRAM™ | HyperBus™ DDR, RWDS | 1.8 V | 200 MHz | `s27ks0641` |
| Infineon | `S25HL512T` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 118 MHz | `s25hl512t` |
| Infineon | `S25HS512T` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 118 MHz | `s25hl512t` |
| Infineon | `S25FL256L` | 256 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 108 MHz | `s25hl512t` |
| ISSI | `IS25LX512` | 512 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `is25lx256` |
| ISSI | `IS25WX512` | 512 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `is25lx256` |
| ISSI | `IS25LX256` | 256 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `is25lx256` |
| ISSI | `IS25WX256` | 256 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `is25lx256` |
| ISSI | `IS25LX128` | 128 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `is25lx256` |
| ISSI | `IS25WX128` | 128 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `is25lx256` |
| ISSI | `IS25LX064` | 64 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `is25lx256` |
| ISSI | `IS25WX064` | 64 Mb | Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `is25lx256` |
| ISSI | `IS26KS512S` | 512 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| ISSI | `IS26KL512S` | 512 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 3.0 V | 166 MHz | `s26ks512s` |
| ISSI | `IS26KS256S` | 256 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| ISSI | `IS26KL256S` | 256 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 3.0 V | 166 MHz | `s26ks512s` |
| ISSI | `IS26KS128S` | 128 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `s26ks512s` |
| ISSI | `IS26KL128S` | 128 Mb | HyperFlash™ NOR | HyperBus™ DDR, RWDS | 3.0 V | 166 MHz | `s26ks512s` |
| ISSI | `IS25LP512M` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 117 MHz | `is25lp256` |
| ISSI | `IS25WP512M` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 112 MHz | `is25lp256` |
| ISSI | `IS25LP256D` | 256 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP256D` | 256 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 104 MHz | `is25lp256` |
| ISSI | `IS25LP128F` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP128F` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 133 MHz | `is25lp256` |
| ISSI | `IS25LP064D` | 64 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP064D` | 64 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 133 MHz | `is25lp256` |
| ISSI | `IS25LP032D` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP032D` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 104 MHz | `is25lp256` |
| ISSI | `IS25LP016D` | 16 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP016D` | 16 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 104 MHz | `is25lp256` |
| ISSI | `IS25LP080D` | 8 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WP080D` | 8 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 133 MHz | `is25lp256` |
| ISSI | `IS25LQ032B` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 104 MHz | `is25lp256` |
| ISSI | `IS25WQ032` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 104 MHz | `is25lp256` |
| ISSI | `IS25LE128` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 133 MHz | `is25lp256` |
| ISSI | `IS25WE128` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 133 MHz | `is25lp256` |
| ISSI | `IS29GL512` | 512 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| ISSI | `IS29GL256` | 256 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| ISSI | `IS29GL128` | 128 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| ISSI | `IS29GL064` | 64 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| ISSI | `IS29GL032` | 32 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| ISSI | `IS66WVO32M8` | 256 Mb | OctalRAM | Octal 8D-8D-8D (XSPI Macronix RAM mode) | 1.8 V | 200 MHz | `is66wvo32m8` |
| ISSI | `IS66WVO16M8` | 128 Mb | OctalRAM | Octal 8D-8D-8D (XSPI Macronix RAM mode) | 1.8 V | 200 MHz | `is66wvo32m8` |
| ISSI | `IS66WVO64M8` | 512 Mb | OctalRAM | Octal 8D-8D-8D (XSPI Macronix RAM mode) | 1.8 V | 200 MHz | `is66wvo32m8` |
| ISSI | `IS66WVO8M8` | 64 Mb | OctalRAM | Octal 8D-8D-8D (XSPI Macronix RAM mode) | 1.8 V | 200 MHz | `is66wvo32m8` |
| ISSI | `IS67WVO8M8` | 64 Mb | OctalRAM | Octal 8D-8D-8D (XSPI Macronix RAM mode) | 1.8 V | 200 MHz | `is66wvo32m8` |
| ISSI | `IS66WVH16M8` | 128 Mb | HyperRAM™ | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `is66wvh16m8` |
| ISSI | `IS66WVH8M8` | 64 Mb | HyperRAM™ | HyperBus™ DDR, RWDS | 1.8 V | 166 MHz | `is66wvh16m8` |
| ISSI | `IS66WVS1M8` | 8 Mb | Quad SPI PSRAM | SPI / QPI | 1.8 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS66WVS2M8` | 16 Mb | Quad SPI PSRAM | SPI / QPI | 1.8 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS66WVS4M8` | 32 Mb | Quad SPI PSRAM | SPI / QPI | 1.8 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS66WVS8M8` | 64 Mb | Quad SPI PSRAM | SPI / QPI | 1.8 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS66WVS16M8` | 128 Mb | Quad SPI PSRAM | SPI / QPI | 1.8 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS67WVS4M8` | 32 Mb | Quad SPI PSRAM | SPI / QPI | 3.0 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS67WVS16M8` | 128 Mb | Quad SPI PSRAM | SPI / QPI | 3.0 V | 104 MHz | `is66wvs16m8` |
| ISSI | `IS62WVS5128` | 4 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 20 MHz | `is62wvs` |
| ISSI | `IS62WVS2568` | 2 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 20 MHz | `is62wvs` |
| ISSI | `IS62WVS1288` | 1 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 20 MHz | `is62wvs` |
| ISSI | `IS62WVS0648` | 512 Kb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 20 MHz | `is62wvs` |
| ISSI | `IS65WVS5128` | 4 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 16 MHz | `is62wvs` |
| ISSI | `IS65WVS2568` | 2 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 16 MHz | `is62wvs` |
| ISSI | `IS65WVS1288` | 1 Mb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 16 MHz | `is62wvs` |
| ISSI | `IS65WVS0648` | 512 Kb | Serial SRAM | SPI / SDI / SQI | 3.0 V | 16 MHz | `is62wvs` |
| ISSI | `IS66WVE4M16` | 64 Mb | Asynchronous PSRAM / SRAM | FMC 16-bit asynchronous | 3.0 V | async, 70 ns access | `is66wv_fmc` |
| ISSI | `IS66WVE2M16` | 32 Mb | Asynchronous PSRAM / SRAM | FMC 16-bit asynchronous | 1.8 V | async, 70 ns access | `is66wv_fmc` |
| ISSI | `IS66WVE1M16` | 16 Mb | Asynchronous PSRAM / SRAM | FMC 16-bit asynchronous | 3.0 V | async, 70 ns access | `is66wv_fmc` |
| ISSI | `IS66WV51216` | 8 Mb | Asynchronous PSRAM / SRAM | FMC 16-bit asynchronous | 3.0 V | async, 70 ns access | `is66wv_fmc` |
| Infineon | `CY62167EV30` | 16 Mb | Asynchronous PSRAM / SRAM | FMC 16-bit asynchronous | 3.0 V | async, 70 ns access | `is66wv_fmc` |
| Micron | `MT35XU02G` | 2 Gb, 4 dice | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `mt35xu512a` |
| Micron | `MT35XU01GBBA` | 1 Gb, 2 dice | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `mt35xu512a` |
| Micron | `MT35XL01GBBA` | 1 Gb, 2 dice | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `mt35xu512a` |
| Micron | `MT35XU512ABA` | 512 Mb | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `mt35xu512a` |
| Micron | `MT35XL512ABA` | 512 Mb | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `mt35xu512a` |
| Micron | `MT35XU256ABA` | 256 Mb | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 1.8 V | 200 MHz | `mt35xu512a` |
| Micron | `MT35XL256ABA` | 256 Mb | Xccela™ Octal NOR Flash | Octal xSPI 8D-8D-8D, DQS | 3.0 V | 133 MHz | `mt35xu512a` |
| Micron | `MT25QU01GBBB` | 1 Gb, 2 dice | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL01GBBB` | 1 Gb, 2 dice | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QU512ABB` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL512ABB` | 512 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QU256ABA` | 256 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL256ABA` | 256 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QU128ABA` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL128ABA` | 128 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QU064ABA` | 64 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL064ABA` | 64 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QU032ABA` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 1.8 V | 125 MHz | `mt25qu512a` |
| Micron | `MT25QL032ABA` | 32 Mb | Quad SPI NOR Flash | Quad SPI 1-4-4 | 3.0 V | 125 MHz | `mt25qu512a` |
| Micron | `MT28EW01GABA` | 1 Gb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| Micron | `MT28EW512ABA` | 512 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| Micron | `MT28EW256ABA` | 256 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |
| Micron | `MT28EW128ABA` | 128 Mb | Parallel NOR Flash | FMC 16-bit asynchronous | 3.0 V | async, 110 ns access | `is29gl_fmc` |

## Notes

- **S28HS02GT**: only the GZ speed grade (models 25/35) runs 200 MHz DDR; the JEDEC ID does not tell the
  grades apart, so the database uses the 166 MHz of every other grade.
- **S25Hx-T** has no quad page program: programming runs in 1S-1S-1S (12h), reads in 1-4-4.
- **MT25Q**: the factory 10 dummy cycles limit QUAD I/O FAST READ to 125 MHz.
- **ISSI quad NOR**: limits are those of QUAD I/O FAST READ with the 11 dummy cycles the driver programs
  (Table 6.11 of each datasheet); IS25LQ/WQ have no Read Register and keep the fixed 6-cycle latency.
- **Stacked dice**: SEMPER and HyperRAM dice are configured one by one; Micron stacks are polled until
  every die reports ready; die erase replaces bulk erase where the datasheet requires it.
- **FMC parts** are asynchronous: the manager derives the FMC timings from the access times above and the
  FMC kernel clock.
