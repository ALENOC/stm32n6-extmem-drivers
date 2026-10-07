# STM32N6 External Memory Driver Suite
### Suite di Driver C per Memorie Flash & PSRAM Esterne Infineon e ISSI

[![CI Test Suite](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml/badge.svg)](https://github.com/ALENOC/stm32n6-extmem-drivers/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-STM32N6%20(Cortex--M55)-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32n6-series.html)
[![Standard](https://img.shields.io/badge/C%20Standard-C11%20%2F%20Cube--BSP-green.svg)](https://www.st.com)

Una suite di driver C modulare, ad alte prestazioni e conforme agli standard architetturali **STM32Cube BSP** di STMicroelectronics, progettata per interfacciare il microcontrollore **STM32N6** (core ARM Cortex-M55 + NPU Neural-ART) con l'intera gamma di memorie esterne ad alta velocità di **Infineon Technologies** e **Integrated Silicon Solution, Inc. (ISSI)**.

Supporta interfacce **XSPI1**, **XSPI2**, **XSPI3** (Single, Quad, Octal DTR fino a 200 MHz, HyperBus™) e **FMC** (bus parallelo asincrono a 16-bit).

---

## 📋 Indice dei Contenuti
- [Caratteristiche Principali](#-caratteristiche-principali)
- [Matrice dei Dispositivi Supportati](#-matrice-dei-dispositivi-supportati)
- [Struttura del Repository](#-struttura-del-repository)
- [Integrazione Rapida in STM32CubeIDE](#-integrazione-rapida-in-stm32cubeide)
- [Esempi di Codice](#-esempi-di-codice)
  - [Inizializzazione con Auto-Discovery](#1-inizializzazione-con-auto-discovery)
  - [Esecuzione Diretta in Place (XIP Memory-Mapped)](#2-abilitazione-xip-memory-mapped)
  - [Scrittura e Lettura PSRAM ad Alto Throughput](#3-scrittura-e-lettura-psram)
- [Suite di Test & Simulazione Host](#-suite-di-test--simulazione-host)
- [Documentazione Dettagliata](#-documentazione-dettagliata)
- [Disclaimer & Limitazione di Responsabilità](#-disclaimer--limitazione-di-responsabilit)
- [Licenza](#-licenza)

---

## 🚀 Caratteristiche Principali

- **Copertura Completa Infineon Technologies**:
  - **SEMPER™ Octal NOR Flash** (`S28HS512T`, `S28HL512T`, `S28HS256T`, `S28HL256T`, `S28HS01GT`, `S28HS02GT`): protocollo xSPI Profile 1.0 (8D-8D-8D DDR fino a 200 MHz / 400 MB/s).
  - **HyperFlash™** (`S26KS512S`, `S26KL512S`, `S26KS256S`, `S26KL256S`, `S26KS128S`, `S26KL128S`): interfaccia nativa Cypress HyperBus™ a 1.8V / 3.0V.
  - **HyperRAM™** (`S27KS0641`, `S27KL0641`, `S27KS128`, `S27KL128`, `S27KS256`, `S27KL256`, `S27KS512`, serie `S27HS/HL` 2.0 e 3.0).
  - **SEMPER™ / FL Quad SPI Flash** (`S25HL512T`, `S25HS512T`, `S25FL256L`, `S25FL128L`, `S25FL512S`).
- **Copertura Completa ISSI (Integrated Silicon Solution Inc.)**:
  - **Octal NOR Flash** (`IS25LX064`, `IS25WX064`, `IS25LX128`, `IS25WX128`, `IS25LX256`, `IS25WX256`, `IS25LX512`, `IS25WX512`): modalità DTR xSPI Profile 1.0/2.0.
  - **Quad SPI NOR Flash** (`IS25LP064/128/256/512`, `IS25WP064/128/256/512`).
  - **Octal PSRAM xSPI Profile 2.0** (`IS66WVO8M8`, `IS66WVO16M8`, `IS66WVO32M8`, `IS66WVO64M8` e serie automotive `IS67WVO`).
  - **HyperRAM™ PSRAM** (`IS66WVH8M8`, `IS66WVH16M8`, `IS66WVH32M8` e serie automotive `IS67WVH`).
  - **Quad SPI PSRAM** (`IS66WVS1M8`, `IS66WVS2M8`, `IS66WVS4M8`, `IS66WVS16M8`).
  - **PSRAM/SRAM Parallele su FMC** (`IS66WV / IS67WV 51216 / 102416` a 16-bit).
- **Auto-Discovery e Riconoscimento Automatico**:
  - Parser integrato JEDEC JESD216 SFDP (Serial Flash Discoverable Parameters) per configurazione dinamica di comandi, dummy cycle e settori.
  - Interrogazione JEDEC ID (0x9F) e registri HyperBus (ID0, ID1) con database di ricerca chip.
- **Supporto XIP (Execute-in-Place)**:
  - Mappatura istantanea in spazio indirizzi CPU a `0x90000000` (XSPI1), `0x70000000` (XSPI2), `0x60000000` (FMC).
- **Gestione Cache Cortex-M55**:
  - Routine di pulizia e invalidazione D-Cache (`SCB_CleanInvalidateDCache_by_Addr`) conformi ad ARMv8.1-M per garantire coerenza dei dati con DMA e memory-mapped I/O.

---

## 📊 Matrice dei Dispositivi Supportati

| Produttore | Famiglia Memoria | Interfaccia Bus | Clock Max | Modalità Principale | Driver Componente |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Infineon** | SEMPER™ Octal NOR (`S28HS/S28HL`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `s28hs512t` |
| **Infineon** | HyperFlash™ (`S26KS/S26KL`) | XSPI1 / XSPI2 | 166 MHz | HyperBus™ | `s26ks512s` |
| **Infineon** | HyperRAM™ (`S27KS/S27KL/S27HS/S27HL`) | XSPI1 / XSPI2 | 200 MHz | HyperBus™ DDR | `s27ks0641` |
| **Infineon** | SEMPER™ / FL Quad (`S25HL/S25HS/S25FL`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `s25hl512t` |
| **ISSI** | Octal NOR Flash (`IS25LX/IS25WX`) | XSPI1 / XSPI2 | 200 MHz | 8D-8D-8D (Octal DTR) | `is25lx256` |
| **ISSI** | Quad NOR Flash (`IS25LP/IS25WP`) | XSPI1 / XSPI2 / XSPI3 | 133 MHz | 1-4-4 Quad SPI | `is25lp256` |
| **ISSI** | Octal PSRAM (`IS66WVO/IS67WVO`) | XSPI1 / XSPI2 | 200 MHz | xSPI Profile 2.0 (8D-8D-8D) | `is66wvo32m8` |
| **ISSI** | HyperRAM™ PSRAM (`IS66WVH/IS67WVH`) | XSPI1 / XSPI2 | 200 MHz | HyperBus™ DDR | `is66wvh16m8` |
| **ISSI** | Quad SPI PSRAM (`IS66WVS/IS67WVS`) | XSPI1 / XSPI2 / XSPI3 | 104 MHz | 1-4-4 Quad SPI | `is66wvs16m8` |
| **ISSI/IFX** | Parallel Asynch PSRAM (`IS66WV/CY62`) | FMC (16-bit) | Asincrono (~10-55ns) | 16-bit Parallel SRAM/PSRAM | `is66wv_fmc` |

*Per la matrice esaustiva con part number specifici, codici d'ordine e package, consultare [Docs/Supported_Memories_Matrix.md](Docs/Supported_Memories_Matrix.md).*

---

## 📁 Struttura del Repository

```text
stm32n6-extmem-drivers/
├── Drivers/
│   └── BSP/
│       ├── Components/
│       │   ├── Common/              # extmem_common.h, sfdp.h, sfdp.c
│       │   ├── s28hs512t/           # Driver Infineon SEMPER Octal Flash
│       │   ├── s26ks512s/           # Driver Infineon HyperFlash
│       │   ├── s27ks0641/           # Driver Infineon HyperRAM
│       │   ├── s25hl512t/           # Driver Infineon Quad SPI Flash
│       │   ├── is25lx256/           # Driver ISSI Octal Flash
│       │   ├── is25lp256/           # Driver ISSI Quad SPI Flash
│       │   ├── is66wvo32m8/         # Driver ISSI Octal PSRAM
│       │   ├── is66wvh16m8/         # Driver ISSI HyperRAM PSRAM
│       │   ├── is66wvs16m8/         # Driver ISSI Quad PSRAM
│       │   └── is66wv_fmc/          # Driver PSRAM Parallela per FMC
│       └── STM32N6_ExtMem/          # Driver Manager di Alto Livello
│           ├── stm32n6_extmem.h
│           ├── stm32n6_extmem.c
│           ├── stm32n6_extmem_conf.h
│           ├── stm32n6_extmem_conf_template.h
│           └── stm32n6_extmem_devices.h
├── Examples/
│   ├── extmem_demo.c                # Diagnostica hardware, autotest ed esecuzione XIP
│   └── extmem_benchmark.c           # Benchmark di lettura/scrittura in MB/s
├── Tests/
│   ├── mock_hal.h                   # Mock dell'HAL STM32N6 (XSPI, FMC, Cache, RCC)
│   ├── mock_hal.c                   # Simulatore di registri e array di memoria
│   ├── extmem_unit_tests.h          # Dichiarazioni dei test di unità
│   ├── extmem_unit_tests.c          # 12 test di unità esaustivi
│   └── main_test.c                  # Test runner per host Linux/macOS/Windows
├── Docs/
│   ├── STM32CubeIDE_Integration_Guide.md # Guida passo-passo integrazione CubeIDE
│   ├── Supported_Memories_Matrix.md      # Matrice comparativa dettagliata
│   └── Hardware_Design_and_Pinout.md     # Pinout STM32N6, layout PCB a 200MHz, VDDIO
├── .github/
│   └── workflows/
│       └── ci.yml                   # Continuous Integration con GitHub Actions
├── Makefile                         # Compilazione ed esecuzione test suite
├── LICENSE                          # Licenza MIT e Disclaimer di responsabilità
└── README.md
```

---

## 🛠 Integrazione Rapida in STM32CubeIDE

1. **Copia delle Cartelle**:
   Copiare `Drivers/BSP/` nel proprio progetto STM32CubeIDE sotto `Drivers/BSP/`.
2. **Include Paths**:
   Aggiungere in *Project Properties -> C/C++ Build -> Settings -> Tool Settings -> MCU GCC Compiler -> Include paths*:
   - `../Drivers/BSP/STM32N6_ExtMem`
   - `../Drivers/BSP/Components/Common`
   - `../Drivers/BSP/Components/s28hs512t` (e le cartelle dei componenti desiderati)
3. **Configurazione dei Parametri**:
   Copiare `stm32n6_extmem_conf_template.h` come `stm32n6_extmem_conf.h` e impostare le frequenze desiderate.
4. **Guida Completa con Linker Script ed MPU**:
   Consultare [Docs/STM32CubeIDE_Integration_Guide.md](Docs/STM32CubeIDE_Integration_Guide.md) per l'allocazione delle sezioni `.extmem_text` (codice XIP) e `.extmem_ram` (framebuffer / buffer NPU).

---

## 💻 Esempi di Codice

### 1. Inizializzazione con Auto-Discovery
```c
#include "stm32n6_extmem.h"

ExtMem_HandleTypeDef hextmem = {0};

void Memory_Setup(void)
{
  /* Configurazione porta XSPI1, prescaler 2 (200 MHz), alimentazione VDDIO a 1.8V */
  hextmem.Config.Bus            = EXTMEM_BUS_XSPI1;
  hextmem.Config.ClockPrescaler = 2;
  hextmem.Config.Force1V8       = true;

  /* Riconoscimento automatico tramite SFDP / JEDEC ID / HyperBus ID */
  if (ExtMem_Init(&hextmem) == EXTMEM_OK)
  {
    printf("Rilevato chip: %s (%ld MB)\r\n", 
           ExtMem_GetDeviceName(&hextmem), 
           hextmem.Geometry.TotalSizeBytes / (1024 * 1024));
  }
}
```

### 2. Abilitazione XIP (Memory-Mapped)
```c
/* Passaggio in modalità XIP a 0x90000000 */
if (ExtMem_EnableMemoryMapped(&hextmem) == EXTMEM_OK)
{
  /* Accesso diretto da CPU senza transazioni software */
  const uint32_t *pExternalCode = (const uint32_t *)hextmem.MemoryMappedBase;
  printf("Primo vettore: 0x%08lX\r\n", pExternalCode[0]);
}
```

### 3. Scrittura e Lettura PSRAM
```c
uint8_t txData[1024];
uint8_t rxData[1024];

/* Inizializza buffer con dati di test */
memset(txData, 0xA5, sizeof(txData));

/* Scrittura a 200 MHz DDR (Octal o HyperRAM) */
ExtMem_Write(&hextmem, 0x00000000, txData, sizeof(txData));

/* Lettura */
ExtMem_Read(&hextmem, 0x00000000, rxData, sizeof(rxData));
```

---

## 🧪 Suite di Test & Simulazione Host

Il progetto include un harness di test automatico con un **Mock Hardware Abstraction Layer** (`Tests/mock_hal.c`) che emula integralmente il comportamento dei controller STM32N6 XSPI/FMC e delle memorie Infineon/ISSI su qualsiasi macchina di sviluppo (Linux, macOS, Windows) senza necessità di avere l'hardware fisico collegato.

### Esecuzione Locale dei Test
```bash
# Compilazione ed esecuzione dei 12 test di unità
make test
```

### Output Atteso
```text
====================================================================
  STM32N6 External Memory Driver Suite - Host Unit Tests
  Target Architecture: STM32N6 (ARM Cortex-M55 + NPU)
  Supported Peripherals: XSPI1, XSPI2, XSPI3, FMC
====================================================================

[INFO] Running 12 unit tests...

[ RUN      ] [ 1/12] SFDP Discovery Parser (JEDEC JESD216)
[       OK ] [ 1/12] SFDP Discovery Parser (JEDEC JESD216)
[ RUN      ] [ 2/12] Infineon SEMPER Octal NOR Flash (S28HS512T)
[       OK ] [ 2/12] Infineon SEMPER Octal NOR Flash (S28HS512T)
[ RUN      ] [ 3/12] Infineon HyperFlash NOR Flash (S26KS512S)
[       OK ] [ 3/12] Infineon HyperFlash NOR Flash (S26KS512S)
[ RUN      ] [ 4/12] Infineon HyperRAM PSRAM (S27KS0641)
[       OK ] [ 4/12] Infineon HyperRAM PSRAM (S27KS0641)
[ RUN      ] [ 5/12] Infineon SEMPER/FL Quad NOR Flash (S25HL512T)
[       OK ] [ 5/12] Infineon SEMPER/FL Quad NOR Flash (S25HL512T)
[ RUN      ] [ 6/12] ISSI Octal NOR Flash (IS25LX256)
[       OK ] [ 6/12] ISSI Octal NOR Flash (IS25LX256)
[ RUN      ] [ 7/12] ISSI Quad NOR Flash (IS25LP256)
[       OK ] [ 7/12] ISSI Quad NOR Flash (IS25LP256)
[ RUN      ] [ 8/12] ISSI Octal PSRAM xSPI Profile 2.0 (IS66WVO32M8)
[       OK ] [ 8/12] ISSI Octal PSRAM xSPI Profile 2.0 (IS66WVO32M8)
[ RUN      ] [ 9/12] ISSI HyperRAM PSRAM (IS66WVH16M8)
[       OK ] [ 9/12] ISSI HyperRAM PSRAM (IS66WVH16M8)
[ RUN      ] [10/12] ISSI Quad SPI PSRAM (IS66WVS16M8)
[       OK ] [10/12] ISSI Quad SPI PSRAM (IS66WVS16M8)
[ RUN      ] [11/12] ISSI FMC 16-bit Parallel PSRAM (IS66WV51216)
[       OK ] [11/12] ISSI FMC 16-bit Parallel PSRAM (IS66WV51216)
[ RUN      ] [12/12] STM32N6 ExtMem Unified Manager & Auto-Detect
[       OK ] [12/12] STM32N6 ExtMem Unified Manager & Auto-Detect

====================================================================
Test Results Summary:
  Total:   12
  Passed:  12
  Failed:  0
====================================================================
>>> ALL TESTS PASSED SUCCESSFULLY! <<<
```

---

## 📚 Documentazione Dettagliata

- [Guida all'Integrazione STM32CubeIDE](Docs/STM32CubeIDE_Integration_Guide.md)
- [Matrice di Compatibilità delle Memorie](Docs/Supported_Memories_Matrix.md)
- [Linee Guida di Design Hardware & Routing PCB](Docs/Hardware_Design_and_Pinout.md)

---

## ⚠️ Disclaimer & Limitazione di Responsabilità

> [!CAUTION]
> **PROGETTO COMMUNITY A SCOPO DIDATTICO ED ESEMPLIFICATIVO**
> 
> Questo repository, il codice sorgente, i driver e la documentazione sono distribuiti **esclusivamente a scopo dimostrativo, didattico e di riferimento aperto per la community degli sviluppatori embedded**.
> 
> 1. **Assenza di Garanzia**: Il software viene fornito "COSÌ COM'È" (*AS-IS*), senza garanzie di alcun tipo, esplicite o implicite, incluse, a titolo esemplificativo, garanzie di commerciabilità, idoneità per uno scopo specifico o non violazione.
> 2. **Esclusione Totale di Responsabilità**: In nessun caso l'autore ([ALENOC](https://github.com/ALENOC)), i manutentori o i contributori potranno essere ritenuti responsabili per qualsivoglia danno diretto, indiretto, incidentale, speciale, punitivo o consequenziale (inclusi, senza limitazione, danneggiamento di microcontrollori STM32N6, memorie esterne, schede PCB, perdita di dati, interruzione dell'attività economica o malfunzionamenti hardware/software), derivante dall'uso o dall'impossibilità d'uso di questo codice.
> 3. **Verifica Hardware Mandataria**: È esclusiva responsabilità dell'utente o dell'integratore verificare la compatibilità elettrica (tensioni 1.8V vs 3.3V, livelli logici I/O, domini VDDIO), le temporizzazioni di setup/hold a 200 MHz, i datasheets ufficiali di STMicroelectronics, Infineon Technologies e ISSI, nonché effettuare tutte le necessarie validazioni e test di sicurezza prima di impiegare questo codice in qualsiasi prototipo o prodotto.

---

## 📄 Licenza

Rilasciato sotto licenza open-source [MIT License](LICENSE).
Copyright (c) 2026 Alessandro Nocivelli (ALENOC) & Community Contributors.
