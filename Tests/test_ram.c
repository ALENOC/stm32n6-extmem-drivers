/**
  ******************************************************************************
  * @file    test_ram.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Protocol level tests for the serial RAM drivers:
  *          ISSI octal PSRAM (IS66WVO), quad PSRAM (IS66WVS) and serial SRAM (IS62WVS).
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

#define RAM_SETUP() do { MockHAL_Reset(); } while (0)

/* ========================================================================= */
/* ISSI IS66WVO octal PSRAM                                                  */
/* ========================================================================= */

bool test_issi_is66wvo32m8_octal_psram(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[3000], rx[3000];
  uint16_t reg = 0, id = 0;
  uint32_t cap = 0;
  RAM_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x0F);

  /* Init: Macronix RAM mode, 1 KB row boundary, CR written first (zero latency), then ID and CR read back */
  ASSERT_EQ(IS66WVO32M8_Init(&h, 1, HAL_XSPI_SIZE_64MB), IS66WVO_OK);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_MACRONIX_RAM);
  ASSERT_EQ(init->Init.MemorySize, HAL_XSPI_SIZE_64MB);
  ASSERT_EQ(init->Init.ChipSelectBoundary, HAL_XSPI_BONDARYOF_8KB); /* 1 KB row */
  ASSERT_EQ(init->Init.DelayHoldQuarterCycle, HAL_XSPI_DHQC_ENABLE);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x4000), A8D(IS66WVO_REG_CR), D8D(2), DUMMY(0), NODQS);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xC000), A8D(IS66WVO_REG_ID), D8D(2), DUMMY(IS66WVO_DUMMY_CYCLES), WITHDQS);
  ASSERT_CMD(Test_NthCommand(2), I8D(0xC000), A8D(IS66WVO_REG_CR), D8D(2), DUMMY(IS66WVO_DUMMY_CYCLES), WITHDQS);
  /* CR: normal power, 24 ohm, LC = 7 (0100b), fixed latency, 32 byte wrap -> 0xF04A, fixed latency gives 2 x 7 - 1 dummy */
  ASSERT_EQ(IS66WVO_CR_INIT_VALUE, 0xF04A);
  ASSERT_EQ(MockHAL_GetOctalRamCR(), 0xF04A);
  ASSERT_EQ(IS66WVO_DUMMY_CYCLES, 13);
  const MockEvent_t *crw = MockHAL_FindTxAfterCommand(0x4000, 0);
  ASSERT_EQ(crw->Data[0], 0x4A);
  ASSERT_EQ(crw->Data[1], 0xF0);

  ASSERT_EQ(IS66WVO32M8_ReadID(&h, &id, &cap), IS66WVO_OK);
  ASSERT_EQ(id, 0x0C93);
  ASSERT_EQ(cap, 8U * 1024U * 1024U);
  ASSERT_EQ(IS66WVO32M8_ReadReg(&h, IS66WVO_REG_CR, &reg, 13), IS66WVO_OK);
  ASSERT_EQ(reg, 0xF04A);

  /* Wrong manufacturer or a configuration that did not stick: init fails */
  RAM_SETUP();
  MockHAL_SetOctalRamId(0x0C91);
  ASSERT_EQ(IS66WVO32M8_Init(&h, 1, HAL_XSPI_SIZE_64MB), IS66WVO_ERROR);
  RAM_SETUP();
  MockHAL_SetOctalRamCrLocked(true);
  ASSERT_EQ(IS66WVO32M8_Init(&h, 1, HAL_XSPI_SIZE_64MB), IS66WVO_ERROR);
  /* ID fields that cannot describe a 32-bit address space give no capacity */
  RAM_SETUP();
  MockHAL_SetOctalRamId(0x1FF3);
  ASSERT_EQ(IS66WVO32M8_ReadID(&h, &id, &cap), IS66WVO_OK);
  ASSERT_EQ(cap, 0);
  RAM_SETUP();

  /* Aligned transfer inside one row */
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Write(&h, tx, 0x800, 128, 0), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x2000), A8D(0x800), D8D(128), DUMMY(13), WITHDQS);
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Read(&h, rx, 0x800, 128, 0), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xA000), A8D(0x800), D8D(128), DUMMY(13), WITHDQS);
  ASSERT_EQ(memcmp(tx, rx, 128), 0);

  /* Unaligned start / odd length / several rows: split at rows, edge bytes read-modify-written */
  memset(MockHAL_GetMemoryBuffer(), 0xEE, 0x4000);
  ASSERT_EQ(IS66WVO32M8_Write(&h, tx, 0x3FF, 2051, 13), IS66WVO_OK);
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x3FE], 0xEE);
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x3FF + 2051], 0xEE);
  ASSERT_EQ(memcmp(&MockHAL_GetMemoryBuffer()[0x3FF], tx, 2051), 0);
  memset(rx, 0, sizeof(rx));
  ASSERT_EQ(IS66WVO32M8_Read(&h, rx, 0x3FF, 2051, 13), IS66WVO_OK);
  ASSERT_EQ(memcmp(tx, rx, 2051), 0);
  ASSERT_EQ(IS66WVO32M8_Read(&h, rx, 0x10, 1, 0), IS66WVO_OK);

  /* DMA: one aligned burst inside a row, anything else is refused */
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 64, 13), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 64, 0), IS66WVO_OK);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 64, 0), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 64, 13), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1001, 64, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 63, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x13F0, 64, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 0, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1001, 64, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 63, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x13F0, 64, 13), IS66WVO_ERROR);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 0, 13), IS66WVO_ERROR);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_EnableMemoryMappedMode(&h, 13, 13), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x2000), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, DUMMY(13), WITHDQS, OPWRITE);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xA000), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, DUMMY(13), WITHDQS, OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_EnableMemoryMappedMode(&h, 0, 0), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x2000), DUMMY(13), OPWRITE);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xA000), DUMMY(13), OPREAD);

  /* Deep power down: CR[15] cleared; wake-up rewrites the configuration */
  ASSERT_EQ(IS66WVO32M8_EnterDeepPowerDown(&h), IS66WVO_OK);
  ASSERT_EQ(MockHAL_GetOctalRamCR(), 0x704A);
  ASSERT_EQ(IS66WVO32M8_LeaveDeepPowerDown(&h), IS66WVO_OK);
  ASSERT_EQ(MockHAL_GetOctalRamCR(), 0xF04A);

  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Init(&h, 1, HAL_XSPI_SIZE_64MB));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_ReadReg(&h, 0, &reg, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_WriteReg(&h, IS66WVO_REG_CR, 0xF04A));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_ReadID(&h, &id, &cap));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Read(&h, rx, 1, 9, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Write(&h, tx, 1, 9, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Read_DMA(&h, rx, 0, 8, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Write_DMA(&h, tx, 0, 8, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_EnableMemoryMappedMode(&h, 13, 13));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_EnterDeepPowerDown(&h));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_LeaveDeepPowerDown(&h));
  return true;
}

