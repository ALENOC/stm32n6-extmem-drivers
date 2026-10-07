/**
  ******************************************************************************
  * @file    s28hs512t.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon SEMPER(TM) Octal NOR Flash
  ******************************************************************************
  */

#include "s28hs512t.h"

int32_t S28HS512T_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S28HS_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  if (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS512T_OK;
}

int32_t S28HS512T_WriteEnable(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
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
    sCmd.Instruction        = S28HS_DTR_CMD_WRITE_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_WRITE_ENABLE;
  }

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_ReadAnyReg(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t *pValue)
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
    sCmd.Instruction        = S28HS_DTR_CMD_READ_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DummyCycles        = 8; /* Latency in octal register read */
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_READ_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 1;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return (HAL_XSPI_Receive(Ctx, pValue, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_WriteAnyReg(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t Value)
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
    sCmd.Instruction        = S28HS_DTR_CMD_WRITE_REG;
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
    sCmd.Instruction        = S28HS_CMD_WRITE_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = RegAddr;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return (HAL_XSPI_Transmit(Ctx, &Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout)
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
    sCmd.Instruction        = S28HS_DTR_CMD_READ_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = S28HS_REG_STATUS1;
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
    sCmd.Instruction        = S28HS_CMD_READ_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = S28HS_REG_STATUS1;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 1;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  sCfg.MatchValue    = 0x00; /* WIP = 0 */
  sCfg.MatchMask     = S28HS_SR1_WIP;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  return (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK) ? S28HS512T_OK : S28HS512T_TIMEOUT;
}

int32_t S28HS512T_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  uint8_t dummyCode = S28HS_CFR3V_LATENCY_200MHZ;
  if (DummyCycles <= 12) dummyCode = S28HS_CFR3V_LATENCY_80MHZ;
  else if (DummyCycles <= 14) dummyCode = S28HS_CFR3V_LATENCY_100MHZ;
  else if (DummyCycles <= 16) dummyCode = S28HS_CFR3V_LATENCY_133MHZ;
  else if (DummyCycles <= 18) dummyCode = S28HS_CFR3V_LATENCY_166MHZ;

  /* 1. Set Dummy Cycles in CFR3V */
  if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_SPI) != S28HS512T_OK) return S28HS512T_ERROR;
  if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_SPI, S28HS_REG_CFR3_V, dummyCode) != S28HS512T_OK) return S28HS512T_ERROR;

  /* 2. Enable Octal DTR mode and 4-byte address in CFR2V */
  if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_SPI) != S28HS512T_OK) return S28HS512T_ERROR;
  if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_SPI, S28HS_REG_CFR2_V, S28HS_CFR2V_ADRBYT_4BYTE | S28HS_CFR2V_OCTAL_DTR_ENABLE) != S28HS512T_OK) return S28HS512T_ERROR;

  return S28HS512T_OK;
}

int32_t S28HS512T_ExitOctalDTRMode(XSPI_HandleTypeDef *Ctx)
{
  if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_OCTAL_DTR) != S28HS512T_OK) return S28HS512T_ERROR;
  if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_OCTAL_DTR, S28HS_REG_CFR2_V, S28HS_CFR2V_ADRBYT_4BYTE) != S28HS512T_OK) return S28HS512T_ERROR;
  return S28HS512T_OK;
}

int32_t S28HS512T_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
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
    sCmd.Instruction        = S28HS_DTR_CMD_READ;
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
    sCmd.Instruction        = S28HS_CMD_READ_FAST_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S28HS512T_WriteEnable(Ctx, Mode) != S28HS512T_OK)
  {
    return S28HS512T_ERROR;
  }

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
    sCmd.Instruction        = S28HS_DTR_CMD_PAGE_PROG;
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
    sCmd.Instruction        = S28HS_CMD_PAGE_PROG_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  if (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS512T_AutoPollingMemReady(Ctx, Mode, 5000);
}

int32_t S28HS512T_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S28HS512T_WriteEnable(Ctx, Mode) != S28HS512T_OK) return S28HS512T_ERROR;

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
    sCmd.Instruction        = S28HS_DTR_CMD_SECTOR_ERASE_4K;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_SECTOR_ERASE_4K_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS512T_AutoPollingMemReady(Ctx, Mode, 1000);
}

int32_t S28HS512T_EraseBlock256K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S28HS512T_WriteEnable(Ctx, Mode) != S28HS512T_OK) return S28HS512T_ERROR;

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
    sCmd.Instruction        = S28HS_DTR_CMD_BLOCK_ERASE_256K;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_BLOCK_ERASE_256K_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS512T_AutoPollingMemReady(Ctx, Mode, 3000);
}

int32_t S28HS512T_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S28HS512T_WriteEnable(Ctx, Mode) != S28HS512T_OK) return S28HS512T_ERROR;

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
    sCmd.Instruction        = S28HS_DTR_CMD_CHIP_ERASE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_CHIP_ERASE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS512T_AutoPollingMemReady(Ctx, Mode, 300000); /* Bulk erase takes up to ~250s */
}

int32_t S28HS512T_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd.Instruction        = S28HS_DTR_CMD_READ;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DummyCycles        = DummyCycles;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;

  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = S28HS_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S28HS512T_ERROR;

  sCmd.Instruction = S28HS_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S28HS512T_ERROR;

  HAL_Delay(1); /* Wait for reset recovery time */
  return S28HS512T_OK;
}

int32_t S28HS512T_GetInfo(S28HS512T_Info_t *pInfo)
{
  if (pInfo == NULL) return S28HS512T_ERROR;
  pInfo->FlashSize      = S28HS512T_FLASH_SIZE;
  pInfo->PageSize       = S28HS512T_PAGE_SIZE;
  pInfo->Sector4KSize   = S28HS512T_SECTOR_4K;
  pInfo->Block256KSize  = S28HS512T_BLOCK_256K;
  pInfo->ManufacturerID = S28HS_MANUFACTURER_ID;
  pInfo->DeviceID       = S28HS_DEVICE_ID_512MB;
  return S28HS512T_OK;
}
