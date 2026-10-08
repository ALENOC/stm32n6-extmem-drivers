/**
  ******************************************************************************
  * @file    stm32n6_extmem.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Unified High-Level External Memory Driver Suite implementation for STM32N6.
  ******************************************************************************
  */

#include "stm32n6_extmem.h"
#include <string.h>

/* Any 32-bit kernel clock divided down to the probing limit must fit the 8-bit prescaler */
_Static_assert(EXTMEM_INIT_MAX_CLOCK_HZ >= (0xFFFFFFFFU / 256U) + 1U, "EXTMEM_INIT_MAX_CLOCK_HZ too low for the XSPI prescaler");

/* CPU accesses to the memory-mapped window. Host unit tests route them to the emulated array. */
static void ExtMem_MappedRead(uint32_t base, uint32_t offset, uint8_t *pData, uint32_t size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)base;
  MockHAL_RamRead(offset, pData, size);
#else
  const volatile uint8_t *pSrc = (const volatile uint8_t *)(uintptr_t)(base + offset);
  for (uint32_t i = 0; i < size; i++)
  {
    pData[i] = pSrc[i];
  }
#endif
}

static void ExtMem_MappedWrite(uint32_t base, uint32_t offset, const uint8_t *pData, uint32_t size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)base;
  MockHAL_RamWrite(offset, pData, size);
#else
  volatile uint8_t *pDst = (volatile uint8_t *)(uintptr_t)(base + offset);
  for (uint32_t i = 0; i < size; i++)
  {
    pDst[i] = pData[i];
  }
#endif
}

/* Power and clock setup for the XSPI port. Pin muxing is board specific and belongs to
 * HAL_XSPI_MspInit() in the application, which HAL_XSPI_Init() calls. */
static void ExtMem_EnablePowerAndClocks(XSPI_HandleTypeDef *hxspi, bool use1V8, uint32_t targetPort)
{
  /* 1. Enable Power and configure VDDIO domain: XSPIM Port 2 pins (PN) are on VDDIO3, Port 1 pins
   *    (PO/PP) on VDDIO2, as in the STM32N6570-DK BSP (NOR on XSPI2 / VDDIO3, PSRAM on XSPI1 / VDDIO2) */
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  __HAL_RCC_PWR_CLK_ENABLE();
#endif

  uint32_t pwrDomain = (targetPort == HAL_XSPIM_IOPORT_2) ? PWR_VDDIO3 : PWR_VDDIO2;
  if (targetPort == HAL_XSPIM_IOPORT_2)
  {
    HAL_PWREx_EnableVddIO3();
  }
  else
  {
    HAL_PWREx_EnableVddIO2();
  }
  HAL_PWREx_ConfigVddIORange(pwrDomain, use1V8 ? PWR_VDDIO_RANGE_1V8 : PWR_VDDIO_RANGE_3V3);

  /* 2. Enable XSPI interface clock & XSPIM */
  if (hxspi->Instance == XSPI1)
  {
    __HAL_RCC_XSPI1_CLK_ENABLE();
    __HAL_RCC_XSPI1_FORCE_RESET();
    __HAL_RCC_XSPI1_RELEASE_RESET();
  }
#if defined(XSPI2)
  else if (hxspi->Instance == XSPI2)
  {
    __HAL_RCC_XSPI2_CLK_ENABLE();
    __HAL_RCC_XSPI2_FORCE_RESET();
    __HAL_RCC_XSPI2_RELEASE_RESET();
  }
#endif
#if defined(XSPI3)
  else if (hxspi->Instance == XSPI3)
  {
    __HAL_RCC_XSPI3_CLK_ENABLE();
    __HAL_RCC_XSPI3_FORCE_RESET();
    __HAL_RCC_XSPI3_RELEASE_RESET();
  }
#endif

  __HAL_RCC_XSPIM_CLK_ENABLE();
}

/* Route the XSPI instance to its I/O port through the XSPI I/O manager (after HAL_XSPI_Init) */
static int32_t ExtMem_ConfigureXspim(XSPI_HandleTypeDef *hxspi, uint32_t targetPort)
{
  XSPIM_CfgTypeDef sXspiManagerCfg = {0};
  sXspiManagerCfg.IOPort      = targetPort;
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.Req2AckTime = 1;
  return (HAL_XSPIM_Config(hxspi, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) ? EXTMEM_OK : EXTMEM_ERROR;
}

/* Helper to calculate XSPI_DCR1 DEVSIZE from capacity in bytes per RM0486 */
static uint32_t ExtMem_CalculateMemorySize(uint32_t totalSizeBytes)
{
  /* Unknown size: 64 MBytes (the HAL names this code HAL_XSPI_SIZE_512MB, i.e. 512 Mbits) */
  if (totalSizeBytes == 0U) return HAL_XSPI_SIZE_512MB;
  uint32_t power = 0;
  /* Smallest power of two that covers the device */
  while ((power < 32U) && ((1ULL << power) < (uint64_t)totalSizeBytes))
  {
    power++;
  }
  /* RM0486: external memory capacity is 2^(DEVSIZE + 1) bytes, so DEVSIZE = power - 1 */
  return (power >= 1U) ? (power - 1U) : 0U;
}

/* Cache Maintenance Helper for Cortex-M55 */
static void ExtMem_CacheCleanInvalidate(uint32_t addr, uint32_t size)
{
#if (EXTMEM_ENABLE_DCACHE_MAINTENANCE == 1)
  if (SCB->CCR & SCB_CCR_DC_Msk)
  {
    SCB_CleanInvalidateDCache_by_Addr((void *)(uintptr_t)addr, (int32_t)size);
  }
#else
  (void)addr;
  (void)size;
#endif
}

/* True when the handle drives one of the FMC NOR/SRAM sub-banks instead of an XSPI port */
static bool ExtMem_IsFmcBus(ExtMem_Bus_t bus)
{
  return (bus == EXTMEM_BUS_FMC_SRAM_BANK1_1) || (bus == EXTMEM_BUS_FMC_SRAM_BANK1_2) ||
         (bus == EXTMEM_BUS_FMC_SRAM_BANK1_3) || (bus == EXTMEM_BUS_FMC_SRAM_BANK1_4);
}

/* FMC sub-bank index (0 for NE1 .. 3 for NE4) */
static uint32_t ExtMem_FmcSubBankIndex(ExtMem_Bus_t bus)
{
  return (uint32_t)bus - (uint32_t)EXTMEM_BUS_FMC_SRAM_BANK1_1;
}

static uint32_t ExtMem_FmcHalBank(ExtMem_Bus_t bus)
{
  static const uint32_t banks[4] = { FMC_NORSRAM_BANK1, FMC_NORSRAM_BANK2, FMC_NORSRAM_BANK3, FMC_NORSRAM_BANK4 };
  return banks[ExtMem_FmcSubBankIndex(bus)];
}

/* Asynchronous timing limits (ns) covering the slowest parallel parts in the database:
 * IS29GL512 / MT28EW tACC 110 ns, tWP 35 ns; IS66WV / CY62167 tAA 70 ns, tWP 50 ns. */
#define EXTMEM_FMC_NOR_TACC_NS     110U
#define EXTMEM_FMC_NOR_TWP_NS       35U
#define EXTMEM_FMC_RAM_TACC_NS      70U
#define EXTMEM_FMC_RAM_TWP_NS       50U
#define EXTMEM_FMC_TAS_NS           10U
#define EXTMEM_FMC_TBUSTURN_NS      20U

/* Clock periods covering 'ns' (rounded up, at least 1 for any non-zero time), clamped to the register field */
static uint32_t ExtMem_NsToCycles(uint32_t ns, uint32_t clockHz, uint32_t maxCycles)
{
  uint64_t cycles = (((uint64_t)ns * clockHz) + 999999999ULL) / 1000000000ULL;
  return (cycles > maxCycles) ? maxCycles : (uint32_t)cycles;
}

/* FMC mode A/B: ADDSET covers the address setup, DATAST covers both the rest of the read access
 * (so ADDSET + DATAST >= tACC) and the write pulse width tWP. */
static void ExtMem_FmcTiming(uint32_t clockHz, uint32_t tAccNs, uint32_t tWpNs,
                             uint32_t *pAddSet, uint32_t *pAddHold, uint32_t *pDataSet, uint32_t *pBusTurn)
{
  uint32_t dataNs = ((tAccNs - EXTMEM_FMC_TAS_NS) > tWpNs) ? (tAccNs - EXTMEM_FMC_TAS_NS) : tWpNs;
  *pAddSet  = ExtMem_NsToCycles(EXTMEM_FMC_TAS_NS, clockHz, 15U);
  *pAddHold = 1U;
  *pDataSet = ExtMem_NsToCycles(dataNs, clockHz, 255U);
  *pBusTurn = ExtMem_NsToCycles(EXTMEM_FMC_TBUSTURN_NS, clockHz, 15U);
}

/* Copy descriptor geometry into the handle; volatility comes from the memory type */
static void ExtMem_ApplyDescriptor(ExtMem_HandleTypeDef *hextmem, const ExtMem_DeviceDescriptor_t *dev)
{
  hextmem->pDevice = dev;
  strncpy(hextmem->Geometry.DeviceName, dev->PartNumber, sizeof(hextmem->Geometry.DeviceName) - 1);
  hextmem->Geometry.Type            = dev->Type;
  hextmem->Geometry.TotalSizeBytes  = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : dev->CapacityBytes;
  hextmem->Geometry.PageSizeBytes   = dev->PageSizeBytes;
  hextmem->Geometry.SectorSizeBytes = dev->SectorSizeBytes;
  hextmem->Geometry.BlockSizeBytes  = dev->BlockSizeBytes;
  hextmem->Geometry.MaxClockFreqMHz = dev->MaxClockFreqMHz;
  hextmem->Geometry.SupportsDTR     = (dev->PreferredMode == EXTMEM_MODE_OCTAL_DTR || dev->PreferredMode == EXTMEM_MODE_HYPERBUS);
  hextmem->Geometry.SupportsMemoryMapped = true;
  hextmem->Geometry.IsNonVolatile   = !ExtMem_TypeIsVolatile(dev->Type);
}

/* XSPI kernel clock of the instance, from the RCC */
static uint32_t ExtMem_XspiKernelClock(const void *instance)
{
  if (instance == XSPI1) return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);
#if defined(XSPI2)
  if (instance == XSPI2) return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI2);
