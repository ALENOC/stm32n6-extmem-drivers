/**
  ******************************************************************************
  * @file    mock_hal.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Mock Hardware Abstraction Layer implementation for host unit testing.
  *
  *          The mock records every HAL call and emulates the memory devices at
  *          the command level: opcodes are decoded from the instruction phase
  *          (the first byte of a 16-bit 8D-8D-8D instruction), write enable
  *          latch, 4-byte address mode, status / configuration registers,
  *          HyperBus register space and the HyperFlash / parallel NOR command
  *          state machines are modelled so tests can check protocol behaviour
  *          and data integrity together.
  ******************************************************************************
  */

#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>

#define MOCK_MEM_BUFFER_SIZE  (256U * 1024U) /* 256 KB virtual array */
#define MOCK_LOG_SIZE         8192U
#define MOCK_ANYREG_SIZE      16U

static uint8_t s_MemoryBuffer[MOCK_MEM_BUFFER_SIZE];

static SCB_Type s_SCB = { .CCR = SCB_CCR_DC_Msk };
SCB_Type *SCB = &s_SCB;

/* Event log */
static MockEvent_t s_Log[MOCK_LOG_SIZE];
static uint32_t    s_LogCount;

/* Fault injection */
static int32_t  s_FailIndex = -1;
static uint32_t s_CallCount;
static bool     s_FaultTriggered;

/* Serial device state */
static uint8_t  s_MfgID;
static uint8_t  s_MemTypeID;
static uint8_t  s_DensityID;
static uint8_t  s_SR1;
static uint8_t  s_CR1;
static uint8_t  s_FSR;
static uint8_t  s_AnyRegV[MOCK_ANYREG_SIZE];   /* 0x00800000 + n */
static uint8_t  s_AnyRegN[MOCK_ANYREG_SIZE];   /* 0x00000000 + n */
static uint8_t  s_VCR[16];
static uint8_t  s_MR[16];
static uint8_t  s_SramMode;
static bool     s_FourByte;
static bool     s_ResetEnabled;
static bool     s_NorFlash;
static uint32_t s_BlockEraseSize;
static bool     s_PollTimeout;
static const uint8_t *s_SfdpTable;
static uint32_t s_SfdpSize;

/* HyperBus state */
static uint16_t s_HyperID0;
static uint16_t s_HyperID1;
static uint16_t s_HyperCR0;
static uint16_t s_HyperCR1;
static bool     s_HyperFlash;
static uint32_t s_HfState;
static bool     s_HfStatusPending;
static bool     s_HfCfi;
static uint16_t s_HfStatus;

static uint32_t s_HclkHz = 200000000U;

/* Memory-mapped RAM fault */
static uint32_t s_StuckOffset;
static uint8_t  s_StuckMask;

/* Parallel NOR (FMC) state */
static uint32_t s_NorState;
static bool     s_NorAutoselect;
static bool     s_NorBusy;
static bool     s_NorDq5;
static uint32_t s_NorBusyReads;

/* Current command context */
static XSPI_RegularCmdTypeDef  s_LastCmd;
static XSPI_HyperbusCmdTypeDef s_LastHyperCmd;
static bool                    s_IsHyperbus;

/* Default SFDP image: header, JEDEC parameter header and a 16 DWORD BFPT at 0x30 */
static uint8_t s_DefaultSfdp[0x30 + 64];

static void Mock_BuildDefaultSfdp(void)
{
  memset(s_DefaultSfdp, 0, sizeof(s_DefaultSfdp));
  s_DefaultSfdp[0] = 'S'; s_DefaultSfdp[1] = 'F'; s_DefaultSfdp[2] = 'D'; s_DefaultSfdp[3] = 'P';
  s_DefaultSfdp[4] = 0x06; s_DefaultSfdp[5] = 0x01; s_DefaultSfdp[6] = 0x00; s_DefaultSfdp[7] = 0xFF;
  /* Parameter header: ID LSB 0x00, rev 1.6, 16 DWORDs, pointer 0x000030, ID MSB 0xFF */
  s_DefaultSfdp[8]  = 0x00; s_DefaultSfdp[9]  = 0x06; s_DefaultSfdp[10] = 0x01; s_DefaultSfdp[11] = 16;
  s_DefaultSfdp[12] = 0x30; s_DefaultSfdp[13] = 0x00; s_DefaultSfdp[14] = 0x00; s_DefaultSfdp[15] = 0xFF;

  uint32_t bfpt[16] = {0};
  /* DWORD1: 4 KB erase (01b), 4 KB opcode 0x20, 3 or 4-byte addressing (01b at [18:17]),
   *         1-1-4 (bit 22) and 1-4-4 (bit 21) fast read supported */
  bfpt[0] = 0x01U | (0x20U << 8) | (0x1U << 17) | (1U << 21) | (1U << 22);
  /* DWORD2: 512 Mbit = 2^29 bits */
  bfpt[1] = 0x80000000U | 29U;
  /* DWORD3: 1-4-4: 4 wait states, 2 mode clocks, opcode 0xEB; 1-1-4: 8 wait states, 0 mode, opcode 0x6B */
  bfpt[2] = 0x04U | (0x2U << 5) | (0xEBU << 8) | (0x08U << 16) | (0x0U << 21) | (0x6BU << 24);
  /* DWORD8: type 1 = 4 KB / 0x20, type 2 = 64 KB / 0xD8 */
  bfpt[7] = 12U | (0x20U << 8) | (16U << 16) | (0xD8U << 24);
  /* DWORD9: type 3 = 32 KB / 0x52, type 4 unused */
  bfpt[8] = 15U | (0x52U << 8);
  /* DWORD11: page size 2^8 */
  bfpt[10] = (8U << 4);
  memcpy(&s_DefaultSfdp[0x30], bfpt, sizeof(bfpt));
}

