/**
  ******************************************************************************
  * @file    test_sfdp_db.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Tests for the JESD216 SFDP parser and the device database.
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

/* Builds an SFDP image with a BFPT of 'dwords' DWORDs at 0x30 */
static uint8_t s_Sfdp[0x30 + 64];

static void BuildSfdp(const uint32_t *bfpt, uint8_t dwords)
{
  memset(s_Sfdp, 0, sizeof(s_Sfdp));
  memcpy(s_Sfdp, "SFDP", 4);
  s_Sfdp[4] = 0x06; s_Sfdp[5] = 0x01; s_Sfdp[6] = 0x00; s_Sfdp[7] = 0xFF;
  s_Sfdp[8] = 0x00; s_Sfdp[9] = 0x06; s_Sfdp[10] = 0x01; s_Sfdp[11] = dwords;
  s_Sfdp[12] = 0x30; s_Sfdp[13] = 0x00; s_Sfdp[14] = 0x00; s_Sfdp[15] = 0xFF;
  memcpy(&s_Sfdp[0x30], bfpt, (size_t)((dwords > 16U) ? 16U : dwords) * 4U);
  MockHAL_SetSfdpTable(s_Sfdp, sizeof(s_Sfdp));
}

bool test_sfdp_parser(void)
{
  XSPI_HandleTypeDef h = {0};
  SFDP_FlashParams_t p;

  /* Default image: 512 Mbit, 4 KB erase 0x20, 1-4-4 0xEB (4 wait + 2 mode), 1-1-4 0x6B (8 wait),
   * erase types 4 KB / 64 KB / 32 KB, page 256 */
  MockHAL_Reset();
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_OK);
  ASSERT_CMD(Test_NthCommand(0), I1S(0x5A), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x000000), D1S(8), DUMMY(8), NODQS);
  ASSERT_CMD(Test_NthCommand(1), I1S(0x5A), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x000008), D1S(8), DUMMY(8));
  ASSERT_CMD(Test_NthCommand(2), I1S(0x5A), A1S(HAL_XSPI_ADDRESS_24_BITS, 0x000030), D1S(64), DUMMY(8));
  ASSERT_EQ(p.DensityBytes, 64U * 1024U * 1024U);
  ASSERT_TRUE(p.Supports4KBErase);
  ASSERT_EQ(p.Opcode4KBErase, 0x20);
  ASSERT_EQ(p.AddressBytes, 4);
  ASSERT_TRUE(p.SupportsQuad_1_4_4);
  ASSERT_EQ(p.OpcodeQuad_1_4_4, 0xEB);
  ASSERT_EQ(p.DummyQuad_1_4_4, 6);
  ASSERT_TRUE(p.SupportsQuad_1_1_4);
  ASSERT_EQ(p.OpcodeQuad_1_1_4, 0x6B);
  ASSERT_EQ(p.DummyQuad_1_1_4, 8);
  ASSERT_TRUE(p.SupportsSectorErase);
  ASSERT_EQ(p.OpcodeSectorErase, 0xD8);
  ASSERT_EQ(p.SectorSizeBytes, 64U * 1024U);
  ASSERT_EQ(p.PageSizeBytes, 256);
  ASSERT_TRUE(!p.SupportsOctal_8D_8D_8D);

  /* Linear density form, 3-byte only, no quad, no erase types, 9 DWORD table (no page size) */
  uint32_t t1[9] = {0};
  t1[0] = 0x00U;                          /* no 4 KB erase, 3-byte only */
  t1[1] = (32U * 1024U * 1024U) - 1U;     /* 32 Mbit = 4 MByte */
  BuildSfdp(t1, 9);
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_OK);
  ASSERT_EQ(p.DensityBytes, 4U * 1024U * 1024U);
  ASSERT_EQ(p.AddressBytes, 3);
  ASSERT_TRUE(!p.Supports4KBErase);
  ASSERT_TRUE(!p.SupportsQuad_1_4_4);
  ASSERT_TRUE(!p.SupportsQuad_1_1_4);
  ASSERT_TRUE(!p.SupportsSectorErase);
  ASSERT_EQ(p.SectorSizeBytes, 65536);
  ASSERT_EQ(p.PageSizeBytes, 256);

  /* Exponent density: 2 Gbit fits, 32 Gbit (2^35) does not fit in 32-bit bytes */
  uint32_t t2[16] = {0};
  t2[0] = (0x2U << 17);                   /* 4-byte only */
  t2[1] = 0x80000000U | 31U;
  t2[7] = 12U | (0x20U << 8) | (18U << 16) | (0xDCU << 24); /* 4 KB, 256 KB */
  t2[8] = 20U | (0x99U << 8);              /* 1 MB erase type is ignored (not a block) */
  t2[10] = (9U << 4);                      /* 512 byte page */
  BuildSfdp(t2, 16);
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_OK);
  ASSERT_EQ(p.DensityBytes, 256U * 1024U * 1024U);
  ASSERT_EQ(p.AddressBytes, 4);
  ASSERT_EQ(p.SectorSizeBytes, 256U * 1024U);
  ASSERT_EQ(p.OpcodeSectorErase, 0xDC);
  ASSERT_EQ(p.PageSizeBytes, 512);
  t2[1] = 0x80000000U | 35U;
  BuildSfdp(t2, 20);                       /* longer tables are truncated to 16 DWORDs */
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_OK);
  ASSERT_EQ(p.DensityBytes, 0);
  t2[1] = 0x80000000U | 2U;
  BuildSfdp(t2, 16);
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_OK);
  ASSERT_EQ(p.DensityBytes, 0);

  /* Invalid images */
  BuildSfdp(t1, 8);                        /* BFPT shorter than JESD216 minimum */
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_NOT_SUPPORTED);
  BuildSfdp(t1, 9);
  s_Sfdp[0] = 'X';                         /* bad signature */
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_NOT_SUPPORTED);
  BuildSfdp(t1, 9);
  s_Sfdp[8] = 0x01;                        /* not the JEDEC BFPT */
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_NOT_SUPPORTED);
  BuildSfdp(t1, 9);
  s_Sfdp[15] = 0x00;
  ASSERT_EQ(SFDP_ReadAndParse(&h, &p), EXTMEM_NOT_SUPPORTED);

  ASSERT_EQ(SFDP_ReadAndParse(NULL, &p), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(SFDP_ReadAndParse(&h, NULL), EXTMEM_INVALID_PARAM);

  FAULT_SWEEP(MockHAL_Reset(), SFDP_ReadAndParse(&h, &p));
  return true;
}

