/**
  ******************************************************************************
  * @file    test_octal_nor.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Protocol level tests for the octal NOR drivers:
  *          Infineon SEMPER S28Hx-T, ISSI IS25LX/WX and Micron MT35XU/XL.
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

#define FLASH_SETUP(mfg, type, dens) do { MockHAL_Reset(); MockHAL_SetFlashSemantics(true); \
                                          MockHAL_SetEmulatedChip((mfg), (type), (dens)); } while (0)

/* ========================================================================= */
/* Infineon SEMPER S28HS512T                                                 */
/* ========================================================================= */

static bool s28hs_octal_entry(void)
{
  XSPI_HandleTypeDef h = {0};
  FLASH_SETUP(0x34, 0x5B, 0x1A);

  ASSERT_EQ(S28HS512T_EnterOctalDTRMode(&h, 24), S28HS512T_OK);

  /* EN4B (no write enable needed), RDAR CFR2V, WREN, WRAR CFR2V, RDAR CFR3V, WREN, WRAR CFR3V, WREN, WRAR CFR5V */
  ASSERT_SEQUENCE(0, 0xB7, 0x65, 0x06, 0x71, 0x65, 0x06, 0x71, 0x06, 0x71);
  ASSERT_EQ(Test_CommandCount(), 9);
  ASSERT_CMD(Test_NthCommand(0), I1S(0xB7), NOADDR, NODATA, DUMMY(0));
  /* SPI volatile register reads: no latency with the factory VRGLAT = 00 */
  ASSERT_CMD(Test_NthCommand(1), I1S(0x65), A1S(HAL_XSPI_ADDRESS_32_BITS, S28HS_REG_CFR2_V), D1S(1), DUMMY(0), NODQS);
  ASSERT_CMD(Test_NthCommand(3), I1S(0x71), A1S(HAL_XSPI_ADDRESS_32_BITS, S28HS_REG_CFR2_V), D1S(1), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(4), I1S(0x65), A1S(HAL_XSPI_ADDRESS_32_BITS, S28HS_REG_CFR3_V), D1S(1), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(6), I1S(0x71), A1S(HAL_XSPI_ADDRESS_32_BITS, S28HS_REG_CFR3_V), D1S(1), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(8), I1S(0x71), A1S(HAL_XSPI_ADDRESS_32_BITS, S28HS_REG_CFR5_V), D1S(1), DUMMY(0));

  /* Register contents per datasheets 002-18216 / 002-23755: CFR2V = ADRBYT | MEMLAT 1011b (24 cycles in 8D),
   * CFR3V VRGLAT = 11b (6 cycles, 200 MHz DDR), CFR5V = 0x43 (OPI + DDR + bit 6) */
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800003), 0x8B);
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800004), 0xC0);
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800006), 0x43);
  ASSERT_EQ(S28HS_OCTAL_DTR_READ_DUMMY, 24);
  ASSERT_EQ(S28HS_OCTAL_DTR_REG_DUMMY, 6);

  /* Exit: 8D software reset reloads the factory (SPI) configuration */
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ExitOctalDTRMode(&h), S28HS512T_OK);
  ASSERT_EQ(Test_CommandCount(), 2);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x6666), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x9999), NOADDR, NODATA);
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800006), 0x40);
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800003), 0x08);
  ASSERT_TRUE(!MockHAL_Is4ByteMode());

  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_EnterOctalDTRMode(&h, 24));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_ExitOctalDTRMode(&h));
  return true;
}