/* ===================================================================== */
/* Internal helpers                                                       */
/* ===================================================================== */

static MockEvent_t *Mock_NewEvent(MockEventType_t type)
{
  static MockEvent_t overflow;
  MockEvent_t *ev = (s_LogCount < MOCK_LOG_SIZE) ? &s_Log[s_LogCount++] : &overflow;
  memset(ev, 0, sizeof(*ev));
  ev->Type = type;
  return ev;
}

static bool Mock_ShouldFail(void)
{
  bool fail = ((s_FailIndex >= 0) && (s_CallCount == (uint32_t)s_FailIndex));
  s_CallCount++;
  if (fail)
  {
    s_FaultTriggered = true;
  }
  return fail;
}

static uint8_t Mock_Opcode(const XSPI_RegularCmdTypeDef *pCmd)
{
  if (pCmd->InstructionWidth == HAL_XSPI_INSTRUCTION_16_BITS)
  {
    return (uint8_t)((pCmd->Instruction >> 8) & 0xFFU);
  }
  return (uint8_t)(pCmd->Instruction & 0xFFU);
}

static uint32_t Mock_Offset(uint32_t address)
{
  return address % MOCK_MEM_BUFFER_SIZE;
}

static void Mock_Fill(uint32_t start, uint32_t len, uint8_t value)
{
  uint32_t offset = Mock_Offset(start);
  if (len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE;
  if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
  memset(&s_MemoryBuffer[offset], value, len);
}

static void Mock_Store(uint32_t address, const uint8_t *pData, uint32_t len, bool norAnd)
{
  for (uint32_t i = 0; i < len; i++)
  {
    uint32_t offset = Mock_Offset(address + i);
    s_MemoryBuffer[offset] = norAnd ? (uint8_t)(s_MemoryBuffer[offset] & pData[i]) : pData[i];
  }
}

static void Mock_Load(uint32_t address, uint8_t *pData, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++)
  {
    pData[i] = s_MemoryBuffer[Mock_Offset(address + i)];
  }
}

static void Mock_CopyToEvent(MockEvent_t *ev, const uint8_t *pData, uint32_t len)
{
  ev->DataLen = len;
  memcpy(ev->Data, pData, (len < MOCK_EVENT_DATA_BYTES) ? len : MOCK_EVENT_DATA_BYTES);
}

static bool Mock_IsMicron(void)
{
  return (s_MfgID == 0x2CU) || (s_MfgID == 0x20U);
}

static uint8_t *Mock_AnyRegSlot(uint32_t addr)
{
  if (addr >= 0x00800000U && addr < (0x00800000U + MOCK_ANYREG_SIZE)) return &s_AnyRegV[addr - 0x00800000U];
  if (addr < MOCK_ANYREG_SIZE) return &s_AnyRegN[addr];
  return NULL;
}

/* ===================================================================== */
/* Mock control API                                                       */
/* ===================================================================== */

