/**
  ******************************************************************************
  * @file    test_hyperbus.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Protocol level tests for the HyperBus drivers:
  *          Infineon / ISSI HyperFlash (S26KS) and HyperRAM (S27KS, IS66WVH).
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

#define HF_SETUP() do { MockHAL_Reset(); MockHAL_SetHyperFlashMode(true); } while (0)
#define HR_SETUP() do { MockHAL_Reset(); } while (0)

/* Returns the n-th HyperBus transmit event */
static const MockEvent_t *HyperTx(uint32_t n)
{
  for (uint32_t i = 0; i < MockHAL_GetEventCount(); i++)
  {
    const MockEvent_t *ev = MockHAL_GetEvent(i);
    if (ev->Type == MOCK_EV_TX && ev->Hyperbus)
    {
      if (n == 0) return ev;
      n--;
    }
  }
  return NULL;
}

static bool CheckHyperWord(const MockEvent_t *ev, uint32_t addr, uint16_t word)
{
  if (ev == NULL) { printf("       [FAIL] missing HyperBus write 0x%04X @0x%lX\r\n", word, (unsigned long)addr); return false; }
  if (ev->HCmd.Address != addr || ev->HCmd.AddressSpace != HAL_XSPI_MEMORY_ADDRESS_SPACE || ev->DataLen != 2 ||
      ev->Data[0] != (uint8_t)(word >> 8) || ev->Data[1] != (uint8_t)(word & 0xFF))
  {
    printf("       [FAIL] HyperBus write @0x%lX = %02X %02X, expected @0x%lX = %02X %02X\r\n",
           (unsigned long)ev->HCmd.Address, ev->Data[0], ev->Data[1], (unsigned long)addr, word >> 8, word & 0xFF);
    return false;
  }
  return true;
}
#define ASSERT_HWORD(n, addr, word) ASSERT_TRUE(CheckHyperWord(HyperTx(n), (addr), (word)))

/* ========================================================================= */
/* HyperFlash S26KS512S / IS26KS                                             */
/* ========================================================================= */

