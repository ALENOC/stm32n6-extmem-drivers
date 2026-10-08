/**
  ******************************************************************************
  * @file    test_manager.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Tests for the unified STM32N6 external memory manager. Every device
  *          of the database is brought up through ExtMem_Init and exercised
  *          end to end (program/erase/read, memory-mapped mode, DMA, reset,
  *          power down, deinit), including HAL fault propagation.
  ******************************************************************************
  */

#include "test_common.h"
#include "extmem_unit_tests.h"

static ExtMem_HandleTypeDef s_h;

static bool IsFmcType(ExtMem_Type_t t)
{
  return (t == EXTMEM_TYPE_PSRAM_PARALLEL_FMC) || (t == EXTMEM_TYPE_NOR_PARALLEL_FMC);
}

static bool IsOctalNor(ExtMem_Type_t t)
{
  return (t == EXTMEM_TYPE_NOR_OCTAL_SEMPER) || (t == EXTMEM_TYPE_NOR_OCTAL_ISSI) || (t == EXTMEM_TYPE_NOR_OCTAL_MICRON);
}

static bool IsHyperFlash(ExtMem_Type_t t)
{
  return (t == EXTMEM_TYPE_HYPERFLASH_INFINEON) || (t == EXTMEM_TYPE_HYPERFLASH_ISSI);
}

static bool IsHyperRam(ExtMem_Type_t t)
{
  return (t == EXTMEM_TYPE_HYPERRAM_INFINEON) || (t == EXTMEM_TYPE_HYPERRAM_ISSI);
}

/* Self-refreshing XSPI RAMs: deep power down, no software reset, CS# refresh counter */
static bool IsSelfRefreshRam(ExtMem_Type_t t)
{
  return IsHyperRam(t) || (t == EXTMEM_TYPE_PSRAM_OCTAL_ISSI);
}

static ExtMem_Mode_t ExpectedMode(ExtMem_Type_t t)
{
  if (IsOctalNor(t) || t == EXTMEM_TYPE_PSRAM_OCTAL_ISSI) return EXTMEM_MODE_OCTAL_DTR;
  if (IsHyperFlash(t) || IsHyperRam(t)) return EXTMEM_MODE_HYPERBUS;
  if (IsFmcType(t)) return EXTMEM_MODE_PARALLEL_16BIT;
  if (t == EXTMEM_TYPE_SRAM_SERIAL_ISSI) return EXTMEM_MODE_QUAD_4_4_4;
  return EXTMEM_MODE_QUAD_1_4_4;
}

static uint32_t Log2(uint32_t v)
{
  uint32_t p = 0;
  while ((1ULL << p) < v) p++;
  return p;
}

/* Prepares the emulator and the handle for one database device */
static void SetupDevice(const ExtMem_DeviceDescriptor_t *d)
{
  MockHAL_Reset();
  MockHAL_SetEmulatedChip(d->ManufacturerID, d->MemoryTypeID, d->DensityID);
  MockHAL_SetBlockEraseSize(d->BlockSizeBytes ? d->BlockSizeBytes : 65536U);
  if (IsHyperFlash(d->Type))
  {
    MockHAL_SetHyperFlashMode(true);
  }
  else if (!ExtMem_TypeIsVolatile(d->Type) && !IsFmcType(d->Type))
  {
    MockHAL_SetFlashSemantics(true);
  }
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus              = IsFmcType(d->Type) ? EXTMEM_BUS_FMC_SRAM_BANK1_1 : EXTMEM_BUS_XSPI1;
  s_h.Config.ForcedPartNumber = d->PartNumber;
  s_h.Config.ForcedDeviceType = (d->Type == EXTMEM_TYPE_NOR_PARALLEL_FMC) ? EXTMEM_TYPE_NOR_PARALLEL_FMC : EXTMEM_TYPE_UNKNOWN;
  /* Fastest divider of the 400 MHz kernel clock the part supports */
  s_h.Config.ClockPrescaler   = (400U + d->MaxClockFreqMHz - 1U) / d->MaxClockFreqMHz;
}

#define SETUP_INIT(d) do { SetupDevice(d); (void)ExtMem_Init(&s_h); MockHAL_ClearLog(); } while (0)