void MockHAL_Reset(void)
{
  memset(s_MemoryBuffer, 0xFF, MOCK_MEM_BUFFER_SIZE);
  s_LogCount      = 0;
  s_FailIndex     = -1;
  s_CallCount     = 0;
  s_FaultTriggered = false;
  s_MfgID         = 0x34;
  s_MemTypeID     = 0x5B;
  s_DensityID     = 0x1A;
  s_SR1           = 0x00;
  s_CR1           = 0x00;
  s_FSR           = 0x80;
  memset(s_AnyRegV, 0, sizeof(s_AnyRegV));
  memset(s_AnyRegN, 0, sizeof(s_AnyRegN));
  s_AnyRegV[3]    = 0x08; /* CFR2V default MEMLAT = 8 */
  s_AnyRegV[6]    = 0x40; /* CFR5V default: SPI, bit 6 set */
  memset(s_VCR, 0xFF, sizeof(s_VCR));
  memset(s_MR, 0, sizeof(s_MR));
  s_SramMode      = 0x40;
  s_FourByte      = false;
  s_ResetEnabled  = false;
  s_NorFlash      = false;
  s_BlockEraseSize = 64U * 1024U;
  s_PollTimeout   = false;
  Mock_BuildDefaultSfdp();
  s_SfdpTable     = s_DefaultSfdp;
  s_SfdpSize      = sizeof(s_DefaultSfdp);
  s_HyperID0      = 0x0C81; /* Infineon HyperRAM: manufacturer 0001b */
  s_HyperID1      = 0x0000;
  s_HyperCR0      = 0x8F1F; /* Datasheet power-on default */
  s_HyperCR1      = 0xFFC1;
  s_HyperFlash    = false;
  s_HfState       = 0;
  s_HfStatusPending = false;
  s_HfCfi         = false;
  s_HfStatus      = 0x0080;
  s_HclkHz        = 200000000U;
  s_StuckOffset   = 0;
  s_StuckMask     = 0;
  s_NorState      = 0;
  s_NorAutoselect = false;
  s_NorBusy       = false;
  s_NorDq5        = false;
  s_NorBusyReads  = 0;
  s_IsHyperbus    = false;
  memset(&s_LastCmd, 0, sizeof(s_LastCmd));
  memset(&s_LastHyperCmd, 0, sizeof(s_LastHyperCmd));
  s_SCB.CCR       = SCB_CCR_DC_Msk;
}

void MockHAL_SetEmulatedChip(uint8_t mfg, uint8_t memType, uint8_t density)
{
  s_MfgID     = mfg;
  s_MemTypeID = memType;
  s_DensityID = density;
}

void MockHAL_SetHyperBusID(uint16_t id0, uint16_t id1)
{
  s_HyperID0 = id0;
  s_HyperID1 = id1;
}

void MockHAL_SetFlashSemantics(bool norFlash)    { s_NorFlash = norFlash; }
void MockHAL_SetBlockEraseSize(uint32_t bytes)   { s_BlockEraseSize = bytes; }
void MockHAL_SetHyperFlashMode(bool enable)      { s_HyperFlash = enable; }
void MockHAL_SetStatusRegister(uint8_t sr1)      { s_SR1 = sr1; }
void MockHAL_SetFlagStatusRegister(uint8_t fsr)  { s_FSR = fsr; }
void MockHAL_SetPollTimeout(bool timeout)        { s_PollTimeout = timeout; }
void MockHAL_SetHyperFlashStatus(uint16_t st)    { s_HfStatus = st; }
void MockHAL_SetFmcNorStuck(bool busy, bool dq5) { s_NorBusy = busy; s_NorDq5 = dq5; s_NorBusyReads = 0; }
void MockHAL_SetFmcNorBusyReads(uint32_t reads)  { s_NorBusyReads = reads; }

void MockHAL_SetSfdpTable(const uint8_t *pTable, uint32_t size)
{
  if (pTable == NULL)
  {
    s_SfdpTable = s_DefaultSfdp;
    s_SfdpSize  = sizeof(s_DefaultSfdp);
  }
  else
  {
    s_SfdpTable = pTable;
    s_SfdpSize  = size;
  }
}

uint8_t  MockHAL_GetStatusRegister(void)   { return s_SR1; }
uint8_t  MockHAL_GetConfigRegister1(void)  { return s_CR1; }
uint8_t  MockHAL_GetVCR(uint32_t addr)     { return (addr < sizeof(s_VCR)) ? s_VCR[addr] : 0U; }
uint8_t  MockHAL_GetMR(uint32_t addr)      { return (addr < sizeof(s_MR)) ? s_MR[addr] : 0U; }
bool     MockHAL_Is4ByteMode(void)         { return s_FourByte; }
uint8_t  MockHAL_GetSramModeRegister(void) { return s_SramMode; }

uint8_t MockHAL_GetAnyReg(uint32_t addr)
{
  const uint8_t *slot = Mock_AnyRegSlot(addr);
  return (slot != NULL) ? *slot : 0U;
}

uint16_t MockHAL_GetHyperReg(uint32_t addr)
{
  switch (addr)
  {
    case 0x00000000U: return s_HyperID0;
    case 0x00000002U: return s_HyperID1;
    case 0x00001000U: return s_HyperCR0;
    case 0x00001002U: return s_HyperCR1;
    default:          return 0U;
  }
}

void MockHAL_FailCall(int32_t index)
{
  s_FailIndex      = index;
  s_CallCount      = 0;
  s_FaultTriggered = false;
}

