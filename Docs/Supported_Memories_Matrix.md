# Supported Memories Matrix
## Complete Infineon, ISSI & Micron Flash and PSRAM Matrix for STM32N6

The following matrix lists all Flash and PSRAM parts from **Infineon Technologies**, **ISSI (Integrated Silicon Solution, Inc.)**, and **Micron Technology** that are electrically compatible with STM32N6 **XSPI (Octal/Quad/HyperBus)** and **FMC** controllers and fully supported by this driver suite:

| Manufacturer | Family / Part Number | Memory Technology | Bus Interface / Protocol | Operating Voltage | Max Frequency | Component Driver |
|:---|:---|:---|:---|:---|:---|:---|
| **Infineon** | `S28HS512T` (512Mb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HL512T` (512Mb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `s28hs512t` |
| **Infineon** | `S28HS256T` (256Mb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HL256T` (256Mb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `s28hs512t` |
| **Infineon** | `S28HS01GT` (1Gb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HS02GT` (2Gb, 2 dice) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S26KS512S` (512Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S26KL512S` (512Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 100 MHz | `s26ks512s` |
| **Infineon** | `S26KS256S` (256Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S26KS128S` (128Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S27KS0641` (64Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s27ks0641` |
| **Infineon** | `S27KL0641` (64Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 100 MHz | `s27ks0641` |
| **Infineon** | `S27KS128` (128Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s27ks0641` |
| **Infineon** | `S27KS256` (256Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 200 MHz | `s27ks0641` |
| **Infineon** | `S27HS064/128/256` | HyperRAM™ 3.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 200 MHz | `s27ks0641` |
| **Infineon** | `S25HS512T` (512Mb) | SEMPER™ Quad Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 118 MHz | `s25hl512t` |
| **Infineon** | `S25HL512T` (512Mb) | SEMPER™ Quad Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 118 MHz | `s25hl512t` |
| **Infineon** | `S25FL256L/128L` | FL-L Quad Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 108 MHz | `s25hl512t` |
| **Infineon** | `CY62167EV30` (16Mb) | MoBL Asynch SRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **Infineon** | `CY62157EV30` (8Mb) | MoBL Asynch SRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS25LX512` (512Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX512` (512Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25LX256` (256Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX256` (256Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25LX128` (128Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX128` (128Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25LX064` (64Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX064` (64Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS26KS512S` (512Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS26KL512S` (512Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS26KS256S` (256Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS26KL256S` (256Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS26KS128S` (128Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS26KL128S` (128Mb) | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 166 MHz | `s26ks512s` |
| **ISSI** | `IS25LP512M` (512Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP512M` (512Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP256D` (256Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP256D` (256Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP128F` (128Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP128F` (128Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP064D` (64Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP064D` (64Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP032D` (32Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP032D` (32Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP016D` (16Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP016D` (16Mb) | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP080/040/020` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP080/040/020` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LE128/064` | Ultra-Low Power Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WE128/064` | Ultra-Low Power Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LQ032/016/080` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 104 MHz | `is25lp256` |
| **ISSI** | `IS25WQ032/016/080` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 104 MHz | `is25lp256` |
| **ISSI** | `IS29GL512` (512Mb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 110 ns (Page 25ns) | `is29gl_fmc` |
| **ISSI** | `IS29GL256` (256Mb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 90 ns (Page 25ns) | `is29gl_fmc` |
| **ISSI** | `IS29GL128` (128Mb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 90 ns (Page 25ns) | `is29gl_fmc` |
| **ISSI** | `IS29GL064` (64Mb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 70 ns (Page 25ns) | `is29gl_fmc` |
| **ISSI** | `IS29GL032` (32Mb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 70 ns (Page 25ns) | `is29gl_fmc` |
| **ISSI** | `IS66WVO8M8` (64Mb) | OctalRAM DDR | 8D-8D-8D OPI (XSPI Macronix RAM mode) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO16M8` (128Mb)| OctalRAM DDR | 8D-8D-8D OPI (XSPI Macronix RAM mode) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO32M8` (256Mb)| OctalRAM DDR | 8D-8D-8D OPI (XSPI Macronix RAM mode) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO64M8` (512Mb)| OctalRAM DDR | 8D-8D-8D OPI (XSPI Macronix RAM mode) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS67WVO...` (Auto) | OctalRAM DDR | 8D-8D-8D OPI (Automotive Grade) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVH8M8` (64Mb) | HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVH16M8` (128Mb)| HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVH32M8` (256Mb)| HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVS1M8` (8Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS2M8` (16Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS4M8` (32Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS8M8` (64Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS16M8` (128Mb)| Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS67WVS...` (Auto) | Quad SPI PSRAM | Quad SPI / QPI (Automotive Grade) | 1.8V/3.0V | 104 MHz | `is66wvs16m8` |
| **ISSI** | `IS62WVS5128` (4Mb) | Serial Static RAM | SPI / SDI / SQI Serial SRAM | 1.8V/3.0V/3.3V | 20 MHz | `is62wvs` |
| **ISSI** | `IS62WVS2568` (2Mb) | Serial Static RAM | SPI / SDI / SQI Serial SRAM | 1.8V/3.0V/3.3V | 20 MHz | `is62wvs` |
| **ISSI** | `IS62WVS1288` (1Mb) | Serial Static RAM | SPI / SDI / SQI Serial SRAM | 1.8V/3.0V/3.3V | 20 MHz | `is62wvs` |
| **ISSI** | `IS62WVS0648` (512Kb)| Serial Static RAM | SPI / SDI / SQI Serial SRAM | 1.8V/3.0V/3.3V | 20 MHz | `is62wvs` |
| **ISSI** | `IS65WVS...` (Auto) | Serial Static RAM | SPI / SDI / SQI (Automotive Grade) | 1.8V/3.0V | 16 MHz | `is62wvs` |
| **ISSI** | `IS66WV51216` (8Mb) | Parallel PSRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV102416` (16Mb)| Parallel PSRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV204816` (32Mb)| Parallel PSRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV409616` (64Mb)| Parallel PSRAM | 16-bit Parallel Asynchronous FMC | 3.0V | 55 ns | `is66wv_fmc` |
| **Micron** | `MT35XU02G` (2Gb) | Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `mt35xu512a` |
| **Micron** | `MT35XU01GBBA` (1Gb) | Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `mt35xu512a` |
| **Micron** | `MT35XL01GBBA` (1Gb) | Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `mt35xu512a` |
| **Micron** | `MT35XU512ABA` (512Mb)| Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `mt35xu512a` |
| **Micron** | `MT35XL512ABA` (512Mb)| Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `mt35xu512a` |
| **Micron** | `MT35XU256ABA` (256Mb)| Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `mt35xu512a` |
| **Micron** | `MT35XL256ABA` (256Mb)| Xccela™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `mt35xu512a` |
| **Micron** | `MT25QU01GBBB` (1Gb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL01GBBB` (1Gb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QU512ABB` (512Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL512ABB` (512Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QU256ABA` (256Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL256ABA` (256Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 4-byte Addr) | 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QU128ABA` (128Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3/4-byte Addr)| 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL128ABA` (128Mb)| MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3/4-byte Addr)| 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QU064ABA` (64Mb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3-byte Addr) | 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL064ABA` (64Mb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3-byte Addr) | 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QU032ABA` (32Mb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3-byte Addr) | 1.8V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT25QL032ABA` (32Mb) | MT25Q Quad SPI Flash | Quad SPI (1-4-4, 3-byte Addr) | 3.0V | 133 MHz | `mt25qu512a` |
| **Micron** | `MT28EW01GABA` (1Gb) | Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 100 MHz (Page 25ns) | FMC NOR / `is29gl_fmc` |
| **Micron** | `MT28EW512ABA` (512Mb)| Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 100 MHz (Page 25ns) | FMC NOR / `is29gl_fmc` |
| **Micron** | `MT28EW256ABA` (256Mb)| Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 100 MHz (Page 25ns) | FMC NOR / `is29gl_fmc` |
| **Micron** | `MT28EW128ABA` (128Mb)| Parallel NOR Flash | 16-bit Parallel Asynchronous FMC | 3.0V | 100 MHz (Page 25ns) | FMC NOR / `is29gl_fmc` |

