# Guida Hardware & Linee Guida PCB per STM32N6
## Interfacciamento Elettrico con Memorie Flash & PSRAM Infineon & ISSI

### 1. Compatibilità Elettrica e Domini di Tensione (VDDIO)

L'STM32N6 dispone di domini di alimentazione I/O indipendenti (`VDDIO1` ... `VDDIO5`):
* **Frequenze Elevate (> 133 MHz fino a 200 MHz)**:
  Tutte le memorie Octal DTR (Infineon SEMPER `S28HS`, ISSI `IS25LX`, ISSI Octal PSRAM `IS66WVO`) e HyperBus (`S27KS`, `S26KS`, `IS66WVH`) ad alte prestazioni richiedono alimentazione a **1.8V nominali**.
  Il rispettivo pin di alimentazione VDDIO dell'STM32N6 deve essere connesso a una sorgente stabile a 1.8V e configurato nel registro PWR:
  ```c
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_1V8);
  ```
* **Frequenze Standard (fino a 133 MHz)**:
  Le varianti a 3.0V / 3.3V (Infineon `S28HL`, `S25HL`, `S26KL`, `S27KL`, ISSI `IS25WX`, `IS25LP`) operano con VDDIO a 3.3V:
  ```c
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_3V3);
  ```

---

### 2. Mappatura Segnali e Pinout STM32N6

#### Interfaccia Octal SPI (8D-8D-8D) & HyperBus™ (Porta XSPI1 / XSPIM P1)
| Segnale Memoria | Funzione | Pin STM32N6 (Tipico BGA / LQFP) | AF Mode |
|---|---|---|---|
| **CLK / CK** | Clock differenziale o single-ended (fino a 200 MHz) | PB1 o PA3 | AF9 (XSPIM_P1_CLK) |
| **CS# / NCS** | Chip Select attivo basso | PB11 o PA2 | AF9 (XSPIM_P1_NCS1) |
| **DQS / RWDS** | Data Strobe bidirezionale (indispensabile per DTR) | PB2 o PD2 | AF9 (XSPIM_P1_DQS0) |
| **IO0 / DQ0** | Dati bit 0 | PC9 o PD7 | AF9 (XSPIM_P1_IO0) |
| **IO1 / DQ1** | Dati bit 1 | PC10 o PD6 | AF9 (XSPIM_P1_IO1) |
| **IO2 / DQ2** | Dati bit 2 | PE2 o PD5 | AF9 (XSPIM_P1_IO2) |
| **IO3 / DQ3** | Dati bit 3 | PD13 o PD4 | AF9 (XSPIM_P1_IO3) |
| **IO4 / DQ4** | Dati bit 4 | PE7 o PD11 | AF9 (XSPIM_P1_IO4) |
| **IO5 / DQ5** | Dati bit 5 | PE8 o PD12 | AF9 (XSPIM_P1_IO5) |
| **IO6 / DQ6** | Dati bit 6 | PE9 o PD14 | AF9 (XSPIM_P1_IO6) |
| **IO7 / DQ7** | Dati bit 7 | PE10 o PD15 | AF9 (XSPIM_P1_IO7) |
| **RESET#** | Reset hardware attivo basso | GPIO standard (es. PC13 o PB5) | Output push-pull |

---

### 3. Regole di Sbroglio PCB (Layout ad Alta Frequenza)

Quando si opera a 200 MHz DDR (frequenza di campionamento 400 MHz equivalente):
1. **Controllo di Impedenza**:
   - Piste a impedenza controllata a **50 Ω** single-ended (± 10%).
2. **Length Matching (Skew Matching)**:
   - Le tracce `IO[0..7]` e `DQS` devono essere equalizzate in lunghezza rispetto al segnale `CLK` con una tolleranza massima di **± 1.0 mm (± 40 mil)**.
   - `DQS` deve essere trattato con la stessa priorità e geometria del clock.
3. **Resistenze di Terminazione Serie**:
   - Inserire resistenze di terminazione serie (damping resistor) da **22 Ω a 33 Ω** vicino ai pin di uscita del microcontrollore per ridurre overshoot e ringing.
4. **Piano di Riferimento di Massa (GND)**:
   - Tutte le piste del bus XSPI devono correre ininterrotte sopra un piano di massa continuo (Solid GND Reference Plane). Evitare assolutamente cambi di piano attraverso split o intagli di massa.
5. **Condensatori di Disaccoppiamento**:
   - Posizionare condensatori ceramici da 100 nF (0402) e 1 µF a bassissima ESR il più vicino possibile ai pin VDD/VDDQ di ogni memoria esterna.
