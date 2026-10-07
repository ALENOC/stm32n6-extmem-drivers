# Matrice delle Memorie Supportate
## Infineon & ISSI External Flash and PSRAM for STM32N6

La seguente tabella elenca tutti i componenti Flash e PSRAM di **Infineon** e **ISSI** elettricamente compatibili con i controller **XSPI** e **FMC** di STM32N6 e pienamente supportati da questa suite di librerie:

| Costruttore | Famiglia / Part Number | Tipo Memoria | Interfaccia / Protocollo | Tensione | Max Freq | Component Driver |
|---|---|---|---|---|---|---|
| **Infineon** | `S28HS512T` | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HL512T` | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `s28hs512t` |
| **Infineon** | `S28HS256T` | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HL256T` | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `s28hs512t` |
| **Infineon** | `S28HS01GT` (1Gb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S28HS02GT` (2Gb) | SEMPER™ Octal NOR Flash | Octal SPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `s28hs512t` |
| **Infineon** | `S26KS512S` | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S26KL512S` | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 100 MHz | `s26ks512s` |
| **Infineon** | `S26KS256S` | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S26KS128S` | HyperFlash™ NOR | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s26ks512s` |
| **Infineon** | `S27KS0641` (64Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s27ks0641` |
| **Infineon** | `S27KL0641` (64Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 3.0V | 100 MHz | `s27ks0641` |
| **Infineon** | `S27KS128` (128Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `s27ks0641` |
| **Infineon** | `S27KS256` (256Mb) | HyperRAM™ 2.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 200 MHz | `s27ks0641` |
| **Infineon** | `S27HS064/128/256` | HyperRAM™ 3.0 | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 200 MHz | `s27ks0641` |
| **Infineon** | `S25HS512T` | SEMPER™ Quad Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `s25hl512t` |
| **Infineon** | `S25HL512T` | SEMPER™ Quad Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `s25hl512t` |
| **Infineon** | `S25FL256L/128L` | FL-L Quad Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 108 MHz | `s25hl512t` |
| **Infineon** | `CY62167EV30` (16Mb) | MoBL Asynch SRAM | Parallela 16-bit Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
| **Infineon** | `CY62157EV30` (8Mb) | MoBL Asynch SRAM | Parallela 16-bit Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS25LX064` (64Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX064` (64Mb) | Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `is25lx256` |
| **ISSI** | `IS25LX128` (128Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX128` (128Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `is25lx256` |
| **ISSI** | `IS25LX256` (256Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX256` (256Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `is25lx256` |
| **ISSI** | `IS25LX512` (512Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 1.8V | 200 MHz | `is25lx256` |
| **ISSI** | `IS25WX512` (512Mb)| Octal NOR Flash | Octal SPI / xSPI (8D-8D-8D, DQS) | 3.0V | 133 MHz | `is25lx256` |
| **ISSI** | `IS25LP256` (256Mb)| Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP256` (256Mb)| Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25LP128/064` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 3.0V | 133 MHz | `is25lp256` |
| **ISSI** | `IS25WP128/064` | Quad SPI Flash | Quad SPI (1-4-4, 4-4-4) | 1.8V | 133 MHz | `is25lp256` |
| **ISSI** | `IS66WVO8M8` (64Mb) | Octal PSRAM DDR | Octal SPI DDR / xSPI Profile 2.0 | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO16M8` (128Mb)| Octal PSRAM DDR | Octal SPI DDR / xSPI Profile 2.0 | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO32M8` (256Mb)| Octal PSRAM DDR | Octal SPI DDR / xSPI Profile 2.0 | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVO64M8` (512Mb)| Octal PSRAM DDR | Octal SPI DDR / xSPI Profile 2.0 | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS67WVO...` (Auto) | Octal PSRAM DDR | Octal SPI DDR (Automotive Grade) | 1.8V | 200 MHz | `is66wvo32m8` |
| **ISSI** | `IS66WVH8M8` (64Mb) | HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVH16M8` (128Mb)| HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVH32M8` (256Mb)| HyperRAM™ PSRAM | HyperBus™ (8-bit DDR, RWDS) | 1.8V | 166 MHz | `is66wvh16m8` |
| **ISSI** | `IS66WVS1M8` (8Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 133 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS2M8` (16Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 133 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS4M8` (32Mb) | Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 133 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WVS16M8` (128Mb)| Quad SPI PSRAM | Quad SPI / QPI PSRAM | 1.8V/3.0V | 133 MHz | `is66wvs16m8` |
| **ISSI** | `IS66WV51216` (8Mb) | PSRAM Parallela | 16-bit Parallela Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV102416` (16Mb)| PSRAM Parallela | 16-bit Parallela Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV204816` (32Mb)| PSRAM Parallela | 16-bit Parallela Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
| **ISSI** | `IS66WV409616` (64Mb)| PSRAM Parallela | 16-bit Parallela Asincrona | 3.0V | 55 ns | `is66wv_fmc` |