static bool ExerciseDevice(const ExtMem_DeviceDescriptor_t *d)
{
  uint8_t tx[300], rx[300];
  uint8_t *mem = MockHAL_GetMemoryBuffer();
  bool flash = !ExtMem_TypeIsVolatile(d->Type);
  Test_Pattern(tx, sizeof(tx), (uint8_t)d->DensityID);

  SetupDevice(d);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_TRUE(s_h.pDevice == d);
  ASSERT_EQ(strcmp(ExtMem_GetDeviceName(&s_h), d->PartNumber), 0);
  ASSERT_EQ(ExtMem_GetCapacity(&s_h), d->CapacityBytes);
  ASSERT_EQ(s_h.Geometry.Type, d->Type);
  ASSERT_EQ(ExtMem_IsFlash(&s_h), flash);
  ASSERT_EQ(ExtMem_IsRAM(&s_h), !flash);
  ASSERT_EQ(s_h.ActiveMode, ExpectedMode(d->Type));
  ASSERT_EQ(s_h.State, IsFmcType(d->Type) ? EXTMEM_STATE_MEMORY_MAPPED : EXTMEM_STATE_INDIRECT);

  if (!IsFmcType(d->Type))
  {
    /* DEVSIZE: 2^(DEVSIZE + 1) bytes = capacity */
    const MockEvent_t *last = NULL;
    for (uint32_t i = 0; MockHAL_FindEvent(MOCK_EV_XSPI_INIT, i) != NULL; i++) last = MockHAL_FindEvent(MOCK_EV_XSPI_INIT, i);
    ASSERT_NOT_NULL(last);
    ASSERT_EQ(last->Init.MemorySize, Log2(d->CapacityBytes) - 1U);
    /* XSPIM routing configured after the first HAL_XSPI_Init */
    const MockEvent_t *xspim = MockHAL_FindEvent(MOCK_EV_XSPIM_CONFIG, 0);
    ASSERT_NOT_NULL(xspim);
    ASSERT_EQ(xspim->Xspim.IOPort, HAL_XSPIM_IOPORT_1);
    ASSERT_EQ(xspim->Xspim.Req2AckTime, 1);
  }
  if (d->Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)
  {
    ASSERT_EQ(MockHAL_GetAnyReg(0x00800006), 0x43);
    ASSERT_EQ(s_h.DummyCycles, 24);
    /* Stacked dice: second die configured too, bursts bounded by the 1 Gbit die */
    ASSERT_EQ(MockHAL_GetAnyReg(0x08800006), (d->DieCount > 1U) ? 0x43 : 0x40);
    ASSERT_EQ(s_h.hxspi.Init.ChipSelectBoundary, (d->DieCount > 1U) ? HAL_XSPI_BONDARYOF_1GB : HAL_XSPI_BONDARYOF_NONE);
  }
  if (d->Type == EXTMEM_TYPE_NOR_OCTAL_ISSI || d->Type == EXTMEM_TYPE_NOR_OCTAL_MICRON)
  {
    ASSERT_EQ(MockHAL_GetVCR(0), 0xE7);
    ASSERT_EQ(MockHAL_GetVCR(1), d->DefaultReadDummyCycles);
  }
  if (d->Type == EXTMEM_TYPE_NOR_QUAD_MICRON || d->Type == EXTMEM_TYPE_NOR_QUAD_ISSI)
  {
    ASSERT_EQ(MockHAL_Is4ByteMode(), d->CapacityBytes > 16U * 1024U * 1024U);
  }
  if (d->Type == EXTMEM_TYPE_NOR_QUAD_ISSI)
  {
    ASSERT_EQ(MockHAL_GetIssiReadParams(), IS25LP_FAST_QUAD_IO_DUMMY << 3);
    ASSERT_EQ(s_h.DummyCycles, IS25LP_FAST_QUAD_IO_DUMMY);
  }
  if (d->Type == EXTMEM_TYPE_NOR_QUAD_INFINEON)
  {
    ASSERT_EQ(s_h.DummyCycles, S25HL_DEFAULT_READ_LATENCY);
  }
  if (!IsFmcType(d->Type))
  {
    /* CS# refresh counter only on self-refreshing RAMs: 1 us at the configured bus clock, minus 4 clocks */
    bool needsRefresh = IsSelfRefreshRam(d->Type) || d->Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI;
    uint32_t expect = needsRefresh ? (s_h.BusClockHz / 1000000U) - 4U : 0U;
    ASSERT_EQ(s_h.hxspi.Init.Refresh, expect);
  }
  if (d->Type == EXTMEM_TYPE_SRAM_SERIAL_ISSI)
  {
    ASSERT_EQ(MockHAL_GetSramModeRegister(), IS62WVS_MODE_SEQUENTIAL);
  }

  /* Program / write across a page boundary and read back */
  uint32_t addr = 0x1F0;
  if (flash)
  {
    ASSERT_EQ(ExtMem_EraseSector(&s_h, 0), EXTMEM_OK);
  }
  MockHAL_ClearLog();
  ASSERT_EQ(ExtMem_Write(&s_h, addr, tx, sizeof(tx)), EXTMEM_OK);
  memset(rx, 0, sizeof(rx));
  ASSERT_EQ(ExtMem_Read(&s_h, addr, rx, sizeof(rx)), EXTMEM_OK);
  ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);

  if (flash)
  {
    if (!IsFmcType(d->Type) && !IsHyperFlash(d->Type))
    {
      /* No program command may cross a page */
      uint32_t page = d->PageSizeBytes;
      for (uint32_t i = 0; i < MockHAL_GetEventCount(); i++)
      {
        const MockEvent_t *ev = MockHAL_GetEvent(i);
        if (ev->Type == MOCK_EV_TX && !ev->Hyperbus)
        {
          ASSERT_TRUE((ev->Cmd.Address / page) == ((ev->Cmd.Address + ev->Cmd.DataLength - 1U) / page));
        }
      }
    }
    /* Erase brings the area back to 0xFF */
    ASSERT_EQ(ExtMem_EraseSector(&s_h, addr), EXTMEM_OK);
    ASSERT_EQ(mem[addr], 0xFF);
    ASSERT_EQ(ExtMem_Write(&s_h, addr, tx, 16), EXTMEM_OK);
    ASSERT_EQ(ExtMem_EraseBlock(&s_h, 0), EXTMEM_OK);
    ASSERT_EQ(mem[addr], 0xFF);
    memset(mem, 0x00, 64);
    MockHAL_ClearLog();
    ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_OK);
    ASSERT_EQ(mem[0], 0xFF);
    if (d->Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)
    {
      ASSERT_EQ(MockHAL_CountCommands(0x619E), (d->DieCount > 1U) ? d->DieCount : 0U);
    }
    if (d->Type == EXTMEM_TYPE_NOR_QUAD_MICRON || d->Type == EXTMEM_TYPE_NOR_OCTAL_MICRON)
    {
      uint32_t dice = (d->DieCount > 1U) ? d->DieCount : 0U;
      uint32_t dieOps = MockHAL_CountCommands(0xC4) + MockHAL_CountCommands(0xC4C4);
      ASSERT_EQ(dieOps, dice);
    }
  }
  else
  {
    ASSERT_EQ(ExtMem_EraseSector(&s_h, 0), EXTMEM_OK);
    ASSERT_EQ(ExtMem_EraseBlock(&s_h, 0), EXTMEM_OK);
    ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_OK);
  }

  /* DMA paths */
  ASSERT_EQ(ExtMem_WriteDMA(&s_h, 0x800, tx, 64), EXTMEM_OK);
  ASSERT_EQ(ExtMem_ReadDMA(&s_h, 0x800, rx, 64), EXTMEM_OK);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);

  /* Memory-mapped mode */
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.State, EXTMEM_STATE_MEMORY_MAPPED);
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
  memset(rx, 0, sizeof(rx));
  ASSERT_EQ(ExtMem_Read(&s_h, 0x800, rx, 64), EXTMEM_OK);
  ASSERT_EQ(memcmp(tx, rx, 64), 0);
  if (!flash)
  {
    MockHAL_ClearLog();
    ASSERT_EQ(ExtMem_Write(&s_h, 0x900, tx, 32), EXTMEM_OK);
    ASSERT_EQ(memcmp(&mem[0x900], tx, 32), 0);
    ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_CACHE_MAINT, 0));
    ASSERT_EQ(s_h.State, EXTMEM_STATE_MEMORY_MAPPED);
  }
  else
  {
    /* Programming leaves memory-mapped mode first */
    if (!IsFmcType(d->Type))
    {
      ASSERT_EQ(ExtMem_EraseSector(&s_h, 0x900), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
      MockHAL_ClearLog();
      ASSERT_EQ(ExtMem_Write(&s_h, 0x900, tx, 32), EXTMEM_OK);
      ASSERT_NOT_NULL(MockHAL_FindEvent(MOCK_EV_ABORT, 0));
      ASSERT_EQ(s_h.State, EXTMEM_STATE_INDIRECT);
      ASSERT_EQ(memcmp(&mem[0x900], tx, 32), 0);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EraseSector(&s_h, 0x900), EXTMEM_OK);
      ASSERT_EQ(s_h.State, EXTMEM_STATE_INDIRECT);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EraseBlock(&s_h, 0), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
    }
    else
    {
      ASSERT_EQ(ExtMem_EraseSector(&s_h, 0x900), EXTMEM_OK);
      ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
      ASSERT_EQ(ExtMem_Write(&s_h, 0x900, tx, 32), EXTMEM_OK);
      ASSERT_EQ(memcmp(&mem[0x900], tx, 32), 0);
    }
  }
  MockHAL_ClearLog();
  ASSERT_EQ(ExtMem_DisableMemoryMapped(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.State, EXTMEM_STATE_INDIRECT);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_ABORT, 0) != NULL, !IsFmcType(d->Type));
  ASSERT_EQ(ExtMem_DisableMemoryMapped(&s_h), EXTMEM_OK);

  /* Power down is a HyperRAM / OctalRAM feature */
  if (IsSelfRefreshRam(d->Type))
  {
    ASSERT_EQ(ExtMem_EnterDeepPowerDown(&s_h), EXTMEM_OK);
    if (IsHyperRam(d->Type)) ASSERT_EQ(MockHAL_GetHyperReg(0x1000) & 0x8000, 0);
    else ASSERT_EQ(MockHAL_GetOctalRamCR() & 0x8000, 0);
    ASSERT_EQ(ExtMem_LeaveDeepPowerDown(&s_h), EXTMEM_OK);
  }
  else
  {
    ASSERT_EQ(ExtMem_EnterDeepPowerDown(&s_h), EXTMEM_NOT_SUPPORTED);
    ASSERT_EQ(ExtMem_LeaveDeepPowerDown(&s_h), EXTMEM_NOT_SUPPORTED);
  }

  /* Software reset (from memory-mapped mode to exercise the abort) */
  if (!IsFmcType(d->Type)) ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
  MockHAL_ClearLog();
  int32_t rst = ExtMem_Reset(&s_h);
  ASSERT_EQ(rst, IsSelfRefreshRam(d->Type) ? EXTMEM_NOT_SUPPORTED : EXTMEM_OK);
  if (IsOctalNor(d->Type))
  {
    ASSERT_EQ(s_h.ActiveMode, EXTMEM_MODE_SPI);
    ASSERT_TRUE(MockHAL_CountCommands(0x66) == 1 && MockHAL_CountCommands(0x99) == 1);
    /* Reset again from SPI: no octal exit sequence */
    MockHAL_ClearLog();
    ASSERT_EQ(ExtMem_Reset(&s_h), EXTMEM_OK);
    ASSERT_SEQUENCE(0, 0x66, 0x99);
    /* Memory-mapped octal read is only set up for 8D-8D-8D */
    ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_NOT_SUPPORTED);
  }
  if (d->Type == EXTMEM_TYPE_NOR_PARALLEL_FMC)
  {
    ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_FMC_WRITE16, 0)->Value, 0x00F0);
  }

  MockHAL_ClearLog();
  ASSERT_EQ(ExtMem_DeInit(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.State, EXTMEM_STATE_UNINITIALIZED);
  ASSERT_NOT_NULL(MockHAL_FindEvent(IsFmcType(d->Type) ? MOCK_EV_SRAM_DEINIT : MOCK_EV_XSPI_DEINIT, 0));

  /* Every HAL failure along these paths is reported to the caller */
  FAULT_SWEEP(SetupDevice(d), ExtMem_Init(&s_h));
  FAULT_SWEEP(SETUP_INIT(d), ExtMem_Write(&s_h, addr, tx, sizeof(tx)));
  FAULT_SWEEP(SETUP_INIT(d), ExtMem_Read(&s_h, addr, rx, 16));
  FAULT_SWEEP(SETUP_INIT(d), ExtMem_ReadDMA(&s_h, addr, rx, 16));
  FAULT_SWEEP(SETUP_INIT(d), ExtMem_WriteDMA(&s_h, addr, tx, 16));
  if (flash)
  {
    FAULT_SWEEP(SETUP_INIT(d), ExtMem_EraseSector(&s_h, 0));
    FAULT_SWEEP(SETUP_INIT(d), ExtMem_EraseBlock(&s_h, 0));
    FAULT_SWEEP(SETUP_INIT(d), ExtMem_EraseChip(&s_h));
  }
  FAULT_SWEEP(SETUP_INIT(d), ExtMem_EnableMemoryMapped(&s_h));
  if (flash && !IsFmcType(d->Type))
  {
    /* Leaving memory-mapped mode before programming must report a failed abort */
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_Write(&s_h, addr, tx, 8));
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_EraseSector(&s_h, 0));
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_EraseBlock(&s_h, 0));
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_EraseChip(&s_h));
  }
  FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_DisableMemoryMapped(&s_h));
  FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_DeInit(&s_h));
  if (!IsSelfRefreshRam(d->Type))
  {
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_Reset(&s_h));
  }
  else
  {
    FAULT_SWEEP(SETUP_INIT(d), ExtMem_EnterDeepPowerDown(&s_h));
    FAULT_SWEEP(SETUP_INIT(d), ExtMem_LeaveDeepPowerDown(&s_h));
  }
  if (!flash && !IsFmcType(d->Type))
  {
    FAULT_SWEEP(SETUP_INIT(d); (void)ExtMem_EnableMemoryMapped(&s_h), ExtMem_Write(&s_h, addr, tx, 8));
  }
  return true;
}