void MockHAL_FailNone(void)
{
  MockHAL_FailCall(-1);
}

uint32_t MockHAL_GetCallCount(void)  { return s_CallCount; }
bool     MockHAL_FaultTriggered(void) { return s_FaultTriggered; }

void MockHAL_ClearLog(void)
{
  s_LogCount = 0;
}

uint32_t MockHAL_GetEventCount(void)
{
  return s_LogCount;
}

const MockEvent_t *MockHAL_GetEvent(uint32_t index)
{
  return (index < s_LogCount) ? &s_Log[index] : NULL;
}

uint32_t MockHAL_CountCommands(uint32_t instruction)
{
  uint32_t n = 0;
  for (uint32_t i = 0; i < s_LogCount; i++)
  {
    if (s_Log[i].Type == MOCK_EV_CMD && s_Log[i].Cmd.Instruction == instruction) n++;
  }
  return n;
}

const MockEvent_t *MockHAL_FindCommand(uint32_t instruction, uint32_t nth)
{
  for (uint32_t i = 0; i < s_LogCount; i++)
  {
    if (s_Log[i].Type == MOCK_EV_CMD && s_Log[i].Cmd.Instruction == instruction)
    {
      if (nth == 0) return &s_Log[i];
      nth--;
    }
  }
  return NULL;
}

const MockEvent_t *MockHAL_LastCommand(void)
{
  for (uint32_t i = s_LogCount; i > 0; i--)
  {
    if (s_Log[i - 1].Type == MOCK_EV_CMD) return &s_Log[i - 1];
  }
  return NULL;
}

const MockEvent_t *MockHAL_FindEvent(MockEventType_t type, uint32_t nth)
{
  for (uint32_t i = 0; i < s_LogCount; i++)
  {
    if (s_Log[i].Type == type)
    {
      if (nth == 0) return &s_Log[i];
      nth--;
    }
  }
  return NULL;
}

const MockEvent_t *MockHAL_FindTxAfterCommand(uint32_t instruction, uint32_t nth)
{
  for (uint32_t i = 0; i < s_LogCount; i++)
  {
    if (s_Log[i].Type == MOCK_EV_CMD && s_Log[i].Cmd.Instruction == instruction)
    {
      if (nth == 0)
      {
        for (uint32_t j = i + 1; j < s_LogCount; j++)
        {
          if (s_Log[j].Type == MOCK_EV_TX || s_Log[j].Type == MOCK_EV_TX_DMA) return &s_Log[j];
          if (s_Log[j].Type == MOCK_EV_CMD) return NULL;
        }
        return NULL;
      }
      nth--;
    }
  }
  return NULL;
}

uint8_t* MockHAL_GetMemoryBuffer(void)
{
  return s_MemoryBuffer;
}

uint32_t MockHAL_GetMemoryBufferSize(void)
{
  return MOCK_MEM_BUFFER_SIZE;
}

/* ===================================================================== */
/* XSPI HAL                                                               */
/* ===================================================================== */