static bool s28hs_dtr_operations(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t tx[256], rx[256];
  FLASH_SETUP(0x34, 0x5B, 0x1A);
  Test_Pattern(tx, sizeof(tx), 0x5A);

  ASSERT_EQ(S28HS512T_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0x00002000, tx, sizeof(tx)), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x0606), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x1212), A8D(0x00002000), D8D(256), DUMMY(0), WITHDQS);
  /* Status poll: RDAR SR1V (volatile) with 4-byte address, VRGLAT 11 latency (6), 2 bytes */
  ASSERT_CMD(Test_NthCommand(2), I8D(0x6565), A8D(S28HS_REG_STATUS1_V), D8D(2), DUMMY(6), WITHDQS);
  const MockEvent_t *poll = MockHAL_FindEvent(MOCK_EV_AUTOPOLL, 0);
  ASSERT_NOT_NULL(poll);
  ASSERT_EQ(poll->Poll.MatchMask, 0x01);
  ASSERT_EQ(poll->Poll.MatchValue, 0x00);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0x00002000, rx, sizeof(rx), 24), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xEEEE), A8D(0x00002000), D8D(256), DUMMY(24), WITHDQS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0x00002000), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x2121), A8D(0x00002000), NODATA);
  ASSERT_EQ(MockHAL_GetMemoryBuffer()[0x2000], 0xFF);

  MockHAL_SetBlockEraseSize(256U * 1024U);
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseBlock256K(&h, EXTMEM_MODE_OCTAL_DTR, 0x00000000), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xDCDC), A8D(0x00000000), NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x6060), NOADDR, NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EnableMemoryMappedModeDTR(&h, 24), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xEEEE), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, D8D(TEST_ANY), DUMMY(24), WITHDQS, OPREAD);
  ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_MEMMAPPED, 0));

  uint8_t reg = 0;
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ReadAnyReg(&h, EXTMEM_MODE_OCTAL_DTR, S28HS_REG_CFR2_V, &reg), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x6565), A8D(S28HS_REG_CFR2_V), D8D(2), DUMMY(6));
  ASSERT_EQ(reg, 0x08);
  ASSERT_EQ(S28HS512T_WriteEnable(&h, EXTMEM_MODE_OCTAL_DTR), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_WriteAnyReg(&h, EXTMEM_MODE_OCTAL_DTR, S28HS_REG_CFR2_V, 0x8B), S28HS512T_OK);
  ASSERT_EQ(MockHAL_GetAnyReg(S28HS_REG_CFR2_V), 0x8B);

  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0, tx, 16));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0, rx, 16, 24));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_EraseBlock256K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_EnableMemoryMappedModeDTR(&h, 24));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_ReadAnyReg(&h, EXTMEM_MODE_OCTAL_DTR, 0, &reg));
  return true;
}