#endif
  return HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI3);
}

/* XSPI DCR4.REFRESH: CS# is released every REFRESH + 1 clocks. Keep 4 clocks of margin as the
 * STM32N6570-DK BSP does, and assume the fastest bus when the RCC does not report a clock. */
static uint32_t ExtMem_RefreshCycles(uint32_t busClockHz)
{
  uint64_t clk = (busClockHz > 0U) ? busClockHz : EXTMEM_XSPI_FALLBACK_BUS_CLOCK_HZ;
  uint64_t cycles = (clk * EXTMEM_PSRAM_MAX_CS_LOW_NS) / 1000000000ULL;
  return (cycles > 8U) ? (uint32_t)(cycles - 4U) : 4U;
}

/* Read latency from the matched database entry, or the protocol default for SFDP / generic parts */
static uint8_t ExtMem_ReadDummy(const ExtMem_HandleTypeDef *hextmem, uint8_t fallback)
{
  return (hextmem->pDevice != NULL) ? hextmem->pDevice->DefaultReadDummyCycles : fallback;
}

int32_t ExtMem_Init(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL)
  {
    return EXTMEM_INVALID_PARAM;
  }

  hextmem->State = EXTMEM_STATE_UNINITIALIZED;

  /* Select Peripheral Instance */
  if (hextmem->Config.Bus == EXTMEM_BUS_XSPI1)
  {
    hextmem->hxspi.Instance   = XSPI1;
    hextmem->MemoryMappedBase = EXTMEM_XSPI1_BASE_ADDR;
  }
  else if (hextmem->Config.Bus == EXTMEM_BUS_XSPI2)
  {
#if defined(XSPI2)
    hextmem->hxspi.Instance   = XSPI2;
    hextmem->MemoryMappedBase = EXTMEM_XSPI2_BASE_ADDR;
#else
    return EXTMEM_NOT_SUPPORTED;
#endif
  }
  else if (hextmem->Config.Bus == EXTMEM_BUS_XSPI3)
  {
#if defined(XSPI3)
    hextmem->hxspi.Instance   = XSPI3;
    hextmem->MemoryMappedBase = EXTMEM_XSPI3_BASE_ADDR;
#else
    return EXTMEM_NOT_SUPPORTED;
#endif
  }
  else /* FMC */
  {
    if (!ExtMem_IsFmcBus(hextmem->Config.Bus))
    {
      return EXTMEM_INVALID_PARAM;
    }

    /* Each NEx sub-bank owns a 64 MB window starting at 0x60000000 */
    hextmem->MemoryMappedBase = EXTMEM_FMC_BANK1_BASE_ADDR + (ExtMem_FmcSubBankIndex(hextmem->Config.Bus) * 0x04000000U);
    hextmem->ActiveMode       = EXTMEM_MODE_PARALLEL_16BIT;
    uint32_t fmcClock = (hextmem->Config.FmcClockHz > 0U) ? hextmem->Config.FmcClockHz : HAL_RCC_GetHCLKFreq();

    if (hextmem->Config.ForcedDeviceType == EXTMEM_TYPE_NOR_PARALLEL_FMC)
    {
      const ExtMem_DeviceDescriptor_t *dev = NULL;
      if (hextmem->Config.ForcedPartNumber != NULL)
      {
        dev = ExtMem_FindDeviceByPartNumber(hextmem->Config.ForcedPartNumber);
      }
      else if (hextmem->Config.ForcedCapacityBytes > 0)
      {
        dev = ExtMem_FindDeviceByTypeAndCapacity(EXTMEM_TYPE_NOR_PARALLEL_FMC, hextmem->Config.ForcedCapacityBytes);
      }

      if (dev != NULL)
      {
        hextmem->pDevice = dev;
        hextmem->Geometry.TotalSizeBytes = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : dev->CapacityBytes;
        strncpy(hextmem->Geometry.DeviceName, dev->PartNumber, sizeof(hextmem->Geometry.DeviceName) - 1);
      }
      else
      {
        hextmem->Geometry.TotalSizeBytes = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : (64 * 1024 * 1024);
        strncpy(hextmem->Geometry.DeviceName, "IS29GL_Parallel_FMC", sizeof(hextmem->Geometry.DeviceName) - 1);
      }

      hextmem->Geometry.Type = EXTMEM_TYPE_NOR_PARALLEL_FMC;
      hextmem->Geometry.IsNonVolatile = true;
      hextmem->Geometry.SupportsMemoryMapped = true;
      hextmem->Geometry.PageSizeBytes = 0;
      hextmem->Geometry.SectorSizeBytes = (dev != NULL) ? dev->SectorSizeBytes : (128 * 1024);
      hextmem->Geometry.BlockSizeBytes  = (dev != NULL) ? dev->BlockSizeBytes : (128 * 1024);

      IS29GL_FMC_Timing_t norTiming;
      ExtMem_FmcTiming(fmcClock, EXTMEM_FMC_NOR_TACC_NS, EXTMEM_FMC_NOR_TWP_NS, &norTiming.AddressSetupTime,
                       &norTiming.AddressHoldTime, &norTiming.DataSetupTime, &norTiming.BusTurnAroundDuration);
      if (IS29GL_FMC_Init(&hextmem->hsram, ExtMem_FmcHalBank(hextmem->Config.Bus), &norTiming) != IS29GL_FMC_OK)
      {
        return EXTMEM_ERROR;
      }
    }
    else
    {
      const ExtMem_DeviceDescriptor_t *dev = NULL;
      if (hextmem->Config.ForcedPartNumber != NULL)
      {
        dev = ExtMem_FindDeviceByPartNumber(hextmem->Config.ForcedPartNumber);
      }
      else if (hextmem->Config.ForcedCapacityBytes > 0)
      {
        dev = ExtMem_FindDeviceByTypeAndCapacity(EXTMEM_TYPE_PSRAM_PARALLEL_FMC, hextmem->Config.ForcedCapacityBytes);
      }

      if (dev != NULL)
      {
        hextmem->pDevice = dev;
        hextmem->Geometry.TotalSizeBytes = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : dev->CapacityBytes;
        strncpy(hextmem->Geometry.DeviceName, dev->PartNumber, sizeof(hextmem->Geometry.DeviceName) - 1);
      }
      else
      {
        hextmem->Geometry.TotalSizeBytes = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : (4 * 1024 * 1024);
        strncpy(hextmem->Geometry.DeviceName, "IS66WV_Parallel_FMC", sizeof(hextmem->Geometry.DeviceName) - 1);
      }

      hextmem->Geometry.Type    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC;
      hextmem->Geometry.IsNonVolatile = false;
      hextmem->Geometry.SupportsMemoryMapped = true;

      IS66WV_FMC_Timing_t timing;
      ExtMem_FmcTiming(fmcClock, EXTMEM_FMC_RAM_TACC_NS, EXTMEM_FMC_RAM_TWP_NS, &timing.AddressSetupTime,
                       &timing.AddressHoldTime, &timing.DataSetupTime, &timing.BusTurnAroundDuration);
      if (IS66WV_FMC_Init(&hextmem->hsram, ExtMem_FmcHalBank(hextmem->Config.Bus), &timing) != IS66WV_FMC_OK)
      {
        return EXTMEM_ERROR;
      }
    }
    hextmem->State = EXTMEM_STATE_MEMORY_MAPPED;
    return EXTMEM_OK;
  }

  /* Determine physical XSPIM port (Port 1 vs Port 2) */
  uint32_t targetPort = hextmem->Config.IOPort;
  if (targetPort == 0)
  {
    targetPort = (hextmem->Config.Bus == EXTMEM_BUS_XSPI2) ? HAL_XSPIM_IOPORT_2 : HAL_XSPIM_IOPORT_1;
  }

  /* Power domain and clocks */
  ExtMem_EnablePowerAndClocks(&hextmem->hxspi, hextmem->Config.Force1V8, targetPort);

  /* Clock divider: DCR2.PRESCALER holds divider - 1 (Fclk = Fkernel / (PRESCALER + 1)) */
  uint32_t divider = (hextmem->Config.ClockPrescaler > 0U) ? hextmem->Config.ClockPrescaler : EXTMEM_DEFAULT_CLOCK_PRESCALER;
  if (divider > 256U)
  {
    return EXTMEM_INVALID_PARAM;
  }
  uint32_t prescaler = divider - 1U;
  uint32_t kernelHz = ExtMem_XspiKernelClock(hextmem->hxspi.Instance);
  hextmem->BusClockHz = kernelHz / divider;

  /* Probing and mode switching run in 1S-1S-1S, slower than the final protocol on most parts:
   * keep the bus at or below EXTMEM_INIT_MAX_CLOCK_HZ until the memory is configured */
  uint32_t initPrescaler = prescaler;
  if (kernelHz > 0U)
  {
    /* At most 4294967295 / EXTMEM_INIT_MAX_CLOCK_HZ: always a valid divider (checked below) */
    uint32_t initDivider = (kernelHz + EXTMEM_INIT_MAX_CLOCK_HZ - 1U) / EXTMEM_INIT_MAX_CLOCK_HZ;
    if (initDivider > divider) initPrescaler = initDivider - 1U;
  }

  /* Basic XSPI Init in Single SPI mode */
  hextmem->hxspi.Init.FifoThresholdByte       = 4;
  hextmem->hxspi.Init.MemoryType              = HAL_XSPI_MEMTYPE_MICRON;
  hextmem->hxspi.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hextmem->hxspi.Init.MemorySize              = ExtMem_CalculateMemorySize(0); /* Placeholder until detection */
  hextmem->hxspi.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  hextmem->hxspi.Init.ChipSelectHighTimeCycle = 4;
  hextmem->hxspi.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hextmem->hxspi.Init.ClockPrescaler          = initPrescaler;
  hextmem->hxspi.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hextmem->hxspi.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  hextmem->hxspi.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hextmem->hxspi.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hextmem->hxspi.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hextmem->hxspi.Init.MaxTran                 = 0;
  hextmem->hxspi.Init.Refresh                 = 0;

  if (HAL_XSPI_Init(&hextmem->hxspi) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  if (ExtMem_ConfigureXspim(&hextmem->hxspi, targetPort) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Run Auto-Detection or use forced configuration */
  int32_t detect = ExtMem_AutoDetect(hextmem);
  if (detect != EXTMEM_OK)
  {
    return detect;
  }

  /* Update XSPI DEVSIZE to match the detected capacity per RM0486 (detection never reports 0 bytes) */
  uint32_t memSize = ExtMem_CalculateMemorySize(hextmem->Geometry.TotalSizeBytes);
  hextmem->hxspi.Init.MemorySize = memSize;

  /* Stacked-die octal NOR: bursts restart at each die (CSBOUND code n = 2^n bytes) */
  uint32_t dice = (hextmem->pDevice != NULL && hextmem->pDevice->DieCount > 1U) ? hextmem->pDevice->DieCount : 1U;
  if (dice > 1U && hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)
  {
    hextmem->hxspi.Init.ChipSelectBoundary = ExtMem_CalculateMemorySize(hextmem->Geometry.TotalSizeBytes / dice) + 1U;
  }
  if (HAL_XSPI_Init(&hextmem->hxspi) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  /* The configured clock must not exceed what the detected part supports */
  if (hextmem->pDevice != NULL && hextmem->BusClockHz > (hextmem->pDevice->MaxClockFreqMHz * 1000000U))
  {
    return EXTMEM_INVALID_PARAM;
  }

  /* Self-refreshing RAMs need CS# released before tCSM: the drivers keep this setting */
  if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI || hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI ||
      hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON || hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
  {
    hextmem->hxspi.Init.Refresh = ExtMem_RefreshCycles(hextmem->BusClockHz);
  }

  /* Commands stay in 1S-1S-1S until a high performance mode is confirmed below */
  hextmem->ActiveMode = EXTMEM_MODE_SPI;

  bool is4Byte = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));

  /* Switch memory to its high performance mode */
  if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)
  {
    /* Stacked dice: every die has its own registers, located through the SFDP SCCR tables */
    S28HS512T_DieLayout_t layout = { 1U, 0U, { S28HS_REG_VOLATILE_BASE, 0U, 0U, 0U } };
    if (dice > 1U)
    {
      uint8_t sfdpDice = 0;
      int32_t map = SFDP_ReadDieRegisterMap(&hextmem->hxspi, layout.VregBase, &sfdpDice);
      if (map != EXTMEM_OK) return map;
      /* The 2 Gb parts describe 4 dice in SFDP but have 2 (same correction as Linux spi-nor) */
      if (sfdpDice < dice) return EXTMEM_NOT_SUPPORTED;
      layout.Dice    = (uint8_t)dice;
      layout.DieSize = hextmem->Geometry.TotalSizeBytes / dice;
    }
    if (S28HS512T_SetDieLayout(&hextmem->hxspi, &layout) != S28HS512T_OK) return EXTMEM_ERROR;

    /* SEMPER latency is programmed as MEMLAT = 0xB, which always means 24 cycles */
    hextmem->DummyCycles = S28HS_OCTAL_DTR_READ_DUMMY;
    if (S28HS512T_EnterOctalDTRMode(&hextmem->hxspi, hextmem->DummyCycles) != S28HS512T_OK)
    {
      return EXTMEM_ERROR;
    }
    hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_ISSI)
  {
    hextmem->DummyCycles = ExtMem_ReadDummy(hextmem, 20);
    if (IS25LX256_EnterOctalDTRMode(&hextmem->hxspi, hextmem->DummyCycles) != IS25LX_OK)
    {
      return EXTMEM_ERROR;
    }
    hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_MICRON)
  {
    hextmem->DummyCycles = ExtMem_ReadDummy(hextmem, 20);
    if (MT35XU_EnterOctalDTRMode(&hextmem->hxspi, hextmem->DummyCycles) != MT35XU_OK)
    {
      return EXTMEM_ERROR;
    }
    hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    hextmem->DummyCycles = IS66WVO_DUMMY_CYCLES;
    if (IS66WVO32M8_Init(&hextmem->hxspi, prescaler, memSize) != IS66WVO_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    if (S27KS0641_Init(&hextmem->hxspi, prescaler, memSize) != S27KS_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
  {
    if (IS66WVH16M8_Init(&hextmem->hxspi, prescaler, memSize) != IS66WVH_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERFLASH_INFINEON ||
           hextmem->Geometry.Type == EXTMEM_TYPE_HYPERFLASH_ISSI)
  {
    if (S26KS512S_Init(&hextmem->hxspi, prescaler, memSize) != S26KS512S_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_QUAD_INFINEON)
  {
    if (S25HL512T_EnableQuadMode(&hextmem->hxspi) != S25HL512T_OK) return EXTMEM_ERROR;
    hextmem->DummyCycles = S25HL_DEFAULT_READ_LATENCY;
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_QUAD_ISSI)
  {
    if (IS25LP256_EnableQuadMode(&hextmem->hxspi) != IS25LP_OK) return EXTMEM_ERROR;
    /* The factory latency only covers 81 MHz on 1-4-4 reads: program the volatile Read Register */
    if (IS25LP256_SetReadDummyCycles(&hextmem->hxspi, IS25LP_FAST_QUAD_IO_DUMMY, &hextmem->DummyCycles) != IS25LP_OK) return EXTMEM_ERROR;
    if (is4Byte && (IS25LP_Enter4ByteAddressMode(&hextmem->hxspi) != IS25LP_OK))
    {
      return EXTMEM_ERROR;
    }
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_QUAD_MICRON)
  {
    hextmem->DummyCycles = ExtMem_ReadDummy(hextmem, 10);
    if (is4Byte && (MT25QU_Enter4ByteAddressMode(&hextmem->hxspi) != MT25Q_OK))
    {
      return EXTMEM_ERROR;
    }
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI)
  {
    /* Stay in SPI protocol: 0xEB / 0x38 are 1-4-4 commands. Entering QPI (0x35) would require
     * every following instruction to be sent on 4 lines. */
    if (IS66WVS16M8_Init(&hextmem->hxspi, prescaler, memSize) != IS66WVS_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else /* EXTMEM_TYPE_SRAM_SERIAL_ISSI: the last type ExtMem_AutoDetect can report */
  {
    hextmem->DummyCycles = ExtMem_ReadDummy(hextmem, 2);
    if (IS62WVS_Init(&hextmem->hxspi, prescaler, memSize) != IS62WVS_OK) return EXTMEM_ERROR;
    if (IS62WVS_WriteModeRegister(&hextmem->hxspi, IS62WVS_MODE_SEQUENTIAL) != IS62WVS_OK) return EXTMEM_ERROR;
    if (IS62WVS_EnterQuadMode(&hextmem->hxspi) != IS62WVS_OK) return EXTMEM_ERROR;
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_4_4_4; /* SQI: instruction, address and data on 4 lines */
  }

  /* Memory configured: switch to the requested clock (drivers that re-initialize the XSPI already use it) */
  if (hextmem->hxspi.Init.ClockPrescaler != prescaler)
  {
    if (HAL_XSPI_SetClockPrescaler(&hextmem->hxspi, prescaler) != HAL_OK) return EXTMEM_ERROR;
  }

  hextmem->State = EXTMEM_STATE_INDIRECT;
  return EXTMEM_OK;
}

int32_t ExtMem_AutoDetect(ExtMem_HandleTypeDef *hextmem)
{
  /* If device part number is explicitly forced by user */
  if (hextmem->Config.ForcedPartNumber != NULL)
  {
    const ExtMem_DeviceDescriptor_t *dev = ExtMem_FindDeviceByPartNumber(hextmem->Config.ForcedPartNumber);
    if (dev != NULL)
    {
      ExtMem_ApplyDescriptor(hextmem, dev);
      return EXTMEM_OK;
    }
  }

  /* If device type is explicitly forced by user, find matching database entry */
  if (hextmem->Config.ForcedDeviceType != EXTMEM_TYPE_UNKNOWN)
  {
    const ExtMem_DeviceDescriptor_t *dev = NULL;
    if (hextmem->Config.ForcedCapacityBytes > 0)
    {
      dev = ExtMem_FindDeviceByTypeAndCapacity(hextmem->Config.ForcedDeviceType, hextmem->Config.ForcedCapacityBytes);
    }
    if (dev == NULL)
    {
      for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
      {
        if (ExtMem_DeviceDatabase[i].Type == hextmem->Config.ForcedDeviceType)
        {
          dev = &ExtMem_DeviceDatabase[i];
          break;
        }
      }
    }

    if (dev != NULL)
    {
      ExtMem_ApplyDescriptor(hextmem, dev);
      return EXTMEM_OK;
    }
  }

  uint8_t id[3] = {0};
  memset(hextmem->RawID, 0, sizeof(hextmem->RawID));

  /* 1. Try Standard SPI 0x9F Read ID. A failed transfer is a controller error, not a missing device. */
  if (S28HS512T_ReadID(&hextmem->hxspi, id) != S28HS512T_OK)
  {
    return EXTMEM_ERROR;
  }
  memcpy(hextmem->RawID, id, 3);
  const ExtMem_DeviceDescriptor_t *idDev = ExtMem_FindDevice(id[0], id[1], id[2]);
  if (idDev != NULL)
  {
    ExtMem_ApplyDescriptor(hextmem, idDev);
    return EXTMEM_OK;
  }

  /* 2. Try SFDP (JESD216) Probe. Only quad NOR vendors with a driver in this suite are accepted:
   *    the quad enable sequence and the opcodes differ between vendors. */
#if (EXTMEM_ENABLE_SFDP_PROBE == 1)
  SFDP_FlashParams_t sfdpParams;
  int32_t sfdp = SFDP_ReadAndParse(&hextmem->hxspi, &sfdpParams);
  if (sfdp == EXTMEM_ERROR)
  {
    return EXTMEM_ERROR;
  }
  if (sfdp == EXTMEM_OK)
  {
    ExtMem_Type_t sfdpType = EXTMEM_TYPE_UNKNOWN;
    switch (hextmem->RawID[0])
    {
      case EXTMEM_MFG_ISSI:              sfdpType = EXTMEM_TYPE_NOR_QUAD_ISSI;     break;
      case EXTMEM_MFG_NUMONYX_LEGACY:    sfdpType = EXTMEM_TYPE_NOR_QUAD_MICRON;   break;
      case EXTMEM_MFG_INFINEON_SPANSION:
      case EXTMEM_MFG_CYPRESS_LEGACY:    sfdpType = EXTMEM_TYPE_NOR_QUAD_INFINEON; break;
      default:                           break;
    }

    if (sfdpType != EXTMEM_TYPE_UNKNOWN && sfdpParams.DensityBytes > 0U)
    {
      hextmem->pDevice = NULL;
      strncpy(hextmem->Geometry.DeviceName, "JEDEC_SFDP_Flash", sizeof(hextmem->Geometry.DeviceName) - 1);
      hextmem->Geometry.Type            = sfdpType;
      hextmem->Geometry.TotalSizeBytes  = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : sfdpParams.DensityBytes;
      hextmem->Geometry.PageSizeBytes   = sfdpParams.PageSizeBytes;
      hextmem->Geometry.SectorSizeBytes = sfdpParams.Supports4KBErase ? 4096U : sfdpParams.SectorSizeBytes;
      hextmem->Geometry.BlockSizeBytes  = sfdpParams.SectorSizeBytes;
      hextmem->Geometry.SupportsDTR     = false;
      hextmem->Geometry.SupportsMemoryMapped = true;
      hextmem->Geometry.IsNonVolatile   = true;
      return EXTMEM_OK;
    }
  }
#endif

  /* 3. Try HyperBus Register Space Probe (HyperRAM). HyperFlash has no register space and
   *    must be selected with Config.ForcedDeviceType or Config.ForcedPartNumber. */
  /* The power-on latency is 7 clocks on 200 MHz parts and 6 clocks on 166 MHz parts: the ID0 read
   * only returns valid data when the controller latency matches, so both are tried. */
  static const uint32_t probeLatency[2] = { 7U, 6U };
  uint16_t hyperId0 = 0;
  uint32_t capacity = 0;

  hextmem->hxspi.Init.MemoryType = HAL_XSPI_MEMTYPE_HYPERBUS;
  if (HAL_XSPI_Init(&hextmem->hxspi) != HAL_OK) return EXTMEM_ERROR;

  for (uint32_t i = 0; (i < 2U) && (capacity == 0U); i++)
  {
    XSPI_HyperbusCfgTypeDef sHyperCfg = {
      .RWRecoveryTimeCycle = 4,
      .AccessTimeCycle     = probeLatency[i],
      .WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE,
      .LatencyMode         = HAL_XSPI_VARIABLE_LATENCY
    };
    if (HAL_XSPI_HyperbusCfg(&hextmem->hxspi, &sHyperCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return EXTMEM_ERROR;
    if (S27KS0641_ReadRegister(&hextmem->hxspi, S27KS_REG_ID0, &hyperId0) != S27KS_OK) return EXTMEM_ERROR;

    /* A valid ID0 names Cypress/Infineon (0001b) or ISSI (0011b) and a decodable geometry */
    uint8_t idMfg = (uint8_t)(hyperId0 & 0x0FU);
    if (idMfg == EXTMEM_HYPERRAM_MFG_CYPRESS || idMfg == EXTMEM_HYPERRAM_MFG_ISSI)
    {
      /* ID0[12:8] = row address bits - 1, ID0[7:4] = column address bits - 1, 16-bit words */
      uint32_t rowBits = ((uint32_t)(hyperId0 >> 8) & 0x1FU) + 1U;
      uint32_t colBits = ((uint32_t)(hyperId0 >> 4) & 0x0FU) + 1U;
      uint32_t idBytes = (rowBits + colBits + 1U < 32U) ? (1UL << (rowBits + colBits + 1U)) : 0U;
      if (idBytes > 0U)
      {
        capacity = (hextmem->Config.ForcedCapacityBytes > 0) ? hextmem->Config.ForcedCapacityBytes : idBytes;
      }
    }
  }

  if (capacity > 0U)
  {
    uint8_t mfg = (uint8_t)(hyperId0 & 0x0F);
    ExtMem_Type_t hyperType = (mfg == EXTMEM_HYPERRAM_MFG_CYPRESS) ? EXTMEM_TYPE_HYPERRAM_INFINEON : EXTMEM_TYPE_HYPERRAM_ISSI;
    const ExtMem_DeviceDescriptor_t *dev = ExtMem_FindDeviceByTypeAndCapacity(hyperType, capacity);

    if (dev != NULL)
    {
      ExtMem_ApplyDescriptor(hextmem, dev);
      hextmem->Geometry.TotalSizeBytes = capacity;
    }
    else
    {
      hextmem->pDevice = NULL;
      strncpy(hextmem->Geometry.DeviceName, (hyperType == EXTMEM_TYPE_HYPERRAM_INFINEON) ? "S27KS_HyperRAM" : "IS66WVH_HyperRAM",
              sizeof(hextmem->Geometry.DeviceName) - 1);
      hextmem->Geometry.Type            = hyperType;
      hextmem->Geometry.TotalSizeBytes  = capacity;
      hextmem->Geometry.PageSizeBytes   = 0;
      hextmem->Geometry.SectorSizeBytes = 0;
      hextmem->Geometry.BlockSizeBytes  = 0;
      hextmem->Geometry.SupportsDTR     = true;
      hextmem->Geometry.SupportsMemoryMapped = true;
      hextmem->Geometry.IsNonVolatile   = false;
    }
    return EXTMEM_OK;
  }

  /* HyperBus probe failed: return the controller to regular command mode */
  hextmem->hxspi.Init.MemoryType = HAL_XSPI_MEMTYPE_MICRON;
  if (HAL_XSPI_Init(&hextmem->hxspi) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Nothing answered: never guess a device, programming the wrong part can corrupt it */
  hextmem->pDevice = NULL;
  hextmem->Geometry.Type = EXTMEM_TYPE_UNKNOWN;
  return EXTMEM_NOT_SUPPORTED;
}

int32_t ExtMem_EnableMemoryMapped(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL || hextmem->State == EXTMEM_STATE_UNINITIALIZED) return EXTMEM_INVALID_PARAM;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED) return EXTMEM_OK;

  int32_t ret = EXTMEM_ERROR;

  /* Octal NOR memory-mapped read is only configured for 8D-8D-8D */
  if ((hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER ||
       hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_ISSI ||
       hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_MICRON) &&
      hextmem->ActiveMode != EXTMEM_MODE_OCTAL_DTR)
  {
    return EXTMEM_NOT_SUPPORTED;
  }

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      ret = S28HS512T_EnableMemoryMappedModeDTR(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      ret = IS25LX256_EnableMemoryMappedModeDTR(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      ret = MT35XU_EnableMemoryMappedModeDTR(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      ret = S26KS512S_EnableMemoryMappedMode(&hextmem->hxspi);
      break;

    case EXTMEM_TYPE_HYPERRAM_INFINEON:
      ret = S27KS0641_EnableMemoryMappedMode(&hextmem->hxspi);
      break;

    case EXTMEM_TYPE_HYPERRAM_ISSI:
      ret = IS66WVH16M8_EnableMemoryMappedMode(&hextmem->hxspi);
      break;

    case EXTMEM_TYPE_PSRAM_OCTAL_ISSI:
      ret = IS66WVO32M8_EnableMemoryMappedMode(&hextmem->hxspi, hextmem->DummyCycles, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      ret = S25HL512T_EnableMemoryMappedMode(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_NOR_QUAD_ISSI:
    {
      bool is4B = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
      ret = IS25LP_EnableMemoryMappedModeEx(&hextmem->hxspi, hextmem->DummyCycles, is4B ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS);
      break;
    }

    case EXTMEM_TYPE_NOR_QUAD_MICRON:
    {
      bool is4B = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
      ret = MT25QU_EnableMemoryMappedModeEx(&hextmem->hxspi, hextmem->DummyCycles, is4B ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS);
      break;
    }

    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      ret = IS66WVS16M8_EnableMemoryMappedMode(&hextmem->hxspi, 6);
      break;

    case EXTMEM_TYPE_SRAM_SERIAL_ISSI:
      ret = IS62WVS_EnableMemoryMappedMode(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_PSRAM_PARALLEL_FMC:
    case EXTMEM_TYPE_NOR_PARALLEL_FMC:
      ret = EXTMEM_OK; /* FMC is permanently mapped at 0x60000000 */
      break;

    default:
      ret = EXTMEM_NOT_SUPPORTED;
      break;
  }

  if (ret == EXTMEM_OK)
  {
    hextmem->State = EXTMEM_STATE_MEMORY_MAPPED;
  }

  return ret;
}

int32_t ExtMem_DisableMemoryMapped(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (hextmem->State != EXTMEM_STATE_MEMORY_MAPPED) return EXTMEM_OK;

  if (!ExtMem_IsFmcBus(hextmem->Config.Bus))
  {
    if (HAL_XSPI_Abort(&hextmem->hxspi) != HAL_OK)
    {
      return EXTMEM_ERROR;
    }
  }

  hextmem->State = EXTMEM_STATE_INDIRECT;
  return EXTMEM_OK;
}

int32_t ExtMem_Read(ExtMem_HandleTypeDef *hextmem, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  if (hextmem == NULL || pData == NULL || Size == 0) return EXTMEM_INVALID_PARAM;

  /* Boundary check against total device capacity */
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)Address + Size > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }

  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    /* Drop lines cached before an indirect program/erase so the CPU sees the device content */
    ExtMem_CacheCleanInvalidate(hextmem->MemoryMappedBase + Address, Size);
    ExtMem_MappedRead(hextmem->MemoryMappedBase, Address, pData, Size);
    return EXTMEM_OK;
  }

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_Read(&hextmem->hxspi, hextmem->ActiveMode, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_Read(&hextmem->hxspi, hextmem->ActiveMode, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      return MT35XU_Read(&hextmem->hxspi, hextmem->ActiveMode, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      return S26KS512S_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_HYPERRAM_INFINEON:
      return S27KS0641_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_HYPERRAM_ISSI:
      return IS66WVH16M8_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_PSRAM_OCTAL_ISSI:
      return IS66WVO32M8_Read(&hextmem->hxspi, pData, Address, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_ReadQuad(&hextmem->hxspi, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_QUAD_ISSI:
    {
      bool is4B = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
      return IS25LP_ReadQuadEx(&hextmem->hxspi, Address, pData, Size, hextmem->DummyCycles, is4B ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS);
    }

    case EXTMEM_TYPE_NOR_QUAD_MICRON:
    {
      bool is4B = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
      return MT25QU_ReadQuadEx(&hextmem->hxspi, Address, pData, Size, hextmem->DummyCycles, is4B ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS);
    }

    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      return IS66WVS16M8_ReadQuad(&hextmem->hxspi, Address, pData, Size, 6);

    case EXTMEM_TYPE_SRAM_SERIAL_ISSI:
      return IS62WVS_ReadQuad(&hextmem->hxspi, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_PSRAM_PARALLEL_FMC:
      return IS66WV_FMC_Read(hextmem->MemoryMappedBase, Address, pData, Size);

    case EXTMEM_TYPE_NOR_PARALLEL_FMC:
      return IS29GL_FMC_Read(hextmem->MemoryMappedBase, Address, pData, Size);

    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_Write(ExtMem_HandleTypeDef *hextmem, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  if (hextmem == NULL || pData == NULL || Size == 0) return EXTMEM_INVALID_PARAM;

  /* Boundary check against total device capacity */
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)Address + Size > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }

  /* If Memory Mapped, PSRAM and FMC can be written directly */
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    if (!hextmem->Geometry.IsNonVolatile)
    {
      ExtMem_MappedWrite(hextmem->MemoryMappedBase, Address, pData, Size);
      ExtMem_CacheCleanInvalidate(hextmem->MemoryMappedBase + Address, Size);
      return EXTMEM_OK;
    }
    else
    {
      /* Flash cannot be programmed in read-only Memory Mapped mode without returning to indirect */
      if (ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) return EXTMEM_ERROR;
    }
  }

  /* PSRAM / RAM Writes */
  if (!hextmem->Geometry.IsNonVolatile)
  {
    if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
    {
      return S27KS0641_Write(&hextmem->hxspi, Address, pData, Size);
    }
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
    {
      return IS66WVH16M8_Write(&hextmem->hxspi, Address, pData, Size);
    }
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
    {
      return IS66WVO32M8_Write(&hextmem->hxspi, pData, Address, Size, hextmem->DummyCycles);
    }
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI)
    {
      return IS66WVS16M8_WriteQuad(&hextmem->hxspi, Address, pData, Size);
    }
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_SRAM_SERIAL_ISSI)
    {
      return IS62WVS_WriteQuad(&hextmem->hxspi, Address, pData, Size);
    }
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_PARALLEL_FMC)
    {
      return IS66WV_FMC_Write(hextmem->MemoryMappedBase, Address, pData, Size);
    }
  }

  /* Parallel NOR Flash FMC programming */
  if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_PARALLEL_FMC)
  {
    return IS29GL_FMC_ProgramBuffer(hextmem->MemoryMappedBase, Address, pData, Size);
  }

  /* Flash Page Program with page boundary splitting */
  uint32_t pageSize = (hextmem->Geometry.PageSizeBytes > 0) ? hextmem->Geometry.PageSizeBytes : 256;
  uint32_t currAddr = Address;
  uint32_t endAddr  = Address + Size;
  const uint8_t *pCurrData = pData;
  bool is4Byte = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
  uint32_t addrWidth = is4Byte ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS;

  while (currAddr < endAddr)
  {
    uint32_t chunk = pageSize - (currAddr % pageSize);
    if (chunk > (endAddr - currAddr)) chunk = endAddr - currAddr;

    int32_t status = EXTMEM_ERROR;
    switch (hextmem->Geometry.Type)
    {
      case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
        status = S28HS512T_PageProgram(&hextmem->hxspi, hextmem->ActiveMode, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_OCTAL_ISSI:
        status = IS25LX256_PageProgram(&hextmem->hxspi, hextmem->ActiveMode, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_OCTAL_MICRON:
        status = MT35XU_PageProgram(&hextmem->hxspi, hextmem->ActiveMode, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_HYPERFLASH_INFINEON:
      case EXTMEM_TYPE_HYPERFLASH_ISSI:
        status = S26KS512S_ProgramBuffer(&hextmem->hxspi, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_QUAD_INFINEON:
        status = S25HL512T_PageProgramQuad(&hextmem->hxspi, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_QUAD_ISSI:
        status = IS25LP_PageProgramQuadEx(&hextmem->hxspi, currAddr, pCurrData, chunk, addrWidth);
        break;
      case EXTMEM_TYPE_NOR_QUAD_MICRON:
        status = MT25QU_PageProgramQuadEx(&hextmem->hxspi, currAddr, pCurrData, chunk, addrWidth);
        break;
      default:
        return EXTMEM_NOT_SUPPORTED;
    }

    if (status != EXTMEM_OK) return status;

    currAddr  += chunk;
    pCurrData += chunk;
  }

  return EXTMEM_OK;
}

int32_t ExtMem_ReadDMA(ExtMem_HandleTypeDef *hextmem, uint32_t Address, uint8_t *pData, uint32_t Size)
{
  if (hextmem == NULL || pData == NULL || Size == 0) return EXTMEM_INVALID_PARAM;
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)Address + Size > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_Read_DMA(&hextmem->hxspi, Address, pData, Size);
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    return IS66WVO32M8_Read_DMA(&hextmem->hxspi, pData, Address, Size, hextmem->DummyCycles);
  }
  return ExtMem_Read(hextmem, Address, pData, Size);
}

int32_t ExtMem_WriteDMA(ExtMem_HandleTypeDef *hextmem, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  if (hextmem == NULL || pData == NULL || Size == 0) return EXTMEM_INVALID_PARAM;
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)Address + Size > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_Write_DMA(&hextmem->hxspi, Address, pData, Size);
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    return IS66WVO32M8_Write_DMA(&hextmem->hxspi, pData, Address, Size, hextmem->DummyCycles);
  }
  return ExtMem_Write(hextmem, Address, pData, Size);
}

int32_t ExtMem_EraseSector(ExtMem_HandleTypeDef *hextmem, uint32_t SectorAddress)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK; /* RAM needs no erase */

  /* Boundary check against total device capacity */
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)SectorAddress + hextmem->Geometry.SectorSizeBytes > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }

  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED && ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) return EXTMEM_ERROR;

  bool is4Byte = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
  uint32_t addrWidth = is4Byte ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS;

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      /* SEMPER 4 KB erase only applies to the hybrid parameter sectors: the uniform unit is 256 KB */
      return S28HS512T_EraseBlock256K(&hextmem->hxspi, hextmem->ActiveMode, SectorAddress);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_EraseSector4K(&hextmem->hxspi, hextmem->ActiveMode, SectorAddress);
    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      return MT35XU_EraseSector4K(&hextmem->hxspi, hextmem->ActiveMode, SectorAddress);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      return S26KS512S_EraseSector(&hextmem->hxspi, SectorAddress);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      /* SEMPER 4 KB erase only applies to the parameter sectors: use the uniform 256 KB sector */
      if (hextmem->Geometry.SectorSizeBytes > S25HL512T_SECTOR_4K)
      {
        return S25HL512T_EraseBlock(&hextmem->hxspi, SectorAddress);
      }
      return S25HL512T_EraseSector4K(&hextmem->hxspi, SectorAddress);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP_EraseSector4KEx(&hextmem->hxspi, SectorAddress, addrWidth);
    case EXTMEM_TYPE_NOR_QUAD_MICRON:
      return MT25QU_EraseSector4KEx(&hextmem->hxspi, SectorAddress, addrWidth);
    case EXTMEM_TYPE_NOR_PARALLEL_FMC:
      return IS29GL_FMC_EraseSector(hextmem->MemoryMappedBase, SectorAddress);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EraseBlock(ExtMem_HandleTypeDef *hextmem, uint32_t BlockAddress)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK;

  /* Boundary check against total device capacity */
  if (hextmem->Geometry.TotalSizeBytes > 0 &&
      ((uint64_t)BlockAddress + hextmem->Geometry.BlockSizeBytes > (uint64_t)hextmem->Geometry.TotalSizeBytes))
  {
    return EXTMEM_INVALID_PARAM;
  }

  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED && ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) return EXTMEM_ERROR;

  bool is4Byte = (hextmem->Geometry.TotalSizeBytes > (16U * 1024U * 1024U));
  uint32_t addrWidth = is4Byte ? HAL_XSPI_ADDRESS_32_BITS : HAL_XSPI_ADDRESS_24_BITS;

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_EraseBlock256K(&hextmem->hxspi, hextmem->ActiveMode, BlockAddress);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_EraseBlock128K(&hextmem->hxspi, hextmem->ActiveMode, BlockAddress);
    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      return MT35XU_EraseBlock128K(&hextmem->hxspi, hextmem->ActiveMode, BlockAddress);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      return S26KS512S_EraseSector(&hextmem->hxspi, BlockAddress);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_EraseBlock(&hextmem->hxspi, BlockAddress);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP_EraseBlock64KEx(&hextmem->hxspi, BlockAddress, addrWidth);
    case EXTMEM_TYPE_NOR_QUAD_MICRON:
      return MT25QU_EraseBlock64KEx(&hextmem->hxspi, BlockAddress, addrWidth);
    case EXTMEM_TYPE_NOR_PARALLEL_FMC:
      return IS29GL_FMC_EraseSector(hextmem->MemoryMappedBase, BlockAddress);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EraseChip(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED && ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) return EXTMEM_ERROR;

  /* Die count from the database; MT25Q parts found through SFDP use 512 Mbit dice */
  uint32_t dice = 1U;
  if (hextmem->pDevice != NULL)
  {
    dice = (hextmem->pDevice->DieCount > 1U) ? hextmem->pDevice->DieCount : 1U;
  }
  else if (hextmem->Geometry.TotalSizeBytes > MT25Q_DIE_SIZE)
  {
    dice = hextmem->Geometry.TotalSizeBytes / MT25Q_DIE_SIZE;
  }

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_ChipErase(&hextmem->hxspi, hextmem->ActiveMode);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_ChipErase(&hextmem->hxspi, hextmem->ActiveMode);
    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      if (dice > 1U)
      {
        /* Stacked dice reject bulk erase: erase each die at its base address */
        for (uint32_t die = 0; die < dice; die++)
        {
          int32_t st = MT35XU_EraseDie(&hextmem->hxspi, hextmem->ActiveMode, die * (hextmem->Geometry.TotalSizeBytes / dice));
          if (st != MT35XU_OK) return st;
        }
        return EXTMEM_OK;
      }
      return MT35XU_EraseChip(&hextmem->hxspi, hextmem->ActiveMode);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      return S26KS512S_EraseChip(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_ChipErase(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_ChipErase(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_QUAD_MICRON:
      if (dice > 1U)
      {
        /* Stacked dice reject bulk erase: erase each die at its base address (4-byte mode) */
        for (uint32_t die = 0; die < dice; die++)
        {
          int32_t st = MT25QU_EraseDie(&hextmem->hxspi, die * (hextmem->Geometry.TotalSizeBytes / dice));
          if (st != MT25Q_OK) return st;
        }
        return EXTMEM_OK;
      }
      return MT25QU_EraseChip(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_PARALLEL_FMC:
      return IS29GL_FMC_EraseChip(hextmem->MemoryMappedBase);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EnterDeepPowerDown(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_EnterDeepPowerDown(&hextmem->hxspi);
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
  {
    return IS66WVH16M8_EnterDeepPowerDown(&hextmem->hxspi);
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    return IS66WVO32M8_EnterDeepPowerDown(&hextmem->hxspi);
  }
  return EXTMEM_NOT_SUPPORTED;
}

int32_t ExtMem_LeaveDeepPowerDown(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_LeaveDeepPowerDown(&hextmem->hxspi);
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
  {
    return IS66WVH16M8_LeaveDeepPowerDown(&hextmem->hxspi);
  }
  if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    return IS66WVO32M8_LeaveDeepPowerDown(&hextmem->hxspi);
  }
  return EXTMEM_NOT_SUPPORTED;
}

int32_t ExtMem_Reset(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (ExtMem_IsFmcBus(hextmem->Config.Bus))
  {
    if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_PARALLEL_FMC)
    {
      return IS29GL_FMC_Reset(hextmem->MemoryMappedBase);
    }
    return EXTMEM_OK;
  }

  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    if (ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) return EXTMEM_ERROR;
  }

  int32_t ret;
  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
      /* The 1S-1S-1S reset is not decoded in 8D-8D-8D: return to SPI first */
      if (hextmem->ActiveMode == EXTMEM_MODE_OCTAL_DTR)
      {
        if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)      ret = S28HS512T_ExitOctalDTRMode(&hextmem->hxspi);
        else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_ISSI)   ret = IS25LX256_ExitOctalDTRMode(&hextmem->hxspi);
        else                                                             ret = MT35XU_ExitOctalMode(&hextmem->hxspi);
        if (ret != EXTMEM_OK) return ret;
        hextmem->ActiveMode = EXTMEM_MODE_SPI;
      }
      if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)        ret = S28HS512T_Reset(&hextmem->hxspi);
      else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_ISSI)     ret = IS25LX256_Reset(&hextmem->hxspi);
      else                                                               ret = MT35XU_Reset(&hextmem->hxspi);
      return ret;

    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_Reset(&hextmem->hxspi);

    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_Reset(&hextmem->hxspi);

    case EXTMEM_TYPE_NOR_QUAD_MICRON:
      return MT25QU_Reset(&hextmem->hxspi);

    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
    case EXTMEM_TYPE_HYPERFLASH_ISSI:
      return S26KS512S_Reset(&hextmem->hxspi);


    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      return IS66WVS16M8_Reset(&hextmem->hxspi);

    case EXTMEM_TYPE_SRAM_SERIAL_ISSI:
      /* Leaving SQI returns the device to SPI: restore the SQI mode used by this driver */
      if (IS62WVS_Reset(&hextmem->hxspi) != IS62WVS_OK) return EXTMEM_ERROR;
      if (IS62WVS_WriteModeRegister(&hextmem->hxspi, IS62WVS_MODE_SEQUENTIAL) != IS62WVS_OK) return EXTMEM_ERROR;
      return IS62WVS_EnterQuadMode(&hextmem->hxspi);

    default:
      /* HyperRAM and OctalRAM have no software reset command (hardware RESET# only) */
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_DeInit(ExtMem_HandleTypeDef *hextmem)
{
  int32_t ret = EXTMEM_OK;

  if (hextmem == NULL) return EXTMEM_INVALID_PARAM;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    if (ExtMem_DisableMemoryMapped(hextmem) != EXTMEM_OK) ret = EXTMEM_ERROR;
  }
  if (!ExtMem_IsFmcBus(hextmem->Config.Bus))
  {
    (void)S28HS512T_SetDieLayout(&hextmem->hxspi, NULL); /* Releases a stacked-die registration */
    if (HAL_XSPI_DeInit(&hextmem->hxspi) != HAL_OK) ret = EXTMEM_ERROR;
  }
  else
  {
    if (HAL_SRAM_DeInit(&hextmem->hsram) != HAL_OK) ret = EXTMEM_ERROR;
  }
  hextmem->State = EXTMEM_STATE_UNINITIALIZED;
  return ret;
}

const char* ExtMem_GetDeviceName(const ExtMem_HandleTypeDef *hextmem)
{
  return hextmem ? hextmem->Geometry.DeviceName : "UNKNOWN";
}

uint32_t ExtMem_GetCapacity(const ExtMem_HandleTypeDef *hextmem)
{
  return hextmem ? hextmem->Geometry.TotalSizeBytes : 0;
}

bool ExtMem_IsFlash(const ExtMem_HandleTypeDef *hextmem)
{
  return hextmem ? hextmem->Geometry.IsNonVolatile : false;
}

bool ExtMem_IsRAM(const ExtMem_HandleTypeDef *hextmem)
{
  return hextmem ? !hextmem->Geometry.IsNonVolatile : false;
}
