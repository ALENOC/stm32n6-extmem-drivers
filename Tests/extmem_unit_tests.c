/**
  ******************************************************************************
  * @file    extmem_unit_tests.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Unit test implementations verifying all memory drivers.
  ******************************************************************************
  */

#include "extmem_unit_tests.h"
#include "mock_hal.h"
#include "stm32n6_extmem.h"
#include <stdio.h>
#include <string.h>

#define ASSERT_TRUE(cond) do { if (!(cond)) { printf("       [FAIL] Line %d: %s\r\n", __LINE__, #cond); return false; } } while (0)
#define ASSERT_EQ(a, b)   do { if ((a) != (b)) { printf("       [FAIL] Line %d: %s != %s (%ld != %ld)\r\n", __LINE__, #a, #b, (long)(a), (long)(b)); return false; } } while (0)

/* 1. SFDP Parser Test */
bool test_sfdp_parser(void)
{
  MockHAL_Reset();
  XSPI_HandleTypeDef hxspi = {0};
  SFDP_FlashParams_t params;

  int32_t ret = SFDP_ReadAndParse(&hxspi, &params);
  ASSERT_EQ(ret, EXTMEM_OK);
  ASSERT_EQ(params.DensityBytes, (64 * 1024 * 1024));
  ASSERT_TRUE(params.Supports4KBErase);
  ASSERT_EQ(params.PageSizeBytes, 256);
  ASSERT_TRUE(params.SupportsQuad_1_4_4);
  ASSERT_TRUE(params.SupportsOctal_8D_8D_8D);
  return true;
}

/* 2. Infineon SEMPER Octal NOR Flash (S28HS512T) */
bool test_infineon_s28hs512t_octal_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x34, 0x5B, 0x1A); /* S28HS512T */
  XSPI_HandleTypeDef hxspi = {0};
  uint8_t id[3] = {0};

  ASSERT_EQ(S28HS512T_ReadID(&hxspi, id), S28HS512T_OK);
  ASSERT_EQ(id[0], 0x34);
  ASSERT_EQ(id[1], 0x5B);
  ASSERT_EQ(id[2], 0x1A);

  ASSERT_EQ(S28HS512T_WriteEnable(&hxspi, EXTMEM_MODE_SPI), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_EnterOctalDTRMode(&hxspi, 20), S28HS512T_OK);

  /* Write and Read back */
  uint8_t writeBuf[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
  uint8_t readBuf[16] = {0};

  ASSERT_EQ(S28HS512T_PageProgram(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00001000, writeBuf, sizeof(writeBuf)), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_Read(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00001000, readBuf, sizeof(readBuf), 20), S28HS512T_OK);
  ASSERT_EQ(memcmp(writeBuf, readBuf, sizeof(writeBuf)), 0);

  /* Erase Sector */
  ASSERT_EQ(S28HS512T_EraseSector4K(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00001000), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_Read(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00001000, readBuf, sizeof(readBuf), 20), S28HS512T_OK);
  for (int i = 0; i < 16; i++) ASSERT_EQ(readBuf[i], 0xFF);

  /* Memory Mapped Activation */
  ASSERT_EQ(S28HS512T_EnableMemoryMappedModeDTR(&hxspi, 20), S28HS512T_OK);
  ASSERT_EQ(S28HS512T_ExitOctalDTRMode(&hxspi), S28HS512T_OK);
  return true;
}

