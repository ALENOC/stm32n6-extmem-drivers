/**
  ******************************************************************************
  * @file    test_quad_nor.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Protocol level tests for the quad NOR drivers:
  *          Infineon S25Hx-T / S25FL-L, ISSI IS25LP/WP family, Micron MT25Q.
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

#define FLASH_SETUP(mfg, type, dens) do { MockHAL_Reset(); MockHAL_SetFlashSemantics(true); \
                                          MockHAL_SetEmulatedChip((mfg), (type), (dens)); } while (0)

/* ========================================================================= */
/* Infineon S25HL512T                                                        */
/* ========================================================================= */

bool test_infineon_s25hl512t_quad_flash(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], tx[256], rx[256];
  FLASH_SETUP(0x34, 0x2A, 0x1A);
  Test_Pattern(tx, sizeof(tx), 0x21);

  ASSERT_EQ(S25HL512T_ReadID(&h, id), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x9F), NOADDR, D1S(3), DUMMY(0));
  ASSERT_EQ(id[1], 0x2A);

  /* Quad enable: RDSR1, RDCR1, WREN, WRR(SR1, CR1 | QUAD), poll */
  MockHAL_ClearLog();
  MockHAL_SetStatusRegister(0x00);
  ASSERT_EQ(S25HL512T_EnableQuadMode(&h), S25HL512T_OK);
  ASSERT_SEQUENCE(0, 0x05, 0x35, 0x06, 0x01, 0x05);
  ASSERT_CMD(Test_NthCommand(3), I1S(0x01), NOADDR, D1S(2), DUMMY(0));
  ASSERT_EQ(MockHAL_GetConfigRegister1() & S25HL_CR1_QUAD_ENABLE, S25HL_CR1_QUAD_ENABLE);
  /* Already enabled: read only */
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_EnableQuadMode(&h), S25HL512T_OK);
  ASSERT_EQ(Test_CommandCount(), 2);

  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_PageProgramQuad(&h, 0x1000, tx, sizeof(tx)), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x06), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x34), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x1000), D4S(256), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(2), I1S(0x05), NOADDR, D1S(1));

  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_ReadQuad(&h, 0x1000, rx, sizeof(rx), 8), S25HL512T_OK);
  /* 2 mode cycles (0x00, no continuous read) are not part of the 8 latency cycles */
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), A4S(HAL_XSPI_ADDRESS_32_BITS, 0x1000), .AlternateBytesMode = HAL_XSPI_ALT_BYTES_4_LINES,
             D4S(256), DUMMY(8));
  ASSERT_EQ(Test_NthCommand(0)->Cmd.AlternateBytes, 0x00);
  ASSERT_EQ(Test_NthCommand(0)->Cmd.AlternateBytesWidth, HAL_XSPI_ALT_BYTES_8_BITS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_ReadQuad(&h, 0x1000, rx, 4, 0), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), DUMMY(S25HL_DEFAULT_READ_LATENCY));

  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_EraseSector4K(&h, 0x1000), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x21), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x1000), NODATA);
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x1000], 0xFF);
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_EraseBlock(&h, 0x40000), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xDC), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x40000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_ChipErase(&h), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x60), NOADDR, NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_EnableMemoryMappedMode(&h, 8), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), .AddressMode = HAL_XSPI_ADDRESS_4_LINES, .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
             .DataMode = HAL_XSPI_DATA_4_LINES, DUMMY(8), OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_EnableMemoryMappedMode(&h, 0), S25HL512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), .AlternateBytesMode = HAL_XSPI_ALT_BYTES_4_LINES, DUMMY(8), OPREAD);

  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_Reset(&h), S25HL512T_OK);
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(S25HL512T_AutoPollingMemReady(&h, 1), S25HL512T_TIMEOUT);
  MockHAL_SetPollTimeout(false);

  /* SEMPER failure: busy with PRGERR until CLPEF (0x82) */
  FLASH_SETUP(0x34, 0x2A, 0x1A);
  MockHAL_SetSemperFailure(true);
  ASSERT_EQ(S25HL512T_PageProgramQuad(&h, 0, tx, 8), S25HL512T_ERROR);
  ASSERT_EQ(MockHAL_CountCommands(0x82), 1);
  ASSERT_EQ(MockHAL_GetStatusRegister() & 0x61, 0);
  FAULT_SWEEP_EXPECT(FLASH_SETUP(0x34, 0x2A, 0x1A); MockHAL_SetSemperFailure(true), S25HL512T_EraseSector4K(&h, 0), S25HL512T_ERROR);

  /* S25FL-L failure: SR2 P_ERR, WIP stays set until CLSR (0x30); SEMPER CLPEF is not sent */
  FLASH_SETUP(0x01, 0x60, 0x19);
  MockHAL_SetFlLFailure(true);
  MockHAL_ClearLog();
  ASSERT_EQ(S25HL512T_PageProgramQuad(&h, 0, tx, 8), S25HL512T_ERROR);
  ASSERT_EQ(MockHAL_CountCommands(0x07), 1);
  ASSERT_EQ(MockHAL_CountCommands(0x30), 1);
  ASSERT_EQ(MockHAL_CountCommands(0x82), 0);
  ASSERT_EQ(MockHAL_GetStatusRegister2() & 0x60, 0);
  ASSERT_EQ(MockHAL_GetStatusRegister() & 0x01, 0);
  FAULT_SWEEP_EXPECT(FLASH_SETUP(0x01, 0x60, 0x19); MockHAL_SetFlLFailure(true), S25HL512T_EraseSector4K(&h, 0), S25HL512T_ERROR);
  /* S25FL-L protection bits in SR1[6:5] are not mistaken for errors on a plain timeout */
  FLASH_SETUP(0x01, 0x60, 0x19);
  MockHAL_SetPollTimeout(true);
  MockHAL_SetStatusRegister(0x00);
  ASSERT_EQ(S25HL512T_AutoPollingMemReady(&h, 1), S25HL512T_TIMEOUT);
  MockHAL_SetPollTimeout(false);

  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_EnableQuadMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_PageProgramQuad(&h, 0, tx, 16));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_ReadQuad(&h, 0, rx, 16, 8));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_EraseSector4K(&h, 0));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_EraseBlock(&h, 0));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_ChipErase(&h));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_EnableMemoryMappedMode(&h, 8));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x2A, 0x1A), S25HL512T_Reset(&h));
  return true;
}

