# Hardware Design Notes for STM32N6 External Memories
## Voltage domains, XSPIM pin mapping and layout considerations

These notes cover what the drivers assume about the hardware. Pin assignments come from the ST
STM32N6570-DK board support package (`stm32n6570_discovery_xspi.h`); voltage domains and alternate
functions from the STM32CubeN6 HAL. The layout section is general high-speed practice and has not been
validated on a board built for this project. Always check the STM32N6 datasheet, the reference manual
(RM0486), ST's hardware design guidance for the STM32N6 and the memory datasheet before a layout.

---

### 1. I/O voltage domains

The STM32N6 has a main `VDDIO` supply and separate domains `VDDIO2` to `VDDIO5`
(`PWR_VDDIO2` ... `PWR_VDDIO5` in the HAL). The XSPI I/O manager ports used by the drivers are powered as
follows (STM32N6570-DK BSP):

| XSPIM port | GPIO ports | Supply domain |
|:---|:---|:---|
| Port 1 | PO, PP | `VDDIO2` |
| Port 2 | PN | `VDDIO3` |

`ExtMem_Init()` enables the domain of the selected port and sets its range with
`HAL_PWREx_ConfigVddIORange()`: 1.8 V when `Config.Force1V8` is set, 3.3 V otherwise. The range must match
the voltage actually supplied to that domain on the board and the I/O voltage of the memory.

The memory voltage is a property of each part (for example S28HS = 1.8 V, S28HL = 3.0 V; IS25WX = 1.8 V,
IS25LX = 3.0 V; MT35XU = 1.8 V, MT35XL = 3.0 V). The maximum clock also depends on the part and its
voltage, not on the domain alone: see the per-part limits in
[Supported_Memories_Matrix.md](Supported_Memories_Matrix.md).

> [!CAUTION]
> Driving a 1.8 V memory from a domain supplied at 3.3 V can damage it. Check the board supply, not only the
> software setting.

---

### 2. XSPIM pin mapping (STM32N6570-DK reference)

All XSPI signals use alternate function **AF9** (`GPIO_AF9_XSPIM_P1` / `GPIO_AF9_XSPIM_P2`), very high
speed. Port 1 supports up to 16 data lines; the drivers of this suite use at most 8.

| Signal | Port 1 (DK: 16-bit PSRAM on XSPI1) | Port 2 (DK: octal NOR on XSPI2) |
|:---|:---|:---|
| CLK | PO4 | PN6 |
| NCS | PO0 | PN1 |
| DQS / RWDS | PO2 (DQS0), PO3 (DQS1) | PN0 |
| IO0 to IO3 | PP0 to PP3 | PN2 to PN5 |
| IO4 to IO7 | PP4 to PP7 | PN8 to PN11 |
| IO8 to IO15 | PP8 to PP15 | not available |

The pin muxing itself belongs to `HAL_XSPI_MspInit()` of the application (CubeMX generated);
`ExtMem_Init()` only routes the XSPI instance to the port through the XSPI I/O manager. A memory hardware
reset pin, where present, is a board-specific GPIO.

> [!NOTE]
> The STM32N6570-DK memories (Macronix MX66UW1G45G NOR and AP Memory APS256XX PSRAM) are not among the
> parts supported by this suite. The table is a pin reference for boards that reuse the same ports.

---

### 3. Layout considerations for DTR operation up to 200 MHz

The points below are common practice for 8-line DDR buses with a data strobe. They are starting points, not
validated values: final impedance, length matching and termination come from the memory vendor's layout
guidance, the board stack-up and signal integrity simulation or measurement.

1. **Impedance**: controlled single-ended impedance for CLK, NCS, DQS and IO lines (50 ohm is a typical
   target; follow the memory and stack-up requirements).
2. **Length matching**: match IO0 to IO7 and DQS to each other and to CLK; DQS shares the data timing and
   should follow the same layers and topology as the data lines.
3. **Termination**: series resistors close to the driver are commonly used to damp reflections; the value
   depends on the driver strength settings and trace impedance.
4. **Reference plane**: route the bus over a continuous ground plane, without crossing plane splits.
5. **Decoupling**: place the decoupling capacitors recommended by the memory datasheet close to its VCC /
   VCCQ pins.
6. **Timing margins at full speed**: 166 to 200 MHz DTR usually needs XSPI delay-block and sample-shift
   tuning on the actual board. The drivers do not tune them; this is listed as open work in
   [Project_Context.md](Project_Context.md).