static bool s28hs_spi_operations(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3] = {0}, tx[32], rx[32];
  FLASH_SETUP(0x34, 0x5B, 0x1A);
  Test_Pattern(tx, sizeof(tx), 0x11);

  ASSERT_EQ(S28HS512T_ReadID(&h, id), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x9F), NOADDR, D1S(3), DUMMY(0));
  ASSERT_EQ(id[0], 0x34); ASSERT_EQ(id[1], 0x5B); ASSERT_EQ(id[2], 0x1A);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_PageProgram(&h, EXTMEM_MODE_SPI, 0x100, tx, sizeof(tx)), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x06), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x12), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x100), D1S(32), DUMMY(0));
  /* SPI status poll uses RDSR1 (no address, no latency) */
  ASSERT_CMD(Test_NthCommand(2), I1S(0x05), NOADDR, D1S(1), DUMMY(0));

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_Read(&h, EXTMEM_MODE_SPI, 0x100, rx, sizeof(rx), 0), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x0B), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x100), D1S(32), DUMMY(8));
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseSector4K(&h, EXTMEM_MODE_SPI, 0x100), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x21), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x100), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseBlock256K(&h, EXTMEM_MODE_SPI, 0x40000), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xDC), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x40000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ChipErase(&h, EXTMEM_MODE_SPI), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x60), NOADDR, NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_Reset(&h), S28HS512T_OK);
  /* The reset reloads the factory address length: EN4BA restores 4-byte addressing for 0Bh / RDAR */
  ASSERT_SEQUENCE(0, 0x66, 0x99, 0xB7);
  ASSERT_TRUE(MockHAL_Is4ByteMode());

  /* Busy device: the poll times out, status shows no failure */
  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(S28HS512T_AutoPollingMemReady(&h, EXTMEM_MODE_SPI, 10), S28HS512T_TIMEOUT);
  ASSERT_EQ(S28HS512T_AutoPollingMemReady(&h, EXTMEM_MODE_OCTAL_DTR, 10), S28HS512T_TIMEOUT);
  MockHAL_SetPollTimeout(false);

  /* Failed program: SEMPER stays busy with PRGERR until CLPEF, the driver reports and clears it */
  for (int dtr = 0; dtr < 2; dtr++)
  {
    ExtMem_Mode_t m = dtr ? EXTMEM_MODE_OCTAL_DTR : EXTMEM_MODE_SPI;
    FLASH_SETUP(0x34, 0x5B, 0x1A);
    MockHAL_SetSemperFailure(true);
    ASSERT_EQ(S28HS512T_PageProgram(&h, m, 0x100, tx, 8), S28HS512T_ERROR);
    ASSERT_EQ(MockHAL_CountCommands(dtr ? 0x8282 : 0x82), 1);
    ASSERT_EQ(MockHAL_GetStatusRegister() & 0x61, 0);
    MockHAL_SetSemperFailure(false);
    FAULT_SWEEP_EXPECT(FLASH_SETUP(0x34, 0x5B, 0x1A); MockHAL_SetSemperFailure(true), S28HS512T_EraseSector4K(&h, m, 0), S28HS512T_ERROR);
  }

  /* Deep power down: ENDPD, then any short command (CS# pulse) wakes the device after tEXTDPD */
  for (int dtr = 0; dtr < 2; dtr++)
  {
    ExtMem_Mode_t m = dtr ? EXTMEM_MODE_OCTAL_DTR : EXTMEM_MODE_SPI;
    FLASH_SETUP(0x34, 0x5B, 0x1A);
    ASSERT_EQ(S28HS512T_EnterDeepPowerDown(&h, m), S28HS512T_OK);
    if (dtr) ASSERT_CMD(Test_NthCommand(0), I8D(0xB9B9), NOADDR, NODATA);
    else     ASSERT_CMD(Test_NthCommand(0), I1S(0xB9), NOADDR, NODATA);
    ASSERT_EQ(S28HS512T_LeaveDeepPowerDown(&h, m), S28HS512T_OK);
    if (dtr) ASSERT_CMD(Test_NthCommand(1), I8D(0x0404), NOADDR, NODATA);
    else     ASSERT_CMD(Test_NthCommand(1), I1S(0x04), NOADDR, NODATA);
    ASSERT_EQ(Test_CommandCount(), 2);
    FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_EnterDeepPowerDown(&h, m));
    FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_LeaveDeepPowerDown(&h, m));
  }

  S28HS512T_Info_t info;
  ASSERT_EQ(S28HS512T_GetInfo(NULL), S28HS512T_ERROR);
  ASSERT_EQ(S28HS512T_GetInfo(&info), S28HS512T_OK);
  ASSERT_EQ(info.FlashSize, 64U * 1024U * 1024U);
  ASSERT_EQ(info.ManufacturerID, 0x34);
  ASSERT_EQ(info.DeviceID, 0x5B);

  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_PageProgram(&h, EXTMEM_MODE_SPI, 0, tx, 8));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_Reset(&h));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1A), S28HS512T_WriteAnyReg(&h, EXTMEM_MODE_SPI, S28HS_REG_CFR2_V, 0));
  return true;
}