bool test_extmem_manager_all_devices(void)
{
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (!ExerciseDevice(&ExtMem_DeviceDatabase[i]))
    {
      printf("       [FAIL] device %s\r\n", ExtMem_DeviceDatabase[i].PartNumber);
      return false;
    }
  }
  return true;
}

/* ========================================================================= */
/* Auto-detection                                                            */
/* ========================================================================= */

static void SetupAuto(uint8_t m, uint8_t t, uint8_t d)
{
  MockHAL_Reset();
  MockHAL_SetFlashSemantics(true);
  MockHAL_SetEmulatedChip(m, t, d);
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_XSPI1;
  s_h.Config.ClockPrescaler = 4; /* 100 MHz: within every part found by auto-detection */
}

static const uint8_t s_BadSfdp[16] = { 'N', 'O', 'P', 'E' };

bool test_extmem_manager_unified_autodetect(void)
{
  /* JEDEC ID detection for every serial NOR in the database */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    const ExtMem_DeviceDescriptor_t *d = &ExtMem_DeviceDatabase[i];
    if (!ExtMem_TypeAnswersJedecId(d->Type) || d->Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI) continue;
    SetupAuto(d->ManufacturerID, d->MemoryTypeID, d->DensityID);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_TRUE(s_h.pDevice == ExtMem_FindDevice(d->ManufacturerID, d->MemoryTypeID, d->DensityID));
    ASSERT_EQ(s_h.RawID[0], d->ManufacturerID);
    ASSERT_EQ(s_h.RawID[1], d->MemoryTypeID);
    ASSERT_EQ(s_h.RawID[2], d->DensityID);
    ASSERT_CMD(Test_NthCommand(0), I1S(0x9F), NOADDR, D1S(3));
  }

  /* SFDP fallback for unknown densities of supported quad vendors */
  struct { uint8_t mfg; ExtMem_Type_t type; } sfdpVendors[] = {
    { 0x9D, EXTMEM_TYPE_NOR_QUAD_ISSI }, { 0x20, EXTMEM_TYPE_NOR_QUAD_MICRON },
    { 0x01, EXTMEM_TYPE_NOR_QUAD_INFINEON }, { 0x34, EXTMEM_TYPE_NOR_QUAD_INFINEON },
  };
  for (size_t i = 0; i < sizeof(sfdpVendors) / sizeof(sfdpVendors[0]); i++)
  {
    SetupAuto(sfdpVendors[i].mfg, 0x77, 0x30);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(s_h.Geometry.Type, sfdpVendors[i].type);
    ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "JEDEC_SFDP_Flash"), 0);
    ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 64U * 1024U * 1024U);
    ASSERT_EQ(s_h.Geometry.SectorSizeBytes, 4096);
    ASSERT_EQ(s_h.Geometry.BlockSizeBytes, 65536);
    ASSERT_TRUE(s_h.pDevice == NULL);
    ASSERT_TRUE(ExtMem_IsFlash(&s_h));
    ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_OK);
  }
  /* MT25Q found through SFDP above 512 Mbit: 512 Mbit dice */
  SetupAuto(0x20, 0xBB, 0x30);
  s_h.Config.ForcedCapacityBytes = 128U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  MockHAL_ClearLog();
  ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_OK);
  ASSERT_EQ(MockHAL_CountCommands(0xC4), 2);
  ASSERT_EQ(MockHAL_FindCommand(0xC4, 1)->Cmd.Address, 64U * 1024U * 1024U);
  /* SFDP without 4 KB erase: sector = largest erase type */
  {
    uint32_t bfpt[16] = {0};
    static uint8_t img[0x30 + 64];
    bfpt[0] = (0x1U << 17);
    bfpt[1] = 0x80000000U | 27U;
    bfpt[7] = 16U | (0xD8U << 8);
    bfpt[10] = (8U << 4);
    memset(img, 0, sizeof(img));
    memcpy(img, "SFDP", 4); img[4] = 6; img[5] = 1; img[7] = 0xFF;
    img[9] = 6; img[10] = 1; img[11] = 16; img[12] = 0x30; img[15] = 0xFF;
    memcpy(&img[0x30], bfpt, sizeof(bfpt));
    SetupAuto(0x9D, 0x77, 0x30);
    MockHAL_SetSfdpTable(img, sizeof(img));
    s_h.Config.ForcedCapacityBytes = 8U * 1024U * 1024U;
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(s_h.Geometry.SectorSizeBytes, 65536);
    ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 8U * 1024U * 1024U);
    /* Zero density from SFDP is rejected */
    bfpt[1] = 0x80000000U | 40U;
    memcpy(&img[0x30], bfpt, sizeof(bfpt));
    SetupAuto(0x9D, 0x77, 0x30);
    MockHAL_SetSfdpTable(img, sizeof(img));
    MockHAL_SetHyperBusID(0xFFFF, 0);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
  }

  /* Unknown vendor: SFDP is not trusted, HyperBus probe finds nothing, controller back to SPI */
  SetupAuto(0xEF, 0x40, 0x18);
  MockHAL_SetHyperBusID(0xFFFF, 0xFFFF);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(s_h.Geometry.Type, EXTMEM_TYPE_UNKNOWN);
  ASSERT_EQ(s_h.hxspi.Init.MemoryType, HAL_XSPI_MEMTYPE_MICRON);
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  MockHAL_SetHyperBusID(0x0000, 0x0000);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  MockHAL_SetHyperBusID(0x1FF1, 0x0000);   /* undecodable geometry */
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);

  /* HyperRAM detection from ID0: manufacturer and row/column geometry */
  struct { uint16_t id0; const char *pn; uint32_t cap; ExtMem_Type_t type; } hr[] = {
    { 0x0C81, "S27KS0641",   8U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_INFINEON },
    { 0x0D81, "S27KS128",   16U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_INFINEON },
    { 0x0C83, "IS66WVH8M8",  8U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_ISSI },
    { 0x0D83, "IS66WVH16M8", 16U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_ISSI },
    { 0x0F83, "IS66WVH_HyperRAM", 64U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_ISSI },
    { 0x0F81, "S27KS_HyperRAM",   64U * 1024U * 1024U, EXTMEM_TYPE_HYPERRAM_INFINEON },
  };
  for (size_t i = 0; i < sizeof(hr) / sizeof(hr[0]); i++)
  {
    SetupAuto(0xFF, 0xFF, 0xFF);
    MockHAL_SetFlashSemantics(false);
    MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
    MockHAL_SetHyperBusID(hr[i].id0, 0x0001);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, hr[i].pn), 0);
    ASSERT_EQ(s_h.Geometry.TotalSizeBytes, hr[i].cap);
    ASSERT_EQ(s_h.Geometry.Type, hr[i].type);
    ASSERT_EQ(s_h.ActiveMode, EXTMEM_MODE_HYPERBUS);
    ASSERT_TRUE(ExtMem_IsRAM(&s_h));
  }
  /* Power-on latency 7 (200 MHz parts) is found on the first try, 6 (166 MHz parts) on the second */
  for (int lat7 = 0; lat7 < 2; lat7++)
  {
    SetupAuto(0xFF, 0xFF, 0xFF);
    MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
    MockHAL_SetHyperCR0(lat7 ? 0x8F2F : 0x8F1F);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(s_h.Geometry.Type, EXTMEM_TYPE_HYPERRAM_INFINEON);
    ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_HYPER_CFG, 0)->HCfg.AccessTimeCycle, 7);
    if (!lat7) ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_HYPER_CFG, 1)->HCfg.AccessTimeCycle, 6);
    /* After init the driver runs both sides at 7 clocks */
    ASSERT_EQ(MockHAL_GetHyperReg(0x1000), 0x8F27);
  }
  /* Neither latency gives a valid ID: nothing detected */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  MockHAL_SetHyperCR0(0x8F5F);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
  /* Refresh counter: 1 us at 200 MHz minus 4 clocks of margin */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.hxspi.Init.Refresh, 96);
  /* Unknown kernel clock: fallback bus clock */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  MockHAL_SetXspiKernelClock(0);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.BusClockHz, 0);
  ASSERT_EQ(s_h.hxspi.Init.Refresh, 196);
  /* Very slow bus: minimum refresh period */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  MockHAL_SetXspiKernelClock(4000000U);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.hxspi.Init.Refresh, 4);

  /* Forced capacity overrides the ID0 geometry */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  s_h.Config.ForcedCapacityBytes = 32U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "S27KS256"), 0);

  /* Forced type: by capacity, first of type, unknown capacity falls back to first of type */
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_QUAD_MICRON;
  s_h.Config.ForcedCapacityBytes = 16U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "MT25QU128ABA"), 0);
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_QUAD_MICRON;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "MT25QU01GBBB"), 0);
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_QUAD_MICRON;
  s_h.Config.ForcedCapacityBytes = 3U * 1024U * 1024U;  /* no such part: first entry, forced size kept */
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 3U * 1024U * 1024U);
  ASSERT_EQ(s_h.hxspi.Init.MemorySize, Log2(4U * 1024U * 1024U) - 1U); /* DEVSIZE rounds up */
  /* Unknown part number: falls through to the forced type, then to detection */
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ForcedPartNumber = "XYZ123";
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "S28HS512T"), 0);
  /* Forced type with no database entry is ignored */
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ForcedDeviceType = (ExtMem_Type_t)0x7F;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "S28HS512T"), 0);

  /* Stacked-die SEMPER without the SFDP die map, or with fewer dice than the part, is refused */
  {
    static uint8_t img[0x30 + 64];
    memset(img, 0, sizeof(img));
    memcpy(img, "SFDP", 4); img[4] = 6; img[5] = 1; img[7] = 0xFF;
    img[8] = 0x87; img[11] = 1; img[12] = 0x30; img[15] = 0xFF;   /* SCCR only: one die */
    uint32_t base0 = 0x00800000U;
    memcpy(&img[0x30], &base0, 4);
    SetupAuto(0x34, 0x5B, 0x1C);
    s_h.Config.ClockPrescaler = 2;
    MockHAL_SetSfdpTable(img, sizeof(img));
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
    SetupAuto(0x34, 0x5B, 0x1C);
    MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
    (void)S28HS512T_SetDieLayout(&s_h.hxspi, NULL);
    /* No free die-layout slot: init fails cleanly */
    XSPI_HandleTypeDef others[3] = {0};
    S28HS512T_DieLayout_t l2 = { 2U, 0x08000000U, { 0x00800000U, 0x08800000U, 0U, 0U } };
    for (int i = 0; i < 3; i++) ASSERT_EQ(S28HS512T_SetDieLayout(&others[i], &l2), S28HS512T_OK);
    SetupAuto(0x34, 0x5B, 0x1C);
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_ERROR);
    for (int i = 0; i < 3; i++) ASSERT_EQ(S28HS512T_SetDieLayout(&others[i], NULL), S28HS512T_OK);
  }

  FAULT_SWEEP(SetupAuto(0x9D, 0x77, 0x30), ExtMem_Init(&s_h));
  FAULT_SWEEP_EXPECT(SetupAuto(0xEF, 0x40, 0x18); MockHAL_SetHyperBusID(0xFFFF, 0xFFFF), ExtMem_Init(&s_h), EXTMEM_NOT_SUPPORTED);
  FAULT_SWEEP(SetupAuto(0xFF, 0xFF, 0xFF); MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp)), ExtMem_Init(&s_h));
  return true;
}

