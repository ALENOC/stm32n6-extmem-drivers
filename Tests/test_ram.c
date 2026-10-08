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
  uint8_t tx[128], rx[128], mr = 0;
  RAM_SETUP();
  Test_Pattern(tx, sizeof(tx), 0x0F);

  ASSERT_EQ(IS66WVO32M8_Init(&h, 2, HAL_XSPI_SIZE_256MB), IS66WVO_OK);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_APMEM);
  ASSERT_EQ(init->Init.MemorySize, HAL_XSPI_SIZE_256MB);
  ASSERT_EQ(init->Init.ChipSelectBoundary, HAL_XSPI_BONDARYOF_16KB); /* 2 KB page */
  /* Global reset, then MR0 and MR4 written as DTR byte pairs */
  ASSERT_CMD(Test_NthCommand(0), I8S(0xFF), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8S(0xC0), A8D(IS66WVO_MR0_ADDR), D8D(2), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(2), I8S(0xC0), A8D(IS66WVO_MR4_ADDR), D8D(2), DUMMY(0));
  ASSERT_EQ(MockHAL_GetMR(0), IS66WVO_MR0_READ_LATENCY_5 | IS66WVO_MR0_VARIABLE_LATENCY);
  ASSERT_EQ(MockHAL_GetMR(4), IS66WVO_MR4_WRITE_LATENCY_5);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_ReadReg(&h, IS66WVO_MR0_ADDR, &mr, 4), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x40), A8D(0), D8D(2), DUMMY(4), WITHDQS);
  ASSERT_EQ(mr, IS66WVO_MR0_READ_LATENCY_5);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Write(&h, tx, 0x800, sizeof(tx), 5), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x80), A8D(0x800), D8D(128), DUMMY(5), WITHDQS);
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Read(&h, rx, 0x800, sizeof(rx), 5), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x00), A8D(0x800), D8D(128), DUMMY(5), WITHDQS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  /* Zero dummy selects the default latency */
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Read(&h, rx, 0x800, 2, 0), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x00), DUMMY(5));
  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_Write(&h, tx, 0x900, 2, 0), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x80), DUMMY(5));

  memset(rx, 0, sizeof(rx));
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 64, 5), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 64, 5), IS66WVO_OK);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);
  ASSERT_EQ(IS66WVO32M8_Write_DMA(&h, tx, 0x1000, 2, 0), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read_DMA(&h, rx, 0x1000, 2, 0), IS66WVO_OK);

  MockHAL_ClearLog();
  ASSERT_EQ(IS66WVO32M8_EnableMemoryMappedMode(&h, 5, 5), IS66WVO_OK);
  ASSERT_CMD(Test_NthCommand(0), I8S(0x80), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, DUMMY(5), WITHDQS, OPWRITE);
  ASSERT_CMD(Test_NthCommand(1), I8S(0x00), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, DUMMY(5), WITHDQS, OPREAD);

  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Init(&h, 2, HAL_XSPI_SIZE_256MB));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_ReadReg(&h, 0, &mr, 4));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_WriteReg(&h, 0, 0));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Read(&h, rx, 0, 8, 5));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Write(&h, tx, 0, 8, 5));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Read_DMA(&h, rx, 0, 8, 5));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Write_DMA(&h, tx, 0, 8, 5));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_EnableMemoryMappedMode(&h, 5, 5));
  FAULT_SWEEP(RAM_SETUP(), IS66WVO32M8_Reset(&h));
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
