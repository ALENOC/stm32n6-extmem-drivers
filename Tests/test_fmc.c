/**
  ******************************************************************************
  * @file    test_fmc.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Tests for the FMC drivers: parallel PSRAM/SRAM (IS66WV, CY62167)
  *          and parallel NOR flash (IS29GL, MT28EW).
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

#define FMC_SETUP() do { MockHAL_Reset(); } while (0)
#define NOR_BASE    0x60000000U

/* Returns the n-th FMC 16-bit write */
static const MockEvent_t *FmcWrite(uint32_t n)
{
  return MockHAL_FindEvent(MOCK_EV_FMC_WRITE16, n);
}

static bool CheckFmcWrite(uint32_t n, uint32_t addr, uint16_t data, int line)
{
  const MockEvent_t *ev = FmcWrite(n);
  if (ev == NULL || ev->Address != addr || ev->Value != data)
  {
    printf("       [FAIL] test_fmc.c:%d: FMC write #%lu = 0x%04lX @0x%08lX, expected 0x%04X @0x%08lX\r\n", line,
           (unsigned long)n, ev ? (unsigned long)ev->Value : 0UL, ev ? (unsigned long)ev->Address : 0UL, data, (unsigned long)addr);
    return false;
  }
  return true;
}
#define ASSERT_FMC_WRITE(n, addr, data) ASSERT_TRUE(CheckFmcWrite((n), (addr), (data), __LINE__))

bool test_issi_is66wv_fmc_parallel_psram(void)
{
  SRAM_HandleTypeDef hsram = {0};
  IS66WV_FMC_Timing_t t = { 3, 1, 5, 1 };
  uint8_t tx[100], rx[100];
  FMC_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x61);

  ASSERT_EQ(IS66WV_FMC_Init(&hsram, FMC_NORSRAM_BANK2, &t), IS66WV_FMC_OK);
  const MockEvent_t *ev = MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0);
  ASSERT_NOT_NULL(ev);
  ASSERT_EQ(ev->SramInit.NSBank, FMC_NORSRAM_BANK2);
  ASSERT_EQ(ev->SramInit.MemoryType, FMC_MEMORY_TYPE_PSRAM);
  ASSERT_EQ(ev->SramInit.MemoryDataWidth, FMC_NORSRAM_MEM_BUS_WIDTH_16);
  ASSERT_EQ(ev->SramInit.WriteOperation, FMC_WRITE_OPERATION_ENABLE);
  ASSERT_EQ(ev->Timing.AddressSetupTime, 3);
  ASSERT_EQ(ev->Timing.DataSetupTime, 5);
  ASSERT_EQ(ev->Timing.AccessMode, FMC_ACCESS_MODE_A);

  FMC_SETUP();
  ASSERT_EQ(IS66WV_FMC_Init(&hsram, FMC_NORSRAM_BANK1, NULL), IS66WV_FMC_OK);
  ev = MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0);
  ASSERT_EQ(ev->Timing.AddressSetupTime, 3);
  ASSERT_EQ(ev->Timing.DataSetupTime, 5);

  ASSERT_EQ(IS66WV_FMC_Write(FMC_BANK1_2_BASE_ADDR, 0x123, tx, sizeof(tx)), IS66WV_FMC_OK);
  ASSERT_EQ(IS66WV_FMC_Read(FMC_BANK1_2_BASE_ADDR, 0x123, rx, sizeof(rx)), IS66WV_FMC_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  ASSERT_EQ(IS66WV_FMC_Write(FMC_BANK1_2_BASE_ADDR, 0, NULL, 1), IS66WV_FMC_ERROR);
  ASSERT_EQ(IS66WV_FMC_Read(FMC_BANK1_2_BASE_ADDR, 0, NULL, 1), IS66WV_FMC_ERROR);

  /* Memory test: passes on good RAM, fails on a stuck data bit */
  ASSERT_EQ(IS66WV_FMC_TestPattern(FMC_BANK1_1_BASE_ADDR, 4096), IS66WV_FMC_OK);
  MockHAL_SetRamStuckAt0(0x102, 0x04); /* byte 2 of every word is 0x55: bit 2 set */
  ASSERT_EQ(IS66WV_FMC_TestPattern(FMC_BANK1_1_BASE_ADDR, 4096), IS66WV_FMC_ERROR);
  MockHAL_SetRamStuckAt0(0, 0);

  FAULT_SWEEP(FMC_SETUP(), IS66WV_FMC_Init(&hsram, FMC_NORSRAM_BANK1, &t));
  return true;
}

