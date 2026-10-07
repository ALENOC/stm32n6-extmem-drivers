/**
  ******************************************************************************
  * @file    s26ks512s.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon HyperFlash(TM) (S26KS / S26KL).
  ******************************************************************************
  */

#include "s26ks512s.h"

static int32_t HyperFlash_WriteWord(XSPI_HandleTypeDef *Ctx, uint32_t Addr, uint16_t Val)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Addr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S26KS512S_ERROR;
  }

  uint8_t buf[2] = { (uint8_t)(Val & 0xFF), (uint8_t)((Val >> 8) & 0xFF) };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S26KS512S_OK : S26KS512S_ERROR;
}

static int32_t HyperFlash_ReadWord(XSPI_HandleTypeDef *Ctx, uint32_t Addr, uint16_t *pVal)
{
  XSPI_HyperbusCmdTypeDef sCmd = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = Addr;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DataLength   = 2;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S26KS512S_ERROR;
  }

  uint8_t buf[2];
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S26KS512S_ERROR;
  }

  *pVal = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
  return S26KS512S_OK;
}

int32_t S26KS512S_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
{
  XSPI_HyperbusCfgTypeDef sHyperbusCfg = {0};

  Ctx->Init.FifoThresholdByte       = 8;
  Ctx->Init.MemoryType              = HAL_XSPI_MEMTYPE_HYPERBUS;
  Ctx->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  Ctx->Init.MemorySize              = (MemorySize > 0) ? MemorySize : HAL_XSPI_SIZE_64MB;
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
    return S26KS512S_ERROR;
  }

  /* HyperBus timing setup */
  sHyperbusCfg.RWRecoveryTimeCycle = 4;
  sHyperbusCfg.AccessTimeCycle     = S26KS_INITIAL_LATENCY_CYCLES;
  sHyperbusCfg.WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperbusCfg.LatencyMode         = HAL_XSPI_VARIABLE_LATENCY;

  if (HAL_XSPI_HyperbusCfg(Ctx, &sHyperbusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S26KS512S_ERROR;
  }

  return S26KS512S_Reset(Ctx);
}

int32_t S26KS512S_Reset(XSPI_HandleTypeDef *Ctx)
{
  if (HyperFlash_WriteWord(Ctx, 0x00000000, S26KS_CMD_RESET_CFI_EXIT) != S26KS512S_OK)
  {
    return S26KS512S_ERROR;
  }
  HAL_Delay(1);
  return S26KS512S_OK;
}

int32_t S26KS512S_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
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
    return S26KS512S_ERROR;
  }

  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S26KS512S_OK : S26KS512S_ERROR;
}

int32_t S26KS512S_ReadStatus(XSPI_HandleTypeDef *Ctx, uint16_t *pStatus)
{
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_READ_STATUS) != S26KS512S_OK)
  {
    return S26KS512S_ERROR;
  }
  return HyperFlash_ReadWord(Ctx, 0x00000000, pStatus);
}

int32_t S26KS512S_ClearStatus(XSPI_HandleTypeDef *Ctx)
{
  return HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_CLEAR_STATUS);
}

int32_t S26KS512S_WaitUntilReady(XSPI_HandleTypeDef *Ctx, uint32_t TimeoutMs)
{
  uint32_t tickstart = HAL_GetTick();
  uint16_t status = 0;

  while ((HAL_GetTick() - tickstart) < TimeoutMs)
  {
    if (S26KS512S_ReadStatus(Ctx, &status) == S26KS512S_OK)
    {
      if (status & S26KS_SR_DEVICE_READY)
      {
        if (status & (S26KS_SR_ERASE_ERROR | S26KS_SR_PROGRAM_ERROR))
        {
          S26KS512S_ClearStatus(Ctx);
          return S26KS512S_ERROR;
        }
        return S26KS512S_OK;
      }
    }
    HAL_Delay(1);
  }
  return S26KS512S_TIMEOUT;
}

int32_t S26KS512S_ProgramWord(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint16_t Data)
{
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_UNLOCK_DATA1) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR2, S26KS_CMD_UNLOCK_DATA2) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_WORD_PROGRAM) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, Address, Data) != S26KS512S_OK) return S26KS512S_ERROR;

  return S26KS512S_WaitUntilReady(Ctx, 50);
}

int32_t S26KS512S_ProgramBuffer(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  uint32_t words = Size / 2;
  const uint16_t *pWords = (const uint16_t *)pData;

  for (uint32_t i = 0; i < words; i++)
  {
    if (S26KS512S_ProgramWord(Ctx, Address + (i * 2), pWords[i]) != S26KS512S_OK)
    {
      return S26KS512S_ERROR;
    }
  }
  return S26KS512S_OK;
}

int32_t S26KS512S_EraseSector(XSPI_HandleTypeDef *Ctx, uint32_t SectorAddress)
{
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_UNLOCK_DATA1) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR2, S26KS_CMD_UNLOCK_DATA2) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_SECTOR_ERASE_SETUP) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_UNLOCK_DATA1) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR2, S26KS_CMD_UNLOCK_DATA2) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, SectorAddress, S26KS_CMD_SECTOR_ERASE_CONFIRM) != S26KS512S_OK) return S26KS512S_ERROR;

  return S26KS512S_WaitUntilReady(Ctx, 2000); /* Sector erase can take up to ~1.5s */
}

int32_t S26KS512S_EraseChip(XSPI_HandleTypeDef *Ctx)
{
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_UNLOCK_DATA1) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR2, S26KS_CMD_UNLOCK_DATA2) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_SECTOR_ERASE_SETUP) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_UNLOCK_DATA1) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR2, S26KS_CMD_UNLOCK_DATA2) != S26KS512S_OK) return S26KS512S_ERROR;
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_CHIP_ERASE_CONFIRM) != S26KS512S_OK) return S26KS512S_ERROR;

  return S26KS512S_WaitUntilReady(Ctx, 250000);
}

int32_t S26KS512S_ReadCFI(XSPI_HandleTypeDef *Ctx, uint32_t WordOffset, uint16_t *pData)
{
  if (HyperFlash_WriteWord(Ctx, S26KS_UNLOCK_ADDR1, S26KS_CMD_ENTER_CFI) != S26KS512S_OK) return S26KS512S_ERROR;
  int32_t res = HyperFlash_ReadWord(Ctx, WordOffset * 2, pData);
  S26KS512S_Reset(Ctx);
  return res;
}

int32_t S26KS512S_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_HyperbusCmdTypeDef  sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
  sCmd.Address      = 0x00000000;
  sCmd.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode     = HAL_XSPI_DATA_8_LINES;
  sCmd.DQSMode      = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_HyperbusCmd(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S26KS512S_ERROR;
  }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? S26KS512S_OK : S26KS512S_ERROR;
}
