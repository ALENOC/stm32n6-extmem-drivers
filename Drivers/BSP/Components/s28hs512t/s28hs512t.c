/**
  ******************************************************************************
  * @file    s28hs512t.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Infineon SEMPER(TM) Octal NOR Flash
  ******************************************************************************
  */

#include "s28hs512t.h"
#include <string.h>

/* Stacked-die layout per XSPI handle (single die at the default register base when not registered) */
#define S28HS_MAX_LAYOUTS 3U
static struct {
  XSPI_HandleTypeDef     *Ctx;
  S28HS512T_DieLayout_t   Layout;
} s_DieLayouts[S28HS_MAX_LAYOUTS];

static const S28HS512T_DieLayout_t s_SingleDie = { 1U, 0U, { S28HS_REG_VOLATILE_BASE, 0U, 0U, 0U } };

int32_t S28HS512T_SetDieLayout(XSPI_HandleTypeDef *Ctx, const S28HS512T_DieLayout_t *pLayout)
{
  uint32_t freeSlot = S28HS_MAX_LAYOUTS;

  if (pLayout != NULL && (pLayout->Dice == 0U || pLayout->Dice > S28HS_MAX_DICE || (pLayout->Dice > 1U && pLayout->DieSize == 0U)))
  {
    return S28HS512T_ERROR;
  }
  for (uint32_t i = 0; i < S28HS_MAX_LAYOUTS; i++)
  {
    if (s_DieLayouts[i].Ctx == Ctx)
    {
      s_DieLayouts[i].Ctx = NULL;
    }
    if (s_DieLayouts[i].Ctx == NULL && freeSlot == S28HS_MAX_LAYOUTS)
    {
      freeSlot = i;
    }
  }
  if (pLayout == NULL || pLayout->Dice == 1U)
  {
    return S28HS512T_OK;
  }
  if (freeSlot == S28HS_MAX_LAYOUTS)
  {
    return S28HS512T_ERROR;
  }
  s_DieLayouts[freeSlot].Ctx    = Ctx;
  s_DieLayouts[freeSlot].Layout = *pLayout;
  return S28HS512T_OK;
}

static const S28HS512T_DieLayout_t *S28HS_GetLayout(const XSPI_HandleTypeDef *Ctx)
{
  for (uint32_t i = 0; i < S28HS_MAX_LAYOUTS; i++)
  {
    if (s_DieLayouts[i].Ctx == Ctx) return &s_DieLayouts[i].Layout;
  }
  return &s_SingleDie;
}

/* Volatile register base of the die that holds Address */
static uint32_t S28HS_VregForAddress(const XSPI_HandleTypeDef *Ctx, uint32_t Address)
{
  const S28HS512T_DieLayout_t *l = S28HS_GetLayout(Ctx);
  uint32_t die = (l->Dice > 1U) ? (Address / l->DieSize) : 0U;
  if (die >= l->Dice) die = l->Dice - 1U;
  return l->VregBase[die];
}

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

