/**
  ******************************************************************************
  * @file    is62wvs.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI Serial SRAM (IS62WVS / IS65WVS).
  ******************************************************************************
  */

#include "is62wvs.h"

int32_t IS62WVS_ReadModeRegister(XSPI_HandleTypeDef *Ctx, uint8_t *pMode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_RDMR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Receive(Ctx, pMode, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

int32_t IS62WVS_WriteModeRegister(XSPI_HandleTypeDef *Ctx, uint8_t Mode)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_WRMR;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = 1;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Transmit(Ctx, &Mode, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

int32_t IS62WVS_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
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
  /* Memory-mapped bursts must restart at the 2 Mbit die boundary of the 4 Mbit part */
  Ctx->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_2MB; /* 2 Mbits = 256 KBytes */
  Ctx->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  Ctx->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(Ctx) != HAL_OK) { return IS62WVS_ERROR; }

  /* Reset to single SPI mode and configure Sequential mode for continuous read/write */
  if (IS62WVS_Reset(Ctx) != IS62WVS_OK) { return IS62WVS_ERROR; }

  return IS62WVS_WriteModeRegister(Ctx, IS62WVS_MODE_SEQUENTIAL);
}

int32_t IS62WVS_EnterQuadMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_ENTER_QUAD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

int32_t IS62WVS_ExitQuadMode(XSPI_HandleTypeDef *Ctx)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_4_LINES;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_EXIT_QUAD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

static int32_t IS62WVS_Read_Chunk(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_READ;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

static int32_t IS62WVS_Write_Chunk(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_WRITE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

static int32_t IS62WVS_ReadQuad_Chunk(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_4_LINES; /* SQI: instruction on 4 lines */
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_READ;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = DummyCycles;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

static int32_t IS62WVS_WriteQuad_Chunk(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_4_LINES; /* SQI: instruction on 4 lines */
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_WRITE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = Address;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = Size;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }
  return (HAL_XSPI_Transmit(Ctx, (const uint8_t *)pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

int32_t IS62WVS_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd = {0};
  XSPI_MemoryMappedTypeDef sMem = {0};

  /* Configure Write Command */
  sCmd.OperationType      = HAL_XSPI_OPTYPE_WRITE_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_4_LINES; /* SQI: instruction on 4 lines */
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = IS62WVS_CMD_WRITE;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_4_LINES;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_4_LINES;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }

  /* Configure Read Command */
  sCmd.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  sCmd.Instruction   = IS62WVS_CMD_READ;
  sCmd.DummyCycles   = DummyCycles;

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) { return IS62WVS_ERROR; }

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS62WVS_OK : IS62WVS_ERROR;
}

int32_t IS62WVS_Reset(XSPI_HandleTypeDef *Ctx)
{
  /* RSTIO (0xFF in SQI) returns the device to SPI; in SPI mode the byte is ignored */
  int32_t ret = IS62WVS_ExitQuadMode(Ctx);
  HAL_Delay(1);
  return ret;
}

int32_t IS62WVS_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  /* The 4 Mbit part stacks two 2 Mbit dice: sequential access cannot cross 0x40000 */
  while (Size > 0U)
  {
    uint32_t chunk = IS62WVS_DIE_SIZE - (Address % IS62WVS_DIE_SIZE);
    if (chunk > Size) { chunk = Size; }
    int32_t ret = IS62WVS_Read_Chunk(Ctx, Address, pData, chunk);
    if (ret != IS62WVS_OK) { return ret; }
    Address += chunk;
    pData += chunk;
    Size -= chunk;
  }
  return IS62WVS_OK;
}

int32_t IS62WVS_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  /* The 4 Mbit part stacks two 2 Mbit dice: sequential access cannot cross 0x40000 */
  while (Size > 0U)
  {
    uint32_t chunk = IS62WVS_DIE_SIZE - (Address % IS62WVS_DIE_SIZE);
    if (chunk > Size) { chunk = Size; }
    int32_t ret = IS62WVS_Write_Chunk(Ctx, Address, pData, chunk);
    if (ret != IS62WVS_OK) { return ret; }
    Address += chunk;
    pData += chunk;
    Size -= chunk;
  }
  return IS62WVS_OK;
}

int32_t IS62WVS_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles)
{
  /* The 4 Mbit part stacks two 2 Mbit dice: sequential access cannot cross 0x40000 */
  while (Size > 0U)
  {
    uint32_t chunk = IS62WVS_DIE_SIZE - (Address % IS62WVS_DIE_SIZE);
    if (chunk > Size) { chunk = Size; }
    int32_t ret = IS62WVS_ReadQuad_Chunk(Ctx, Address, pData, chunk, DummyCycles);
    if (ret != IS62WVS_OK) { return ret; }
    Address += chunk;
    pData += chunk;
    Size -= chunk;
  }
  return IS62WVS_OK;
}

int32_t IS62WVS_WriteQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  /* The 4 Mbit part stacks two 2 Mbit dice: sequential access cannot cross 0x40000 */
  while (Size > 0U)
  {
    uint32_t chunk = IS62WVS_DIE_SIZE - (Address % IS62WVS_DIE_SIZE);
    if (chunk > Size) { chunk = Size; }
    int32_t ret = IS62WVS_WriteQuad_Chunk(Ctx, Address, pData, chunk);
    if (ret != IS62WVS_OK) { return ret; }
    Address += chunk;
    pData += chunk;
    Size -= chunk;
  }
  return IS62WVS_OK;
}
