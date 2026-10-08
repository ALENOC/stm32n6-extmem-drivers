/**
  ******************************************************************************
  * @file    mock_hal.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Mock Hardware Abstraction Layer for host unit testing.
  *
  *          Constant names and the values that carry meaning (DEVSIZE codes,
  *          chip select boundaries, FMC banks, memory types) follow the
  *          STM32CubeN6 HAL headers (stm32n6xx_hal_xspi.h, stm32n6xx_ll_fmc.h).
  *          Every HAL call is recorded in an event log so tests can check the
  *          exact protocol phases sent to the memory, and any call can be made
  *          to fail through fault injection.
  ******************************************************************************
  */

#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* HAL Status Type */
typedef enum {
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

#define HAL_XSPI_TIMEOUT_DEFAULT_VALUE  5000U

/* XSPI Operation Type */
#define HAL_XSPI_OPTYPE_COMMON_CFG       0x00U
#define HAL_XSPI_OPTYPE_READ_CFG         0x01U
#define HAL_XSPI_OPTYPE_WRITE_CFG        0x02U
#define HAL_XSPI_OPTYPE_WRAP_CFG         0x03U

/* Instruction phase (value = number of lines) */
#define HAL_XSPI_INSTRUCTION_NONE        0x00U
#define HAL_XSPI_INSTRUCTION_1_LINE      0x01U
#define HAL_XSPI_INSTRUCTION_2_LINES     0x02U
#define HAL_XSPI_INSTRUCTION_4_LINES     0x04U
#define HAL_XSPI_INSTRUCTION_8_LINES     0x08U

#define HAL_XSPI_INSTRUCTION_8_BITS      0x00U
#define HAL_XSPI_INSTRUCTION_16_BITS     0x01U
#define HAL_XSPI_INSTRUCTION_24_BITS     0x02U
#define HAL_XSPI_INSTRUCTION_32_BITS     0x03U

#define HAL_XSPI_INSTRUCTION_DTR_DISABLE 0x00U
#define HAL_XSPI_INSTRUCTION_DTR_ENABLE  0x01U

/* Address phase (value = number of lines) */
#define HAL_XSPI_ADDRESS_NONE            0x00U
#define HAL_XSPI_ADDRESS_1_LINE          0x01U
#define HAL_XSPI_ADDRESS_2_LINES         0x02U
#define HAL_XSPI_ADDRESS_4_LINES         0x04U
#define HAL_XSPI_ADDRESS_8_LINES         0x08U

#define HAL_XSPI_ADDRESS_8_BITS          0x00U
#define HAL_XSPI_ADDRESS_16_BITS         0x01U
#define HAL_XSPI_ADDRESS_24_BITS         0x02U
#define HAL_XSPI_ADDRESS_32_BITS         0x03U

#define HAL_XSPI_ADDRESS_DTR_DISABLE     0x00U
#define HAL_XSPI_ADDRESS_DTR_ENABLE      0x01U

/* Alternate bytes phase (value = number of lines) */
#define HAL_XSPI_ALT_BYTES_NONE          0x00U
#define HAL_XSPI_ALT_BYTES_1_LINE        0x01U
#define HAL_XSPI_ALT_BYTES_2_LINES       0x02U
#define HAL_XSPI_ALT_BYTES_4_LINES       0x04U
#define HAL_XSPI_ALT_BYTES_8_LINES       0x08U

#define HAL_XSPI_ALT_BYTES_8_BITS        0x00U
#define HAL_XSPI_ALT_BYTES_16_BITS       0x01U
#define HAL_XSPI_ALT_BYTES_24_BITS       0x02U
#define HAL_XSPI_ALT_BYTES_32_BITS       0x03U

#define HAL_XSPI_ALT_BYTES_DTR_DISABLE   0x00U
#define HAL_XSPI_ALT_BYTES_DTR_ENABLE    0x01U

/* Data phase (value = number of lines) */
#define HAL_XSPI_DATA_NONE               0x00U
#define HAL_XSPI_DATA_1_LINE             0x01U
#define HAL_XSPI_DATA_2_LINES            0x02U
#define HAL_XSPI_DATA_4_LINES            0x04U
#define HAL_XSPI_DATA_8_LINES            0x08U
#define HAL_XSPI_DATA_16_LINES           0x10U

#define HAL_XSPI_DATA_DTR_DISABLE        0x00U
#define HAL_XSPI_DATA_DTR_ENABLE         0x01U

#define HAL_XSPI_DQS_DISABLE             0x00U
#define HAL_XSPI_DQS_ENABLE              0x01U

#define HAL_XSPI_MATCH_MODE_AND          0x00U
#define HAL_XSPI_MATCH_MODE_OR           0x01U
#define HAL_XSPI_AUTOMATIC_STOP_DISABLE  0x00U
#define HAL_XSPI_AUTOMATIC_STOP_ENABLE   0x01U
#define HAL_XSPI_TIMEOUT_COUNTER_DISABLE 0x00U
#define HAL_XSPI_TIMEOUT_COUNTER_ENABLE  0x01U

/* Memory type (DCR1.MTYP, same encoding as the HAL) */
#define HAL_XSPI_MEMTYPE_MICRON          0x00000000U
#define HAL_XSPI_MEMTYPE_MACRONIX        0x01000000U
#define HAL_XSPI_MEMTYPE_APMEM           0x02000000U
#define HAL_XSPI_MEMTYPE_MACRONIX_RAM    0x03000000U
#define HAL_XSPI_MEMTYPE_HYPERBUS        0x04000000U
#define HAL_XSPI_MEMTYPE_APMEM_16BITS    0x06000000U

#define HAL_XSPI_SINGLE_MEM              0x00U

/* Memory size (DCR1.DEVSIZE). The HAL names the size in BITS: code n means 2^(n+1) bytes. */
#define HAL_XSPI_SIZE_16B                0x00U  /*  16 bits  =   2 Bytes */
#define HAL_XSPI_SIZE_32B                0x01U
#define HAL_XSPI_SIZE_64B                0x02U
#define HAL_XSPI_SIZE_128B               0x03U
#define HAL_XSPI_SIZE_256B               0x04U
#define HAL_XSPI_SIZE_512B               0x05U
#define HAL_XSPI_SIZE_1KB                0x06U
#define HAL_XSPI_SIZE_2KB                0x07U
#define HAL_XSPI_SIZE_4KB                0x08U
#define HAL_XSPI_SIZE_8KB                0x09U
#define HAL_XSPI_SIZE_16KB               0x0AU
#define HAL_XSPI_SIZE_32KB               0x0BU
#define HAL_XSPI_SIZE_64KB               0x0CU
#define HAL_XSPI_SIZE_128KB              0x0DU
#define HAL_XSPI_SIZE_256KB              0x0EU
#define HAL_XSPI_SIZE_512KB              0x0FU  /* 512 Kbits =  64 KBytes */
#define HAL_XSPI_SIZE_1MB                0x10U
#define HAL_XSPI_SIZE_2MB                0x11U
#define HAL_XSPI_SIZE_4MB                0x12U
#define HAL_XSPI_SIZE_8MB                0x13U  /*   8 Mbits =   1 MByte  */
#define HAL_XSPI_SIZE_16MB               0x14U
#define HAL_XSPI_SIZE_32MB               0x15U
#define HAL_XSPI_SIZE_64MB               0x16U  /*  64 Mbits =   8 MBytes */
#define HAL_XSPI_SIZE_128MB              0x17U
#define HAL_XSPI_SIZE_256MB              0x18U
#define HAL_XSPI_SIZE_512MB              0x19U  /* 512 Mbits =  64 MBytes */
#define HAL_XSPI_SIZE_1GB                0x1AU
#define HAL_XSPI_SIZE_2GB                0x1BU
#define HAL_XSPI_SIZE_4GB                0x1CU
#define HAL_XSPI_SIZE_8GB                0x1DU
#define HAL_XSPI_SIZE_16GB               0x1EU
#define HAL_XSPI_SIZE_32GB               0x1FU

/* XSPI Manager (XSPIM) */
#define HAL_XSPIM_IOPORT_1               0x01U
#define HAL_XSPIM_IOPORT_2               0x02U

#define HAL_XSPI_CSSEL_OVR_NCS1          0x00U
#define HAL_XSPI_CSSEL_OVR_NCS2          0x01U
#define HAL_XSPI_CSSEL_OVR_DISABLED      0x02U

#define HAL_XSPI_CSSEL_NCS1              0x00U
#define HAL_XSPI_CLOCK_MODE_0            0x00U
#define HAL_XSPI_SAMPLE_SHIFT_NONE       0x00U
#define HAL_XSPI_DHQC_DISABLE            0x00U
#define HAL_XSPI_DHQC_ENABLE             0x01U

/* Chip select boundary (DCR3.CSBOUND). The HAL names the boundary in BITS: code n means 2^n bytes. */
#define HAL_XSPI_BONDARYOF_NONE          0x00U
#define HAL_XSPI_BONDARYOF_1KB           0x07U  /*  1 Kbits = 128 Bytes */
#define HAL_XSPI_BONDARYOF_2KB           0x08U
#define HAL_XSPI_BONDARYOF_4KB           0x09U
#define HAL_XSPI_BONDARYOF_8KB           0x0AU  /*  8 Kbits =   1 KByte */
#define HAL_XSPI_BONDARYOF_16KB          0x0BU  /* 16 Kbits =   2 KBytes */
#define HAL_XSPI_BONDARYOF_32KB          0x0CU
#define HAL_XSPI_BONDARYOF_2MB           0x12U  /*  2 Mbits = 256 KBytes */
#define HAL_XSPI_BONDARYOF_256MB         0x19U  /* 256 Mbits = 32 MBytes */
#define HAL_XSPI_BONDARYOF_1GB           0x1BU  /*  1 Gbit  = 128 MBytes */

#define HAL_XSPI_FREERUNCLK_DISABLE      0x00U
#define HAL_XSPI_WRAP_NOT_SUPPORTED      0x00U

#define HAL_XSPI_MEMORY_ADDRESS_SPACE    0x00U
#define HAL_XSPI_REGISTER_ADDRESS_SPACE  0x01U

#define HAL_XSPI_VARIABLE_LATENCY        0x00U
#define HAL_XSPI_FIXED_LATENCY           0x01U
#define HAL_XSPI_LATENCY_ON_WRITE        0x00U
#define HAL_XSPI_NO_LATENCY_ON_WRITE     0x01U

/* FMC / SRAM Constants (stm32n6xx_ll_fmc.h encoding) */
#define FMC_NORSRAM_BANK1                0x00000000U
#define FMC_NORSRAM_BANK2                0x00000002U
#define FMC_NORSRAM_BANK3                0x00000004U
#define FMC_NORSRAM_BANK4                0x00000006U
#define FMC_NORSRAM_DEVICE               ((void *)0x60000000)
#define FMC_NORSRAM_EXTENDED_DEVICE      ((void *)0x60000104)
#define FMC_DATA_ADDRESS_MUX_DISABLE     0x00U
#define FMC_MEMORY_TYPE_SRAM             0x00U
#define FMC_MEMORY_TYPE_PSRAM            0x04U
#define FMC_MEMORY_TYPE_NOR              0x08U
#define FMC_NORSRAM_MEM_BUS_WIDTH_16     0x10U
#define FMC_BURST_ACCESS_MODE_DISABLE    0x00U
#define FMC_WAIT_SIGNAL_POLARITY_LOW     0x00U
#define FMC_WAIT_TIMING_BEFORE_WS        0x00U
#define FMC_WRITE_OPERATION_ENABLE       0x01U
#define FMC_WAIT_SIGNAL_DISABLE          0x00U
#define FMC_EXTENDED_MODE_DISABLE        0x00U
#define FMC_ASYNCHRONOUS_WAIT_DISABLE    0x00U
#define FMC_WRITE_BURST_DISABLE          0x00U
#define FMC_CONTINUOUS_CLOCK_SYNC_ONLY   0x00U
#define FMC_PAGE_SIZE_NONE               0x00U
#define FMC_NBL_SETUPTIME_0              0x00U
#define FMC_ACCESS_MODE_A                0x00U
#define FMC_ACCESS_MODE_B                0x01U

#define PWR_VDDIO2                       0x02U
#define PWR_VDDIO3                       0x03U
#define PWR_VDDIO_RANGE_1V8              0x00U
#define PWR_VDDIO_RANGE_3V3              0x01U

#define GPIO_MODE_AF_PP                  0x02U
#define GPIO_NOPULL                      0x00U
#define GPIO_SPEED_FREQ_VERY_HIGH        0x03U
#define GPIO_AF9_XSPIM_P1                0x09U
#define GPIO_AF9_XSPIM_P2                0x09U

#define XSPI1                            ((void *)0x58025000)
#define XSPI2                            ((void *)0x5802A000)
#define XSPI3                            ((void *)0x5802D000)

/* Cortex-M & Cache Mocks */
typedef struct {
  uint32_t CCR;
} SCB_Type;

#define SCB_CCR_DC_Msk                   (1U << 16)
extern SCB_Type *SCB;

void SCB_CleanInvalidateDCache_by_Addr(void *addr, int32_t size);

/* Struct definitions */
typedef struct {
  uint32_t FifoThresholdByte;
  uint32_t MemoryMode;
  uint32_t MemoryType;
  uint32_t MemorySize;
  uint32_t ChipSelectHighTimeCycle;
  uint32_t FreeRunningClock;
  uint32_t ClockMode;
  uint32_t WrapSize;
  uint32_t ClockPrescaler;
  uint32_t SampleShifting;
  uint32_t DelayHoldQuarterCycle;
  uint32_t ChipSelectBoundary;
  uint32_t MaxTran;
  uint32_t Refresh;
  uint32_t MemorySelect;
} XSPI_InitTypeDef;

typedef struct {
  void             *Instance;
  XSPI_InitTypeDef Init;
} XSPI_HandleTypeDef;

typedef struct {
  uint32_t OperationType;
  uint32_t IOSelect;
  uint32_t Instruction;
  uint32_t InstructionMode;
  uint32_t InstructionWidth;
  uint32_t InstructionDTRMode;
  uint32_t Address;
  uint32_t AddressMode;
  uint32_t AddressWidth;
  uint32_t AddressDTRMode;
  uint32_t AlternateBytes;
  uint32_t AlternateBytesMode;
  uint32_t AlternateBytesWidth;
  uint32_t AlternateBytesDTRMode;
  uint32_t DataMode;
  uint32_t DataLength;
  uint32_t DataDTRMode;
  uint32_t DummyCycles;
  uint32_t DQSMode;
} XSPI_RegularCmdTypeDef;

typedef struct {
  uint32_t AddressSpace;
  uint32_t Address;
  uint32_t AddressWidth;
  uint32_t DataLength;
  uint32_t DQSMode;
  uint32_t DataMode;
} XSPI_HyperbusCmdTypeDef;

typedef struct {
  uint32_t RWRecoveryTimeCycle;
  uint32_t AccessTimeCycle;
  uint32_t WriteZeroLatency;
  uint32_t LatencyMode;
} XSPI_HyperbusCfgTypeDef;

typedef struct {
  uint32_t TimeOutActivation;
  uint32_t TimeoutPeriodClock;
} XSPI_MemoryMappedTypeDef;

typedef struct {
  uint32_t MatchValue;
  uint32_t MatchMask;
  uint32_t MatchMode;
  uint32_t AutomaticStop;
  uint32_t IntervalTime;
} XSPI_AutoPollingTypeDef;

typedef struct {
  uint32_t NSBank;
  uint32_t DataAddressMux;
  uint32_t MemoryType;
  uint32_t MemoryDataWidth;
  uint32_t BurstAccessMode;
  uint32_t WaitSignalPolarity;
  uint32_t WaitSignalActive;
  uint32_t WriteOperation;
  uint32_t WaitSignal;
  uint32_t ExtendedMode;
  uint32_t AsynchronousWait;
  uint32_t WriteBurst;
  uint32_t ContinuousClock;
  uint32_t PageSize;
  uint32_t NBLSetupTime;
} SRAM_InitTypeDef;

typedef struct {
  void             *Instance;
  void             *Extended;
  SRAM_InitTypeDef Init;
} SRAM_HandleTypeDef;

typedef struct {
  uint32_t AddressSetupTime;
  uint32_t AddressHoldTime;
  uint32_t DataSetupTime;
  uint32_t DataHoldTime;
  uint32_t BusTurnAroundDuration;
  uint32_t CLKDivision;
  uint32_t DataLatency;
  uint32_t AccessMode;
} FMC_NORSRAM_TimingTypeDef;

typedef struct {
  uint32_t Pin;
  uint32_t Mode;
  uint32_t Pull;
  uint32_t Speed;
  uint32_t Alternate;
} GPIO_InitTypeDef;

typedef struct {
  uint32_t nCSOverride;
  uint32_t IOPort;
  uint32_t Req2AckTime;
} XSPIM_CfgTypeDef;

/* HAL Function Prototypes */
HAL_StatusTypeDef HAL_XSPI_Init(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPI_DeInit(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPIM_Config(XSPI_HandleTypeDef *hxspi, const XSPIM_CfgTypeDef *pCfg, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Command(XSPI_HandleTypeDef *hxspi, const XSPI_RegularCmdTypeDef *pCmd, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Transmit(XSPI_HandleTypeDef *hxspi, const uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Receive(XSPI_HandleTypeDef *hxspi, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Transmit_DMA(XSPI_HandleTypeDef *hxspi, const uint8_t *pData);
HAL_StatusTypeDef HAL_XSPI_Receive_DMA(XSPI_HandleTypeDef *hxspi, uint8_t *pData);
HAL_StatusTypeDef HAL_XSPI_AutoPolling(XSPI_HandleTypeDef *hxspi, const XSPI_AutoPollingTypeDef *pCfg, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_MemoryMapped(XSPI_HandleTypeDef *hxspi, const XSPI_MemoryMappedTypeDef *pCfg);
HAL_StatusTypeDef HAL_XSPI_Abort(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPI_SetClockPrescaler(XSPI_HandleTypeDef *hxspi, uint32_t Prescaler);
HAL_StatusTypeDef HAL_XSPI_HyperbusCfg(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCfgTypeDef *pCfg, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_HyperbusCmd(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCmdTypeDef *pCmd, uint32_t Timeout);

HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, const FMC_NORSRAM_TimingTypeDef *pTiming, const FMC_NORSRAM_TimingTypeDef *pExtTiming);
HAL_StatusTypeDef HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram);

uint32_t HAL_GetTick(void);

/* Stacked Micron parts: the next direct FSR reads report "busy" (another die still working) */
#define MOCK_STACKED_BUSY_FOREVER 0xFFFFFFFFU
void MockHAL_SetStackedBusyReads(uint32_t reads);
uint32_t HAL_RCC_GetHCLKFreq(void);
uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint64_t PeriphClk);

#define RCC_PERIPHCLK_XSPI1              (0x0040000000000000UL)
#define RCC_PERIPHCLK_XSPI2              (0x0080000000000000UL)
#define RCC_PERIPHCLK_XSPI3              (0x0100000000000000UL)
void HAL_Delay(uint32_t Delay);

void HAL_PWREx_EnableVddIO2(void);
void HAL_PWREx_EnableVddIO3(void);
void HAL_PWREx_ConfigVddIORange(uint32_t Domain, uint32_t Range);

#define __HAL_RCC_PWR_CLK_ENABLE()       ((void)0)
#define __HAL_RCC_XSPI1_CLK_ENABLE()     ((void)0)
#define __HAL_RCC_XSPI1_FORCE_RESET()    ((void)0)
#define __HAL_RCC_XSPI1_RELEASE_RESET()  ((void)0)
#define __HAL_RCC_XSPI2_CLK_ENABLE()     ((void)0)
#define __HAL_RCC_XSPI2_FORCE_RESET()    ((void)0)
#define __HAL_RCC_XSPI2_RELEASE_RESET()  ((void)0)
#define __HAL_RCC_XSPI3_CLK_ENABLE()     ((void)0)
#define __HAL_RCC_XSPI3_FORCE_RESET()    ((void)0)
#define __HAL_RCC_XSPI3_RELEASE_RESET()  ((void)0)
#define __HAL_RCC_XSPIM_CLK_ENABLE()     ((void)0)
#define __HAL_RCC_XSPIM_FORCE_RESET()    ((void)0)
#define __HAL_RCC_XSPIM_RELEASE_RESET()  ((void)0)

/* ===================================================================== */
/* Mock control, event log and memory device emulation                   */
/* ===================================================================== */

typedef enum {
  MOCK_EV_XSPI_INIT = 0,
  MOCK_EV_XSPI_DEINIT,
  MOCK_EV_XSPIM_CONFIG,
  MOCK_EV_CMD,
  MOCK_EV_TX,
  MOCK_EV_RX,
  MOCK_EV_TX_DMA,
  MOCK_EV_RX_DMA,
  MOCK_EV_AUTOPOLL,
  MOCK_EV_MEMMAPPED,
  MOCK_EV_ABORT,
  MOCK_EV_HYPER_CFG,
  MOCK_EV_HYPER_CMD,
  MOCK_EV_SRAM_INIT,
  MOCK_EV_SRAM_DEINIT,
  MOCK_EV_FMC_WRITE16,
  MOCK_EV_FMC_READ16,
  MOCK_EV_PWR_VDDIO,
  MOCK_EV_CACHE_MAINT,
  MOCK_EV_SET_PRESCALER
} MockEventType_t;

#define MOCK_EVENT_DATA_BYTES 8U

typedef struct {
  MockEventType_t           Type;
  XSPI_InitTypeDef          Init;      /*!< MOCK_EV_XSPI_INIT                      */
  XSPI_RegularCmdTypeDef    Cmd;       /*!< MOCK_EV_CMD, and the command of TX/RX  */
  XSPI_HyperbusCmdTypeDef   HCmd;      /*!< MOCK_EV_HYPER_CMD, and HyperBus TX/RX  */
  XSPI_HyperbusCfgTypeDef   HCfg;      /*!< MOCK_EV_HYPER_CFG                      */
  XSPI_AutoPollingTypeDef   Poll;      /*!< MOCK_EV_AUTOPOLL                       */
  XSPIM_CfgTypeDef          Xspim;     /*!< MOCK_EV_XSPIM_CONFIG                   */
  SRAM_InitTypeDef          SramInit;  /*!< MOCK_EV_SRAM_INIT                      */
  FMC_NORSRAM_TimingTypeDef Timing;    /*!< MOCK_EV_SRAM_INIT                      */
  bool                      Hyperbus;  /*!< TX/RX belongs to a HyperBus command    */
  uint32_t                  Address;   /*!< FMC / cache maintenance address        */
  uint32_t                  Value;     /*!< FMC word, PWR domain/range             */
  uint32_t                  DataLen;   /*!< Bytes moved by TX/RX                   */
  uint8_t                   Data[MOCK_EVENT_DATA_BYTES]; /*!< First bytes moved    */
} MockEvent_t;

/* Mock lifecycle */
void MockHAL_Reset(void);

/* Emulated serial device identity and behaviour */
void MockHAL_SetEmulatedChip(uint8_t mfg, uint8_t memType, uint8_t density);
void MockHAL_SetHyperBusID(uint16_t id0, uint16_t id1);
void MockHAL_SetHyperCR0(uint16_t cr0);                      /*!< HyperRAM CR0 (sets the device latency)  */
void MockHAL_SetFlashSemantics(bool norFlash);              /*!< true: program ANDs bits, erase needed  */
void MockHAL_SetBlockEraseSize(uint32_t bytes);              /*!< Size erased by D8h/DCh (default 64 KB) */
void MockHAL_SetHyperFlashMode(bool enable);                 /*!< Decode HyperFlash command cycles       */
void MockHAL_SetSfdpTable(const uint8_t *pTable, uint32_t size); /*!< NULL restores the default table   */
void MockHAL_SetStatusRegister(uint8_t sr1);
void MockHAL_SetSemperFailure(bool fail);
void MockHAL_SetFlLFailure(bool fail);                       /*!< Next program/erase fails: SR2 P_ERR, busy until CLSR */
uint8_t MockHAL_GetStatusRegister2(void);
void MockHAL_SetIssiReadRegister(bool supported);            /*!< ISSI SRPV/RDRP Read Register present    */
uint8_t MockHAL_GetIssiReadParams(void);                    /*!< Next program/erase fails: PRGERR + busy until CLPEF */
void MockHAL_SetFlagStatusRegister(uint8_t fsr);
void MockHAL_SetPollTimeout(bool timeout);                   /*!< AutoPolling returns HAL_TIMEOUT        */
void MockHAL_SetHyperFlashStatus(uint16_t status);           /*!< Value returned after a 0x70 command    */
void MockHAL_SetFmcNorStuck(bool busy, bool dq5);            /*!< FMC NOR never completes / reports DQ5  */
void MockHAL_SetFmcNorBusyReads(uint32_t reads);             /*!< FMC NOR completes after N status reads */
void MockHAL_SetHclkFreq(uint32_t hz);                       /*!< Value returned by HAL_RCC_GetHCLKFreq   */
void MockHAL_SetXspiKernelClock(uint32_t hz);                /*!< Value returned for RCC_PERIPHCLK_XSPIx  */
void MockHAL_SetOctalRamId(uint16_t id);                     /*!< ISSI OctalRAM ID register              */
void MockHAL_SetOctalRamCrLocked(bool locked);               /*!< OctalRAM ignores CR writes             */
uint16_t MockHAL_GetOctalRamCR(void);                        /*!< ISSI OctalRAM configuration register   */

/* Emulated register views */
uint8_t  MockHAL_GetStatusRegister(void);
uint8_t  MockHAL_GetConfigRegister1(void);
uint8_t  MockHAL_GetAnyReg(uint32_t addr);                   /*!< SEMPER Read/Write Any Register space   */
uint8_t  MockHAL_GetVCR(uint32_t addr);
uint8_t  MockHAL_GetMR(uint32_t addr);
uint16_t MockHAL_GetHyperReg(uint32_t addr);
bool     MockHAL_Is4ByteMode(void);
uint8_t  MockHAL_GetSramModeRegister(void);

/* HAL parameter checks (assert_param and state checks of stm32n6xx_hal_xspi.c) violated so far */
uint32_t    MockHAL_GetAssertViolations(void);
const char *MockHAL_GetLastViolation(void);
void        MockHAL_ClearAssertViolations(void);

/* Fault injection: the HAL call with index N (0-based, counted from the last arm) fails */
void     MockHAL_FailCall(int32_t index);
void     MockHAL_FailNone(void);
uint32_t MockHAL_GetCallCount(void);
bool     MockHAL_FaultTriggered(void);

/* Event log */
void               MockHAL_ClearLog(void);
uint32_t           MockHAL_GetEventCount(void);
const MockEvent_t *MockHAL_GetEvent(uint32_t index);
uint32_t           MockHAL_CountCommands(uint32_t instruction);
const MockEvent_t *MockHAL_FindCommand(uint32_t instruction, uint32_t nth);
const MockEvent_t *MockHAL_LastCommand(void);
const MockEvent_t *MockHAL_FindEvent(MockEventType_t type, uint32_t nth);
const MockEvent_t *MockHAL_FindTxAfterCommand(uint32_t instruction, uint32_t nth);

/* Backing array shared by all emulated devices */
uint8_t* MockHAL_GetMemoryBuffer(void);
uint32_t MockHAL_GetMemoryBufferSize(void);

/* CPU accesses to memory-mapped windows (FMC banks, XSPI memory-mapped mode) when EXTMEM_UNIT_TEST
 * is defined. Offsets wrap on the backing array; a stuck-at-0 fault can be armed on one byte. */
void     MockHAL_RamWrite(uint32_t Offset, const uint8_t *pData, uint32_t Size);
void     MockHAL_RamRead(uint32_t Offset, uint8_t *pData, uint32_t Size);
void     MockHAL_SetRamStuckAt0(uint32_t Offset, uint8_t Mask);  /*!< Mask 0 disables the fault */

/* FMC data bus hooks used by the FMC drivers when EXTMEM_UNIT_TEST is defined */
void     MockHAL_FmcWrite16(uint32_t BaseAddr, uint32_t ByteOffset, uint16_t Data);
uint16_t MockHAL_FmcRead16(uint32_t BaseAddr, uint32_t ByteOffset);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_HAL_H */
