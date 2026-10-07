/**
  ******************************************************************************
  * @file    mock_hal.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Mock Hardware Abstraction Layer for host unit testing.
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

/* XSPI Constants */
#define HAL_XSPI_OPTYPE_COMMON_CFG       0x00U
#define HAL_XSPI_OPTYPE_READ_CFG         0x01U
#define HAL_XSPI_OPTYPE_WRITE_CFG        0x02U

#define HAL_XSPI_INSTRUCTION_NONE        0x00U
#define HAL_XSPI_INSTRUCTION_1_LINE      0x01U
#define HAL_XSPI_INSTRUCTION_2_LINES     0x02U
#define HAL_XSPI_INSTRUCTION_4_LINES     0x04U
#define HAL_XSPI_INSTRUCTION_8_LINES     0x08U

#define HAL_XSPI_INSTRUCTION_8_BITS      0x00U
#define HAL_XSPI_INSTRUCTION_16_BITS     0x01U

#define HAL_XSPI_INSTRUCTION_DTR_DISABLE 0x00U
#define HAL_XSPI_INSTRUCTION_DTR_ENABLE  0x01U

#define HAL_XSPI_ADDRESS_NONE            0x00U
#define HAL_XSPI_ADDRESS_1_LINE          0x01U
#define HAL_XSPI_ADDRESS_2_LINES         0x02U
#define HAL_XSPI_ADDRESS_4_LINES         0x04U
#define HAL_XSPI_ADDRESS_8_LINES         0x08U

#define HAL_XSPI_ADDRESS_24_BITS         0x00U
#define HAL_XSPI_ADDRESS_32_BITS         0x01U

#define HAL_XSPI_ADDRESS_DTR_DISABLE     0x00U
#define HAL_XSPI_ADDRESS_DTR_ENABLE      0x01U

#define HAL_XSPI_ALT_BYTES_NONE          0x00U

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
#define HAL_XSPI_AUTOMATIC_STOP_ENABLE   0x01U
#define HAL_XSPI_TIMEOUT_COUNTER_DISABLE 0x00U

#define HAL_XSPI_MEMTYPE_MICRON          0x00U
#define HAL_XSPI_MEMTYPE_MACRONIX        0x01U
#define HAL_XSPI_MEMTYPE_APMEM           0x02U
#define HAL_XSPI_MEMTYPE_HYPERBUS        0x04U

#define HAL_XSPI_SINGLE_MEM              0x00U
#define HAL_XSPI_SIZE_16MB               0x17U
#define HAL_XSPI_SIZE_32MB               0x18U
#define HAL_XSPI_SIZE_64MB               0x19U
#define HAL_XSPI_CSSEL_NCS1              0x00U
#define HAL_XSPI_CLOCK_MODE_0            0x00U
#define HAL_XSPI_SAMPLE_SHIFT_NONE       0x00U
#define HAL_XSPI_DHQC_ENABLE             0x01U
#define HAL_XSPI_BONDARYOF_NONE          0x00U
#define HAL_XSPI_BONDARYOF_16KB          0x01U
#define HAL_XSPI_FREERUNCLK_DISABLE      0x00U
#define HAL_XSPI_WRAP_NOT_SUPPORTED      0x00U

#define HAL_XSPI_MEMORY_ADDRESS_SPACE    0x00U
#define HAL_XSPI_REGISTER_ADDRESS_SPACE  0x01U

#define HAL_XSPI_VARIABLE_LATENCY        0x00U
#define HAL_XSPI_FIXED_LATENCY           0x01U
#define HAL_XSPI_LATENCY_ON_WRITE        0x00U

/* FMC / SRAM Constants */
#define FMC_NORSRAM_BANK1                0x00000000U
#define FMC_NORSRAM_DEVICE               ((void *)0x60000000)
#define FMC_NORSRAM_EXTENDED_DEVICE      ((void *)0x60000000)
#define FMC_DATA_ADDRESS_MUX_DISABLE     0x00U
#define FMC_MEMORY_TYPE_PSRAM            0x00U
#define FMC_MEMORY_TYPE_NOR              0x08U
#define FMC_NORSRAM_MEM_BUS_WIDTH_16     0x00U
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
#define FMC_ACCESS_MODE_A                0x00U
#define FMC_ACCESS_MODE_B                0x01U

#define PWR_VDDIO2                       0x02U
#define PWR_VDDIO_RANGE_1V8              0x00U
#define PWR_VDDIO_RANGE_3V3              0x01U

#define GPIO_MODE_AF_PP                  0x02U
#define GPIO_NOPULL                      0x00U
#define GPIO_SPEED_FREQ_VERY_HIGH        0x03U
#define GPIO_AF9_XSPIM_P1                0x09U

#define XSPI1                            ((void *)0x52005000)
#define XSPI2                            ((void *)0x52006000)
#define XSPI3                            ((void *)0x52007000)

/* Cortex-M & Cache Mocks */
typedef struct {
  uint32_t CCR;
} SCB_Type;

