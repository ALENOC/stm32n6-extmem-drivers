/**
  ******************************************************************************
  * @file    mt35xu512a.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Micron Xccela(TM) Octal NOR Flash (MT35XU).
  ******************************************************************************
  */

#include "mt35xu512a.h"

static int32_t MT35XU_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_AutoPollingTypeDef sCfg = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_READ_FLAG_STATUS;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DataLength         = 2;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = MT35XU_CMD_READ_FLAG_STATUS_REG;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DataLength         = 1;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, Timeout) != HAL_OK) return MT35XU_ERROR;

  sCfg.MatchValue    = MT35XU_FSR_READY;
  sCfg.MatchMask     = MT35XU_FSR_READY;
  sCfg.MatchMode     = HAL_XSPI_MATCH_MODE_AND;
  sCfg.IntervalTime  = 0x10;
  sCfg.AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE;

  return (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_READ_ID;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 3;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return (HAL_XSPI_Receive(Ctx, pID, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_ReadStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pStatus)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_READ_STATUS_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return (HAL_XSPI_Receive(Ctx, pStatus, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_ReadFlagStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pFlagStatus)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_READ_FLAG_STATUS_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return (HAL_XSPI_Receive(Ctx, pFlagStatus, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_WriteEnable(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_WRITE_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

static int32_t MT35XU_WriteEnableOctal(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd.Instruction        = MT35XU_OCTAL_CMD_WRITE_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

static int32_t MT35XU_SpiSimpleCommand(XSPI_HandleTypeDef *Ctx, uint8_t Opcode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = Opcode;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

static int32_t MT35XU_WriteVCRSpi(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint8_t Value)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_WRITE_VCR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = RegAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return (HAL_XSPI_Transmit(Ctx, &Value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  /* 1. Extended SPI defaults to 3-byte addressing: the 32-bit VCR writes below need 4-byte mode */
  if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;
  if (MT35XU_SpiSimpleCommand(Ctx, MT35XU_CMD_ENTER_4BYTE_ADDR) != MT35XU_OK) return MT35XU_ERROR;

  /* 2. Dummy cycles for 8D-8D-8D array reads (VCR address 0x01 holds the cycle count) */
  if (MT35XU_WriteVCRSpi(Ctx, MT35XU_VCR_ADDR_DUMMY_CYCLES, (DummyCycles > 0U) ? DummyCycles : 20U) != MT35XU_OK) return MT35XU_ERROR;

  /* 3. Octal DDR with DQS (VCR address 0x00) */
  if (MT35XU_WriteVCRSpi(Ctx, MT35XU_VCR_ADDR_IO_MODE, MT35XU_VCR_IO_MODE_OCTAL_DTR) != MT35XU_OK) return MT35XU_ERROR;

  HAL_Delay(1);
  return MT35XU_OK;
}

int32_t MT35XU_ExitOctalMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};
  /* 8D writes are 2 bytes wide: restore VCR 0x00 (I/O mode) and VCR 0x01 (dummy cycles) together */
  uint8_t vcrValue[2] = { MT35XU_VCR_IO_MODE_EXT_SPI, MT35XU_VCR_DUMMY_DEFAULT };

  if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd.Instruction        = MT35XU_OCTAL_CMD_WRITE_VCR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = MT35XU_VCR_ADDR_IO_MODE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = 2;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  if (HAL_XSPI_Transmit(Ctx, vcrValue, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;

  HAL_Delay(1);
  return MT35XU_OK;
}

int32_t MT35XU_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
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

  if (HAL_XSPI_Init(Ctx) != HAL_OK) return MT35XU_ERROR;

  return MT35XU_Reset(Ctx);
}

int32_t MT35XU_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataLength         = Size;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_FAST_READ_DTR;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 16;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = MT35XU_CMD_FAST_READ_4B;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 8;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_PAGE_PROGRAM;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = Address;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DataLength         = Size;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }
  else
  {
    if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = 0x12U; /* 4-byte Page Program */
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
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  if (HAL_XSPI_Transmit(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;

  return MT35XU_AutoPollingMemReady(Ctx, Mode, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
}

int32_t MT35XU_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_SECTOR_ERASE_4K;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = Address;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }
  else
  {
    if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = 0x21U; /* 4-byte 4KB Erase */
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = Address;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return MT35XU_AutoPollingMemReady(Ctx, Mode, 2000);
}

int32_t MT35XU_EraseBlock128K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_BLOCK_ERASE;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
    sCmd.Address            = Address;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }
  else
  {
    if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = 0xDCU; /* 4-byte 128KB/64KB Erase */
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = Address;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return MT35XU_AutoPollingMemReady(Ctx, Mode, 3000);
}

int32_t MT35XU_EraseChip(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_CHIP_ERASE;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }
  else
  {
    if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;

    sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = 0xC7U;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
    sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_NONE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return MT35XU_AutoPollingMemReady(Ctx, Mode, 600000);
}

int32_t MT35XU_EraseDie(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t DieAddress)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.Address            = DieAddress;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (MT35XU_WriteEnableOctal(Ctx) != MT35XU_OK) return MT35XU_ERROR;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = MT35XU_OCTAL_CMD_DIE_ERASE;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  }
  else
  {
    if (MT35XU_WriteEnable(Ctx) != MT35XU_OK) return MT35XU_ERROR;
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = MT35XU_CMD_DIE_ERASE;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;
  return MT35XU_AutoPollingMemReady(Ctx, Mode, 600000);
}

int32_t MT35XU_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd.Instruction        = MT35XU_OCTAL_CMD_FAST_READ_DTR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 16;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? MT35XU_OK : MT35XU_ERROR;
}

int32_t MT35XU_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = MT35XU_CMD_RESET_ENABLE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;

  sCmd.Instruction = MT35XU_CMD_RESET;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return MT35XU_ERROR;

  HAL_Delay(2);
  return MT35XU_OK;
}