/* 3. Infineon HyperFlash (S26KS512S) */
bool test_infineon_s26ks512s_hyperflash(void)
{
  MockHAL_Reset();
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(S26KS512S_Init(&hxspi, 2), S26KS512S_OK);

  uint8_t data[8] = {0xAA, 0x55, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  uint8_t rdata[8] = {0};

  ASSERT_EQ(S26KS512S_ProgramBuffer(&hxspi, 0x00002000, data, sizeof(data)), S26KS512S_OK);
  ASSERT_EQ(S26KS512S_Read(&hxspi, 0x00002000, rdata, sizeof(rdata)), S26KS512S_OK);
  ASSERT_EQ(memcmp(data, rdata, sizeof(data)), 0);

  ASSERT_EQ(S26KS512S_EraseSector(&hxspi, 0x00002000), S26KS512S_OK);
  ASSERT_EQ(S26KS512S_Read(&hxspi, 0x00002000, rdata, sizeof(rdata)), S26KS512S_OK);
  for (int i = 0; i < 8; i++) ASSERT_EQ(rdata[i], 0xFF);

  ASSERT_EQ(S26KS512S_EnableMemoryMappedMode(&hxspi), S26KS512S_OK);
  return true;
}

/* 4. Infineon HyperRAM (S27KS0641) */
bool test_infineon_s27ks0641_hyperram(void)
{
  MockHAL_Reset();
  MockHAL_SetHyperBusID(0x0001, 0x0000); /* Cypress / Infineon */
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(S27KS0641_Init(&hxspi, 2, HAL_XSPI_SIZE_32MB), S27KS_OK);

  uint16_t id0 = 0;
  ASSERT_EQ(S27KS0641_ReadRegister(&hxspi, S27KS_REG_ID0, &id0), S27KS_OK);
  ASSERT_EQ(id0, 0x0001);

  uint8_t tx[32], rx[32];
  for (int i = 0; i < 32; i++) tx[i] = (uint8_t)(i + 0x40);

  ASSERT_EQ(S27KS0641_Write(&hxspi, 0x00000100, tx, sizeof(tx)), S27KS_OK);
  ASSERT_EQ(S27KS0641_Read(&hxspi, 0x00000100, rx, sizeof(rx)), S27KS_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  ASSERT_EQ(S27KS0641_EnableMemoryMappedMode(&hxspi), S27KS_OK);
  return true;
}

/* 5. Infineon SEMPER / FL Quad Flash (S25HL512T) */
bool test_infineon_s25hl512t_quad_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x34, 0x2A, 0x1A);
  XSPI_HandleTypeDef hxspi = {0};

  uint8_t id[3] = {0};
  ASSERT_EQ(S25HL512T_ReadID(&hxspi, id), S25HL512T_OK);
  ASSERT_EQ(id[0], 0x34);

  ASSERT_EQ(S25HL512T_EnableQuadMode(&hxspi), S25HL512T_OK);

  uint8_t tx[16] = {0xDE, 0xAD, 0xBE, 0xEF};
  uint8_t rx[16] = {0};

  ASSERT_EQ(S25HL512T_PageProgramQuad(&hxspi, 0x00001000, tx, 4), S25HL512T_OK);
  ASSERT_EQ(S25HL512T_ReadQuad(&hxspi, 0x00001000, rx, 4, 6), S25HL512T_OK);
  ASSERT_EQ(memcmp(tx, rx, 4), 0);

  ASSERT_EQ(S25HL512T_EraseSector4K(&hxspi, 0x00001000), S25HL512T_OK);
  ASSERT_EQ(S25HL512T_EnableMemoryMappedMode(&hxspi, 6), S25HL512T_OK);
  return true;
}

/* 6. ISSI Octal NOR Flash (IS25LX256) */
bool test_issi_is25lx256_octal_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x9D, 0x5B, 0x19); /* IS25LX256 */
  XSPI_HandleTypeDef hxspi = {0};

  uint8_t id[3] = {0};
  ASSERT_EQ(IS25LX256_ReadID(&hxspi, id), IS25LX_OK);
  ASSERT_EQ(id[0], 0x9D);
  ASSERT_EQ(id[1], 0x5B);

  ASSERT_EQ(IS25LX256_EnterOctalDTRMode(&hxspi, 20), IS25LX_OK);

  uint8_t tx[8] = {10, 20, 30, 40, 50, 60, 70, 80};
  uint8_t rx[8] = {0};

  ASSERT_EQ(IS25LX256_PageProgram(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00000500, tx, sizeof(tx)), IS25LX_OK);
  ASSERT_EQ(IS25LX256_Read(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00000500, rx, sizeof(rx), 20), IS25LX_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  ASSERT_EQ(IS25LX256_EraseSector4K(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00000500), IS25LX_OK);
  ASSERT_EQ(IS25LX256_EnableMemoryMappedModeDTR(&hxspi, 20), IS25LX_OK);
  ASSERT_EQ(IS25LX256_ExitOctalDTRMode(&hxspi), IS25LX_OK);
  return true;
}