/* Instruction-only command (no address, no data) in the current interface mode */
static int32_t S28HS_SimpleCmd(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t SpiOpcode, uint32_t DtrOpcode)
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
    sCmd.Instruction        = DtrOpcode;
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = SpiOpcode;
  }

  return (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

int32_t S28HS512T_WriteEnable(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  return S28HS_SimpleCmd(Ctx, Mode, S28HS_CMD_WRITE_ENABLE, S28HS_DTR_CMD_WRITE_ENABLE);
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
    sCmd.DummyCycles        = S28HS_OCTAL_DTR_REG_DUMMY;
    sCmd.DataLength         = 2; /* 8D transfers carry an even number of bytes */
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
    sCmd.DummyCycles        = 0; /* Volatile registers have no read latency in 1S-1S-1S */
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  uint8_t buf[2] = {0};
  if (HAL_XSPI_Receive(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }
  *pValue = buf[0];
  return S28HS512T_OK;
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
    sCmd.DataLength         = 2; /* 8D transfers carry an even number of bytes */
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
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

  uint8_t buf[2] = { Value, Value };
  return (HAL_XSPI_Transmit(Ctx, buf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? S28HS512T_OK : S28HS512T_ERROR;
}

static int32_t S28HS_PollDie(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Vreg, uint32_t Timeout);

int32_t S28HS512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout)
{
  return S28HS_PollDie(Ctx, Mode, S28HS_GetLayout(Ctx)->VregBase[0], Timeout);
}

/* Waits for RDYBSY = 0 in STR1V of the die whose registers start at Vreg */
static int32_t S28HS_PollDie(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Vreg, uint32_t Timeout)
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
    sCmd.Address            = Vreg + S28HS_REG_OFS_STATUS1;
    sCmd.DataMode           = HAL_XSPI_DATA_8_LINES;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_ENABLE;
    sCmd.DataLength         = 2;
    sCmd.DummyCycles        = S28HS_OCTAL_DTR_REG_DUMMY;
    sCmd.DQSMode            = HAL_XSPI_DQS_ENABLE;
  }
  else if (Vreg == S28HS_REG_VOLATILE_BASE)
  {
    /* RDSR1 reads the volatile status register without address or latency */
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_READ_STATUS1;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 0;
    sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;
  }
  else
  {
    /* On a stacked-die part RDSR1 only reads die 0: read STR1V of the target die instead */
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = S28HS_CMD_READ_REG;
    sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
    sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
    sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
    sCmd.Address            = Vreg + S28HS_REG_OFS_STATUS1;
    sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
    sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
    sCmd.DummyCycles        = 0;
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

  if (HAL_XSPI_AutoPolling(Ctx, &sCfg, Timeout) == HAL_OK)
  {
    return S28HS512T_OK;
  }

  /* A failed program/erase keeps RDYBSY set until the error flags are cleared:
   * read STR1V once to tell a failure from a plain timeout */
  uint8_t sr1 = 0;
  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    if (S28HS512T_ReadAnyReg(Ctx, Mode, Vreg + S28HS_REG_OFS_STATUS1, &sr1) != S28HS512T_OK) return S28HS512T_ERROR;
  }
  else
  {
    sCmd.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
    if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S28HS512T_ERROR;
    if (HAL_XSPI_Receive(Ctx, &sr1, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return S28HS512T_ERROR;
  }
  if ((sr1 & (S28HS_SR1_PRG_ERR | S28HS_SR1_ERS_ERR)) == 0U)
  {
    return S28HS512T_TIMEOUT;
  }

  XSPI_RegularCmdTypeDef sClr = {0};
  sClr.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sClr.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sClr.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sClr.DataMode           = HAL_XSPI_DATA_NONE;
  sClr.DummyCycles        = 0;
  sClr.DQSMode            = HAL_XSPI_DQS_DISABLE;
  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sClr.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sClr.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sClr.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sClr.Instruction        = S28HS_DTR_CMD_CLEAR_ERRORS;
  }
  else
  {
    sClr.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sClr.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sClr.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sClr.Instruction        = S28HS_CMD_CLEAR_ERRORS;
  }
  (void)HAL_XSPI_Command(Ctx, &sClr, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  return S28HS512T_ERROR;
}

/* Waits for the program/erase at Address to finish. WRENB sets WRPGEN in every die but only the die
 * that runs the operation clears it: on a stacked part WRDIS clears it in the others (002-23755, 5.7.1). */
static int32_t S28HS_Complete(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Vreg, uint32_t Timeout)
{
  int32_t ret = S28HS_PollDie(Ctx, Mode, Vreg, Timeout);
  if (ret == S28HS512T_OK && S28HS_GetLayout(Ctx)->Dice > 1U)
  {
    ret = S28HS_SimpleCmd(Ctx, Mode, S28HS_CMD_WRITE_DISABLE, S28HS_DTR_CMD_WRITE_DISABLE);
  }
  return ret;
}

static int32_t S28HS512T_Enter4ByteAddressMode(XSPI_HandleTypeDef *Ctx)
{
  /* EN4BA needs no write enable and switches every die of a stacked part at once */
  return S28HS_SimpleCmd(Ctx, EXTMEM_MODE_SPI, S28HS_CMD_ENTER_4BYTE_ADDR, 0U);
}

int32_t S28HS512T_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles)
{
  const S28HS512T_DieLayout_t *layout = S28HS_GetLayout(Ctx);

  /* Memory latency is fixed at MEMLAT = 0xB (24 cycles), valid for every clock up to 200 MHz */
  (void)DummyCycles;

  /* 1. Factory default is 3-byte addressing: switch to 4-byte before any 32-bit register access */
  if (S28HS512T_Enter4ByteAddressMode(Ctx) != S28HS512T_OK) return S28HS512T_ERROR;

  /* Each die of a stacked part has its own volatile registers: configure them all (die 0 last,
   * so the register accesses of the other dice are still sent in 1S-1S-1S) */
  for (uint32_t n = layout->Dice; n > 0U; n--)
  {
    uint32_t vreg = layout->VregBase[n - 1U];
    uint8_t cfr2 = 0, cfr3 = 0;

    /* 2. Memory array read latency in CFR2V[3:0], keep 4-byte addressing */
    if (S28HS512T_ReadAnyReg(Ctx, EXTMEM_MODE_SPI, vreg + S28HS_REG_OFS_CFR2, &cfr2) != S28HS512T_OK) return S28HS512T_ERROR;
    cfr2 = (uint8_t)((cfr2 & ~S28HS_CFR2V_MEMLAT_MASK) | S28HS_CFR2V_MEMLAT_24_CYCLES | S28HS_CFR2V_ADRBYT_4BYTE);
    if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_SPI) != S28HS512T_OK) return S28HS512T_ERROR;
    if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_SPI, vreg + S28HS_REG_OFS_CFR2, cfr2) != S28HS512T_OK) return S28HS512T_ERROR;

    /* 3. Volatile register read latency for 8D-8D-8D at full speed: VRGLAT = 11 (6 cycles) in CFR3V[7:6].
     *    The factory value (3 cycles) is only valid up to 25 MHz in DDR. */
    if (S28HS512T_ReadAnyReg(Ctx, EXTMEM_MODE_SPI, vreg + S28HS_REG_OFS_CFR3, &cfr3) != S28HS512T_OK) return S28HS512T_ERROR;
    cfr3 = (uint8_t)((cfr3 & ~S28HS_CFR3V_VRGLAT_MASK) | S28HS_CFR3V_VRGLAT_CODE_11);
    if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_SPI) != S28HS512T_OK) return S28HS512T_ERROR;
    if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_SPI, vreg + S28HS_REG_OFS_CFR3, cfr3) != S28HS512T_OK) return S28HS512T_ERROR;

    /* 4. Switch the interface of this die to 8D-8D-8D through CFR5V */
    if (S28HS512T_WriteEnable(Ctx, EXTMEM_MODE_SPI) != S28HS512T_OK) return S28HS512T_ERROR;
    if (S28HS512T_WriteAnyReg(Ctx, EXTMEM_MODE_SPI, vreg + S28HS_REG_OFS_CFR5, S28HS_CFR5V_OCTAL_DTR) != S28HS512T_OK) return S28HS512T_ERROR;
  }

  /* Every die is now in 8D-8D-8D: clear the WRPGEN still set in the dice not written last */
  if (layout->Dice > 1U)
  {
    return S28HS_SimpleCmd(Ctx, EXTMEM_MODE_OCTAL_DTR, S28HS_CMD_WRITE_DISABLE, S28HS_DTR_CMD_WRITE_DISABLE);
  }
  return S28HS512T_OK;
}