/* ========================================================================= */
/* Bus routing, FMC banks and timings                                        */
/* ========================================================================= */

bool test_multi_density_shared_drivers(void)
{
  /* XSPI instances, base addresses, XSPIM ports and VDDIO domains */
  struct { ExtMem_Bus_t bus; uint32_t port; void *inst; uint32_t base; uint32_t vddio; } xs[] = {
    { EXTMEM_BUS_XSPI1, 0, XSPI1, 0x90000000U, PWR_VDDIO2 },
    { EXTMEM_BUS_XSPI2, 0, XSPI2, 0x70000000U, PWR_VDDIO3 },
    { EXTMEM_BUS_XSPI3, 0, XSPI3, 0x80000000U, PWR_VDDIO2 },
    { EXTMEM_BUS_XSPI2, HAL_XSPIM_IOPORT_1, XSPI2, 0x70000000U, PWR_VDDIO2 },
  };
  for (size_t i = 0; i < sizeof(xs) / sizeof(xs[0]); i++)
  {
    for (int v18 = 0; v18 < 2; v18++)
    {
      SetupAuto(0x34, 0x5B, 0x1A);
      s_h.Config.Bus = xs[i].bus;
      s_h.Config.IOPort = xs[i].port;
      s_h.Config.Force1V8 = (v18 != 0);
      ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
      ASSERT_TRUE(s_h.hxspi.Instance == xs[i].inst);
      ASSERT_EQ(s_h.MemoryMappedBase, xs[i].base);
      uint32_t port = xs[i].port ? xs[i].port : ((xs[i].bus == EXTMEM_BUS_XSPI2) ? HAL_XSPIM_IOPORT_2 : HAL_XSPIM_IOPORT_1);
      ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPIM_CONFIG, 0)->Xspim.IOPort, port);
      ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_PWR_VDDIO, 0)->Value, 0x200U | xs[i].vddio);
      ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_PWR_VDDIO, 1)->Value, (xs[i].vddio << 4) | (v18 ? PWR_VDDIO_RANGE_1V8 : PWR_VDDIO_RANGE_3V3));
    }
  }
  /* Default prescaler and explicit prescaler */
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ClockPrescaler = 0;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  /* Probing runs at 400 / 8 = 50 MHz, the configured clock is applied once the memory is set up */
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0)->Init.ClockPrescaler, 7);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SET_PRESCALER, 0)->Value, EXTMEM_DEFAULT_CLOCK_PRESCALER - 1U);
  /* A slower configured clock than the probing limit is used from the start */
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ClockPrescaler = 10;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0)->Init.ClockPrescaler, 9);
  ASSERT_TRUE(MockHAL_FindEvent(MOCK_EV_SET_PRESCALER, 0) == NULL);
  /* Kernel clock so fast that even the largest divider exceeds the probing limit */
  SetupAuto(0x34, 0x5B, 0x1A);
  MockHAL_SetXspiKernelClock(4000000000U);
  s_h.Config.ClockPrescaler = 256;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0)->Init.ClockPrescaler, 255);
  /* Clock above the part maximum is refused */
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedPartNumber = "IS62WVS5128";
  s_h.Config.ClockPrescaler = 2;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_INVALID_PARAM);
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedPartNumber = "IS62WVS5128";
  s_h.Config.ClockPrescaler = 20;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ClockPrescaler = 0;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  /* DCR2.PRESCALER = divider - 1: the default divider 2 gives 200 MHz from a 400 MHz kernel clock */
  ASSERT_EQ(s_h.hxspi.Init.ClockPrescaler, EXTMEM_DEFAULT_CLOCK_PRESCALER - 1U);
  ASSERT_EQ(s_h.BusClockHz, 200000000U);
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ClockPrescaler = 5;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.hxspi.Init.ClockPrescaler, 4);
  ASSERT_EQ(s_h.BusClockHz, 80000000U);

  /* Placeholder DEVSIZE before detection: 64 MBytes */
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_XSPI_INIT, 0)->Init.MemorySize, HAL_XSPI_SIZE_512MB);
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.ClockPrescaler = 257;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_INVALID_PARAM);

  /* Invalid bus */
  SetupAuto(0x34, 0x5B, 0x1A);
  s_h.Config.Bus = (ExtMem_Bus_t)42;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Init(NULL), EXTMEM_INVALID_PARAM);

  /* FMC sub-banks: base address and NSBank */
  struct { ExtMem_Bus_t bus; uint32_t base; uint32_t bank; } fb[] = {
    { EXTMEM_BUS_FMC_SRAM_BANK1_1, 0x60000000U, FMC_NORSRAM_BANK1 },
    { EXTMEM_BUS_FMC_SRAM_BANK1_2, 0x64000000U, FMC_NORSRAM_BANK2 },
    { EXTMEM_BUS_FMC_SRAM_BANK1_3, 0x68000000U, FMC_NORSRAM_BANK3 },
    { EXTMEM_BUS_FMC_SRAM_BANK1_4, 0x6C000000U, FMC_NORSRAM_BANK4 },
  };
  for (size_t i = 0; i < 4; i++)
  {
    for (int nor = 0; nor < 2; nor++)
    {
      MockHAL_Reset();
      memset(&s_h, 0, sizeof(s_h));
      s_h.Config.Bus = fb[i].bus;
      s_h.Config.ForcedDeviceType = nor ? EXTMEM_TYPE_NOR_PARALLEL_FMC : EXTMEM_TYPE_PSRAM_PARALLEL_FMC;
      ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
      ASSERT_EQ(s_h.MemoryMappedBase, fb[i].base);
      const MockEvent_t *ev = MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0);
      ASSERT_EQ(ev->SramInit.NSBank, fb[i].bank);
      ASSERT_EQ(ev->SramInit.MemoryType, nor ? FMC_MEMORY_TYPE_NOR : FMC_MEMORY_TYPE_PSRAM);
      ASSERT_EQ(s_h.Geometry.Type, nor ? EXTMEM_TYPE_NOR_PARALLEL_FMC : EXTMEM_TYPE_PSRAM_PARALLEL_FMC);
      /* Generic part when nothing is forced */
      ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, nor ? "IS29GL_Parallel_FMC" : "IS66WV_Parallel_FMC"), 0);
      ASSERT_EQ(s_h.Geometry.TotalSizeBytes, nor ? 64U * 1024U * 1024U : 4U * 1024U * 1024U);
      /* Reset on FMC: NOR returns to read array mode, RAM needs nothing */
      MockHAL_ClearLog();
      ASSERT_EQ(ExtMem_Reset(&s_h), EXTMEM_OK);
      ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_FMC_WRITE16, 0) != NULL, nor != 0);
    }
  }

  /* FMC timings derived from the kernel clock (NOR tACC 110 ns, RAM tAA 70 ns) */
  struct { uint32_t hz; int nor; uint32_t addset, datast, busturn; } tm[] = {
    { 200000000U, 1, 2, 20, 4 },   /* 5 ns: 22 cycles access, 2 + 20 */
    { 200000000U, 0, 2, 12, 4 },   /* 70 ns -> 14 cycles, tWP 50 ns -> 10 */
    {  50000000U, 1, 1,  5, 1 },
    {  50000000U, 0, 1,  3, 1 },
    { 400000000U, 1, 4, 40, 8 },
  };
  for (size_t i = 0; i < sizeof(tm) / sizeof(tm[0]); i++)
  {
    MockHAL_Reset();
    memset(&s_h, 0, sizeof(s_h));
    s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
    s_h.Config.ForcedDeviceType = tm[i].nor ? EXTMEM_TYPE_NOR_PARALLEL_FMC : EXTMEM_TYPE_PSRAM_PARALLEL_FMC;
    if (tm[i].hz == 200000000U) MockHAL_SetHclkFreq(tm[i].hz); else s_h.Config.FmcClockHz = tm[i].hz;
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    const MockEvent_t *ev = MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0);
    ASSERT_EQ(ev->Timing.AddressSetupTime, tm[i].addset);
    ASSERT_EQ(ev->Timing.DataSetupTime, tm[i].datast);
    ASSERT_EQ(ev->Timing.BusTurnAroundDuration, tm[i].busturn);
    /* Read cycle covers the access time */
    uint64_t readNs = (uint64_t)(ev->Timing.AddressSetupTime + ev->Timing.DataSetupTime) * 1000000000ULL / tm[i].hz;
    ASSERT_TRUE(readNs >= (tm[i].nor ? 110U : 70U));
  }
  /* Very slow and very fast clocks are clamped to the FMC field limits */
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_PARALLEL_FMC;
  s_h.Config.FmcClockHz = 4000000000U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.AddressSetupTime, 15);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.DataSetupTime, 255);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.BusTurnAroundDuration, 15);
  MockHAL_Reset();
  s_h.Config.FmcClockHz = 1000000U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.AddressSetupTime, 1);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.DataSetupTime, 1);
  ASSERT_EQ(MockHAL_FindEvent(MOCK_EV_SRAM_INIT, 0)->Timing.BusTurnAroundDuration, 1);

  /* FMC forced by capacity / part number */
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_PARALLEL_FMC;
  s_h.Config.ForcedCapacityBytes = 8U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "IS29GL064"), 0);
  ASSERT_EQ(s_h.Geometry.SectorSizeBytes, 64U * 1024U);
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
  s_h.Config.ForcedCapacityBytes = 2U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "IS66WV102416"), 0);
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_PARALLEL_FMC;
  s_h.Config.ForcedCapacityBytes = 3U * 1024U * 1024U;  /* no such NOR: generic with forced size */
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 3U * 1024U * 1024U);
  ASSERT_EQ(s_h.Geometry.SectorSizeBytes, 128U * 1024U);
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
  s_h.Config.ForcedCapacityBytes = 3U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 3U * 1024U * 1024U);
  MockHAL_Reset();
  memset(&s_h, 0, sizeof(s_h));
  s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_2;
  s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_PARALLEL_FMC;
  s_h.Config.ForcedPartNumber = "MT28EW01GABA";
  s_h.Config.ForcedCapacityBytes = 64U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(strcmp(s_h.Geometry.DeviceName, "MT28EW01GABA"), 0);
  ASSERT_EQ(s_h.Geometry.TotalSizeBytes, 64U * 1024U * 1024U);

  /* FMC RAM accessed indirectly after leaving the memory-mapped state */
  {
    uint8_t tx[40], rx[40];
    Test_Pattern(tx, sizeof(tx), 0x99);
    MockHAL_Reset();
    memset(&s_h, 0, sizeof(s_h));
    s_h.Config.Bus = EXTMEM_BUS_FMC_SRAM_BANK1_1;
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(ExtMem_DisableMemoryMapped(&s_h), EXTMEM_OK);
    ASSERT_EQ(s_h.State, EXTMEM_STATE_INDIRECT);
    ASSERT_EQ(ExtMem_Write(&s_h, 0x40, tx, sizeof(tx)), EXTMEM_OK);
    ASSERT_EQ(ExtMem_Read(&s_h, 0x40, rx, sizeof(rx)), EXTMEM_OK);
    ASSERT_EQ(memcmp(tx, rx, sizeof(tx)), 0);
    ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
    ASSERT_EQ(s_h.State, EXTMEM_STATE_MEMORY_MAPPED);
    s_h.Config.ForcedDeviceType = EXTMEM_TYPE_NOR_PARALLEL_FMC;
    MockHAL_Reset();
    ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
    ASSERT_EQ(ExtMem_DisableMemoryMapped(&s_h), EXTMEM_OK);
    ASSERT_EQ(ExtMem_Read(&s_h, 0x40, rx, 4), EXTMEM_OK);
    ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
  }

  /* Forced capacity smaller than the part (shared drivers across densities) */
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedPartNumber = "S28HS512T";
  s_h.Config.ForcedCapacityBytes = 32U * 1024U * 1024U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(ExtMem_GetCapacity(&s_h), 32U * 1024U * 1024U);
  ASSERT_EQ(s_h.hxspi.Init.MemorySize, HAL_XSPI_SIZE_256MB);
  return true;
}

