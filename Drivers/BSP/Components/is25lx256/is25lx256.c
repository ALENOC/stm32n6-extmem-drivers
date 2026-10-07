/**
  ******************************************************************************
  * @file    is25lx256.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI Octal NOR Flash (IS25LX / IS25WX).
  ******************************************************************************
  */

#include "is25lx256.h"

int32_t IS25LX256_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LX_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_WriteEnable(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_WRITE_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_WRITE_ENABLE;
  }

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_ReadVCR(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t *pValue)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataLength         = 1;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_READ_VCR;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_READ_VCR;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return (HAL_XSPI_Receive(Ctx, pValue, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_WriteVCR(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t Value)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_WRITE_VCR;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_WRITE_VCR;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return (HAL_XSPI_Transmit(Ctx, &Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef  sCmd = {0};
  XSPI_AutoPollingTypeDef sCfg = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataLength         = 1;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_READ_STATUS;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = 0x00000000;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_READ_STATUS;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;

  sCfg.MatchValue    = 0x00;
  sCfg.MatchMask     = IS25LX_SR_WIP;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  return (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK) ? IS25LX_OK : IS25LX_TIMEOUT;
}

int32_t IS25LX256_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  /* 1. Set Dummy Cycles in VCR Address 0x00 */
  if (IS25LX256_WriteEnable(Ctx, EXTMEM_MODE_SPI) != IS25LX_OK) return IS25LX_ERROR;
  if (IS25LX256_WriteVCR(Ctx, EXTMEM_MODE_SPI, IS25LX_VCR_ADDR_DUMMY_CYCLES, DummyCycles) != IS25LX_OK) return IS25LX_ERROR;

  /* 2. Set Octal DDR mode in VCR Address 0x01 */
  if (IS25LX256_WriteEnable(Ctx, EXTMEM_MODE_SPI) != IS25LX_OK) return IS25LX_ERROR;
  if (IS25LX256_WriteVCR(Ctx, EXTMEM_MODE_SPI, IS25LX_VCR_ADDR_IO_MODE, IS25LX_IO_MODE_OCTAL_DTR) != IS25LX_OK) return IS25LX_ERROR;

  return IS25LX_OK;
}

int32_t IS25LX256_ExitOctalDTRMode(XSPI_HandleTypeDef *Ctx)
{
  if (IS25LX256_WriteEnable(Ctx, EXTMEM_MODE_OCTAL_DTR) != IS25LX_OK) return IS25LX_ERROR;
  if (IS25LX256_WriteVCR(Ctx, EXTMEM_MODE_OCTAL_DTR, IS25LX_VCR_ADDR_IO_MODE, IS25LX_IO_MODE_SPI) != IS25LX_OK) return IS25LX_ERROR;
  return IS25LX_OK;
}

int32_t IS25LX256_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.Address            = Address;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataLength         = Size;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_READ;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DummyCycles        = DummyCycles;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_READ_FAST_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LX256_WriteEnable(Ctx, Mode) != IS25LX_OK) return IS25LX_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.Address            = Address;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_PAGE_PROG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_PAGE_PROG_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  if (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;

  return IS25LX256_AutoPollingMemReady(Ctx, Mode, 5000);
}

int32_t IS25LX256_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LX256_WriteEnable(Ctx, Mode) != IS25LX_OK) return IS25LX_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.Address            = Address;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_SECTOR_ERASE_4K;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_SECTOR_ERASE_4K_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return IS25LX256_AutoPollingMemReady(Ctx, Mode, 1000);
}

int32_t IS25LX256_EraseBlock64K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LX256_WriteEnable(Ctx, Mode) != IS25LX_OK) return IS25LX_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.Address            = Address;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_BLOCK_ERASE_64K;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_BLOCK_ERASE_64K_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return IS25LX256_AutoPollingMemReady(Ctx, Mode, 2000);
}

int32_t IS25LX256_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (IS25LX256_WriteEnable(Ctx, Mode) != IS25LX_OK) return IS25LX_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = IS25LX_DTR_CMD_CHIP_ERASE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = IS25LX_CMD_CHIP_ERASE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;
  return IS25LX256_AutoPollingMemReady(Ctx, Mode, 250000);
}

int32_t IS25LX256_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd.Instruction        = IS25LX_DTR_CMD_READ;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DummyCycles        = DummyCycles;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS25LX_OK : IS25LX_ERROR;
}

int32_t IS25LX256_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS25LX_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;

  sCmd.Instruction = IS25LX_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS25LX_ERROR;

  HAL_Delay(1);
  return IS25LX_OK;
}