HAL_StatusTypeDef HAL_XSPI_Init(XSPI_HandleTypeDef *hxspi)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_XSPI_INIT);
  ev->Init = hxspi->Init;
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_DeInit(XSPI_HandleTypeDef *hxspi)
{
  (void)hxspi;
  (void)Mock_NewEvent(MOCK_EV_XSPI_DEINIT);
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_XSPIM_Config(XSPI_HandleTypeDef *hxspi, const XSPIM_CfgTypeDef *pCfg, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_XSPIM_CONFIG);
  ev->Xspim = *pCfg;
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

static void Mock_EraseOrProgramDone(void)
{
  s_SR1 = (uint8_t)((s_SR1 | 0x01U) & ~0x02U); /* WIP set, WEL cleared */
}

HAL_StatusTypeDef HAL_XSPI_Command(XSPI_HandleTypeDef *hxspi, const XSPI_RegularCmdTypeDef *pCmd, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_CMD);
  ev->Cmd = *pCmd;
  if (Mock_ShouldFail()) return HAL_ERROR;

  s_IsHyperbus = false;
  s_LastCmd = *pCmd;

  /* Memory-mapped configuration only latches the command */
  if (pCmd->OperationType != HAL_XSPI_OPTYPE_COMMON_CFG)
  {
    return HAL_OK;
  }

  uint8_t op = Mock_Opcode(pCmd);
  bool hasAddr = (pCmd->AddressMode != HAL_XSPI_ADDRESS_NONE);
  bool hasData = (pCmd->DataMode != HAL_XSPI_DATA_NONE);
  bool wel = ((s_SR1 & 0x02U) != 0U);

  if (op != 0x99U) s_ResetEnabled = false;

  switch (op)
  {
    case 0x06: /* Write Enable */
      s_SR1 |= 0x02U;
      break;
    case 0x04: /* Write Disable */
      s_SR1 &= (uint8_t)~0x02U;
      break;
    case 0xB7: /* Enter 4-byte address mode (Micron requires WEL) */
      if (!Mock_IsMicron() || wel) s_FourByte = true;
      if (Mock_IsMicron()) s_SR1 &= (uint8_t)~0x02U;
      break;
    case 0xE9: /* Exit 4-byte address mode */
      if (!Mock_IsMicron() || wel) s_FourByte = false;
      if (Mock_IsMicron()) s_SR1 &= (uint8_t)~0x02U;
      break;
    case 0x66:
      s_ResetEnabled = true;
      break;
    case 0x99:
      if (s_ResetEnabled)
      {
        s_FourByte = false;
        s_SR1 &= (uint8_t)~0x03U;
      }
      s_ResetEnabled = false;
      break;
    case 0x20: case 0x21: /* 4 KB erase */
      if (hasAddr && !hasData && (!s_NorFlash || wel))
      {
        Mock_Fill(pCmd->Address & ~0xFFFU, 4096U, 0xFF);
        Mock_EraseOrProgramDone();
      }
      break;
    case 0xD8: case 0xDC: /* Block / sector erase */
      if (hasAddr && !hasData && (!s_NorFlash || wel))
      {
        Mock_Fill(pCmd->Address & ~(s_BlockEraseSize - 1U), s_BlockEraseSize, 0xFF);
        Mock_EraseOrProgramDone();
      }
      break;
    case 0x60: case 0xC7: /* Bulk erase */
      if (!hasAddr && !hasData && (!s_NorFlash || wel))
      {
        Mock_Fill(0, MOCK_MEM_BUFFER_SIZE, 0xFF);
        Mock_EraseOrProgramDone();
      }
      break;
    case 0xC4: /* Die erase */
      if (hasAddr && !hasData && (!s_NorFlash || wel))
      {
        Mock_Fill(0, MOCK_MEM_BUFFER_SIZE, 0xFF);
        Mock_EraseOrProgramDone();
      }
      break;
    default:
      break;
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Transmit(XSPI_HandleTypeDef *hxspi, const uint8_t *pData, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_TX);
  ev->Cmd = s_LastCmd;
  ev->HCmd = s_LastHyperCmd;
  ev->Hyperbus = s_IsHyperbus;
  Mock_CopyToEvent(ev, pData, s_IsHyperbus ? s_LastHyperCmd.DataLength : s_LastCmd.DataLength);
  if (Mock_ShouldFail()) return HAL_ERROR;

  if (s_IsHyperbus)
  {
    uint32_t len  = s_LastHyperCmd.DataLength;
    uint32_t addr = s_LastHyperCmd.Address;

    if (s_LastHyperCmd.AddressSpace == HAL_XSPI_REGISTER_ADDRESS_SPACE)
    {
      /* HyperBus register words travel most significant byte first */
      uint16_t val = (uint16_t)(((uint16_t)pData[0] << 8) | pData[1]);
      if (addr == 0x00001000U) s_HyperCR0 = val;
      else if (addr == 0x00001002U) s_HyperCR1 = val;
      return HAL_OK;
    }

    if (!s_HyperFlash)
    {
      Mock_Store(addr, pData, len, false);
      return HAL_OK;
    }

    /* HyperFlash command decoder (word commands, MSB first) */
    uint16_t w = (len >= 2U) ? (uint16_t)(((uint16_t)pData[0] << 8) | pData[1]) : pData[0];
    switch (s_HfState)
    {
      case 0:
        if (addr == 0x0AAAU && w == 0x00AAU) s_HfState = 1;
        else if (addr == 0x0AAAU && w == 0x0070U) s_HfStatusPending = true;
        else if (addr == 0x0AAAU && w == 0x0071U) s_HfStatus = 0x0080;
        else if (addr == 0x0AAAU && w == 0x0098U) s_HfCfi = true;
        else if (w == 0x00F0U) s_HfCfi = false;
        break;
      case 1:
        s_HfState = (addr == 0x0554U && w == 0x0055U) ? 2U : 0U;
        break;
      case 2:
        if (addr == 0x0AAAU && w == 0x00A0U) s_HfState = 3;
        else if (addr == 0x0AAAU && w == 0x0080U) s_HfState = 4;
        else s_HfState = 0;
        break;
      case 3: /* Word program: raw bytes in memory order, NOR AND semantics */
        Mock_Store(addr, pData, len, true);
        s_HfState = 0;
        break;
      case 4:
        s_HfState = (addr == 0x0AAAU && w == 0x00AAU) ? 5U : 0U;
        break;
      case 5:
        s_HfState = (addr == 0x0554U && w == 0x0055U) ? 6U : 0U;
        break;
      case 6:
        if (w == 0x0030U) Mock_Fill(addr & ~(256U * 1024U - 1U), 256U * 1024U, 0xFF);
        else if (addr == 0x0AAAU && w == 0x0010U) Mock_Fill(0, MOCK_MEM_BUFFER_SIZE, 0xFF);
        s_HfState = 0;
        break;
      default:
        s_HfState = 0;
        break;
    }
    return HAL_OK;
  }

  uint8_t  op   = Mock_Opcode(&s_LastCmd);
  uint32_t len  = s_LastCmd.DataLength;
  uint32_t addr = s_LastCmd.Address;
  bool     wel  = ((s_SR1 & 0x02U) != 0U);

  switch (op)
  {
    case 0x71: /* Write Any Register (SEMPER) */
    {
      uint8_t *slot = Mock_AnyRegSlot(addr);
      if (wel && slot != NULL) *slot = pData[0];
      s_SR1 &= (uint8_t)~0x02U;
      break;
    }
    case 0x81: /* Write Volatile Configuration Register (Xccela) */
      if (!s_NorFlash || wel)
      {
        for (uint32_t i = 0; i < len; i++)
        {
          if ((addr + i) < sizeof(s_VCR)) s_VCR[addr + i] = pData[i];
        }
      }
      s_SR1 &= (uint8_t)~0x02U;
      break;
    case 0xC0: /* Write Mode Register (APMEM) */
      if (addr < sizeof(s_MR)) s_MR[addr] = pData[0];
      break;
    case 0x01: /* Write Status/Config (flash) or Write Mode Register (serial SRAM) */
      if (s_NorFlash)
      {
        if (wel)
        {
          s_SR1 = (uint8_t)(pData[0] & ~0x03U);
          if (len > 1U) s_CR1 = pData[1];
        }
        else
        {
          s_SR1 &= (uint8_t)~0x02U;
        }
      }
      else
      {
        s_SramMode = pData[0];
      }
      break;
    default: /* Program / RAM write */
      if (!s_NorFlash)
      {
        Mock_Store(addr, pData, len, false);
      }
      else if (wel)
      {
        Mock_Store(addr, pData, len, true);
        Mock_EraseOrProgramDone();
      }
      break;
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Receive(XSPI_HandleTypeDef *hxspi, uint8_t *pData, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_RX);
  ev->Cmd = s_LastCmd;
  ev->HCmd = s_LastHyperCmd;
  ev->Hyperbus = s_IsHyperbus;
  if (Mock_ShouldFail()) return HAL_ERROR;

  if (s_IsHyperbus)
  {
    uint32_t len  = s_LastHyperCmd.DataLength;
    uint32_t addr = s_LastHyperCmd.Address;

    if (s_LastHyperCmd.AddressSpace == HAL_XSPI_REGISTER_ADDRESS_SPACE)
    {
      uint16_t val = MockHAL_GetHyperReg(addr);
      pData[0] = (uint8_t)(val >> 8);
      pData[1] = (uint8_t)(val & 0xFFU);
    }
    else if (s_HyperFlash && s_HfStatusPending)
    {
      pData[0] = (uint8_t)(s_HfStatus >> 8);
      pData[1] = (uint8_t)(s_HfStatus & 0xFFU);
      s_HfStatusPending = false;
    }
    else if (s_HyperFlash && s_HfCfi)
    {
      uint32_t word = addr / 2U;
      uint16_t val = (word == 0x10U) ? 0x0051U : (word == 0x11U) ? 0x0052U : (word == 0x12U) ? 0x0059U : 0x0000U;
      pData[0] = (uint8_t)(val >> 8);
      pData[1] = (uint8_t)(val & 0xFFU);
    }
    else
    {
      Mock_Load(addr, pData, len);
    }
    Mock_CopyToEvent(ev, pData, len);
    return HAL_OK;
  }

  uint8_t  op   = Mock_Opcode(&s_LastCmd);
  uint32_t len  = s_LastCmd.DataLength;
  uint32_t addr = s_LastCmd.Address;

  memset(pData, 0, len);
  switch (op)
  {
    case 0x9F: /* Read JEDEC ID */
      if (len > 0U) pData[0] = s_MfgID;
      if (len > 1U) pData[1] = s_MemTypeID;
      if (len > 2U) pData[2] = s_DensityID;
      break;
    case 0x5A: /* Read SFDP */
      for (uint32_t i = 0; i < len; i++)
      {
        pData[i] = ((addr + i) < s_SfdpSize) ? s_SfdpTable[addr + i] : 0xFFU;
      }
      break;
    case 0x05: /* Read Status Register 1 (flash) or Read Mode Register (serial SRAM) */
      pData[0] = s_NorFlash ? s_SR1 : s_SramMode;
      if (len > 1U) pData[1] = pData[0];
      break;
    case 0x70: /* Read Flag Status Register */
      pData[0] = s_FSR;
      if (len > 1U) pData[1] = s_FSR;
      break;
    case 0x35: /* Read Configuration Register 1 */
      pData[0] = s_CR1;
      break;
    case 0x65: /* Read Any Register */
    {
      const uint8_t *slot = Mock_AnyRegSlot(addr);
      pData[0] = (addr == 0x00800000U) ? s_SR1 : ((slot != NULL) ? *slot : 0U);
      if (len > 1U) pData[1] = pData[0];
      break;
    }
    case 0x85: /* Read Volatile Configuration Register */
      for (uint32_t i = 0; i < len; i++)
      {
        pData[i] = ((addr + i) < sizeof(s_VCR)) ? s_VCR[addr + i] : 0U;
      }
      break;
    case 0x40: /* Read Mode Register (APMEM) */
      pData[0] = (addr < sizeof(s_MR)) ? s_MR[addr] : 0U;
      if (len > 1U) pData[1] = pData[0];
      break;
    default: /* Memory array read */
      Mock_Load(addr, pData, len);
      break;
  }
  Mock_CopyToEvent(ev, pData, len);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Transmit_DMA(XSPI_HandleTypeDef *hxspi, const uint8_t *pData)
{
  (void)Mock_NewEvent(MOCK_EV_TX_DMA);
  if (Mock_ShouldFail()) return HAL_ERROR;
  /* Reuse the blocking path without counting a second HAL call */
  int32_t saved = s_FailIndex;
  s_FailIndex = -1;
  HAL_StatusTypeDef st = HAL_XSPI_Transmit(hxspi, pData, 1000);
  s_FailIndex = saved;
  s_CallCount--;
  return st;
}

HAL_StatusTypeDef HAL_XSPI_Receive_DMA(XSPI_HandleTypeDef *hxspi, uint8_t *pData)
{
  (void)Mock_NewEvent(MOCK_EV_RX_DMA);
  if (Mock_ShouldFail()) return HAL_ERROR;
  int32_t saved = s_FailIndex;
  s_FailIndex = -1;
  HAL_StatusTypeDef st = HAL_XSPI_Receive(hxspi, pData, 1000);
  s_FailIndex = saved;
  s_CallCount--;
  return st;
}

HAL_StatusTypeDef HAL_XSPI_AutoPolling(XSPI_HandleTypeDef *hxspi, const XSPI_AutoPollingTypeDef *pCfg, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_AUTOPOLL);
  ev->Poll = *pCfg;
  ev->Cmd  = s_LastCmd;
  if (Mock_ShouldFail()) return HAL_ERROR;
  if (s_PollTimeout) return HAL_TIMEOUT;
  /* The embedded operation completes while the controller polls */
  s_SR1 &= (uint8_t)~0x01U;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_MemoryMapped(XSPI_HandleTypeDef *hxspi, const XSPI_MemoryMappedTypeDef *pCfg)
{
  (void)hxspi; (void)pCfg;
  (void)Mock_NewEvent(MOCK_EV_MEMMAPPED);
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Abort(XSPI_HandleTypeDef *hxspi)
{
  (void)hxspi;
  (void)Mock_NewEvent(MOCK_EV_ABORT);
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_HyperbusCfg(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCfgTypeDef *pCfg, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_HYPER_CFG);
  ev->HCfg = *pCfg;
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_HyperbusCmd(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCmdTypeDef *pCmd, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_HYPER_CMD);
  ev->HCmd = *pCmd;
  if (Mock_ShouldFail()) return HAL_ERROR;
  s_IsHyperbus = true;
  s_LastHyperCmd = *pCmd;
  return HAL_OK;
}

/* ===================================================================== */
/* FMC                                                                    */
/* ===================================================================== */

HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, const FMC_NORSRAM_TimingTypeDef *pTiming, const FMC_NORSRAM_TimingTypeDef *pExtTiming)
{
  (void)pExtTiming;
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_SRAM_INIT);
  ev->SramInit = hsram->Init;
  ev->Timing   = *pTiming;
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram)
{
  (void)hsram;
  (void)Mock_NewEvent(MOCK_EV_SRAM_DEINIT);
  return Mock_ShouldFail() ? HAL_ERROR : HAL_OK;
}

void MockHAL_SetRamStuckAt0(uint32_t Offset, uint8_t Mask)
{
  s_StuckOffset = Mock_Offset(Offset);
  s_StuckMask   = Mask;
}

void MockHAL_RamWrite(uint32_t Offset, const uint8_t *pData, uint32_t Size)
{
  for (uint32_t i = 0; i < Size; i++)
  {
    uint32_t off = Mock_Offset(Offset + i);
    uint8_t v = pData[i];
    if (s_StuckMask != 0U && off == s_StuckOffset) v &= (uint8_t)~s_StuckMask;
    s_MemoryBuffer[off] = v;
  }
}

void MockHAL_RamRead(uint32_t Offset, uint8_t *pData, uint32_t Size)
{
  Mock_Load(Offset, pData, Size);
}

void MockHAL_FmcWrite16(uint32_t BaseAddr, uint32_t ByteOffset, uint16_t Data)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_FMC_WRITE16);
  ev->Address = BaseAddr + ByteOffset;
  ev->Value   = Data;

  uint32_t word = ByteOffset / 2U;
  switch (s_NorState)
  {
    case 0:
      if (word == 0x555U && Data == 0x00AAU) s_NorState = 1;
      else if (Data == 0x00F0U) s_NorAutoselect = false;
      break;
    case 1:
      s_NorState = (word == 0x2AAU && Data == 0x0055U) ? 2U : 0U;
      break;
    case 2:
      if (word == 0x555U && Data == 0x00A0U) s_NorState = 3;
      else if (word == 0x555U && Data == 0x0080U) s_NorState = 4;
      else if (word == 0x555U && Data == 0x0090U) { s_NorAutoselect = true; s_NorState = 0; }
      else s_NorState = 0;
      break;
    case 3:
    {
      /* Little-endian bus: D[7:0] is the byte at the even address */
      uint8_t bytes[2] = { (uint8_t)(Data & 0xFFU), (uint8_t)(Data >> 8) };
      Mock_Store(ByteOffset & ~1U, bytes, 2, true);
      s_NorState = 0;
      break;
    }
    case 4:
      s_NorState = (word == 0x555U && Data == 0x00AAU) ? 5U : 0U;
      break;
    case 5:
      s_NorState = (word == 0x2AAU && Data == 0x0055U) ? 6U : 0U;
      break;
    case 6:
      if (Data == 0x0030U) Mock_Fill(ByteOffset & ~(s_BlockEraseSize - 1U), s_BlockEraseSize, 0xFF);
      else if (word == 0x555U && Data == 0x0010U) Mock_Fill(0, MOCK_MEM_BUFFER_SIZE, 0xFF);
      s_NorState = 0;
      break;
    default:
      s_NorState = 0;
      break;
  }
}