#define SCB_CCR_DC_Msk                   (1U << 16)
extern SCB_Type *SCB;

static inline void SCB_CleanInvalidateDCache_by_Addr(void *addr, int32_t size) {
  (void)addr; (void)size;
}

/* Struct definitions */
typedef struct {
  uint32_t FifoThresholdByte;
  uint32_t MemoryType;
  uint32_t MemoryMode;
  uint32_t MemorySize;
  uint32_t MemorySelect;
  uint32_t ChipSelectHighTimeCycle;
  uint32_t ClockMode;
  uint32_t ClockPrescaler;
  uint32_t SampleShifting;
  uint32_t DelayHoldQuarterCycle;
  uint32_t ChipSelectBoundary;
  uint32_t FreeRunningClock;
  uint32_t WrapSize;
} XSPI_InitTypeDef;

typedef struct {
  void             *Instance;
  XSPI_InitTypeDef Init;
} XSPI_HandleTypeDef;

typedef struct {
  uint32_t OperationType;
  uint32_t InstructionMode;
  uint32_t InstructionWidth;
  uint32_t InstructionDTRMode;
  uint32_t Instruction;
  uint32_t AddressMode;
  uint32_t AddressWidth;
  uint32_t AddressDTRMode;
  uint32_t Address;
  uint32_t AlternateBytesMode;
  uint32_t DataMode;
  uint32_t DataDTRMode;
  uint32_t DataLength;
  uint32_t DummyCycles;
  uint32_t DQSMode;
} XSPI_RegularCmdTypeDef;

typedef struct {
  uint32_t AddressSpace;
  uint32_t Address;
  uint32_t AddressWidth;
  uint32_t DataMode;
  uint32_t DataLength;
  uint32_t DQSMode;
} XSPI_HyperbusCmdTypeDef;

typedef struct {
  uint32_t RWRecoveryTimeCycle;
  uint32_t AccessTimeCycle;
  uint32_t WriteZeroLatency;
  uint32_t LatencyMode;
} XSPI_HyperbusCfgTypeDef;

typedef struct {
  uint32_t TimeOutActivation;
} XSPI_MemoryMappedTypeDef;

typedef struct {
  uint32_t MatchValue;
  uint32_t MatchMask;
  uint32_t MatchMode;
  uint32_t IntervalTime;
  uint32_t AutomaticStop;
} XSPI_AutoPollingTypeDef;

typedef struct {
  uint32_t NSBank;
  uint32_t DataAddressMux;
  uint32_t MemoryType;
  uint32_t MemoryDataWidth;
  uint32_t BurstAccessMode;
  uint32_t WaitSignalPolarity;
  uint32_t WaitTiming;
  uint32_t WriteOperation;
  uint32_t WaitSignal;
  uint32_t ExtendedMode;
  uint32_t AsynchronousWait;
  uint32_t WriteBurst;
  uint32_t ContinuousClock;
  uint32_t PageSize;
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

/* HAL Function Prototypes */
HAL_StatusTypeDef HAL_XSPI_Init(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPI_DeInit(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPI_Command(XSPI_HandleTypeDef *hxspi, const XSPI_RegularCmdTypeDef *pCmd, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Transmit(XSPI_HandleTypeDef *hxspi, const uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Receive(XSPI_HandleTypeDef *hxspi, uint8_t *pData, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_Transmit_DMA(XSPI_HandleTypeDef *hxspi, const uint8_t *pData);
HAL_StatusTypeDef HAL_XSPI_Receive_DMA(XSPI_HandleTypeDef *hxspi, uint8_t *pData);
HAL_StatusTypeDef HAL_XSPI_AutoPolling(XSPI_HandleTypeDef *hxspi, const XSPI_AutoPollingTypeDef *pCfg, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_MemoryMapped(XSPI_HandleTypeDef *hxspi, const XSPI_MemoryMappedTypeDef *pCfg);
HAL_StatusTypeDef HAL_XSPI_Abort(XSPI_HandleTypeDef *hxspi);
HAL_StatusTypeDef HAL_XSPI_HyperbusCfg(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCfgTypeDef *pCfg, uint32_t Timeout);
HAL_StatusTypeDef HAL_XSPI_HyperbusCmd(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCmdTypeDef *pCmd, uint32_t Timeout);

HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, const FMC_NORSRAM_TimingTypeDef *pTiming, const FMC_NORSRAM_TimingTypeDef *pExtTiming);
HAL_StatusTypeDef HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram);

uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t Delay);

void HAL_PWREx_EnableVddIO2(void);
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

/* Mock Control API for Test Cases */
void MockHAL_Reset(void);
void MockHAL_SetEmulatedChip(uint8_t mfg, uint8_t memType, uint8_t density);
void MockHAL_SetHyperBusID(uint16_t id0, uint16_t id1);
uint8_t* MockHAL_GetMemoryBuffer(void);
uint32_t MockHAL_GetMemoryBufferSize(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_HAL_H */
