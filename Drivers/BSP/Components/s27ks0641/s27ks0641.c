/**
  ******************************************************************************
  * @file    s27ks0641.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon HyperRAM(TM) (S27KS / S27KL, S70KS / S70KL, S80KS2562).
  ******************************************************************************
  */

#include "s27ks0641.h"

int32_t S27KS0641_ReadRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
  sCmd.Address      = RegAddr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  uint8_t buf[2];
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  /* HyperBus transfers register words most significant byte first */
  *pValue = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
  return S27KS_OK;
}

int32_t S27KS0641_WriteRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
  sCmd.Address      = RegAddr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  /* HyperBus transfers register words most significant byte first */
  const uint8_t buf[2U] = { (uint8_t)((Value >> 8U) & 0xFFU), (uint8_t)(Value & 0xFFU) };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize, uint8_t Dice)
{
  XSPI_HyperbusCfgTypeDef sHyperbusCfg = {0};

  if (Dice == 0U || Dice > S27KS_MAX_DICE)
  {
    return S27KS_ERROR;
  }

  Ctx->Init.FifoThresholdByte       = 8;
  Ctx->Init.MemoryType              = HAL_XSPI_MEMTYPE_HYPERBUS;
  Ctx->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  Ctx->Init.MemorySize              = MemorySize;
  Ctx->Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  Ctx->Init.ChipSelectHighTimeCycle = 4;
  Ctx->Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  Ctx->Init.ClockPrescaler          = ClockPrescaler;
  Ctx->Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  Ctx->Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  Ctx->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  Ctx->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  Ctx->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(Ctx) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  /* HyperBus timing setup: 6 latency cycles for <= 200MHz */
  sHyperbusCfg.RWRecoveryTimeCycle = 4;
  sHyperbusCfg.AccessTimeCycle     = S27KS_LATENCY_CLOCKS; /* Must match CR0[7:4] */
  sHyperbusCfg.WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode         = HAL_XSPI_VARIABLE_LATENCY;

  if (HAL_XSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  /* Configuration Register 0 of every die: 7 clocks, variable latency */
  for (uint32_t die = 0; die < Dice; die++)
  {
    if (S27KS0641_WriteRegister(Ctx, (die * S27KS_DIE_STRIDE) + S27KS_REG_CR0, S27KS_CR0_INIT_VALUE) != S27KS_OK)
    {
      return S27KS_ERROR;
    }
  }

  return S27KS_OK;
}

int32_t S27KS0641_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  return (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_Read_DMA(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  return (HAL_XSPI_Receive_DMA(Ctx, pData) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_Write_DMA(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  return (HAL_XSPI_Transmit_DMA(Ctx, (const uint8_t *)pData) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_HyperbusCmdTypeDef  sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  /* Configure read command */
  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = 0x00000000;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 1; /* Ignored in memory-mapped mode, but the HAL requires at least 1 */
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx, uint8_t Dice)
{
  if (Dice == 0U || Dice > S27KS_MAX_DICE) { return S27KS_ERROR; }

  /* Deep power down is entered by writing 0 to CR0[15] of each die. Waking up needs no
   * per-die access: every die sees the shared CS# of the next transaction. */
  for (uint32_t die = 0; die < Dice; die++)
  {
    uint32_t reg = (die * S27KS_DIE_STRIDE) + S27KS_REG_CR0;
    uint16_t cr0 = 0;
    if (S27KS0641_ReadRegister(Ctx, reg, &cr0) != S27KS_OK) { return S27KS_ERROR; }
    cr0 = (uint16_t)(cr0 & ~S27KS_CR0_DPD_NORMAL);
    if (S27KS0641_WriteRegister(Ctx, reg, cr0) != S27KS_OK) { return S27KS_ERROR; }
  }
  return S27KS_OK;
}

int32_t S27KS0641_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  uint16_t dummy = 0;

  /* Any transaction holding CS# low for tDPDCSL wakes the device; the data returned is ignored */
  int32_t ret = S27KS0641_ReadRegister(Ctx, S27KS_REG_ID0, &dummy);
  HAL_Delay(S27KS_DPD_EXIT_TIME_MS);
  return ret;
}