/* ========================================================================= */
/* Device database                                                           */
/* ========================================================================= */

static bool IsPow2(uint32_t v) { return v != 0U && (v & (v - 1U)) == 0U; }

bool test_device_database_consistency(void)
{
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    const ExtMem_DeviceDescriptor_t *d = &ExtMem_DeviceDatabase[i];
    ASSERT_NOT_NULL(d->PartNumber);
    ASSERT_TRUE(strlen(d->PartNumber) < sizeof(((ExtMem_Geometry_t *)0)->DeviceName));
    ASSERT_TRUE(d->Type != EXTMEM_TYPE_UNKNOWN);
    ASSERT_TRUE(IsPow2(d->CapacityBytes));
    ASSERT_TRUE(d->MaxClockFreqMHz > 0U);

    bool isVolatile = ExtMem_TypeIsVolatile(d->Type);
    if (!isVolatile)
    {
      ASSERT_TRUE(IsPow2(d->SectorSizeBytes));
      ASSERT_TRUE(IsPow2(d->BlockSizeBytes));
      ASSERT_TRUE(d->SectorSizeBytes <= d->BlockSizeBytes);
      ASSERT_TRUE(d->BlockSizeBytes <= d->CapacityBytes);
      if (d->Type != EXTMEM_TYPE_NOR_PARALLEL_FMC)
      {
        ASSERT_TRUE(d->PageSizeBytes == 256U || d->PageSizeBytes == 512U);
      }
    }
    else if (d->Type != EXTMEM_TYPE_SRAM_SERIAL_ISSI)
    {
      ASSERT_EQ(d->PageSizeBytes, 0);
    }

    /* Every JEDEC-identified part must be found from its own ID with identical protocol data */
    if (ExtMem_TypeAnswersJedecId(d->Type))
    {
      const ExtMem_DeviceDescriptor_t *f = ExtMem_FindDevice(d->ManufacturerID, d->MemoryTypeID, d->DensityID);
      ASSERT_NOT_NULL(f);
      ASSERT_EQ(f->Type, d->Type);
      ASSERT_EQ(f->CapacityBytes, d->CapacityBytes);
    }

    ASSERT_TRUE(ExtMem_FindDeviceByPartNumber(d->PartNumber) == d);
    ASSERT_NOT_NULL(ExtMem_FindDeviceByTypeAndCapacity(d->Type, d->CapacityBytes));
  }

  /* Identification codes checked against vendor documentation / JEDEC data */
  struct { const char *pn; uint8_t m, t, d; } ids[] = {
    { "S28HS512T",    0x34, 0x5B, 0x1A }, { "S28HL512T",    0x34, 0x5A, 0x1A },
    { "S25HL512T",    0x34, 0x2A, 0x1A }, { "S25HS512T",    0x34, 0x2B, 0x1A },
    { "IS25LX256",    0x9D, 0x5A, 0x19 }, { "IS25WX256",    0x9D, 0x5B, 0x19 },
    { "IS25LP256D",   0x9D, 0x60, 0x19 }, { "IS25WP256D",   0x9D, 0x70, 0x19 },
    { "MT35XU512ABA", 0x2C, 0x5B, 0x1A }, { "MT35XL512ABA", 0x2C, 0x5A, 0x1A },
    { "MT25QU512ABB", 0x20, 0xBB, 0x20 }, { "MT25QL512ABB", 0x20, 0xBA, 0x20 },
    { "MT25QU01GBBB", 0x20, 0xBB, 0x21 }, { "MT25QL256ABA", 0x20, 0xBA, 0x19 },
  };
  for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
  {
    const ExtMem_DeviceDescriptor_t *d = ExtMem_FindDevice(ids[i].m, ids[i].t, ids[i].d);
    ASSERT_NOT_NULL(d);
    ASSERT_EQ(strcmp(d->PartNumber, ids[i].pn), 0);
  }

  /* Geometry facts that drive erase commands */
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("S28HS512T")->SectorSizeBytes, 256U * 1024U);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("S28HS512T")->DefaultReadDummyCycles, 24);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("S25HL512T")->BlockSizeBytes, 256U * 1024U);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("S25FL256L")->SectorSizeBytes, 4096);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("IS25LX256")->BlockSizeBytes, 128U * 1024U);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("MT35XU512ABA")->BlockSizeBytes, 128U * 1024U);
  ASSERT_EQ(ExtMem_FindDeviceByPartNumber("IS29GL064")->SectorSizeBytes, 64U * 1024U);

  /* Unknown Infineon IDs must not fall back to HyperFlash (it never answers 0x9F) */
  ASSERT_TRUE(ExtMem_FindDevice(0x34, 0x99, 0x99) == NULL);
  ASSERT_TRUE(ExtMem_FindDevice(0x00, 0x00, 0x00) == NULL);
  ASSERT_TRUE(ExtMem_FindDeviceByPartNumber(NULL) == NULL);
  ASSERT_TRUE(ExtMem_FindDeviceByPartNumber("NOPE1234") == NULL);
  ASSERT_TRUE(ExtMem_FindDeviceByPartNumber("MT25QU512") != NULL); /* prefix match */
  ASSERT_TRUE(ExtMem_FindDeviceByTypeAndCapacity(EXTMEM_TYPE_NOR_OCTAL_SEMPER, 3U) == NULL);
  ASSERT_NOT_NULL(ExtMem_FindDeviceByTypeAndCapacity(EXTMEM_TYPE_NOR_OCTAL_SEMPER, 0U));

  ASSERT_TRUE(ExtMem_TypeIsVolatile(EXTMEM_TYPE_HYPERRAM_ISSI));
  ASSERT_TRUE(!ExtMem_TypeIsVolatile(EXTMEM_TYPE_HYPERFLASH_ISSI));
  ASSERT_TRUE(!ExtMem_TypeAnswersJedecId(EXTMEM_TYPE_HYPERFLASH_INFINEON));
  return true;
}