int32_t S28HS512T_ExitOctalDTRMode(XSPI_HandleTypeDef *Ctx)
{
  /* An 8D software reset reloads every volatile register (interface, MEMLAT, VRGLAT, address length)
   * from the non-volatile defaults, so the device is back in 1S-1S-1S with factory latencies.
   * Reset enable and reset act on every die in parallel. */
  if (S28HS_SimpleCmd(Ctx, EXTMEM_MODE_OCTAL_DTR, 0U, S28HS_DTR_CMD_RESET_ENABLE) != S28HS512T_OK) return S28HS512T_ERROR;
  if (S28HS_SimpleCmd(Ctx, EXTMEM_MODE_OCTAL_DTR, 0U, S28HS_DTR_CMD_RESET) != S28HS512T_OK) return S28HS512T_ERROR;

  HAL_Delay(1); /* tSR = 83 us */
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

  return S28HS_Complete(Ctx, Mode, S28HS_VregForAddress(Ctx, Address), S28HS_TIMEOUT_PAGE_PROG_MS);
}

/* Only for the hybrid 4 KB parameter sectors (CFR3V[3] = 0): with the factory uniform architecture
 * the device ignores ER004 without setting an error (002-23755, 5.9.1) */
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

  return S28HS_Complete(Ctx, Mode, S28HS_VregForAddress(Ctx, Address), S28HS_TIMEOUT_ERASE_4K_MS);
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

  return S28HS_Complete(Ctx, Mode, S28HS_VregForAddress(Ctx, Address), S28HS_TIMEOUT_ERASE_256K_MS);
}