static bool s28hs_multi_die(void)
{
  XSPI_HandleTypeDef h = {0}, h2 = {0}, h3 = {0}, h4 = {0};
  uint8_t tx[16];
  S28HS512T_DieLayout_t l = { 2U, 128U * 1024U * 1024U, { 0x00800000U, 0x08800000U, 0U, 0U } };
  Test_Pattern(tx, sizeof(tx), 0x2D);

  /* Layout validation and registry */
  S28HS512T_DieLayout_t bad = l;
  bad.Dice = 0; ASSERT_EQ(S28HS512T_SetDieLayout(&h, &bad), S28HS512T_ERROR);
  bad.Dice = 5; ASSERT_EQ(S28HS512T_SetDieLayout(&h, &bad), S28HS512T_ERROR);
  bad.Dice = 2; bad.DieSize = 0; ASSERT_EQ(S28HS512T_SetDieLayout(&h, &bad), S28HS512T_ERROR);
  ASSERT_EQ(S28HS512T_SetDieLayout(&h, &l), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_SetDieLayout(&h, &l), S28HS512T_OK);   /* re-registering replaces */
  ASSERT_EQ(S28HS512T_SetDieLayout(&h2, &l), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_SetDieLayout(&h3, &l), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_SetDieLayout(&h4, &l), S28HS512T_ERROR); /* no free slot */
  ASSERT_EQ(S28HS512T_SetDieLayout(&h2, NULL), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_SetDieLayout(&h3, NULL), S28HS512T_OK);

  /* Octal entry configures every die, die 0 last */
  FLASH_SETUP(0x34, 0x5B, 0x1C);
  ASSERT_EQ(S28HS512T_EnterOctalDTRMode(&h, 24), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x65), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x08800003U), D1S(1));
  /* WRENB set WRPGEN in both dice: one 8D WRDIS once every die is in 8D-8D-8D (002-23755, 5.7.1) */
  ASSERT_CMD(Test_NthCommand(Test_CommandCount() - 1U), I8D(0x0404), NOADDR, NODATA);
  ASSERT_EQ(MockHAL_GetAnyReg(0x08800003U), 0x8B);
  ASSERT_EQ(MockHAL_GetAnyReg(0x08800004U), 0xC0);
  ASSERT_EQ(MockHAL_GetAnyReg(0x08800006U), 0x43);
  ASSERT_EQ(MockHAL_GetAnyReg(0x00800006U), 0x43);
  ASSERT_EQ(MockHAL_CountCommands(0x71), 6);

  /* Program in die 1 polls STR1V of die 1 */
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0x08000100U, tx, sizeof(tx)), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(2), I8D(0x6565), A8D(0x08800000U), D8D(2), DUMMY(6));
  ASSERT_CMD(Test_NthCommand(3), I8D(0x0404), NOADDR, NODATA);
  ASSERT_EQ(Test_CommandCount(), 4);
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseSector4K(&h, EXTMEM_MODE_SPI, 0x09000000U), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(2), I1S(0x65), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x08800000U), D1S(1), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseBlock256K(&h, EXTMEM_MODE_SPI, 0x00000000U), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(2), I1S(0x05), NOADDR, D1S(1));
  /* Addresses beyond the last die map to the last die */
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_EraseBlock256K(&h, EXTMEM_MODE_SPI, 0x20000000U), S28HS512T_OK);
  ASSERT_CMD(Test_NthCommand(2), I1S(0x65), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x08800000U));

  /* Chip erase = one DIE ERASE per die at the die base, each polled on its own die */
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR), S28HS512T_OK);
  ASSERT_CMD(MockHAL_FindCommand(0x6161, 0), I8D(0x6161), A8D(0x00000000U), NODATA);
  ASSERT_CMD(MockHAL_FindCommand(0x6161, 1), I8D(0x6161), A8D(0x08000000U), NODATA);
  ASSERT_EQ(MockHAL_CountCommands(0x6060), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(S28HS512T_ChipErase(&h, EXTMEM_MODE_SPI), S28HS512T_OK);
  ASSERT_CMD(MockHAL_FindCommand(0x61, 1), I1S(0x61), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x08000000U), NODATA);
  ASSERT_EQ(MockHAL_CountCommands(0x04), 2);

  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1C), S28HS512T_EnterOctalDTRMode(&h, 24));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1C), S28HS512T_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1C), S28HS512T_ChipErase(&h, EXTMEM_MODE_SPI));
  FAULT_SWEEP(FLASH_SETUP(0x34, 0x5B, 0x1C), S28HS512T_EraseSector4K(&h, EXTMEM_MODE_SPI, 0x09000000U));

  ASSERT_EQ(S28HS512T_SetDieLayout(&h, NULL), S28HS512T_OK);
  return true;
}

bool test_infineon_s28hs512t_octal_flash(void)
{
  return s28hs_octal_entry() && s28hs_dtr_operations() && s28hs_spi_operations() && s28hs_multi_die();
}

/* ========================================================================= */
/* ISSI IS25LX / IS25WX                                                      */
/* ========================================================================= */

static bool is25lx_octal_entry(void)
{
  XSPI_HandleTypeDef h = {0};
  FLASH_SETUP(0x9D, 0x5A, 0x19);

  ASSERT_EQ(IS25LX256_EnterOctalDTRMode(&h, 20), IS25LX_OK);
  /* WREN, EN4B, WREN, WRVCR 0x01 (dummy), WREN, WRVCR 0x00 (I/O mode) */
  ASSERT_SEQUENCE(0, 0x06, 0xB7, 0x06, 0x81, 0x06, 0x81);
  ASSERT_CMD(Test_NthCommand(3), I1S(0x81), A1S(HAL_XSPI_ADDRESS_32_BITS, IS25LX_VCR_ADDR_DUMMY_CYCLES), D1S(1), DUMMY(0));
  ASSERT_CMD(Test_NthCommand(5), I1S(0x81), A1S(HAL_XSPI_ADDRESS_32_BITS, IS25LX_VCR_ADDR_IO_MODE), D1S(1), DUMMY(0));
  ASSERT_EQ(IS25LX_VCR_ADDR_IO_MODE, 0x00);
  ASSERT_EQ(IS25LX_VCR_ADDR_DUMMY_CYCLES, 0x01);
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  ASSERT_EQ(MockHAL_GetVCR(0), 0xE7);   /* Octal DDR with DQS */
  ASSERT_EQ(MockHAL_GetVCR(1), 20);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_ExitOctalDTRMode(&h), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x0606), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x8181), A8D(0x00), D8D(2), DUMMY(0));
  ASSERT_EQ(MockHAL_GetVCR(0), 0xFF);
  ASSERT_EQ(MockHAL_GetVCR(1), 0xFF);   /* default dummy cycle setting */

  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_EnterOctalDTRMode(&h, 20));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_ExitOctalDTRMode(&h));
  return true;
}