bool test_infineon_s26ks512s_hyperflash(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[64], rx[64];
  uint16_t st = 0, cfi = 0;
  HF_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x19);

  ASSERT_EQ(S26KS512S_Init(&h, 2, HAL_XSPI_SIZE_512MB), S26KS512S_OK);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_HYPERBUS);
  ASSERT_EQ(init->Init.MemorySize, HAL_XSPI_SIZE_512MB);
  const MockEvent_t *cfg = MockHAL_FindEvent(MOCK_EV_HYPER_CFG, 0);
  ASSERT_EQ(cfg->HCfg.WriteZeroLatency, HAL_XSPI_NO_LATENCY_ON_WRITE);
  ASSERT_EQ(cfg->HCfg.LatencyMode, HAL_XSPI_FIXED_LATENCY);
  ASSERT_EQ(cfg->HCfg.AccessTimeCycle, S26KS_INITIAL_LATENCY_CYCLES);
  ASSERT_HWORD(0, 0x0000, 0x00F0); /* Reset / ASO exit */
  /* Size 0 selects the 512 Mbit default */
  HF_SETUP();
  ASSERT_EQ(S26KS512S_Init(&h, 2, 0), S26KS512S_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0)->Init.MemorySize, HAL_XSPI_SIZE_512MB);

  /* Word program: unlock cycles, then the data bytes in memory order */
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ProgramWord(&h, 0x100, 0xBEEF), S26KS512S_OK);
  ASSERT_HWORD(0, 0x0AAA, 0x00AA);
  ASSERT_HWORD(1, 0x0554, 0x0055);
  ASSERT_HWORD(2, 0x0AAA, 0x00A0);
  ASSERT_EQ(HyperTx(3)->HCmd.Address, 0x100);
  ASSERT_EQ(HyperTx(3)->Data[0], 0xEF);
  ASSERT_EQ(HyperTx(3)->Data[1], 0xBE);
  ASSERT_HWORD(4, 0x0AAA, 0x0070); /* status read command */
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x100], 0xEF);
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x101], 0xBE);

  /* Buffer program: even aligned, odd tail, odd head; neighbours stay erased */
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ProgramBuffer(&h, 0x200, tx, 64), S26KS512S_OK);
  ASSERT_EQ(S26KS512S_Read(&h, 0x200, rx, 64), S26KS512S_OK);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ProgramBuffer(&h, 0x301, tx, 5), S26KS512S_OK);
  uint8_t *mem = MockHAL_GetMemoryBuffer();
  ASSERT_EQ(mem[0x300], 0xFF);
  ASSERT_EQ(memcmp(&mem[0x301], tx, 5), 0);
  ASSERT_EQ(mem[0x306], 0xFF);
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ProgramBuffer(&h, 0x400, tx, 3), S26KS512S_OK);
  ASSERT_EQ(memcmp(&mem[0x400], tx, 3), 0);
  ASSERT_EQ(mem[0x403], 0xFF);

  /* Sector erase: 6 cycles, confirm 0x30 at the sector address */
  MockHAL_ClearLog();
  ASSERT_EQ(S26KS512S_EraseSector(&h, 0x0), S26KS512S_OK);
  ASSERT_HWORD(0, 0x0AAA, 0x00AA);
  ASSERT_HWORD(1, 0x0554, 0x0055);
  ASSERT_HWORD(2, 0x0AAA, 0x0080);
  ASSERT_HWORD(3, 0x0AAA, 0x00AA);
  ASSERT_HWORD(4, 0x0554, 0x0055);
  ASSERT_HWORD(5, 0x0000, 0x0030);
  ASSERT_EQ(mem[0x400], 0xFF);

  HF_SETUP();
  memset(mem, 0x00, 16);
  ASSERT_EQ(S26KS512S_EraseChip(&h), S26KS512S_OK);
  ASSERT_HWORD(5, 0x0AAA, 0x0010);
  ASSERT_EQ(mem[0], 0xFF);

  /* Status */
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ReadStatus(&h, &st), S26KS512S_OK);
  ASSERT_EQ(st, 0x0080);
  ASSERT_EQ(S26KS512S_ClearStatus(&h), S26KS512S_OK);
  ASSERT_HWORD(1, 0x0AAA, 0x0071);
  MockHAL_SetHyperFlashStatus(0x00A0); /* ready + erase error */
  ASSERT_EQ(S26KS512S_WaitUntilReady(&h, 10), S26KS512S_ERROR);
  MockHAL_SetHyperFlashStatus(0x0090); /* ready + program error */
  ASSERT_EQ(S26KS512S_WaitUntilReady(&h, 10), S26KS512S_ERROR);
  MockHAL_SetHyperFlashStatus(0x0000); /* busy forever */
  ASSERT_EQ(S26KS512S_WaitUntilReady(&h, 10), S26KS512S_TIMEOUT);
  MockHAL_SetHyperFlashStatus(0x0080);

  /* CFI query: 'Q' at word 0x10, followed by reset */
  HF_SETUP();
  ASSERT_EQ(S26KS512S_ReadCFI(&h, 0x10, &cfi), S26KS512S_OK);
  ASSERT_EQ(cfi, 0x0051);
  ASSERT_HWORD(0, 0x0AAA, 0x0098);
  ASSERT_HWORD(1, 0x0000, 0x00F0);

  MockHAL_ClearLog();
  ASSERT_EQ(S26KS512S_EnableMemoryMappedMode(&h), S26KS512S_OK);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_MEMMAPPED, 0));

  FAULT_SWEEP(HF_SETUP(), S26KS512S_Init(&h, 2, HAL_XSPI_SIZE_512MB));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_Read(&h, 0, rx, 8));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_ProgramWord(&h, 0, 0x1234));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_ProgramBuffer(&h, 1, tx, 4));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_EraseSector(&h, 0));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_EraseChip(&h));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_ReadStatus(&h, &st));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_ClearStatus(&h));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_ReadCFI(&h, 0x10, &cfi));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_EnableMemoryMappedMode(&h));
  FAULT_SWEEP(HF_SETUP(), S26KS512S_Reset(&h));
  return true;
}

/* ========================================================================= */
/* HyperRAM (shared checks for S27KS and IS66WVH)                            */
/* ========================================================================= */