uint16_t MockHAL_FmcRead16(uint32_t BaseAddr, uint32_t ByteOffset)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_FMC_READ16);
  ev->Address = BaseAddr + ByteOffset;

  uint16_t val;
  if (s_NorAutoselect)
  {
    val = (ByteOffset == 0U) ? 0x009DU : (ByteOffset == 2U) ? 0x227EU : 0x0000U;
  }
  else
  {
    uint32_t off = Mock_Offset(ByteOffset & ~1U);
    val = (uint16_t)(s_MemoryBuffer[off] | ((uint16_t)s_MemoryBuffer[Mock_Offset(off + 1U)] << 8));
    bool busy = s_NorBusy;
    if (s_NorBusyReads > 0U)
    {
      busy = true;
      s_NorBusyReads--;
    }
    if (busy)
    {
      /* Status while the algorithm runs: DQ7 complement, DQ5 only when the time limit is exceeded */
      val = (uint16_t)((val ^ 0x0080U) & ~0x0020U);
      if (s_NorDq5) val |= 0x0020U;
    }
  }
  ev->Value = val;
  return val;
}

/* ===================================================================== */
/* Misc                                                                   */
/* ===================================================================== */

void SCB_CleanInvalidateDCache_by_Addr(void *addr, int32_t size)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_CACHE_MAINT);
  ev->Address = (uint32_t)(uintptr_t)addr;
  ev->Value   = (uint32_t)size;
}

void MockHAL_SetHclkFreq(uint32_t hz) { s_HclkHz = hz; }

uint32_t HAL_RCC_GetHCLKFreq(void)
{
  return s_HclkHz;
}

uint32_t HAL_GetTick(void)
{
  static uint32_t tick = 0;
  return ++tick;
}

void HAL_Delay(uint32_t Delay)
{
  (void)Delay;
}

void HAL_PWREx_EnableVddIO2(void)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_PWR_VDDIO);
  ev->Value = 0x200U | PWR_VDDIO2;
}

void HAL_PWREx_EnableVddIO3(void)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_PWR_VDDIO);
  ev->Value = 0x200U | PWR_VDDIO3;
}

void HAL_PWREx_ConfigVddIORange(uint32_t Domain, uint32_t Range)
{
  MockEvent_t *ev = Mock_NewEvent(MOCK_EV_PWR_VDDIO);
  ev->Value = (Domain << 4) | Range;
}