/* Bulk erase (single die) or ERCHP_4_0 at DieAddress: stacked dice do not support 60h/C7h (002-23755, 3) */
static int32_t S28HS_EraseUnit(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, bool Die, uint32_t DieAddress, uint32_t Vreg)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  if (S28HS512T_WriteEnable(Ctx, Mode) != S28HS512T_OK) return S28HS512T_ERROR;

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_NONE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_32_BITS;
  sCmd.Address            = DieAddress;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_NONE;
  sCmd.DummyCycles        = 0;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (Mode == EXTMEM_MODE_OCTAL_DTR)
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_8_LINES;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_16_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
    sCmd.Instruction        = Die ? S28HS_DTR_CMD_DIE_ERASE : S28HS_DTR_CMD_CHIP_ERASE;
    if (Die)
    {
      sCmd.AddressMode    = HAL_XSPI_ADDRESS_8_LINES;
      sCmd.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
    }
  }
  else
  {
    sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
    sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
    sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
    sCmd.Instruction        = Die ? S28HS_CMD_DIE_ERASE : S28HS_CMD_CHIP_ERASE;
    if (Die)
    {
      sCmd.AddressMode    = HAL_XSPI_ADDRESS_1_LINE;
      sCmd.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;
    }
  }

  if (HAL_XSPI_Command(Ctx, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return S28HS512T_ERROR;
  }

  return S28HS_Complete(Ctx, Mode, Vreg, S28HS_TIMEOUT_CHIP_ERASE_MS);
}

int32_t S28HS512T_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  const S28HS512T_DieLayout_t *layout = S28HS_GetLayout(Ctx);

  if (layout->Dice == 1U)
  {
    return S28HS_EraseUnit(Ctx, Mode, false, 0U, layout->VregBase[0]);
  }
  for (uint32_t die = 0; die < layout->Dice; die++)
  {
    int32_t ret = S28HS_EraseUnit(Ctx, Mode, true, die * layout->DieSize, layout->VregBase[die]);
    if (ret != S28HS512T_OK) return ret;
  }
  return S28HS512T_OK;
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
  if (S28HS_SimpleCmd(Ctx, EXTMEM_MODE_SPI, S28HS_CMD_RESET_ENABLE, 0U) != S28HS512T_OK) return S28HS512T_ERROR;
  if (S28HS_SimpleCmd(Ctx, EXTMEM_MODE_SPI, S28HS_CMD_RESET, 0U) != S28HS512T_OK) return S28HS512T_ERROR;

  HAL_Delay(1); /* tSR = 83 us */
  return S28HS512T_OK;
}

int32_t S28HS512T_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  /* ENDPD acts on every die; the configuration is kept, so the device leaves DPD in the same mode */
  if (S28HS_SimpleCmd(Ctx, Mode, S28HS_CMD_ENTER_DEEP_POWER_DOWN, S28HS_DTR_CMD_ENTER_DEEP_POWER_DOWN) != S28HS512T_OK)
  {
    return S28HS512T_ERROR;
  }
  HAL_Delay(S28HS_DPD_ENTER_MS); /* tENTDPD */
  return S28HS512T_OK;
}

int32_t S28HS512T_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode)
{
  /* A CS# pulse of at most tCSDPD = 3 us wakes the device: a one-instruction command provides it.
   * The device ignores the instruction itself; WRDIS is harmless if it was already awake. */
  if (S28HS_SimpleCmd(Ctx, Mode, S28HS_CMD_WRITE_DISABLE, S28HS_DTR_CMD_WRITE_DISABLE) != S28HS512T_OK)
  {
    return S28HS512T_ERROR;
  }
  HAL_Delay(S28HS_DPD_EXIT_MS); /* tEXTDPD */
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
