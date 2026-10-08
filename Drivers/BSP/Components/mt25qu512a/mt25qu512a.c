/**
  ******************************************************************************
  * @file    mt25qu512a.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Micron Quad SPI NOR Flash (MT25QU / MT25QL).
  ******************************************************************************
  */

#include "mt25qu512a.h"

static int32_t MT25QU_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_AutoPollingTypeDef sCfg = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_READ_FLAG_STATUS_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  sCfg.MatchValue    = MT25Q_FSR_READY;
  sCfg.MatchMask     = MT25Q_FSR_READY;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  /* A stacked part answers successive FSR reads die by die: the operation is over only when
   * every die reports ready (MT25QU01G datasheet, "outputs 1 for all the die of the stack").
   * A monolithic part simply repeats its own status. */
  uint32_t start = HAL_GetTick();
  for (;;)
  {
    if (HAL_XSPI_Command(Ctx, &sCmd, Timeout) != HAL_OK) return MT25Q_ERROR;
    if (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) != HAL_OK) return MT25Q_ERROR;

    bool allReady = true;
    for (uint32_t die = 1U; die < MT25Q_MAX_DICE; die++)
    {
      uint8_t fsr = 0;
      if (HAL_XSPI_Command(Ctx, &sCmd, Timeout) != HAL_OK) return MT25Q_ERROR;
      if (HAL_XSPI_Receive(Ctx, &fsr, Timeout) != HAL_OK) return MT25Q_ERROR;
      if ((fsr & MT25Q_FSR_READY) == 0U) allReady = false;
    }
    if (allReady) return MT25Q_OK;
    if ((HAL_GetTick() - start) > Timeout) return MT25Q_ERROR;
  }
}

int32_t MT25QU_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_ReadStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pStatus)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_READ_STATUS_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return (HAL_XSPI_Receive(Ctx, pStatus, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_ReadFlagStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pFlagStatus)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_READ_FLAG_STATUS_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return (HAL_XSPI_Receive(Ctx, pFlagStatus, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_WriteEnable(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_WRITE_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_Enter4ByteAddressMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};


  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_ENTER_4BYTE_ADDR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_Exit4ByteAddressMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};


  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_EXIT_4BYTE_ADDR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
{
  Ctx->Init.FifoThresholdByte       = 4;
  Ctx->Init.MemoryType              = HAL_XSPI_MEMTYPE_MICRON;
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

  if (HAL_XSPI_Init(Ctx) != HAL_OK) return MT25Q_ERROR;

  if (MT25QU_Reset(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  /* Above 128 Mbits (16 MBytes, DEVSIZE code HAL_XSPI_SIZE_128MB) the array needs 4-byte addresses */
  if (MemorySize > HAL_XSPI_SIZE_128MB)
  {
    return MT25QU_Enter4ByteAddressMode(Ctx);
  }
  return MT25Q_OK;
}

int32_t MT25QU_ReadQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_FAST_READ_QUAD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 10;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  return MT25QU_ReadQuadEx(Ctx, Address, pData, Size, DummyCycles, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t MT25QU_PageProgramQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_PAGE_PROGRAM_QUAD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  if (HAL_XSPI_Transmit(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;

  return MT25QU_AutoPollingMemReady(Ctx, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
}

int32_t MT25QU_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  return MT25QU_PageProgramQuadEx(Ctx, Address, pData, Size, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t MT25QU_EraseSector4KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_SUBSECTOR_ERASE_4K;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return MT25QU_AutoPollingMemReady(Ctx, 2000);
}

int32_t MT25QU_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  return MT25QU_EraseSector4KEx(Ctx, Address, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t MT25QU_EraseBlock64KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_SECTOR_ERASE_64K;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return MT25QU_AutoPollingMemReady(Ctx, 3000);
}

int32_t MT25QU_EraseBlock64K(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  return MT25QU_EraseBlock64KEx(Ctx, Address, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t MT25QU_EraseChip(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_CHIP_ERASE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return MT25QU_AutoPollingMemReady(Ctx, 600000);
}

int32_t MT25QU_EraseDie(XSPI_HandleTypeDef *Ctx, uint32_t DieAddress)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT25QU_WriteEnable(Ctx) != MT25Q_OK) return MT25Q_ERROR;

  /* Multi-die parts (1 Gb and above) reject BULK ERASE: DIE ERASE needs 4-byte address mode */
  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_DIE_ERASE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = DieAddress;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;
  return MT25QU_AutoPollingMemReady(Ctx, 600000);
}

int32_t MT25QU_EnableMemoryMappedModeEx(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_FAST_READ_QUAD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 10;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? MT25Q_OK : MT25Q_ERROR;
}

int32_t MT25QU_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  return MT25QU_EnableMemoryMappedModeEx(Ctx, DummyCycles, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t MT25QU_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT25Q_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;

  sCmd.Instruction = MT25Q_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT25Q_ERROR;

  HAL_Delay(2);
  return MT25Q_OK;
}