/* ========================================================================= */
/* ISSI IS66WVS quad PSRAM                                                   */
/* ========================================================================= */

bool test_issi_is66wvs16m8_quad_psram(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], tx[128], rx[128];
  RAM_SETUP();
  MockHAL_SetEmulatedChip(0x9D, 0x5D, 0x04);
  Test_Pattern(tx, sizeof(tx), 0x3C);

  ASSERT_EQ(IS66WVS16M8_Init(&h, 2, HAL_XSPI_SIZE_128MB), IS66WVS_OK);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_MICRON);
  ASSERT_EQ(init->Init.ChipSelectBoundary, HAL_XSPI_BONDARYOF_8KB); /* 1 KB page */
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  ASSERT_EQ(IS66WVS16M8_ReadID(&h, id), IS66WVS_OK);
  ASSERT_EQ(id[0], 0x9D); ASSERT_EQ(id[1], 0x5D);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_WriteQuad(&h, 0x400, tx, sizeof(tx)), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x38), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x400), D4S(128), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_ReadQuad(&h, 0x400, rx, sizeof(rx), 6), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x400), D4S(128), DUMMY(6));
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_ReadQuad(&h, 0x400, rx, 4, 0), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), DUMMY(6));

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_EnterQuadMode(&h), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x35), NOADDR, NODATA);
  ASSERT_EQ(IS66WVS16M8_ExitQuadMode(&h), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(1), I4S(0xF5), NOADDR, NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_EnableMemoryMappedMode(&h, 6), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x38), .AddressMode = HAL_XSPI_ADDRESS_4_LINES, DUMMY(0), OPWRITE);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xEB), .AddressMode = HAL_XSPI_ADDRESS_4_LINES, DUMMY(6), OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_EnableMemoryMappedMode(&h, 0), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xEB), DUMMY(6), OPREAD);

  /* Transfers crossing a 1 KB page are split (the device would wrap inside the page) */
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_WriteQuad(&h, 0x3F0, tx, 64), IS66WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x38), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x3F0), D4S(16));
  ASSERT_CMD(Test_NthCommand(1), I1S(0x38), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x400), D4S(48));
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVS16M8_ReadQuad(&h, 0x3F0, rx, 64, 6), IS66WVS_OK);
  ASSERT_EQ(Test_CommandCount(), 2);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);

  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_Init(&h, 2, HAL_XSPI_SIZE_128MB));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_ReadID(&h, id));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_EnterQuadMode(&h));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_ExitQuadMode(&h));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_ReadQuad(&h, 0, rx, 8, 6));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_WriteQuad(&h, 0, tx, 8));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_EnableMemoryMappedMode(&h, 6));
  FAULT_SWEEP(RAM_SETUP(), IS66WVS16M8_Reset(&h));
  return true;
}

