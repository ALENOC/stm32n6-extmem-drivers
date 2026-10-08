/**
  ******************************************************************************
  * @file    is25lp256.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI Quad SPI NOR Flash (IS25LP / IS25WP).
  ******************************************************************************
  */

#include "is25lp256.h"

int32_t IS25LP256_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  return (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

int32_t IS25LP256_WriteEnable(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_WRITE_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

int32_t IS25LP256_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef  sCmd = {0};
  XSPI_AutoPollingTypeDef sCfg = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_READ_STATUS;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  sCfg.MatchValue    = 0x00;
  sCfg.MatchMask     = IS25LP_SR_WIP;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  return (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK) ? IS25LP_OK : IS25LP_TIMEOUT;
}

int32_t IS25LP256_EnableQuadMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  uint8_t status = 0;

  sCmd.OperationType    = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode  = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.Instruction      = IS25LP_CMD_READ_STATUS;
  sCmd.DataMode         = HAL_XSPI_DATA_1_LINE;
  sCmd.DataLength       = 1;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  if (HAL_XSPI_Receive(Ctx, &status, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  if ((status & IS25LP_SR_QE) == 0U)
  {
    status |= IS25LP_SR_QE;
    if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

    sCmd.Instruction = IS25LP_CMD_WRITE_STATUS;
    if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
    if (HAL_XSPI_Transmit(Ctx, &status, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

    return IS25LP256_AutoPollingMemReady(Ctx, 1000);
  }

  return IS25LP_OK;
}

int32_t IS25LP_Enter4ByteAddressMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_ENTER_4BYTE_ADDR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

int32_t IS25LP_Exit4ByteAddressMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_EXIT_4BYTE_ADDR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

/* Wait cycles left after the 2 mode-bit cycles of a Quad I/O read (default total: 6) */
static uint32_t IS25LP_QuadIoDummyAfterMode(uint8_t DummyCycles)
{
  uint32_t total = (DummyCycles > 0U) ? DummyCycles : 6U;
  return (total > 2U) ? (total - 2U) : 0U;
}

int32_t IS25LP_ReadQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  bool is4Byte = (AddressWidth == HAL_XSPI_ADDRESS_32_BITS);

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = is4Byte ? IS25LP_CMD_READ_QUAD_IO_4B : IS25LP_CMD_READ_QUAD_IO;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  /* The ISSI dummy count includes the 2 mode-bit cycles: drive them as 0x00 so the
   * device never sees a floating AXh pattern and enters continuous read mode */
  sCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_4_LINES;
  sCmd.AlternateBytesWidth   = HAL_XSPI_ALT_BYTES_8_BITS;
  sCmd.AlternateBytesDTRMode = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  sCmd.AlternateBytes        = IS25LP_MODE_BITS_NO_XIP;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = IS25LP_QuadIoDummyAfterMode(DummyCycles);
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

int32_t IS25LP256_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  return IS25LP_ReadQuadEx(Ctx, Address, pData, Size, DummyCycles, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t IS25LP_PageProgramQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  bool is4Byte = (AddressWidth == HAL_XSPI_ADDRESS_32_BITS);

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = is4Byte ? IS25LP_CMD_QUAD_PAGE_PROG_4B : IS25LP_CMD_PAGE_PROG_QUAD;
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

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  if (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  return IS25LP256_AutoPollingMemReady(Ctx, 5000);
}

int32_t IS25LP256_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  return IS25LP_PageProgramQuadEx(Ctx, Address, pData, Size, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t IS25LP_EraseSector4KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  bool is4Byte = (AddressWidth == HAL_XSPI_ADDRESS_32_BITS);

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = is4Byte ? IS25LP_CMD_SECTOR_ERASE_4K_4B : IS25LP_CMD_SECTOR_ERASE_4K;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  return IS25LP256_AutoPollingMemReady(Ctx, 1000);
}

int32_t IS25LP256_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  return IS25LP_EraseSector4KEx(Ctx, Address, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t IS25LP_EraseBlock64KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  bool is4Byte = (AddressWidth == HAL_XSPI_ADDRESS_32_BITS);

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = is4Byte ? IS25LP_CMD_BLOCK_ERASE_64K_4B : IS25LP_CMD_BLOCK_ERASE_64K;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  return IS25LP256_AutoPollingMemReady(Ctx, 2000);
}

int32_t IS25LP256_EraseBlock64K(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  return IS25LP_EraseBlock64KEx(Ctx, Address, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t IS25LP256_ChipErase(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LP256_WriteEnable(Ctx) != IS25LP_OK) { return IS25LP_ERROR; }

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_CHIP_ERASE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  return IS25LP256_AutoPollingMemReady(Ctx, 250000);
}

int32_t IS25LP_EnableMemoryMappedModeEx(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint32_t AddressWidth)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};
  bool is4Byte = (AddressWidth == HAL_XSPI_ADDRESS_32_BITS);

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = is4Byte ? IS25LP_CMD_READ_QUAD_IO_4B : IS25LP_CMD_READ_QUAD_IO;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = AddressWidth;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  /* The ISSI dummy count includes the 2 mode-bit cycles: drive them as 0x00 so the
   * device never sees a floating AXh pattern and enters continuous read mode */
  sCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_4_LINES;
  sCmd.AlternateBytesWidth   = HAL_XSPI_ALT_BYTES_8_BITS;
  sCmd.AlternateBytesDTRMode = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  sCmd.AlternateBytes        = IS25LP_MODE_BITS_NO_XIP;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DummyCycles        = IS25LP_QuadIoDummyAfterMode(DummyCycles);
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS25LP_OK : IS25LP_ERROR;
}

int32_t IS25LP256_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  return IS25LP_EnableMemoryMappedModeEx(Ctx, DummyCycles, HAL_XSPI_ADDRESS_32_BITS);
}

int32_t IS25LP256_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  sCmd.Instruction = IS25LP_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  HAL_Delay(1);
  return IS25LP_OK;
}

int32_t IS25LP256_SetReadDummyCycles(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint8_t *pApplied)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  uint8_t params = (uint8_t)((DummyCycles << IS25LP_READ_PARAMS_DUMMY_POS) & IS25LP_READ_PARAMS_DUMMY_MASK);
  uint8_t readBack = 0;

  /* SRPV: P7 = 0 (HOLD#), P[6:3] = dummy cycles, P2..P0 = 0 (no wrap) */
  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LP_CMD_SET_READ_PARAMS_V;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  if (HAL_XSPI_Transmit(Ctx, &params, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  /* RDRP: older parts without a Read Register ignore SRPV and keep the factory latency */
  sCmd.Instruction = IS25LP_CMD_READ_READ_PARAMS;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }
  if (HAL_XSPI_Receive(Ctx, &readBack, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS25LP_ERROR; }

  *pApplied = (readBack == params) ? DummyCycles : IS25LP_DEFAULT_QUAD_IO_DUMMY;
  return IS25LP_OK;
}