static bool is25lx_operations(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], tx[64], rx[64], v = 0;
  FLASH_SETUP(0x9D, 0x5A, 0x19);
  Test_Pattern(tx, sizeof(tx), 0x33);

  ASSERT_EQ(IS25LX256_ReadID(&h, id), IS25LX_OK);
  ASSERT_EQ(id[0], 0x9D); ASSERT_EQ(id[1], 0x5A);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0x3000, tx, sizeof(tx)), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x0606), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x1212), A8D(0x3000), D8D(64), DUMMY(0));
  /* 8D read status: no address, 8 dummy, 2 bytes */
  ASSERT_CMD(Test_NthCommand(2), I8D(0x0505), NOADDR, D8D(2), DUMMY(8), WITHDQS);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0x3000, rx, sizeof(rx), 20), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), A8D(0x3000), D8D(64), DUMMY(20), WITHDQS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0x3000), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x2121), A8D(0x3000), NODATA);
  MockHAL_SetBlockEraseSize(128U * 1024U);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_EraseBlock128K(&h, EXTMEM_MODE_OCTAL_DTR, 0x20000), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xDCDC), A8D(0x20000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xC7C7), NOADDR, NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_EnableMemoryMappedModeDTR(&h, 20), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), .AddressMode = HAL_XSPI_ADDRESS_8_LINES, DUMMY(20), WITHDQS, OPREAD);

  /* Extended SPI paths */
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_PageProgram(&h, EXTMEM_MODE_SPI, 0x10, tx, 8), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x12), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x10), D1S(8));
  ASSERT_CMD(Test_NthCommand(2), I1S(0x05), NOADDR, D1S(1), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_Read(&h, EXTMEM_MODE_SPI, 0x10, rx, 8, 0), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x0C), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x10), D1S(8), DUMMY(8));
  ASSERT_EQ(memcmp(tx, rx, 8), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_EraseSector4K(&h, EXTMEM_MODE_SPI, 0), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x21), A1S(HAL_XSPI_ADDRESS_32_BITS, 0), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_EraseBlock128K(&h, EXTMEM_MODE_SPI, 0), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xDC), A1S(HAL_XSPI_ADDRESS_32_BITS, 0), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_ChipErase(&h, EXTMEM_MODE_SPI), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x60), NOADDR, NODATA);

  /* Volatile configuration register read in both protocols */
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_ReadVCR(&h, EXTMEM_MODE_SPI, IS25LX_VCR_ADDR_IO_MODE, &v), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x85), A1S(HAL_XSPI_ADDRESS_32_BITS, 0), D1S(1), DUMMY(8));
  ASSERT_EQ(v, 0xFF);
  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_ReadVCR(&h, EXTMEM_MODE_OCTAL_DTR, IS25LX_VCR_ADDR_IO_MODE, &v), IS25LX_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x8585), A8D(0), D8D(2), DUMMY(8));

  MockHAL_ClearLog();
  ASSERT_EQ(IS25LX256_Reset(&h), IS25LX_OK);
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(IS25LX256_AutoPollingMemReady(&h, EXTMEM_MODE_SPI, 1), IS25LX_TIMEOUT);
  MockHAL_SetPollTimeout(false);

  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0, tx, 16));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0, rx, 16, 20));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_EraseBlock128K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_ChipErase(&h, EXTMEM_MODE_OCTAL_DTR));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_EnableMemoryMappedModeDTR(&h, 20));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_ReadVCR(&h, EXTMEM_MODE_OCTAL_DTR, 0, &v));
  FAULT_SWEEP(FLASH_SETUP(0x9D, 0x5A, 0x19), IS25LX256_Reset(&h));
  return true;
}