typedef struct {
  int32_t (*Init)(XSPI_HandleTypeDef *, uint32_t, uint32_t);
  int32_t (*ReadReg)(XSPI_HandleTypeDef *, uint32_t, uint16_t *);
  int32_t (*WriteReg)(XSPI_HandleTypeDef *, uint32_t, uint16_t);
  int32_t (*Read)(XSPI_HandleTypeDef *, uint32_t, uint8_t *, uint32_t);
  int32_t (*Write)(XSPI_HandleTypeDef *, uint32_t, const uint8_t *, uint32_t);
  int32_t (*MemoryMapped)(XSPI_HandleTypeDef *);
  int32_t (*EnterDPD)(XSPI_HandleTypeDef *);
  int32_t (*LeaveDPD)(XSPI_HandleTypeDef *);
  uint16_t Cr0Init;
} HyperRamOps_t;

static bool hyperram_common(const HyperRamOps_t *ops)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[96], rx[96];
  uint16_t reg = 0;
  HR_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x2E);

  ASSERT_EQ(ops->Init(&h, 2, HAL_XSPI_SIZE_64MB), 0);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_HYPERBUS);
  ASSERT_EQ(init->Init.MemorySize, HAL_XSPI_SIZE_64MB);
  const MockEvent_t *cfg = MockHAL_FindEvent(MOCK_EV_HYPER_CFG, 0);
  ASSERT_EQ(cfg->HCfg.AccessTimeCycle, 7);
  ASSERT_EQ(cfg->HCfg.LatencyMode, HAL_XSPI_VARIABLE_LATENCY);
  ASSERT_EQ(cfg->HCfg.WriteZeroLatency, HAL_XSPI_LATENCY_ON_WRITE);

  /* CR0: deep power down disabled (bit 15 = 1), reserved [11:8] = 1, 7 clocks (0010b), variable latency,
   * legacy wrap, 32 byte burst. Sent MSB first on the bus. */
  ASSERT_EQ(ops->Cr0Init, 0x8F27);
  const MockEvent_t *crw = HyperTx(0);
  ASSERT_NOT_NULL(crw);
  ASSERT_EQ(crw->HCmd.AddressSpace, HAL_XSPI_REGISTER_ADDRESS_SPACE);
  ASSERT_EQ(crw->HCmd.Address, 0x1000);
  ASSERT_EQ(crw->Data[0], 0x8F);
  ASSERT_EQ(crw->Data[1], 0x27);
  ASSERT_EQ(MockHAL_GetHyperReg(0x1000), 0x8F27);

  /* ID0 read decodes MSB first */
  MockHAL_SetHyperBusID(0x0C81, 0x0001);
  ASSERT_EQ(ops->ReadReg(&h, 0x0000, &reg), 0);
  ASSERT_EQ(reg, 0x0C81);
  ASSERT_EQ(ops->ReadReg(&h, 0x0002, &reg), 0);
  ASSERT_EQ(reg, 0x0001);
  ASSERT_EQ(ops->WriteReg(&h, 0x1002, 0xFFC1), 0);
  ASSERT_EQ(MockHAL_GetHyperReg(0x1002), 0xFFC1);

  ASSERT_EQ(ops->Write(&h, 0x1234, tx, sizeof(tx)), 0);
  ASSERT_EQ(ops->Read(&h, 0x1234, rx, sizeof(rx)), 0);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  MockHAL_ClearLog();
  ASSERT_EQ(ops->MemoryMapped(&h), 0);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_MEMMAPPED, 0));

  /* Deep power down: CR0[15] cleared, everything else preserved; wake-up toggles CS# */
  ASSERT_EQ(ops->EnterDPD(&h), 0);
  ASSERT_EQ(MockHAL_GetHyperReg(0x1000), 0x0F27);
  MockHAL_ClearLog();
  ASSERT_EQ(ops->LeaveDPD(&h), 0);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_HYPER_CMD, 0));

  FAULT_SWEEP(HR_SETUP(), ops->Init(&h, 2, HAL_XSPI_SIZE_64MB));
  FAULT_SWEEP(HR_SETUP(), ops->ReadReg(&h, 0, &reg));
  FAULT_SWEEP(HR_SETUP(), ops->WriteReg(&h, 0x1000, 0x8F27));
  FAULT_SWEEP(HR_SETUP(), ops->Read(&h, 0, rx, 8));
  FAULT_SWEEP(HR_SETUP(), ops->Write(&h, 0, tx, 8));
  FAULT_SWEEP(HR_SETUP(), ops->MemoryMapped(&h));
  FAULT_SWEEP(HR_SETUP(), ops->EnterDPD(&h));
  FAULT_SWEEP(HR_SETUP(), ops->LeaveDPD(&h));
  return true;
}

