/**
  ******************************************************************************
  * @file    is66wvo32m8.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI OctalRAM (IS66WVO / IS67WVO).
  ******************************************************************************
  */

#include "is66wvo32m8.h"
#include <string.h>

/* 8D-8D-8D command template shared by every access */
static void IS66WVO_FillCommand(XSPI_RegularCmdTypeDef *sCmd, uint32_t OperationType, uint32_t Instruction,
                                uint32_t Address, uint32_t Size, uint32_t DummyCycles)
{
  memset(sCmd, 0, sizeof(*sCmd));
  sCmd->OperationType      = OperationType;
  sCmd->InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
  sCmd->InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
  sCmd->InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  sCmd->Instruction        = Instruction;
  sCmd->AddressMode        = HAL_XSPI_ADDRESS_8_LINES;
  sCmd->AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd->AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_ENABLE;
  sCmd->Address            = Address;
  sCmd->AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd->DataMode           = HAL_XSPI_DATA_8_LINES;
  sCmd->DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
  sCmd->DataLength         = Size;
  sCmd->DummyCycles        = DummyCycles;
  sCmd->DQSMode            = HAL_XSPI_DQS_ENABLE;
}

int32_t IS66WVO32M8_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd;
  uint8_t buf[2] = {0};

  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_COMMON_CFG, IS66WVO_CMD_READ_REG, RegAddr, 2U, DummyCycles);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;

  /* Macronix RAM mode already restores the little-endian word order in memory */
  *pValue = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
  return IS66WVO_OK;
}

int32_t IS66WVO32M8_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value)
{
  XSPI_RegularCmdTypeDef sCmd;
  uint8_t buf[2] = { (uint8_t)(Value & 0xFFU), (uint8_t)(Value >> 8) };

  /* Register writes have zero latency and are never masked */
  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_COMMON_CFG, IS66WVO_CMD_WRITE_REG, RegAddr, 2U, 0U);
  sCmd.DQSMode = HAL_XSPI_DQS_DISABLE;
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_ReadID(XSPI_HandleTypeDef *Ctx, uint16_t *pId, uint32_t *pCapacityBytes)
{
  uint16_t id = 0;

  if (IS66WVO32M8_ReadReg(Ctx, IS66WVO_REG_ID, &id, IS66WVO_DUMMY_CYCLES) != IS66WVO_OK) return IS66WVO_ERROR;
  *pId = id;

  /* Byte-addressed rows and columns: capacity = 2^(row bits + column bits) bytes */
  uint32_t rowBits = ((uint32_t)(id >> IS66WVO_ID_ROW_BITS_POS) & 0x1FU) + 1U;
  uint32_t colBits = ((uint32_t)(id >> IS66WVO_ID_COL_BITS_POS) & 0x0FU) + 1U;
  *pCapacityBytes = ((rowBits + colBits) < 32U) ? (1UL << (rowBits + colBits)) : 0U;
  return IS66WVO_OK;
}

int32_t IS66WVO32M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize)
{
  uint16_t id = 0;
  uint32_t capacity = 0;

  /* Ctx->Init.Refresh is left to the caller: it must keep CS# LOW shorter than tCSM */
  Ctx->Init.FifoThresholdByte       = 8;
  Ctx->Init.MemoryType              = HAL_XSPI_MEMTYPE_MACRONIX_RAM;
  Ctx->Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  Ctx->Init.MemorySize              = MemorySize;
  Ctx->Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  Ctx->Init.ChipSelectHighTimeCycle = 5;
  Ctx->Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  Ctx->Init.ClockPrescaler          = ClockPrescaler;
  Ctx->Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  Ctx->Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE; /* Mandatory in DTR */
  Ctx->Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_8KB; /* 8 Kbits = 1 KByte row */
  Ctx->Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  Ctx->Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(Ctx) != HAL_OK)
  {
    return IS66WVO_ERROR;
  }

  /* The OctalRAM has no software reset. A register write has zero latency whatever the current
   * latency setting, so the configuration is written first and every read uses it afterwards. */
  if (IS66WVO32M8_WriteReg(Ctx, IS66WVO_REG_CR, IS66WVO_CR_INIT_VALUE) != IS66WVO_OK) return IS66WVO_ERROR;

  /* Check that an ISSI OctalRAM answers and that the configuration was accepted */
  if (IS66WVO32M8_ReadID(Ctx, &id, &capacity) != IS66WVO_OK) return IS66WVO_ERROR;
  if ((id & IS66WVO_ID_MANUFACTURER_MASK) != IS66WVO_ID_MANUFACTURER_ISSI) return IS66WVO_ERROR;

  uint16_t cr = 0;
  if (IS66WVO32M8_ReadReg(Ctx, IS66WVO_REG_CR, &cr, IS66WVO_DUMMY_CYCLES) != IS66WVO_OK) return IS66WVO_ERROR;
  if ((cr & (IS66WVO_CR_LC_MASK | IS66WVO_CR_FIXED_LATENCY)) != (IS66WVO_CR_LC_7_CLOCKS | IS66WVO_CR_FIXED_LATENCY))
  {
    return IS66WVO_ERROR;
  }
  return IS66WVO_OK;
}