bool test_issi_is25lx256_octal_flash(void)
{
  return is25lx_octal_entry() && is25lx_operations();
}

/* ========================================================================= */
/* Micron MT35XU / MT35XL                                                    */
/* ========================================================================= */

static bool mt35xu_octal_entry(void)
{
  XSPI_HandleTypeDef h = {0};
  FLASH_SETUP(0x2C, 0x5B, 0x1A);

  ASSERT_EQ(MT35XU_Init(&h, 2, HAL_XSPI_SIZE_512MB), MT35XU_OK);
  const MockEvent_t *init = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0);
  ASSERT_NOT_NULL(init);
  ASSERT_EQ(init->Init.MemoryType, HAL_XSPI_MEMTYPE_MICRON);
  ASSERT_EQ(init->Init.MemorySize, HAL_XSPI_SIZE_512MB);
  ASSERT_SEQUENCE(0, 0x66, 0x99);

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EnterOctalDTRMode(&h, 20), MT35XU_OK);
  /* WREN, EN4B (Micron needs WEL), WREN, WRVCR 0x01, WREN, WRVCR 0x00 */
  ASSERT_SEQUENCE(0, 0x06, 0xB7, 0x06, 0x81, 0x06, 0x81);
  ASSERT_CMD(Test_NthCommand(3), I1S(0x81), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x01), D1S(1));
  ASSERT_CMD(Test_NthCommand(5), I1S(0x81), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x00), D1S(1));
  ASSERT_TRUE(MockHAL_Is4ByteMode());
  ASSERT_EQ(MockHAL_GetVCR(0), 0xE7);
  ASSERT_EQ(MockHAL_GetVCR(1), 20);

  /* Default dummy count when the caller passes 0 */
  FLASH_SETUP(0x2C, 0x5B, 0x1A);
  ASSERT_EQ(MT35XU_EnterOctalDTRMode(&h, 0), MT35XU_OK);
  ASSERT_EQ(MockHAL_GetVCR(1), 20);

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_ExitOctalMode(&h), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x0606), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x8181), A8D(0x00), D8D(2));
  ASSERT_EQ(MockHAL_GetVCR(0), 0xFF);
  ASSERT_EQ(MockHAL_GetVCR(1), 0x1F);

  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_Init(&h, 2, HAL_XSPI_SIZE_512MB));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EnterOctalDTRMode(&h, 20));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_ExitOctalMode(&h));
  return true;
}