/* Single-die adapters for the shared HyperRAM checks */
static int32_t S27KS_Init1(XSPI_HandleTypeDef *Ctx, uint32_t Prescaler, uint32_t Size) { return S27KS0641_Init(Ctx, Prescaler, Size, 1U); }
static int32_t S27KS_EnterDPD1(XSPI_HandleTypeDef *Ctx) { return S27KS0641_EnterDeepPowerDown(Ctx, 1U); }

bool test_infineon_s27ks0641_hyperram(void)
{
  static const HyperRamOps_t ops = {
    S27KS_Init1, S27KS0641_ReadRegister, S27KS0641_WriteRegister, S27KS0641_Read, S27KS0641_Write,
    S27KS0641_EnableMemoryMappedMode, S27KS_EnterDPD1, S27KS0641_LeaveDeepPowerDown, S27KS_CR0_INIT_VALUE
  };
  if (!hyperram_common(&ops)) return false;

  /* S70KS1281: CR0 of both dice (CA35 = A22 selects the die), DPD entered on both */
  {
    XSPI_HandleTypeDef hd = {0};
    HR_SETUP();
    ASSERT_EQ(S27KS0641_Init(&hd, 2, HAL_XSPI_SIZE_128MB, 2U), S27KS_OK);
    ASSERT_EQ(MockHAL_GetHyperReg(0x00001000U), S27KS_CR0_INIT_VALUE);
    ASSERT_EQ(MockHAL_GetHyperReg(0x00801000U), S27KS_CR0_INIT_VALUE);
    ASSERT_EQ(S27KS0641_EnterDeepPowerDown(&hd, 2U), S27KS_OK);
    ASSERT_EQ(MockHAL_GetHyperReg(0x00001000U) & 0x8000U, 0U);
    ASSERT_EQ(MockHAL_GetHyperReg(0x00801000U) & 0x8000U, 0U);
    ASSERT_EQ(S27KS0641_Init(&hd, 2, HAL_XSPI_SIZE_128MB, 0U), S27KS_ERROR);
    ASSERT_EQ(S27KS0641_Init(&hd, 2, HAL_XSPI_SIZE_128MB, 3U), S27KS_ERROR);
    ASSERT_EQ(S27KS0641_EnterDeepPowerDown(&hd, 0U), S27KS_ERROR);
    ASSERT_EQ(S27KS0641_EnterDeepPowerDown(&hd, 3U), S27KS_ERROR);
    FAULT_SWEEP(HR_SETUP(), S27KS0641_Init(&hd, 2, HAL_XSPI_SIZE_128MB, 2U));
    FAULT_SWEEP(HR_SETUP(), S27KS0641_EnterDeepPowerDown(&hd, 2U));
  }

  /* DMA variants */
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[32], rx[32];
  HR_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x4C);
  ASSERT_EQ(S27KS0641_Init(&h, 2, HAL_XSPI_SIZE_64MB, 1U), S27KS_OK);
  ASSERT_EQ(S27KS0641_Write_DMA(&h, 0x40, tx, sizeof(tx)), S27KS_OK);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_TX_DMA, 0));
  ASSERT_EQ(S27KS0641_Read_DMA(&h, 0x40, rx, sizeof(rx)), S27KS_OK);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_RX_DMA, 0));
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  FAULT_SWEEP(HR_SETUP(), S27KS0641_Write_DMA(&h, 0, tx, 8));
  FAULT_SWEEP(HR_SETUP(), S27KS0641_Read_DMA(&h, 0, rx, 8));
  return true;
}

bool test_issi_is66wvh16m8_hyperram(void)
{
  static const HyperRamOps_t ops = {
    IS66WVH16M8_Init, IS66WVH16M8_ReadRegister, IS66WVH16M8_WriteRegister, IS66WVH16M8_Read, IS66WVH16M8_Write,
    IS66WVH16M8_EnableMemoryMappedMode, IS66WVH16M8_EnterDeepPowerDown, IS66WVH16M8_LeaveDeepPowerDown, IS66WVH_CR0_INIT_VALUE
  };
  return hyperram_common(&ops);
}
