/**
  ******************************************************************************
  * @file    is66wvh16m8.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI HyperRAM(TM) PSRAM (IS66WVH / IS67WVH).
  ******************************************************************************
  */

#include "is66wvh16m8.h"

int32_t IS66WVH16M8_ReadRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
  sCmd.Address      = RegAddr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;

  uint8_t buf[2];
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;

  /* HyperBus transfers register words most significant byte first */
  *pValue = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
  return IS66WVH_OK;
}

int32_t IS66WVH16M8_WriteRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
  sCmd.Address      = RegAddr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;

  /* HyperBus transfers register words most significant byte first */
  uint8_t buf[2] = { (uint8_t)((Value >> 8) & 0xFF), (uint8_t)(Value & 0xFF) };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVH_OK : IS66WVH_ERROR;
}

int32_t IS66WVH16M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
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

  if (HAL_XSPI_Init(Ctx) != HAL_OK) return IS66WVH_ERROR;

  sHyperbusCfg.RWRecoveryTimeCycle = 4;
  sHyperbusCfg.AccessTimeCycle     = 6;
  sHyperbusCfg.WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode         = HAL_XSPI_VARIABLE_LATENCY;

  if (HAL_XSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;

  uint16_t cr0Val = IS66WVH_CR0_INIT_VALUE;
  return IS66WVH16M8_WriteRegister(Ctx, IS66WVH_REG_CR0, cr0Val);
}

int32_t IS66WVH16M8_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVH_OK : IS66WVH_ERROR;
}

int32_t IS66WVH16M8_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Address;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = Size;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;
  return (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVH_OK : IS66WVH_ERROR;
}

int32_t IS66WVH16M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_HyperbusCmdTypeDef  sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = 0x00000000;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVH_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS66WVH_OK : IS66WVH_ERROR;
}

int32_t IS66WVH16M8_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  uint16_t cr0 = 0;

  /* Deep power down is entered by writing 0 to CR0[15] */
  if (IS66WVH16M8_ReadRegister(Ctx, IS66WVH_REG_CR0, &cr0) != IS66WVH_OK) return IS66WVH_ERROR;
  cr0 = (uint16_t)(cr0 & ~IS66WVH_CR0_DPD_NORMAL);
  return IS66WVH16M8_WriteRegister(Ctx, IS66WVH_REG_CR0, cr0);
}

int32_t IS66WVH16M8_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  uint16_t dummy = 0;

  /* Any transaction holding CS# low for tDPDCSL wakes the device; the data returned is ignored */
  int32_t ret = IS66WVH16M8_ReadRegister(Ctx, IS66WVH_REG_ID0, &dummy);
  HAL_Delay(IS66WVH_DPD_EXIT_TIME_MS);
  return ret;
}