/* ========================================================================= */
/* Parameter checking and state handling                                     */
/* ========================================================================= */

bool test_boundary_protection_and_bounds_checking(void)
{
  uint8_t buf[64] = {0};

  ASSERT_EQ(ExtMem_Read(NULL, 0, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Write(NULL, 0, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_ReadDMA(NULL, 0, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_WriteDMA(NULL, 0, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EraseSector(NULL, 0), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EraseBlock(NULL, 0), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EraseChip(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EnableMemoryMapped(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_DisableMemoryMapped(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EnterDeepPowerDown(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_LeaveDeepPowerDown(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Reset(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_DeInit(NULL), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(strcmp(ExtMem_GetDeviceName(NULL), "UNKNOWN"), 0);
  ASSERT_EQ(ExtMem_GetCapacity(NULL), 0);
  ASSERT_TRUE(!ExtMem_IsFlash(NULL));
  ASSERT_TRUE(!ExtMem_IsRAM(NULL));

  /* 16 MB serial NOR */
  SetupAuto(0x9D, 0x60, 0x18);
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  uint32_t cap = ExtMem_GetCapacity(&s_h);
  ASSERT_EQ(cap, 16U * 1024U * 1024U);
  ASSERT_EQ(ExtMem_Read(&s_h, 0, NULL, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Read(&s_h, 0, buf, 0), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Write(&s_h, 0, NULL, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Write(&s_h, 0, buf, 0), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_ReadDMA(&s_h, 0, buf, 0), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_ReadDMA(&s_h, 0, NULL, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_WriteDMA(&s_h, 0, NULL, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_WriteDMA(&s_h, 0, buf, 0), EXTMEM_INVALID_PARAM);

  /* Geometry without a page size falls back to 256 byte programming chunks */
  {
    uint8_t big[300];
    Test_Pattern(big, sizeof(big), 1);
    s_h.Geometry.PageSizeBytes = 0;
    MockHAL_ClearLog();
    ASSERT_EQ(ExtMem_Write(&s_h, 0x10, big, sizeof(big)), EXTMEM_OK);
    ASSERT_EQ(MockHAL_FindTxAfterCommand(0x32, 0)->DataLen, 240);
    ASSERT_EQ(MockHAL_FindTxAfterCommand(0x32, 1)->DataLen, 60);
    s_h.Geometry.PageSizeBytes = 256;
  }
  ASSERT_EQ(ExtMem_Read(&s_h, cap - 16U, buf, 16), EXTMEM_OK);
  ASSERT_EQ(ExtMem_Read(&s_h, cap - 16U, buf, 17), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_Read(&s_h, 0xFFFFFFF0U, buf, 32), EXTMEM_INVALID_PARAM); /* wrap-around */
  ASSERT_EQ(ExtMem_Write(&s_h, cap, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_ReadDMA(&s_h, cap, buf, 1), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_WriteDMA(&s_h, cap - 1U, buf, 2), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EraseSector(&s_h, cap - 4096U), EXTMEM_OK);
  ASSERT_EQ(ExtMem_EraseSector(&s_h, cap), EXTMEM_INVALID_PARAM);
  ASSERT_EQ(ExtMem_EraseBlock(&s_h, cap - 65536U), EXTMEM_OK);
  ASSERT_EQ(ExtMem_EraseBlock(&s_h, cap - 4096U), EXTMEM_INVALID_PARAM);

  /* Unknown memory type in an initialized handle */
  s_h.Geometry.Type = EXTMEM_TYPE_UNKNOWN;
  ASSERT_EQ(ExtMem_Read(&s_h, 0, buf, 4), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_Write(&s_h, 0, buf, 4), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_EraseSector(&s_h, 0), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_EraseBlock(&s_h, 0), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_EraseChip(&s_h), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_Reset(&s_h), EXTMEM_NOT_SUPPORTED);
  s_h.Geometry.IsNonVolatile = false;
  ASSERT_EQ(ExtMem_Write(&s_h, 0, buf, 4), EXTMEM_NOT_SUPPORTED);
  s_h.Geometry.TotalSizeBytes = 0; /* unknown size: no bounds check */
  ASSERT_EQ(ExtMem_Read(&s_h, 0x7FFFFFFFU, buf, 4), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_Write(&s_h, 0x7FFFFFFFU, buf, 4), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_ReadDMA(&s_h, 0x7FFFFFFFU, buf, 4), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_WriteDMA(&s_h, 0x7FFFFFFFU, buf, 4), EXTMEM_NOT_SUPPORTED);
  s_h.Geometry.IsNonVolatile = true;
  ASSERT_EQ(ExtMem_EraseSector(&s_h, 0x7FFFFFFFU), EXTMEM_NOT_SUPPORTED);
  ASSERT_EQ(ExtMem_EraseBlock(&s_h, 0x7FFFFFFFU), EXTMEM_NOT_SUPPORTED);

  /* Memory-mapped mode needs an initialized handle */
  memset(&s_h, 0, sizeof(s_h));
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_INVALID_PARAM);

  /* Cache maintenance only when the D-cache is enabled */
  SetupAuto(0xFF, 0xFF, 0xFF);
  MockHAL_SetFlashSemantics(false);
  MockHAL_SetSfdpTable(s_BadSfdp, sizeof(s_BadSfdp));
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);   /* HyperRAM */
  ASSERT_EQ(ExtMem_EnableMemoryMapped(&s_h), EXTMEM_OK);
  SCB->CCR = 0;
  MockHAL_ClearLog();
  ASSERT_EQ(ExtMem_Write(&s_h, 0, buf, 8), EXTMEM_OK);
  ASSERT_EQ(ExtMem_Read(&s_h, 0, buf, 8), EXTMEM_OK);
  ASSERT_TRUE(MockHAL_FindEvent(MOCK_EV_CACHE_MAINT, 0) == NULL);
  SCB->CCR = SCB_CCR_DC_Msk;
  ASSERT_EQ(ExtMem_Read(&s_h, 0, buf, 8), EXTMEM_OK);
  const MockEvent_t *cm = MockHAL_FindEvent(MOCK_EV_CACHE_MAINT, 0);
  ASSERT_NOT_NULL(cm);
  ASSERT_EQ(cm->Address, 0x90000000U);
  ASSERT_EQ(cm->Value, 8);
  return true;
}

bool test_rm0486_devsize_register_calculation(void)
{
  /* DEVSIZE = ceil(log2(bytes)) - 1 for every capacity from 64 KB to 256 MB, including
   * sizes that are not a power of two */
  for (uint32_t p = 16; p <= 28; p++)
  {
    uint32_t sizes[2] = { 1UL << p, (1UL << p) + (1UL << (p - 2)) };
    for (int k = 0; k < 2; k++)
    {
      SetupAuto(0x00, 0x00, 0x00);
      s_h.Config.ForcedPartNumber = "S28HS512T";
      s_h.Config.ForcedCapacityBytes = sizes[k];
      ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
      uint32_t expect = (k == 0) ? (p - 1U) : p;
      ASSERT_EQ(s_h.hxspi.Init.MemorySize, expect);
    }
  }
  /* Extremes: 1 byte and 3 GBytes */
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedPartNumber = "S28HS512T";
  s_h.Config.ForcedCapacityBytes = 1U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.hxspi.Init.MemorySize, 0);
  SetupAuto(0x00, 0x00, 0x00);
  s_h.Config.ForcedPartNumber = "S28HS512T";
  s_h.Config.ForcedCapacityBytes = 0xC0000000U;
  ASSERT_EQ(ExtMem_Init(&s_h), EXTMEM_OK);
  ASSERT_EQ(s_h.hxspi.Init.MemorySize, HAL_XSPI_SIZE_32GB);

  /* Named HAL codes (sizes in bits) */
  ASSERT_EQ(HAL_XSPI_SIZE_512MB, 25); /* 64 MBytes  */
  ASSERT_EQ(HAL_XSPI_SIZE_256MB, 24); /* 32 MBytes  */
  ASSERT_EQ(HAL_XSPI_SIZE_64MB, 22);  /*  8 MBytes  */
  ASSERT_EQ(HAL_XSPI_SIZE_1GB, 26);   /* 128 MBytes */
  return true;
}