/* 7. ISSI Quad SPI Flash (IS25LP256) */
bool test_issi_is25lp256_quad_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x9D, 0x60, 0x19);
  XSPI_HandleTypeDef hxspi = {0};

  uint8_t id[3] = {0};
  ASSERT_EQ(IS25LP256_ReadID(&hxspi, id), IS25LP_OK);
  ASSERT_EQ(id[0], 0x9D);

  ASSERT_EQ(IS25LP256_EnableQuadMode(&hxspi), IS25LP_OK);

  uint8_t tx[4] = {0x12, 0x34, 0x56, 0x78};
  uint8_t rx[4] = {0};

  ASSERT_EQ(IS25LP256_PageProgramQuad(&hxspi, 0x00000800, tx, 4), IS25LP_OK);
  ASSERT_EQ(IS25LP256_ReadQuad(&hxspi, 0x00000800, rx, 4, 6), IS25LP_OK);
  ASSERT_EQ(memcmp(tx, rx, 4), 0);

  ASSERT_EQ(IS25LP256_EraseSector4K(&hxspi, 0x00000800), IS25LP_OK);
  ASSERT_EQ(IS25LP256_EnableMemoryMappedMode(&hxspi, 6), IS25LP_OK);
  return true;
}

/* 8. ISSI Octal PSRAM (IS66WVO32M8) */
bool test_issi_is66wvo32m8_octal_psram(void)
{
  MockHAL_Reset();
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(IS66WVO32M8_Init(&hxspi, 2, HAL_XSPI_SIZE_32MB), IS66WVO_OK);

  uint8_t tx[64], rx[64];
  for (int i = 0; i < 64; i++) tx[i] = (uint8_t)i;

  ASSERT_EQ(IS66WVO32M8_Write(&hxspi, tx, 0x00000200, sizeof(tx), 5), IS66WVO_OK);
  ASSERT_EQ(IS66WVO32M8_Read(&hxspi, rx, 0x00000200, sizeof(rx), 5), IS66WVO_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  ASSERT_EQ(IS66WVO32M8_EnableMemoryMappedMode(&hxspi, 5, 5), IS66WVO_OK);
  return true;
}

/* 9. ISSI HyperRAM PSRAM (IS66WVH16M8) */
bool test_issi_is66wvh16m8_hyperram(void)
{
  MockHAL_Reset();
  MockHAL_SetHyperBusID(0x000F, 0x0000); /* ISSI Vendor ID in HyperBus */
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(IS66WVH16M8_Init(&hxspi, 2, HAL_XSPI_SIZE_16MB), IS66WVH_OK);

  uint16_t id0 = 0;
  ASSERT_EQ(IS66WVH16M8_ReadRegister(&hxspi, IS66WVH_REG_ID0, &id0), IS66WVH_OK);
  ASSERT_EQ(id0, 0x000F);

  uint8_t tx[16] = {1, 3, 5, 7, 9, 11, 13, 15};
  uint8_t rx[16] = {0};

  ASSERT_EQ(IS66WVH16M8_Write(&hxspi, 0x00000300, tx, 8), IS66WVH_OK);
  ASSERT_EQ(IS66WVH16M8_Read(&hxspi, 0x00000300, rx, 8), IS66WVH_OK);
  ASSERT_EQ(memcmp(tx, rx, 8), 0);

  ASSERT_EQ(IS66WVH16M8_EnableMemoryMappedMode(&hxspi), IS66WVH_OK);
  return true;
}

/* 10. ISSI Quad SPI PSRAM (IS66WVS16M8) */
bool test_issi_is66wvs16m8_quad_psram(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x9D, 0x5D, 0x02);
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(IS66WVS16M8_Init(&hxspi, 2, HAL_XSPI_SIZE_16MB), IS66WVS_OK);

  uint8_t id[3] = {0};
  ASSERT_EQ(IS66WVS16M8_ReadID(&hxspi, id), IS66WVS_OK);
  ASSERT_EQ(id[0], 0x9D);

  ASSERT_EQ(IS66WVS16M8_EnterQuadMode(&hxspi), IS66WVS_OK);

  uint8_t tx[8] = {0x55, 0xAA, 0x33, 0xCC, 0x11, 0x88, 0x77, 0x00};
  uint8_t rx[8] = {0};

  ASSERT_EQ(IS66WVS16M8_WriteQuad(&hxspi, 0x00000400, tx, sizeof(tx)), IS66WVS_OK);
  ASSERT_EQ(IS66WVS16M8_ReadQuad(&hxspi, 0x00000400, rx, sizeof(rx), 6), IS66WVS_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  ASSERT_EQ(IS66WVS16M8_EnableMemoryMappedMode(&hxspi, 6), IS66WVS_OK);
  return true;
}

/* 11. Parallel Asynchronous PSRAM for FMC */
bool test_issi_is66wv_fmc_parallel_psram(void)
{
  MockHAL_Reset();
  SRAM_HandleTypeDef hsram = {0};
  IS66WV_FMC_Timing_t timing = { .AddressSetupTime = 3, .AddressHoldTime = 1, .DataSetupTime = 5, .BusTurnAroundDuration = 1 };

  ASSERT_EQ(IS66WV_FMC_Init(&hsram, FMC_NORSRAM_BANK1, &timing), IS66WV_FMC_OK);

  uint32_t baseAddr = 0x60000000;

  uint8_t tx[16] = {0xCA, 0xFE, 0xBA, 0xBE};
  uint8_t rx[16] = {0};

  ASSERT_EQ(IS66WV_FMC_Write(baseAddr, 0x0000, tx, 4), IS66WV_FMC_OK);
  ASSERT_EQ(IS66WV_FMC_Read(baseAddr, 0x0000, rx, 4), IS66WV_FMC_OK);
  ASSERT_EQ(memcmp(tx, rx, 4), 0);

  ASSERT_EQ(IS66WV_FMC_TestPattern(baseAddr, 1024), IS66WV_FMC_OK);
  return true;
}

/* 12. ISSI IS29GL Parallel NOR Flash via FMC (16-bit) */
bool test_issi_is29gl_fmc_parallel_nor_flash(void)
{
  MockHAL_Reset();
  SRAM_HandleTypeDef hsram = {0};
  IS29GL_FMC_Timing_t timing = { .AddressSetupTime = 4, .AddressHoldTime = 2, .DataSetupTime = 7, .BusTurnAroundDuration = 2 };

  ASSERT_EQ(IS29GL_FMC_Init(&hsram, FMC_NORSRAM_BANK1, &timing), IS29GL_FMC_OK);
  ASSERT_EQ(IS29GL_FMC_Reset(0x60000000), IS29GL_FMC_OK);

  uint8_t tx[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
  uint8_t rx[8] = {0};

  ASSERT_EQ(IS29GL_FMC_ProgramBuffer(0x60000000, 0x00002000, tx, sizeof(tx)), IS29GL_FMC_OK);
  ASSERT_EQ(IS29GL_FMC_Read(0x60000000, 0x00002000, rx, sizeof(rx)), IS29GL_FMC_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  ASSERT_EQ(IS29GL_FMC_EraseSector(0x60000000, 0x00002000), IS29GL_FMC_OK);
  ASSERT_EQ(IS29GL_FMC_Read(0x60000000, 0x00002000, rx, sizeof(rx)), IS29GL_FMC_OK);
  for (int i = 0; i < 8; i++) ASSERT_EQ(rx[i], 0xFF);

  return true;
}

/* 13. ISSI Serial SRAM (IS62WVS / IS65WVS) */
bool test_issi_is62wvs_serial_sram(void)
{
  MockHAL_Reset();
  XSPI_HandleTypeDef hxspi = {0};

  ASSERT_EQ(IS62WVS_Init(&hxspi, 2, HAL_XSPI_SIZE_512KB), IS62WVS_OK);

  /* Verify Mode Register is Sequential Mode (0x40) */
  uint8_t mode = 0;
  ASSERT_EQ(IS62WVS_ReadModeRegister(&hxspi, &mode), IS62WVS_OK);
  ASSERT_EQ(mode, IS62WVS_MODE_SEQUENTIAL);

  /* Test Quad mode enter & exit */
  ASSERT_EQ(IS62WVS_EnterQuadMode(&hxspi), IS62WVS_OK);
  ASSERT_EQ(IS62WVS_ExitQuadMode(&hxspi), IS62WVS_OK);

  /* Test Data Write and Read */
  uint8_t tx[64];
  uint8_t rx[64];
  for (int i = 0; i < 64; i++) tx[i] = (uint8_t)(0x55 ^ i);

  ASSERT_EQ(IS62WVS_Write(&hxspi, 0x00000400, tx, sizeof(tx)), IS62WVS_OK);
  ASSERT_EQ(IS62WVS_Read(&hxspi, 0x00000400, rx, sizeof(rx)), IS62WVS_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  /* Test Quad Write and Read */
  for (int i = 0; i < 64; i++) tx[i] = (uint8_t)(0xAA ^ i);
  ASSERT_EQ(IS62WVS_WriteQuad(&hxspi, 0x00000500, tx, sizeof(tx)), IS62WVS_OK);
  ASSERT_EQ(IS62WVS_ReadQuad(&hxspi, 0x00000500, rx, sizeof(rx), 2), IS62WVS_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  /* Test Memory Mapped Mode activation */
  ASSERT_EQ(IS62WVS_EnableMemoryMappedMode(&hxspi, 2), IS62WVS_OK);

  /* Test through unified ExtMem manager with forced type */
  ExtMem_HandleTypeDef hextmem = {0};
  hextmem.Config.Bus              = EXTMEM_BUS_XSPI1;
  hextmem.Config.ClockPrescaler   = 2;
  hextmem.Config.ForcedDeviceType = EXTMEM_TYPE_SRAM_SERIAL_ISSI;

  ASSERT_EQ(ExtMem_Init(&hextmem), EXTMEM_OK);
  ASSERT_EQ(hextmem.Geometry.Type, EXTMEM_TYPE_SRAM_SERIAL_ISSI);
  ASSERT_TRUE(ExtMem_IsRAM(&hextmem));
  ASSERT_TRUE(!ExtMem_IsFlash(&hextmem));

  uint8_t mtx[32], mrx[32];
  for (int i = 0; i < 32; i++) mtx[i] = (uint8_t)(i + 0x30);
  ASSERT_EQ(ExtMem_Write(&hextmem, 0x100, mtx, 32), EXTMEM_OK);
  ASSERT_EQ(ExtMem_Read(&hextmem, 0x100, mrx, 32), EXTMEM_OK);
  ASSERT_EQ(memcmp(mtx, mrx, 32), 0);

  ASSERT_EQ(ExtMem_EnableMemoryMapped(&hextmem), EXTMEM_OK);
  ASSERT_EQ(hextmem.State, EXTMEM_STATE_MEMORY_MAPPED);
  ASSERT_EQ(ExtMem_DisableMemoryMapped(&hextmem), EXTMEM_OK);

  return true;
}

/* 14. Micron Xccela Octal NOR Flash (MT35XU512ABA) */
bool test_micron_mt35xu512a_octal_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x2C, 0x5B, 0x1A); /* MT35XU512ABA */

  XSPI_HandleTypeDef hxspi = {0};
  ASSERT_EQ(MT35XU_Init(&hxspi, 2, HAL_XSPI_SIZE_64MB), MT35XU_OK);

  uint8_t id[3] = {0};
  ASSERT_EQ(MT35XU_ReadID(&hxspi, id), MT35XU_OK);
  ASSERT_EQ(id[0], MT35XU_MANUFACTURER_ID);
  ASSERT_EQ(id[1], MT35XU_MEMORY_TYPE_1V8);
  ASSERT_EQ(id[2], 0x1A);

  uint8_t status = 0, flagStatus = 0;
  ASSERT_EQ(MT35XU_ReadStatus(&hxspi, &status), MT35XU_OK);
  ASSERT_EQ(MT35XU_ReadFlagStatus(&hxspi, &flagStatus), MT35XU_OK);
  ASSERT_TRUE((flagStatus & MT35XU_FSR_READY) != 0);

  /* Enter Octal DTR */
  ASSERT_EQ(MT35XU_EnterOctalDTRMode(&hxspi, 16), MT35XU_OK);

  /* Page Program in Octal DTR mode */
  uint8_t tx[128];
  uint8_t rx[128];
  for (int i = 0; i < 128; i++) tx[i] = (uint8_t)(0x3C ^ i);

  ASSERT_EQ(MT35XU_PageProgram(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00004000, tx, sizeof(tx)), MT35XU_OK);
  ASSERT_EQ(MT35XU_Read(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00004000, rx, sizeof(rx), 16), MT35XU_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  /* Erase 4KB Sector */
  ASSERT_EQ(MT35XU_EraseSector4K(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00004000), MT35XU_OK);
  ASSERT_EQ(MT35XU_Read(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00004000, rx, 16, 16), MT35XU_OK);
  for (int i = 0; i < 16; i++) ASSERT_EQ(rx[i], 0xFF);

  /* Erase 128KB Block */
  ASSERT_EQ(MT35XU_EraseBlock128K(&hxspi, EXTMEM_MODE_OCTAL_DTR, 0x00020000), MT35XU_OK);

  /* Memory Mapped Mode */
  ASSERT_EQ(MT35XU_EnableMemoryMappedModeDTR(&hxspi, 16), MT35XU_OK);

  return true;
}

/* 15. Micron Quad SPI NOR Flash (MT25QU512ABB) */
bool test_micron_mt25qu512a_quad_flash(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x2C, 0xBB, 0x20); /* MT25QU512ABB */

  XSPI_HandleTypeDef hxspi = {0};
  ASSERT_EQ(MT25QU_Init(&hxspi, 2, HAL_XSPI_SIZE_64MB), MT25Q_OK);

  uint8_t id[3] = {0};
  ASSERT_EQ(MT25QU_ReadID(&hxspi, id), MT25Q_OK);
  ASSERT_EQ(id[0], MT25Q_MANUFACTURER_ID);
  ASSERT_EQ(id[1], MT25Q_MEMORY_TYPE_1V8);
  ASSERT_EQ(id[2], 0x20);

  uint8_t status = 0, flagStatus = 0;
  ASSERT_EQ(MT25QU_ReadStatus(&hxspi, &status), MT25Q_OK);
  ASSERT_EQ(MT25QU_ReadFlagStatus(&hxspi, &flagStatus), MT25Q_OK);
  ASSERT_TRUE((flagStatus & MT25Q_FSR_READY) != 0);

  /* Quad Page Program & Read */
  uint8_t tx[128];
  uint8_t rx[128];
  for (int i = 0; i < 128; i++) tx[i] = (uint8_t)(0x7E ^ i);

  ASSERT_EQ(MT25QU_PageProgramQuad(&hxspi, 0x00002000, tx, sizeof(tx)), MT25Q_OK);
  ASSERT_EQ(MT25QU_ReadQuad(&hxspi, 0x00002000, rx, sizeof(rx), 10), MT25Q_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  /* Erase 4KB Subsector */
  ASSERT_EQ(MT25QU_EraseSector4K(&hxspi, 0x00002000), MT25Q_OK);
  ASSERT_EQ(MT25QU_ReadQuad(&hxspi, 0x00002000, rx, 16, 10), MT25Q_OK);
  for (int i = 0; i < 16; i++) ASSERT_EQ(rx[i], 0xFF);

  /* Erase 64KB Sector */
  ASSERT_EQ(MT25QU_EraseBlock64K(&hxspi, 0x00010000), MT25Q_OK);

  /* Memory Mapped Mode */
  ASSERT_EQ(MT25QU_EnableMemoryMappedMode(&hxspi, 10), MT25Q_OK);

  return true;
}

/* 16. High-Level ExtMem Manager Unified Auto-Detection & Operations */
bool test_extmem_manager_unified_autodetect(void)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(0x34, 0x5B, 0x1A); /* Emulate S28HS512T */

  ExtMem_HandleTypeDef hextmem = {0};
  hextmem.Config.Bus            = EXTMEM_BUS_XSPI1;
  hextmem.Config.ClockPrescaler = 2;
  hextmem.Config.Force1V8       = true;

  ASSERT_EQ(ExtMem_Init(&hextmem), EXTMEM_OK);
  ASSERT_EQ(hextmem.Geometry.Type, EXTMEM_TYPE_NOR_OCTAL_SEMPER);
  ASSERT_EQ(hextmem.Geometry.TotalSizeBytes, (64 * 1024 * 1024));
  ASSERT_TRUE(ExtMem_IsFlash(&hextmem));
  ASSERT_TRUE(!ExtMem_IsRAM(&hextmem));
  ASSERT_EQ(strcmp(ExtMem_GetDeviceName(&hextmem), "S28HS512T"), 0);

  /* Unified Write and Read with page boundary handling */
  uint8_t tx[512];
  uint8_t rx[512];
  for (int i = 0; i < 512; i++) tx[i] = (uint8_t)(i & 0xFF);

  ASSERT_EQ(ExtMem_Write(&hextmem, 0x00001000, tx, sizeof(tx)), EXTMEM_OK);
  ASSERT_EQ(ExtMem_Read(&hextmem, 0x00001000, rx, sizeof(rx)), EXTMEM_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  /* Unified Erase */
  ASSERT_EQ(ExtMem_EraseSector(&hextmem, 0x00001000), EXTMEM_OK);
  ASSERT_EQ(ExtMem_Read(&hextmem, 0x00001000, rx, 16), EXTMEM_OK);
  for (int i = 0; i < 16; i++) ASSERT_EQ(rx[i], 0xFF);

  /* Memory Mapped Mode */
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&hextmem), EXTMEM_OK);
  ASSERT_EQ(hextmem.State, EXTMEM_STATE_MEMORY_MAPPED);
  ASSERT_EQ(ExtMem_DisableMemoryMapped(&hextmem), EXTMEM_OK);
  ASSERT_EQ(hextmem.State, EXTMEM_STATE_INDIRECT);

  ASSERT_EQ(ExtMem_DeInit(&hextmem), EXTMEM_OK);
  return true;
}