bool test_issi_is29gl_fmc_parallel_nor_flash(void)
{
  SRAM_HandleTypeDef hsram = {0};
  IS29GL_FMC_Timing_t t = { 4, 2, 7, 2 };
  uint16_t mfg = 0, dev = 0;
  uint8_t tx[64], rx[64];
  uint8_t *mem = MockHAL_GetMemoryBuffer();
  FMC_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x2B);

  ASSERT_EQ(IS29GL_FMC_Init(&hsram, FMC_NORSRAM_BANK3, &t), IS29GL_FMC_OK);
  const MockEvent_t *ev = MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0);
  ASSERT_EQ(ev->SramInit.NSBank, FMC_NORSRAM_BANK3);
  ASSERT_EQ(ev->SramInit.MemoryType, FMC_MEMORY_TYPE_NOR);
  ASSERT_EQ(ev->SramInit.MemoryDataWidth, FMC_NORSRAM_MEM_BUS_WIDTH_16);
  ASSERT_EQ(ev->Timing.AccessMode, FMC_ACCESS_MODE_B);
  ASSERT_EQ(ev->Timing.DataSetupTime, 7);
  FMC_SETUP();
  ASSERT_EQ(IS29GL_FMC_Init(&hsram, FMC_NORSRAM_BANK1, NULL), IS29GL_FMC_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.DataSetupTime, 7);

  /* Autoselect: AA@555, 55@2AA, 90@555, read words 0 and 1, then reset F0 */
  FMC_SETUP();
  ASSERT_EQ(IS29GL_FMC_ReadID(NOR_BASE, &mfg, &dev), IS29GL_FMC_OK);
  ASSERT_FMC_WRITE(0, NOR_BASE + 0xAAA, 0x00AA);
  ASSERT_FMC_WRITE(1, NOR_BASE + 0x554, 0x0055);
  ASSERT_FMC_WRITE(2, NOR_BASE + 0xAAA, 0x0090);
  ASSERT_FMC_WRITE(3, NOR_BASE + 0x000, 0x00F0);
  ASSERT_EQ(mfg, 0x009D);
  ASSERT_EQ(dev, 0x227E);
  ASSERT_EQ(IS29GL_FMC_ReadID(NOR_BASE, NULL, &dev), IS29GL_FMC_ERROR);
  ASSERT_EQ(IS29GL_FMC_ReadID(NOR_BASE, &mfg, NULL), IS29GL_FMC_ERROR);

  /* Word program: unlock, A0, data; DQ7 polling completes */
  FMC_SETUP();
  ASSERT_EQ(IS29GL_FMC_ProgramWord(NOR_BASE, 0x200, 0xA55A), IS29GL_FMC_OK);
  ASSERT_FMC_WRITE(0, NOR_BASE + 0xAAA, 0x00AA);
  ASSERT_FMC_WRITE(1, NOR_BASE + 0x554, 0x0055);
  ASSERT_FMC_WRITE(2, NOR_BASE + 0xAAA, 0x00A0);
  ASSERT_FMC_WRITE(3, NOR_BASE + 0x200, 0xA55A);
  ASSERT_EQ(mem[0x200], 0x5A);
  ASSERT_EQ(mem[0x201], 0xA5);

  /* Buffer program with odd start and odd length: padding bytes stay erased */
  FMC_SETUP();
  ASSERT_EQ(IS29GL_FMC_ProgramBuffer(NOR_BASE, 0x301, tx, 6), IS29GL_FMC_OK);
  ASSERT_EQ(mem[0x300], 0xFF);
  ASSERT_EQ(memcmp(&mem[0x301], tx, 6), 0);
  ASSERT_EQ(mem[0x307], 0xFF);
  ASSERT_EQ(IS29GL_FMC_ProgramBuffer(NOR_BASE, 0x400, tx, sizeof(tx)), IS29GL_FMC_OK);
  ASSERT_EQ(IS29GL_FMC_Read(NOR_BASE, 0x400, rx, sizeof(rx)), IS29GL_FMC_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  ASSERT_EQ(IS29GL_FMC_Read(NOR_BASE, 0, NULL, 1), IS29GL_FMC_ERROR);

  /* Sector erase: AA 55 80 AA 55, 30 at the sector address */
  MockHAL_ClearLog();
  MockHAL_SetBlockEraseSize(128U * 1024U);
  ASSERT_EQ(IS29GL_FMC_EraseSector(NOR_BASE, 0x0), IS29GL_FMC_OK);
  ASSERT_FMC_WRITE(2, NOR_BASE + 0xAAA, 0x0080);
  ASSERT_FMC_WRITE(5, NOR_BASE + 0x000, 0x0030);
  ASSERT_EQ(mem[0x400], 0xFF);

  FMC_SETUP();
  memset(mem, 0, 64);
  ASSERT_EQ(IS29GL_FMC_EraseChip(NOR_BASE), IS29GL_FMC_OK);
  ASSERT_FMC_WRITE(5, NOR_BASE + 0xAAA, 0x0010);
  ASSERT_EQ(mem[0], 0xFF);

  /* Device never completes: timeout, then reset to read array mode */
  FMC_SETUP();
  MockHAL_SetFmcNorStuck(true, false);
  ASSERT_EQ(IS29GL_FMC_ProgramWord(NOR_BASE, 0, 0x1234), IS29GL_FMC_TIMEOUT);
  ASSERT_EQ(IS29GL_FMC_EraseSector(NOR_BASE, 0), IS29GL_FMC_TIMEOUT);
  ASSERT_EQ(IS29GL_FMC_ProgramBuffer(NOR_BASE, 0, tx, 2), IS29GL_FMC_TIMEOUT);
  /* DQ5 (exceeded timing limit): error and reset */
  MockHAL_SetFmcNorStuck(true, true);
  ASSERT_EQ(IS29GL_FMC_EraseSector(NOR_BASE, 0), IS29GL_FMC_ERROR);
  ASSERT_EQ(IS29GL_FMC_EraseChip(NOR_BASE), IS29GL_FMC_ERROR);
  const MockEvent_t *last = NULL;
  for (uint32_t i = 0; MockHAL_FindEvent(MOCK_EV_FMC_WRITE16, i) != NULL; i++) last = MockHAL_FindEvent(MOCK_EV_FMC_WRITE16, i);
  ASSERT_EQ(last->Value, 0x00F0);
  /* DQ5 raised while the operation completes: the second DQ7 read confirms success */
  MockHAL_SetFmcNorStuck(false, true);
  MockHAL_SetFmcNorBusyReads(1);
  ASSERT_EQ(IS29GL_FMC_ProgramWord(NOR_BASE, 0x10, 0x00FF), IS29GL_FMC_OK);
  /* Busy for a few polls, then done */
  MockHAL_SetFmcNorStuck(false, false);
  MockHAL_SetFmcNorBusyReads(3);
  ASSERT_EQ(IS29GL_FMC_EraseSector(NOR_BASE, 0), IS29GL_FMC_OK);

  ASSERT_EQ(IS29GL_FMC_Reset(NOR_BASE), IS29GL_FMC_OK);

  FAULT_SWEEP(FMC_SETUP(), IS29GL_FMC_Init(&hsram, FMC_NORSRAM_BANK1, &t));
  return true;
}