static bool mt35xu_operations(void)
{
  XSPI_HandleTypeDef h = {0};
  uint8_t id[3], st = 0, tx[128], rx[128];
  FLASH_SETUP(0x2C, 0x5B, 0x1A);
  Test_Pattern(tx, sizeof(tx), 0x77);

  ASSERT_EQ(MT35XU_ReadID(&h, id), MT35XU_OK);
  ASSERT_EQ(id[0], 0x2C); ASSERT_EQ(id[1], 0x5B); ASSERT_EQ(id[2], 0x1A);
  ASSERT_EQ(MT35XU_WriteEnable(&h), MT35XU_OK);
  ASSERT_EQ(MT35XU_ReadStatus(&h, &st), MT35XU_OK);
  ASSERT_EQ(st & 0x02, 0x02);
  ASSERT_EQ(MT35XU_ReadFlagStatus(&h, &st), MT35XU_OK);
  ASSERT_EQ(st, 0x80);

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0x4000, tx, sizeof(tx)), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0x0606), NOADDR, NODATA);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x1212), A8D(0x4000), D8D(128), DUMMY(0));
  /* Flag status poll: 8D, no address, 8 dummy, 2 bytes, ready bit 7 */
  ASSERT_CMD(Test_NthCommand(2), I8D(0x7070), NOADDR, D8D(2), DUMMY(8), WITHDQS);
  const MockEvent_t *poll = MockHAL_FindEvent(MOCK_EV_AUTOPOLL, 0);
  ASSERT_NOT_NULL(poll);
  ASSERT_EQ(poll->Poll.MatchMask, 0x80);
  ASSERT_EQ(poll->Poll.MatchValue, 0x80);

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0x4000, rx, sizeof(rx), 20), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), A8D(0x4000), D8D(128), DUMMY(20), WITHDQS);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0x4000, rx, 2, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), DUMMY(16));

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0x4000), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0x2121), A8D(0x4000), NODATA);
  MockHAL_SetBlockEraseSize(128U * 1024U);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseBlock128K(&h, EXTMEM_MODE_OCTAL_DTR, 0x20000), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xDCDC), A8D(0x20000), NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseChip(&h, EXTMEM_MODE_OCTAL_DTR), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xC7C7), NOADDR, NODATA);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseDie(&h, EXTMEM_MODE_OCTAL_DTR, 0x04000000), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I8D(0xC4C4), A8D(0x04000000), NODATA);

  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EnableMemoryMappedModeDTR(&h, 20), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), DUMMY(20), WITHDQS, OPREAD);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EnableMemoryMappedModeDTR(&h, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I8D(0xFDFD), DUMMY(16), OPREAD);

  /* Extended SPI */
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_PageProgram(&h, EXTMEM_MODE_SPI, 0x20, tx, 16), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x12), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x20), D1S(16));
  ASSERT_CMD(Test_NthCommand(2), I1S(0x70), NOADDR, D1S(1), DUMMY(0));
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_Read(&h, EXTMEM_MODE_SPI, 0x20, rx, 16, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x0C), A1S(HAL_XSPI_ADDRESS_32_BITS, 0x20), D1S(16), DUMMY(8));
  ASSERT_EQ(memcmp(tx, rx, 16), 0);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseSector4K(&h, EXTMEM_MODE_SPI, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x21), A1S(HAL_XSPI_ADDRESS_32_BITS, 0));
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseBlock128K(&h, EXTMEM_MODE_SPI, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xDC), A1S(HAL_XSPI_ADDRESS_32_BITS, 0));
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseChip(&h, EXTMEM_MODE_SPI), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xC7), NOADDR);
  MockHAL_ClearLog();
  ASSERT_EQ(MT35XU_EraseDie(&h, EXTMEM_MODE_SPI, 0), MT35XU_OK);
  ASSERT_CMD(Test_NthCommand(1), I1S(0xC4), A1S(HAL_XSPI_ADDRESS_32_BITS, 0));

  MockHAL_SetPollTimeout(true);
  ASSERT_EQ(MT35XU_EraseChip(&h, EXTMEM_MODE_SPI), MT35XU_ERROR);
  MockHAL_SetPollTimeout(false);

  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_ReadID(&h, id));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_ReadStatus(&h, &st));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_ReadFlagStatus(&h, &st));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_PageProgram(&h, EXTMEM_MODE_OCTAL_DTR, 0, tx, 16));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_PageProgram(&h, EXTMEM_MODE_SPI, 0, tx, 16));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_Read(&h, EXTMEM_MODE_OCTAL_DTR, 0, rx, 16, 20));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseSector4K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseSector4K(&h, EXTMEM_MODE_SPI, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseBlock128K(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseBlock128K(&h, EXTMEM_MODE_SPI, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseChip(&h, EXTMEM_MODE_OCTAL_DTR));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseChip(&h, EXTMEM_MODE_SPI));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseDie(&h, EXTMEM_MODE_OCTAL_DTR, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EraseDie(&h, EXTMEM_MODE_SPI, 0));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_EnableMemoryMappedModeDTR(&h, 20));
  FAULT_SWEEP(FLASH_SETUP(0x2C, 0x5B, 0x1A), MT35XU_Reset(&h));

  /* Stacked dice: every die must report ready in consecutive FSR reads (3 extra reads after the poll) */
  for (int dtr = 0; dtr < 2; dtr++)
  {
    ExtMem_Mode_t m = dtr ? EXTMEM_MODE_OCTAL_DTR : EXTMEM_MODE_SPI;
    FLASH_SETUP(0x2C, 0x5B, 0x1C);
    MockHAL_SetStackedBusyReads(1);   /* another die still busy once: poll again */
    ASSERT_EQ(MT35XU_EraseSector4K(&h, m, 0x08000000U), MT35XU_OK);
    ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_AUTOPOLL, 1));
    ASSERT_EQ(MockHAL_CountCommands(dtr ? 0x7070 : 0x70), 2U * MT35XU_MAX_DICE);
    MockHAL_SetStackedBusyReads(MOCK_STACKED_BUSY_FOREVER);
    ASSERT_EQ(MT35XU_EraseSector4K(&h, m, 0), MT35XU_ERROR);
    MockHAL_SetStackedBusyReads(0);
  }
  return true;
}

bool test_micron_mt35xu512a_octal_flash(void)
{
  return mt35xu_octal_entry() && mt35xu_operations();
}
