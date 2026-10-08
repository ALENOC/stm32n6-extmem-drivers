/**
  ******************************************************************************
  * @file    is66wvo32m8.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI Octal PSRAM (IS66WVO / IS67WVO).
  ******************************************************************************
  */

#include "is66wvo32m8.h"

int32_t IS66WVO32M8_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint8_t *pValue, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_READ_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = RegAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = 2; /* DDR register access transfers a byte pair */
  sCmd.DummyCycles        = DummyCycles;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  uint8_t buf[2] = {0};
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  *pValue = buf[0];
  return IS66WVO_OK;
}

int32_t IS66WVO32M8_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint8_t Value)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_WRITE_REG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = RegAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = 2; /* DDR register access transfers a byte pair */
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  /* The second byte of the pair is ignored by the device */
  uint8_t buf[2] = { Value, Value };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
{
  Ctx->Init.FifoThresholdByte       = 8;
  Ctx->Init.MemoryType              = HAL_XSPI_MEMTYPE_APMEM; /* 8-bit APMEM / OPI RAM */
  Ctx->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  Ctx->Init.MemorySize              = MemorySize;
  Ctx->Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  Ctx->Init.ChipSelectHighTimeCycle = 5;
  Ctx->Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  Ctx->Init.ClockPrescaler          = ClockPrescaler;
  Ctx->Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  Ctx->Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  Ctx->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_16KB; /* 16 Kbits = 2 KBytes page */
  Ctx->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  Ctx->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(Ctx) != HAL_OK)
  {
    return IS66WVO_ERROR;
  }

  /* Reset chip */
  if (IS66WVO32M8_Reset(Ctx) != IS66WVO_OK) return IS66WVO_ERROR;

  /* Configure MR0: Read latency 5 cycles, variable latency */
  uint8_t mr0 = IS66WVO_MR0_READ_LATENCY_5 | IS66WVO_MR0_VARIABLE_LATENCY | IS66WVO_MR0_DRIVE_STRENGTH_FULL;
  if (IS66WVO32M8_WriteReg(Ctx, IS66WVO_MR0_ADDR, mr0) != IS66WVO_OK) return IS66WVO_ERROR;

  /* Configure MR4: Write latency 5 cycles */
  uint8_t mr4 = IS66WVO_MR4_WRITE_LATENCY_5;
  if (IS66WVO32M8_WriteReg(Ctx, IS66WVO_MR4_ADDR, mr4) != IS66WVO_OK) return IS66WVO_ERROR;

  return IS66WVO_OK;
}

int32_t IS66WVO32M8_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_READ_SYNC;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = ReadAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 5;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Write(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_WRITE_SYNC;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = WriteAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 5;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_READ_SYNC;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = ReadAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 5;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Receive_DMA(Ctx, pData) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Write_DMA(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_WRITE_SYNC;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.Address            = WriteAddr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = (DummyCycles > 0) ? DummyCycles : 5;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Transmit_DMA(Ctx, (const uint8_t *)pData) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint32_t ReadDummyCycles, uint32_t WriteDummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  /* 1. Configure Write command */
  sCmd.OperationType      = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_WRITE_SYNC;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd.DummyCycles        = WriteDummyCycles;
  sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;

  /* 2. Configure Read command */
  sCmd.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.Instruction   = IS66WVO_CMD_READ_SYNC;
  sCmd.DummyCycles   = ReadDummyCycles;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;

  /* 3. Enable Memory Mapped Mode */
  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Reset(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS66WVO_CMD_RESET;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  HAL_Delay(1);
  return IS66WVO_OK;
}
