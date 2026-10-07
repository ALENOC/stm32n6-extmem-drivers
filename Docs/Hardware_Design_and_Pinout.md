# Hardware Design & High-Speed PCB Layout Guide for STM32N6
## Electrical Interfacing with Infineon, ISSI & Micron Flash and PSRAM Memories

### 1. Electrical Compatibility and Voltage Domains (VDDIO)

The STM32N6 microcontroller features multiple independent I/O power domains (`VDDIO1` through `VDDIO5`):
* **High-Frequency Operation (> 133 MHz up to 200 MHz)**:
  All high-speed Octal DTR memories (Infineon SEMPER `S28HS`, ISSI `IS25LX`, ISSI Octal PSRAM `IS66WVO`, Micron Xccela `MT35XU`) and HyperBus devices (Infineon `S27KS`/`S26KS`, ISSI `IS66WVH`/`IS26KS`) require a **nominal 1.8V supply rail**.
  The respective VDDIO power pin of the STM32N6 must be connected to a clean, stable 1.8V power source and configured in the PWR controller register:
  ```c
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_1V8);
  ```
* **Standard-Frequency Operation (up to 133 MHz)**:
  The 3.0V / 3.3V variants (Infineon `S28HL`, `S25HL`, `S26KL`, `S27KL`, ISSI `IS25WX`, `IS25LP`, Micron `MT35XL`, `MT25QL`) operate with VDDIO set to 3.3V:
  ```c
  HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_3V3);
  ```

---

### 2. STM32N6 Signal Mapping and Pinout

#### Octal SPI (8D-8D-8D) & HyperBus™ Interface (XSPI1 / XSPIM Port P1)
| Memory Signal | Function | STM32N6 Pin (Typical BGA / LQFP) | AF Mode |
|:---|:---|:---|:---|
| **CLK / CK** | Differential or single-ended clock (up to 200 MHz) | PB1 or PA3 | AF9 (XSPIM_P1_CLK) |
| **CS# / NCS** | Active-low chip select | PB11 or PA2 | AF9 (XSPIM_P1_NCS1) |
| **DQS / RWDS** | Bidirectional Data Strobe (mandatory for DTR) | PB2 or PD2 | AF9 (XSPIM_P1_DQS0) |
| **IO0 / DQ0** | Data bit 0 | PC9 or PD7 | AF9 (XSPIM_P1_IO0) |
| **IO1 / DQ1** | Data bit 1 | PC10 or PD6 | AF9 (XSPIM_P1_IO1) |
| **IO2 / DQ2** | Data bit 2 | PE2 or PD5 | AF9 (XSPIM_P1_IO2) |
| **IO3 / DQ3** | Data bit 3 | PD13 or PD4 | AF9 (XSPIM_P1_IO3) |
| **IO4 / DQ4** | Data bit 4 | PE7 or PD11 | AF9 (XSPIM_P1_IO4) |
| **IO5 / DQ5** | Data bit 5 | PE8 or PD12 | AF9 (XSPIM_P1_IO5) |
| **IO6 / DQ6** | Data bit 6 | PE9 or PD14 | AF9 (XSPIM_P1_IO6) |
| **IO7 / DQ7** | Data bit 7 | PE10 or PD15 | AF9 (XSPIM_P1_IO7) |
| **RESET#** | Hardware active-low reset | Standard GPIO (e.g., PC13 or PB5) | Output push-pull |

---

### 3. High-Speed PCB Layout Guidelines (200 MHz DDR Routing)

When operating at 200 MHz DDR (equivalent to 400 MSamples/s edge rate):
1. **Characteristic Impedance Control**:
   - Route all signal lines with a target single-ended characteristic impedance of **50 Ω** (± 10%).
2. **Length Matching (Skew Matching)**:
   - Data traces `IO[0..7]` and `DQS` must be tightly length-matched to the `CLK` trace with a maximum skew tolerance of **± 1.0 mm (± 40 mils)**.
   - The `DQS` strobe must be routed with the same layer topology, trace width, and priority as the clock line.
3. **Series Termination Resistors**:
   - Place series damping resistors (**22 Ω to 33 Ω**) near the microcontroller output pins to eliminate signal reflections, overshoot, and ringing.
4. **Solid Ground Reference Plane**:
   - All high-speed XSPI bus traces must run uninterrupted over a continuous, solid GND plane. Never route XSPI high-speed traces across plane splits or voids.
5. **Decoupling Capacitors**:
   - Place ultra-low ESR 100 nF (0402 ceramic) and 1 µF decoupling capacitors as close as possible to the VDD/VDDQ pins of each external memory device.