/* Blocking transfer of one even-aligned, even-sized chunk that stays inside a row */
static int32_t IS66WVO_Transfer(XSPI_HandleTypeDef *Ctx, bool Write, uint8_t *pData, uint32_t Addr, uint32_t Size,
                                uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd;

  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_COMMON_CFG, Write ? IS66WVO_CMD_WRITE_LINEAR : IS66WVO_CMD_READ_LINEAR,
                      Addr, Size, (DummyCycles > 0U) ? DummyCycles : IS66WVO_DUMMY_CYCLES);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  if (Write)
  {
    return (HAL_XSPI_Transmit(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
  }
  return (HAL_XSPI_Receive(Ctx, pData, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

/* Data moves in 16-bit words starting at even column addresses (column A0 must be 0) and a burst
 * must not cross a 1 KB row: split the request, and read-modify-write the odd edge bytes. */
static int32_t IS66WVO_Access(XSPI_HandleTypeDef *Ctx, bool Write, uint8_t *pData, uint32_t Addr, uint32_t Size,
                              uint32_t DummyCycles)
{
  while (Size > 0U)
  {
    if (((Addr & 1U) != 0U) || (Size == 1U))
    {
      uint8_t word[2];
      uint32_t base = Addr & ~1U;
      uint32_t lane = Addr & 1U;
      if (IS66WVO_Transfer(Ctx, false, word, base, 2U, DummyCycles) != IS66WVO_OK) return IS66WVO_ERROR;
      if (Write)
      {
        word[lane] = *pData;
        if (IS66WVO_Transfer(Ctx, true, word, base, 2U, DummyCycles) != IS66WVO_OK) return IS66WVO_ERROR;
      }
      else
      {
        *pData = word[lane];
      }
      pData++;
      Addr++;
      Size--;
      continue;
    }

    uint32_t chunk = IS66WVO_ROW_SIZE - (Addr % IS66WVO_ROW_SIZE);
    if (chunk > Size) chunk = Size;
    chunk &= ~1U;
    if (IS66WVO_Transfer(Ctx, Write, pData, Addr, chunk, DummyCycles) != IS66WVO_OK) return IS66WVO_ERROR;
    pData += chunk;
    Addr  += chunk;
    Size  -= chunk;
  }
  return IS66WVO_OK;
}

int32_t IS66WVO32M8_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles)
{
  return IS66WVO_Access(Ctx, false, pData, ReadAddr, Size, DummyCycles);
}

int32_t IS66WVO32M8_Write(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles)
{
  /* The buffer is only read: the cast keeps a single transfer routine for both directions */
  return IS66WVO_Access(Ctx, true, (uint8_t *)(uintptr_t)pData, WriteAddr, Size, DummyCycles);
}

int32_t IS66WVO32M8_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd;

  /* A single DMA burst must be word aligned and stay inside one row */
  if (((ReadAddr | Size) & 1U) != 0U || Size == 0U || ((ReadAddr % IS66WVO_ROW_SIZE) + Size) > IS66WVO_ROW_SIZE)
  {
    return IS66WVO_ERROR;
  }
  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_COMMON_CFG, IS66WVO_CMD_READ_LINEAR, ReadAddr, Size,
                      (DummyCycles > 0U) ? DummyCycles : IS66WVO_DUMMY_CYCLES);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Receive_DMA(Ctx, pData) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_Write_DMA(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles)
{
  XSPI_RegularCmdTypeDef sCmd;

  if (((WriteAddr | Size) & 1U) != 0U || Size == 0U || ((WriteAddr % IS66WVO_ROW_SIZE) + Size) > IS66WVO_ROW_SIZE)
  {
    return IS66WVO_ERROR;
  }
  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_COMMON_CFG, IS66WVO_CMD_WRITE_LINEAR, WriteAddr, Size,
                      (DummyCycles > 0U) ? DummyCycles : IS66WVO_DUMMY_CYCLES);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;
  return (HAL_XSPI_Transmit_DMA(Ctx, pData) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint32_t ReadDummyCycles, uint32_t WriteDummyCycles)
{
  XSPI_RegularCmdTypeDef   sCmd;
  XSPI_MemoryMappedTypeDef sMem = {0};

  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_WRITE_CFG, IS66WVO_CMD_WRITE_LINEAR, 0U, 0U,
                      (WriteDummyCycles > 0U) ? WriteDummyCycles : IS66WVO_DUMMY_CYCLES);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;

  IS66WVO_FillCommand(&sCmd, HAL_XSPI_OPTYPE_READ_CFG, IS66WVO_CMD_READ_LINEAR, 0U, 0U,
                      (ReadDummyCycles > 0U) ? ReadDummyCycles : IS66WVO_DUMMY_CYCLES);
  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return IS66WVO_ERROR;

  sMem.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  return (HAL_XSPI_MemoryMapped(Ctx, &sMem) == HAL_OK) ? IS66WVO_OK : IS66WVO_ERROR;
}

int32_t IS66WVO32M8_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  /* Writing 0 to CR[15] enters deep power down; a CS# pulse longer than tDPDCSL wakes the device */
  return IS66WVO32M8_WriteReg(Ctx, IS66WVO_REG_CR, (uint16_t)(IS66WVO_CR_INIT_VALUE & ~IS66WVO_CR_DPD_NORMAL));
}

int32_t IS66WVO32M8_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx)
{
  uint16_t dummy = 0;

  /* Any access keeps CS# LOW longer than tDPDX (200 ns) and starts the wake-up (tDPDOUT, 150 us).
   * Array content is lost in deep power down and the configuration register is written again. */
  if (IS66WVO32M8_ReadReg(Ctx, IS66WVO_REG_ID, &dummy, IS66WVO_DUMMY_CYCLES) != IS66WVO_OK) return IS66WVO_ERROR;
  HAL_Delay(1);
  return IS66WVO32M8_WriteReg(Ctx, IS66WVO_REG_CR, IS66WVO_CR_INIT_VALUE);
}
