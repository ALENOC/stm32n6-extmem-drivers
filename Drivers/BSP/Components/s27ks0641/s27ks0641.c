/**
  ******************************************************************************
  * @file    s27ks0641.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon HyperRAM(TM) (S27KS / S27KL / S27HS / HL).
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

  *pValue = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
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

  uint8_t buf[2] = { (uint8_t)(Value & 0xFF), (uint8_t)((Value >> 8) & 0xFF) };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
{
  XSPI_HyperbusCfgTypeDef sHyperbusCfg = {0};

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
  sHyperbusCfg.AccessTimeCycle     = 6;
  sHyperbusCfg.WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode         = HAL_XSPI_VARIABLE_LATENCY;

  if (HAL_XSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  /* Configure Configuration Register 0: 6 cycles, variable latency */
  uint16_t cr0Val = S27KS_CR0_LATENCY_6_CYCLES | S27KS_CR0_VARIABLE_LATENCY | S27KS_CR0_DRIVE_STRENGTH_FULL;
  if (S27KS0641_WriteRegister(Ctx, S27KS_REG_CR0, cr0Val) != S27KS_OK)
  {
    return S27KS_ERROR;
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
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S27KS_ERROR;
  }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? S27KS_OK : S27KS_ERROR;
}

int32_t S27KS0641_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  return S27KS0641_WriteRegister(Ctx, S27KS_REG_CR1, S27KS_CR1_DEEP_POWER_DOWN);
}

int32_t S27KS0641_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  (void)Ctx;
  /* Hardware wake-up sequence requires CS# pulse for tDPDCS (minimum 200ns) */
  HAL_Delay(1);
  return S27KS_OK;
}
