/**
  ******************************************************************************
  * @file    s25hl512t.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon SEMPER(TM) / FL Quad SPI NOR Flash
  ******************************************************************************
  */

#include "s25hl512t.h"

int32_t S25HL512T_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  return (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S25HL512T_OK : S25HL512T_ERROR;
}

int32_t S25HL512T_WriteEnable(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_WRITE_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S25HL512T_OK : S25HL512T_ERROR;
}

int32_t S25HL512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef  sCmd = {0};
  XSPI_AutoPollingTypeDef sCfg = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_READ_STATUS1;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  sCfg.MatchValue    = 0x00;
  sCfg.MatchMask     = S25HL_SR1_WIP;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK)
  {
    return S25HL512T_OK;
  }

  /* Both families keep WIP/RDYBSY set after a failed program/erase until the flags are cleared.
   * S25FL-L reports the failure in SR2V[6:5] (cleared by CLSR 30h); its SR1V[6:5] are protection bits.
   * SEMPER reports it in STR1V[6:5] (cleared by CLPEF 82h); its STR2V[7:5] read as 0. */
  uint8_t sr = 0;
  sCmd.Instruction = S25HL_CMD_READ_STATUS2;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if (HAL_XSPI_Receive(Ctx, &sr, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if ((sr & (S25FL_SR2_PRG_ERR | S25FL_SR2_ERS_ERR)) != 0U)
  {
    sCmd.Instruction = S25FL_CMD_CLEAR_STATUS;
  }
  else
  {
    sCmd.Instruction = S25HL_CMD_READ_STATUS1;
    if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
    if (HAL_XSPI_Receive(Ctx, &sr, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
    if ((sr & (S25HL_SR1_PRG_ERR | S25HL_SR1_ERS_ERR)) == 0U)
    {
      return S25HL512T_TIMEOUT;
    }
    sCmd.Instruction = S25HL_CMD_CLEAR_ERRORS;
  }

  sCmd.DataMode = HAL_XSPI_DATA_NONE;
  (void)HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  return S25HL512T_ERROR;
}

int32_t S25HL512T_EnableQuadMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  uint8_t status1 = 0, config1 = 0;

  /* Read Status1 */
  sCmd.OperationType    = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode  = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.Instruction      = S25HL_CMD_READ_STATUS1;
  sCmd.DataMode         = HAL_XSPI_DATA_1_LINE;
  sCmd.DataLength       = 1;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if (HAL_XSPI_Receive(Ctx, &status1, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  /* Read Config1 */
  sCmd.Instruction = S25HL_CMD_READ_CONFIG1;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if (HAL_XSPI_Receive(Ctx, &config1, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  if ((config1 & S25HL_CR1_QUAD_ENABLE) == 0)
  {
    config1 |= S25HL_CR1_QUAD_ENABLE;
    if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

    /* Write Status1 and Config1 (2 bytes) */
    sCmd.Instruction = S25HL_CMD_WRITE_STATUS1;
    sCmd.DataLength  = 2;
    if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
    uint8_t data[2] = { status1, config1 };
    if (HAL_XSPI_Transmit(Ctx, data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

    return S25HL512T_AutoPollingMemReady(Ctx, 1000);
  }

  return S25HL512T_OK;
}

int32_t S25HL512T_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_READ_QUAD_IO_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_4_LINES;
  sCmd.AlternateBytesWidth   = HAL_XSPI_ALT_BYTES_8_BITS;
  sCmd.AlternateBytesDTRMode = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  sCmd.AlternateBytes        = S25HL_MODE_BITS_NO_CONTINUOUS;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : S25HL_DEFAULT_READ_LATENCY;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S25HL512T_OK : S25HL512T_ERROR;
}

/* SEMPER Quad (S25Hx-T) has no quad page program: the 4-byte address instruction table
 * (SFDP DWORD 1) lists 34h and 3Eh as unsupported, only PRPGE 02h/12h in 1S-1S-1S exists */
int32_t S25HL512T_PageProgram(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_PAGE_PROG_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  return S25HL512T_AutoPollingMemReady(Ctx, 5000);
}

/* Quad Input Page Program (4QPP 34h): S25FL-L only */
int32_t S25HL512T_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_QUAD_PAGE_PROG_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  if (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  return S25HL512T_AutoPollingMemReady(Ctx, 5000);
}

int32_t S25HL512T_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_SECTOR_ERASE_4K_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  return S25HL512T_AutoPollingMemReady(Ctx, 1000);
}

int32_t S25HL512T_EraseBlock(XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_BLOCK_ERASE_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  return S25HL512T_AutoPollingMemReady(Ctx, S25HL_TIMEOUT_BLOCK_ERASE_MS);
}

int32_t S25HL512T_ChipErase(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S25HL512T_WriteEnable(Ctx) != S25HL512T_OK) return S25HL512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_CHIP_ERASE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;
  return S25HL512T_AutoPollingMemReady(Ctx, S25HL_TIMEOUT_CHIP_ERASE_MS);
}

int32_t S25HL512T_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_READ_QUAD_IO_4B;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.AlternateBytesMode    = HAL_XSPI_ALT_BYTES_4_LINES;
  sCmd.AlternateBytesWidth   = HAL_XSPI_ALT_BYTES_8_BITS;
  sCmd.AlternateBytesDTRMode = HAL_XSPI_ALT_BYTES_DTR_DISABLE;
  sCmd.AlternateBytes        = S25HL_MODE_BITS_NO_CONTINUOUS;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : S25HL_DEFAULT_READ_LATENCY;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? S25HL512T_OK : S25HL512T_ERROR;
}

int32_t S25HL512T_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S25HL_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  sCmd.Instruction = S25HL_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S25HL512T_ERROR;

  HAL_Delay(1);
  return S25HL512T_OK;
}