/* ========================================================================= */
/* ISSI IS62WVS / IS65WVS serial SRAM                                        */
/* ========================================================================= */

bool test_issi_is62wvs_serial_sram(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[64], rx[64], mode = 0;
  RAM_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x55);

  /* Init: leave SQI (0xFF on 4 lines), then select sequential mode with WRMR */
  ASSERT_EQ(IS62WVS_Init(&h, 4, HAL_XSPI_SIZE_4MB), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I4S(0xFF), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x01), NOADDR, D1S(1));
  ASSERT_EQ(MockHAL_FindTxAfterCommand(0x01, 0)->Data[0], IS62WVS_MODE_SEQUENTIAL);
  ASSERT_EQ(MockHAL_GetSramModeRegister(), IS62WVS_MODE_SEQUENTIAL);

  ASSERT_EQ(IS62WVS_ReadModeRegister(&h, &mode), IS62WVS_OK);
  ASSERT_EQ(mode, IS62WVS_MODE_SEQUENTIAL);
  ASSERT_EQ(IS62WVS_WriteModeRegister(&h, IS62WVS_MODE_PAGE), IS62WVS_OK);
  ASSERT_EQ(MockHAL_GetSramModeRegister(), IS62WVS_MODE_PAGE);

  /* SPI protocol */
  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_Write(&h, 0x100, tx, 32), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x02), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x100), D1S(32), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_Read(&h, 0x100, rx, 32), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x03), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x100), D1S(32), DUMMY(0));
  ASSERT_EQ(memcmp(tx, rx, 32), 0);

  /* SQI protocol: every phase on 4 lines, one dummy byte (2 clocks) on reads */
  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_EnterQuadMode(&h), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x38), NOADDR, NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_WriteQuad(&h, 0x200, tx, sizeof(tx)), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I4S(0x02), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x200), D4S(64), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_ReadQuad(&h, 0x200, rx, sizeof(rx), 2), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I4S(0x03), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x200), D4S(64), DUMMY(2));
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_EnableMemoryMappedMode(&h, 2), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I4S(0x02), .AddressMode = HAL_XSPI_ADDRESS_4_LINES, DUMMY(0), OPWRITE);
  ASSERT_CMD(Test_NthCommand(1), I4S(0x03), .AddressMode = HAL_XSPI_ADDRESS_4_LINES, DUMMY(2), OPREAD);

  MockHAL_ClearLog();
  ASSERT_EQ(IS62WVS_ExitQuadMode(&h), IS62WVS_OK);
  ASSERT_CMD(Test_NthCommand(0), I4S(0xFF), NOADDR, NODATA);

  /* Sequential access cannot cross the 2 Mbit die boundary of the 4 Mbit part: split at 0x40000 */
  for (int quad = 0; quad < 2; quad++)
  {
    MockHAL_ClearLog();
    ASSERT_EQ(quad ? IS62WVS_WriteQuad(&h, 0x3FFF8, tx, 24) : IS62WVS_Write(&h, 0x3FFF8, tx, 24), IS62WVS_OK);
    ASSERT_EQ(Test_CommandCount(), 2);
    ASSERT_EQ(Test_NthCommand(0)->Cmd.DataLength, 8);
    ASSERT_EQ(Test_NthCommand(1)->Cmd.Address, 0x40000);
    MockHAL_ClearLog();
    ASSERT_EQ(quad ? IS62WVS_ReadQuad(&h, 0x3FFF8, rx, 24, 2) : IS62WVS_Read(&h, 0x3FFF8, rx, 24), IS62WVS_OK);
    ASSERT_EQ(Test_CommandCount(), 2);
    ASSERT_EQ(memcmp(tx, rx, 24), 0);
  }
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0) == NULL, true);

  FAULT_SWEEP(RAM_SETUP(), IS62WVS_Init(&h, 4, HAL_XSPI_SIZE_4MB));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_ReadModeRegister(&h, &mode));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_WriteModeRegister(&h, 0x40));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_EnterQuadMode(&h));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_ExitQuadMode(&h));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_Read(&h, 0, rx, 8));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_Write(&h, 0, tx, 8));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_ReadQuad(&h, 0, rx, 8, 2));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_WriteQuad(&h, 0, tx, 8));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_EnableMemoryMappedMode(&h, 2));
  FAULT_SWEEP(RAM_SETUP(), IS62WVS_Reset(&h));
  return true;
}