/* ========================================================================= */
/* ISSI IS25LP / IS25WP                                                      */
/* ========================================================================= */

bool test_issi_is25lp256_quad_flash(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], tx[256], rx[256];
  FLASH_SETUP(0x9D, 0x60, 0x19);
  Test_Pattern(tx, sizeof(tx), 0x42);

  ASSERT_EQ(IS25LP256_ReadID(&h, id), IS25LP_OK);
  ASSERT_EQ(id[0], 0x9D); ASSERT_EQ(id[1], 0x60); ASSERT_EQ(id[2], 0x19);

  /* Quad enable: RDSR, WREN, WRSR with QE (bit 6), poll */
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_EnableQuadMode(&h), IS25LP_OK);
  ASSERT_SEQUENCE(0, 0x05, 0x06, 0x01, 0x05);
  const MockEvent_t *tx01 = MockHAL_FindTxAfterCommand(0x01, 0);
  ASSERT_NOT_NULL(tx01);
  ASSERT_EQ(tx01->Data[0] & 0x40, 0x40);
  ASSERT_EQ(MockHAL_GetStatusRegister() & 0x40, 0x40);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_EnableQuadMode(&h), IS25LP_OK);
  ASSERT_EQ(Test_CommandCount(), 1);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_Enter4ByteAddressMode(&h), IS25LP_OK);
  ASSERT_SEQUENCE(0, 0x06, 0xB7);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xB7), NOADDR, NODATA);
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  ASSERT_EQ(IS25LP_Exit4ByteAddressMode(&h), IS25LP_OK);
  ASSERT_SEQUENCE(2, 0x06, 0xE9);
  ASSERT_CMD(Test_NthCommand(3), I1S(0xE9), NOADDR, NODATA);
  ASSERT_TRUE(!MockHAL_Is4ByteMode());

  /* 3-byte addressing variants */
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_PageProgramQuadEx(&h, 0x2000, tx, sizeof(tx), HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x32), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x2000), D4S(256), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_ReadQuadEx(&h, 0x2000, rx, sizeof(rx), 6, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  /* Mode bits driven as 0x00 on 4 lines (2 cycles), 4 remaining wait cycles */
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x2000), .AlternateBytesMode = HAL_XSPI_ALT_BYTES_4_LINES,
             D4S(256), DUMMY(4));
  ASSERT_EQ(Test_NthCommand(0)->Cmd.AlternateBytes, 0x00);
  ASSERT_EQ(Test_NthCommand(0)->Cmd.AlternateBytesWidth, HAL_XSPI_ALT_BYTES_8_BITS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_ReadQuadEx(&h, 0x2000, rx, 4, 0, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), DUMMY(4));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_ReadQuadEx(&h, 0x2000, rx, 4, 2, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_EraseSector4KEx(&h, 0x2000, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x20), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x2000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_EraseBlock64KEx(&h, 0x10000, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xD8), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x10000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP_EnableMemoryMappedModeEx(&h, 6, HAL_XSPI_ADDRESS_24_BITS), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), .AlternateBytesMode = HAL_XSPI_ALT_BYTES_4_LINES, DUMMY(4), OPREAD);

  /* 4-byte opcode variants (wrappers) */
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_PageProgramQuad(&h, 0x3000, tx, 32), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x34), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x3000), D4S(32));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_ReadQuad(&h, 0x3000, rx, 32, 6), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), A4S(HAL_XSPI_ADDRESS_32_BITS, 0x3000), D4S(32), DUMMY(4));
  ASSERT_EQ(memcmp(tx, rx, 32), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_EraseSector4K(&h, 0x3000), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x21), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x3000));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_EraseBlock64K(&h, 0x10000), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xDC), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x10000));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_EnableMemoryMappedMode(&h, 6), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEC), OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_ChipErase(&h), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x60), NOADDR, NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_Reset(&h), IS25LP_OK);
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(IS25LP256_AutoPollingMemReady(&h, 1), IS25LP_TIMEOUT);
  MockHAL_SetPollTimeout(false);

  /* Read Register: SRPV with P[6:3] = 11, read back with RDRP */
  uint8_t applied = 0;
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LP256_SetReadDummyCycles(&h, 11, &applied), IS25LP_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xC0), NOADDR, D1S(1), DUMMY(0));
  ASSERT_EQ(MockHAL_FindTxAfterCommand(0xC0, 0)->Data[0], 11U << 3);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x61), NOADDR, D1S(1), DUMMY(0));
  ASSERT_EQ(applied, 11);
  ASSERT_EQ(MockHAL_GetIssiReadParams(), 0x58);
  /* Parts without a Read Register keep the factory 6 cycles */
  FLASH_SETUP(0x9D, 0x40, 0x16);
  MockHAL_SetIssiReadRegister(false);
  ASSERT_EQ(IS25LP256_SetReadDummyCycles(&h, 11, &applied), IS25LP_OK);
  ASSERT_EQ(applied, IS25LP_DEFAULT_QUAD_IO_DUMMY);
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP256_SetReadDummyCycles(&h, 11, &applied));

  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP256_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP256_EnableQuadMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_Enter4ByteAddressMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_Exit4ByteAddressMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_PageProgramQuadEx(&h, 0, tx, 16, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_ReadQuadEx(&h, 0, rx, 16, 6, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_EraseSector4KEx(&h, 0, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_EraseBlock64KEx(&h, 0, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP256_ChipErase(&h));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP_EnableMemoryMappedModeEx(&h, 6, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x60, 0x19), IS25LP256_Reset(&h));
  return true;
}

/* ========================================================================= */
/* Micron MT25QU / MT25QL                                                    */
/* ========================================================================= */

bool test_micron_mt25qu512a_quad_flash(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], st = 0, tx[256], rx[256];
  FLASH_SETUP(0x20, 0xBB, 0x20);
  Test_Pattern(tx, sizeof(tx), 0x6D);

  /* Init on a 512 Mbit part: XSPI init, reset, WREN + EN4B */
  ASSERT_EQ(MT25QU_Init(&h, 2, HAL_XSPI_SIZE_512MB), MT25Q_OK);
  ASSERT_SEQUENCE(0, 0x66, 0x99, 0x06, 0xB7);
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  /* 128 Mbit part stays in 3-byte mode */
  FLASH_SETUP(0x20, 0xBB, 0x18);
  ASSERT_EQ(MT25QU_Init(&h, 2, HAL_XSPI_SIZE_128MB), MT25Q_OK);
  ASSERT_EQ(MockHAL_CountCommands(0xB7), 0);
  ASSERT_TRUE(!MockHAL_Is4ByteMode());

  FLASH_SETUP(0x20, 0xBB, 0x20);
  ASSERT_EQ(MT25QU_ReadID(&h, id), MT25Q_OK);
  ASSERT_EQ(id[0], MT25Q_MANUFACTURER_ID);
  ASSERT_EQ(id[0], 0x20);
  ASSERT_EQ(MT25QU_WriteEnable(&h), MT25Q_OK);
  ASSERT_EQ(MT25QU_ReadStatus(&h, &st), MT25Q_OK);
  ASSERT_EQ(st & 0x02, 0x02);
  ASSERT_EQ(MT25QU_ReadFlagStatus(&h, &st), MT25Q_OK);
  ASSERT_EQ(st, 0x80);

  /* EN4B / EX4B both need the write enable latch on MT25Q */
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_Enter4ByteAddressMode(&h), MT25Q_OK);
  ASSERT_SEQUENCE(0, 0x06, 0xB7);
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_Exit4ByteAddressMode(&h), MT25Q_OK);
  ASSERT_SEQUENCE(0, 0x06, 0xE9);
  ASSERT_TRUE(!MockHAL_Is4ByteMode());

  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_PageProgramQuadEx(&h, 0x5000, tx, sizeof(tx), HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x32), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x5000), D4S(256), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(2), I1S(0x70), NOADDR, D1S(1));
  const MockEvent_t *poll = MockHAL_FindEvent(MOCK_EV_AUTOPOLL, 0);
  ASSERT_NOT_NULL(poll);
  ASSERT_EQ(poll->Poll.MatchMask, 0x80);
  ASSERT_EQ(poll->Poll.MatchValue, 0x80);

  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_ReadQuadEx(&h, 0x5000, rx, sizeof(rx), 10, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), A4S(HAL_XSPI_ADDRESS_24_BITS, 0x5000), NOALT, D4S(256), DUMMY(10));
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_ReadQuadEx(&h, 0x5000, rx, 4, 0, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), DUMMY(10));

  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseSector4KEx(&h, 0x5000, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x20), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x5000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseBlock64KEx(&h, 0x10000, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xD8), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x10000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseChip(&h), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xC7), NOADDR, NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseDie(&h, 0x04000000), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xC4), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x04000000), NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EnableMemoryMappedModeEx(&h, 10, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), .AddressWidth = HAL_XSPI_ADDRESS_24_BITS, DUMMY(10), OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EnableMemoryMappedModeEx(&h, 0, HAL_XSPI_ADDRESS_24_BITS), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), DUMMY(10), OPREAD);

  /* 32-bit wrappers (device in 4-byte mode) */
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_PageProgramQuad(&h, 0x6000, tx, 16), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x32), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x6000), D4S(16));
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_ReadQuad(&h, 0x6000, rx, 16, 10), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), A4S(HAL_XSPI_ADDRESS_32_BITS, 0x6000), D4S(16));
  ASSERT_EQ(memcmp(tx, rx, 16), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseSector4K(&h, 0x6000), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x20), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x6000));
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EraseBlock64K(&h, 0x10000), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xD8), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x10000));
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_EnableMemoryMappedMode(&h, 10), MT25Q_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xEB), .AddressWidth = HAL_XSPI_ADDRESS_32_BITS, OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(MT25QU_Reset(&h), MT25Q_OK);
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(MT25QU_EraseChip(&h), MT25Q_ERROR);
  MockHAL_SetPollTimeout(false);

  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_Init(&h, 2, HAL_XSPI_SIZE_512MB));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_ReadStatus(&h, &st));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_ReadFlagStatus(&h, &st));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_Enter4ByteAddressMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_Exit4ByteAddressMode(&h));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_PageProgramQuadEx(&h, 0, tx, 16, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_ReadQuadEx(&h, 0, rx, 16, 10, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_EraseSector4KEx(&h, 0, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_EraseBlock64KEx(&h, 0, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_EraseChip(&h));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_EraseDie(&h, 0));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_EnableMemoryMappedModeEx(&h, 10, HAL_XSPI_ADDRESS_24_BITS));
  FAULT_SWEEP(FLASH_SETUP(0x20, 0xBB, 0x20), MT25QU_Reset(&h));
  return true;
}
