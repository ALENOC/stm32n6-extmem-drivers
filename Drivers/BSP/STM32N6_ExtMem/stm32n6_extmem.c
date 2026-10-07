/**
  ******************************************************************************
  * @file    stm32n6_extmem.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Unified High-Level External Memory Driver Suite implementation for STM32N6.
  ******************************************************************************
  */

#include "stm32n6_extmem.h"
#include <string.h>

/* Private Hardware MSP Helper */
static void ExtMem_MspInit_XSPI(XSPI_HandleTypeDef *hxspi, bool use1V8)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 1. Enable Power and configure VDDIO domain */
#if defined(__HAL_RCC_PWR_CLK_ENABLE)
  __HAL_RCC_PWR_CLK_ENABLE();
#endif

  if (use1V8)
  {
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_1V8);
  }
  else
  {
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_ConfigVddIORange(PWR_VDDIO2, PWR_VDDIO_RANGE_3V3);
  }

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
  __HAL_RCC_XSPIM_FORCE_RESET();
  __HAL_RCC_XSPIM_RELEASE_RESET();

  /* 3. Common High-Speed Pin Config */
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P1; /* Default XSPI1 Port 1 AF */
  (void)GPIO_InitStruct;

  /* Weak MSP hook allows project override in Core/Src/stm32n6xx_hal_msp.c */
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
    hextmem->MemoryMappedBase = EXTMEM_FMC_BANK1_BASE_ADDR;
    hextmem->ActiveMode       = EXTMEM_MODE_PARALLEL_16BIT;
    hextmem->Geometry.Type    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC;
    hextmem->Geometry.IsNonVolatile = false;
    hextmem->Geometry.SupportsMemoryMapped = true;
    hextmem->Geometry.TotalSizeBytes = 4 * 1024 * 1024;
    strncpy(hextmem->Geometry.DeviceName, "IS66WV_Parallel_FMC", sizeof(hextmem->Geometry.DeviceName) - 1);

    IS66WV_FMC_Timing_t timing = { .AddressSetupTime = 3, .AddressHoldTime = 1, .DataSetupTime = 5, .BusTurnAroundDuration = 1 };
    if (IS66WV_FMC_Init(&hextmem->hsram, FMC_NORSRAM_BANK1, &timing) != IS66WV_FMC_OK)
    {
      return EXTMEM_ERROR;
    }
    hextmem->State = EXTMEM_STATE_MEMORY_MAPPED;
    return EXTMEM_OK;
  }

  /* Hardware MSP Initialization */
  ExtMem_MspInit_XSPI(&hextmem->hxspi, hextmem->Config.Force1V8);

  /* Set Clock Prescaler */
  uint32_t prescaler = (hextmem->Config.ClockPrescaler > 0) ? hextmem->Config.ClockPrescaler : EXTMEM_DEFAULT_CLOCK_PRESCALER;

  /* Basic XSPI Init in Single SPI mode */
  hextmem->hxspi.Init.FifoThresholdByte       = 4;
  hextmem->hxspi.Init.MemoryType              = HAL_XSPI_MEMTYPE_MICRON;
  hextmem->hxspi.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hextmem->hxspi.Init.MemorySize              = HAL_XSPI_SIZE_64MB;
  hextmem->hxspi.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  hextmem->hxspi.Init.ChipSelectHighTimeCycle = 4;
  hextmem->hxspi.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hextmem->hxspi.Init.ClockPrescaler          = prescaler;
  hextmem->hxspi.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hextmem->hxspi.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_ENABLE;
  hextmem->hxspi.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_NONE;
  hextmem->hxspi.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  hextmem->hxspi.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;

  if (HAL_XSPI_Init(&hextmem->hxspi) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Run Auto-Detection or use forced configuration */
  if (ExtMem_AutoDetect(hextmem) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Switch memory to its high performance mode */
  if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_SEMPER)
  {
    hextmem->DummyCycles = hextmem->pDevice ? hextmem->pDevice->DefaultReadDummyCycles : 20;
    if (S28HS512T_EnterOctalDTRMode(&hextmem->hxspi, hextmem->DummyCycles) == S28HS512T_OK)
    {
      hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
    }
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_OCTAL_ISSI)
  {
    hextmem->DummyCycles = hextmem->pDevice ? hextmem->pDevice->DefaultReadDummyCycles : 20;
    if (IS25LX256_EnterOctalDTRMode(&hextmem->hxspi, hextmem->DummyCycles) == IS25LX_OK)
    {
      hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
    }
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_OCTAL_ISSI)
  {
    hextmem->DummyCycles = hextmem->pDevice ? hextmem->pDevice->DefaultReadDummyCycles : 5;
    IS66WVO32M8_Init(&hextmem->hxspi, prescaler, HAL_XSPI_SIZE_32MB);
    hextmem->ActiveMode = EXTMEM_MODE_OCTAL_DTR;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    S27KS0641_Init(&hextmem->hxspi, prescaler, HAL_XSPI_SIZE_32MB);
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_ISSI)
  {
    IS66WVH16M8_Init(&hextmem->hxspi, prescaler, HAL_XSPI_SIZE_16MB);
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERFLASH_INFINEON)
  {
    S26KS512S_Init(&hextmem->hxspi, prescaler);
    hextmem->ActiveMode = EXTMEM_MODE_HYPERBUS;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_QUAD_INFINEON)
  {
    S25HL512T_EnableQuadMode(&hextmem->hxspi);
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_NOR_QUAD_ISSI)
  {
    IS25LP256_EnableQuadMode(&hextmem->hxspi);
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }
  else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_QUAD_ISSI)
  {
    IS66WVS16M8_EnterQuadMode(&hextmem->hxspi);
    hextmem->ActiveMode = EXTMEM_MODE_QUAD_1_4_4;
  }

  hextmem->State = EXTMEM_STATE_INDIRECT;
  return EXTMEM_OK;
}

int32_t ExtMem_AutoDetect(ExtMem_HandleTypeDef *hextmem)
{
  uint8_t id[3] = {0};

  /* 1. Try Standard SPI 0x9F Read ID */
  if (S28HS512T_ReadID(&hextmem->hxspi, id) == S28HS512T_OK)
  {
    memcpy(hextmem->RawID, id, 3);
    const ExtMem_DeviceDescriptor_t *dev = ExtMem_FindDevice(id[0], id[1], id[2]);

    if (dev != NULL)
    {
      hextmem->pDevice = dev;
      strncpy(hextmem->Geometry.DeviceName, dev->PartNumber, sizeof(hextmem->Geometry.DeviceName) - 1);
      hextmem->Geometry.Type            = dev->Type;
      hextmem->Geometry.TotalSizeBytes  = dev->CapacityBytes;
      hextmem->Geometry.PageSizeBytes   = dev->PageSizeBytes;
      hextmem->Geometry.SectorSizeBytes = dev->SectorSizeBytes;
      hextmem->Geometry.BlockSizeBytes  = dev->BlockSizeBytes;
      hextmem->Geometry.MaxClockFreqMHz = dev->MaxClockFreqMHz;
      hextmem->Geometry.SupportsDTR     = (dev->PreferredMode == EXTMEM_MODE_OCTAL_DTR || dev->PreferredMode == EXTMEM_MODE_HYPERBUS);
      hextmem->Geometry.SupportsMemoryMapped = true;
      hextmem->Geometry.IsNonVolatile   = (dev->PageSizeBytes > 0);
      return EXTMEM_OK;
    }
  }

  /* 2. Try SFDP (JESD216) Probe */
#if (EXTMEM_ENABLE_SFDP_PROBE == 1)
  SFDP_FlashParams_t sfdpParams;
  if (SFDP_ReadAndParse(&hextmem->hxspi, &sfdpParams) == EXTMEM_OK)
  {
    strncpy(hextmem->Geometry.DeviceName, "JEDEC_SFDP_Flash", sizeof(hextmem->Geometry.DeviceName) - 1);
    hextmem->Geometry.Type            = (sfdpParams.SupportsOctal_8D_8D_8D) ? EXTMEM_TYPE_NOR_OCTAL_SEMPER : EXTMEM_TYPE_NOR_QUAD_ISSI;
    hextmem->Geometry.TotalSizeBytes  = sfdpParams.DensityBytes;
    hextmem->Geometry.PageSizeBytes   = sfdpParams.PageSizeBytes;
    hextmem->Geometry.SectorSizeBytes = 4096;
    hextmem->Geometry.BlockSizeBytes  = sfdpParams.SectorSizeBytes;
    hextmem->Geometry.SupportsDTR     = sfdpParams.SupportsOctal_8D_8D_8D;
    hextmem->Geometry.SupportsMemoryMapped = true;
    hextmem->Geometry.IsNonVolatile   = true;
    return EXTMEM_OK;
  }
#endif

  /* 3. Try HyperBus Register Space Probe (HyperRAM / HyperFlash) */
  XSPI_HyperbusCfgTypeDef sHyperCfg = {
    .RWRecoveryTimeCycle = 4,
    .AccessTimeCycle     = 6,
    .WriteZeroLatency    = HAL_XSPI_LATENCY_ON_WRITE,
    .LatencyMode         = HAL_XSPI_VARIABLE_LATENCY
  };

  hextmem->hxspi.Init.MemoryType = HAL_XSPI_MEMTYPE_HYPERBUS;
  HAL_XSPI_Init(&hextmem->hxspi);
  HAL_XSPI_HyperbusCfg(&hextmem->hxspi, &sHyperCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);

  uint16_t hyperId0 = 0;
  if (S27KS0641_ReadRegister(&hextmem->hxspi, S27KS_REG_ID0, &hyperId0) == S27KS_OK && hyperId0 != 0x0000 && hyperId0 != 0xFFFF)
  {
    uint8_t mfg = (uint8_t)(hyperId0 & 0x0F);
    if (mfg == 0x01) /* Cypress / Infineon */
    {
      hextmem->pDevice = ExtMem_FindDevice(0x01, 0x00, 0x01);
      strncpy(hextmem->Geometry.DeviceName, "S27KS_HyperRAM", sizeof(hextmem->Geometry.DeviceName) - 1);
      hextmem->Geometry.Type = EXTMEM_TYPE_HYPERRAM_INFINEON;
    }
    else /* ISSI */
    {
      hextmem->pDevice = ExtMem_FindDevice(0x0F, 0x00, 0x02);
      strncpy(hextmem->Geometry.DeviceName, "IS66WVH_HyperRAM", sizeof(hextmem->Geometry.DeviceName) - 1);
      hextmem->Geometry.Type = EXTMEM_TYPE_HYPERRAM_ISSI;
    }
    hextmem->Geometry.TotalSizeBytes  = 16 * 1024 * 1024;
    hextmem->Geometry.SupportsDTR     = true;
    hextmem->Geometry.SupportsMemoryMapped = true;
    hextmem->Geometry.IsNonVolatile   = false;
    return EXTMEM_OK;
  }

  /* Default fallback to SEMPER 512Mb if auto probe unconfirmed */
  hextmem->pDevice = &ExtMem_DeviceDatabase[0];
  strncpy(hextmem->Geometry.DeviceName, hextmem->pDevice->PartNumber, sizeof(hextmem->Geometry.DeviceName) - 1);
  hextmem->Geometry.Type = hextmem->pDevice->Type;
  hextmem->Geometry.TotalSizeBytes = hextmem->pDevice->CapacityBytes;
  hextmem->Geometry.PageSizeBytes  = hextmem->pDevice->PageSizeBytes;
  hextmem->Geometry.SectorSizeBytes = hextmem->pDevice->SectorSizeBytes;
  hextmem->Geometry.BlockSizeBytes  = hextmem->pDevice->BlockSizeBytes;
  hextmem->Geometry.SupportsDTR = true;
  hextmem->Geometry.SupportsMemoryMapped = true;
  hextmem->Geometry.IsNonVolatile = true;

  return EXTMEM_OK;
}

int32_t ExtMem_EnableMemoryMapped(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem == NULL || hextmem->State == EXTMEM_STATE_UNINITIALIZED) return EXTMEM_INVALID_PARAM;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED) return EXTMEM_OK;

  int32_t ret = EXTMEM_ERROR;

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      ret = S28HS512T_EnableMemoryMappedModeDTR(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      ret = IS25LX256_EnableMemoryMappedModeDTR(&hextmem->hxspi, hextmem->DummyCycles);
      break;

    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
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
      ret = S25HL512T_EnableMemoryMappedMode(&hextmem->hxspi, 6);
      break;

    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      ret = IS25LP256_EnableMemoryMappedMode(&hextmem->hxspi, 6);
      break;

    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      ret = IS66WVS16M8_EnableMemoryMappedMode(&hextmem->hxspi, 6);
      break;

    case EXTMEM_TYPE_PSRAM_PARALLEL_FMC:
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

  if (hextmem->Config.Bus != EXTMEM_BUS_FMC_SRAM_BANK1_1)
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

  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    volatile uint8_t *pSrc = (volatile uint8_t *)(uintptr_t)(hextmem->MemoryMappedBase + Address);
    memcpy(pData, (const void *)pSrc, Size);
    return EXTMEM_OK;
  }

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_Read(&hextmem->hxspi, hextmem->ActiveMode, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_Read(&hextmem->hxspi, hextmem->ActiveMode, Address, pData, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
      return S26KS512S_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_HYPERRAM_INFINEON:
      return S27KS0641_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_HYPERRAM_ISSI:
      return IS66WVH16M8_Read(&hextmem->hxspi, Address, pData, Size);

    case EXTMEM_TYPE_PSRAM_OCTAL_ISSI:
      return IS66WVO32M8_Read(&hextmem->hxspi, pData, Address, Size, hextmem->DummyCycles);

    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_ReadQuad(&hextmem->hxspi, Address, pData, Size, 6);

    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_ReadQuad(&hextmem->hxspi, Address, pData, Size, 6);

    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      return IS66WVS16M8_ReadQuad(&hextmem->hxspi, Address, pData, Size, 6);

    case EXTMEM_TYPE_PSRAM_PARALLEL_FMC:
      return IS66WV_FMC_Read(hextmem->MemoryMappedBase, Address, pData, Size);

    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_Write(ExtMem_HandleTypeDef *hextmem, uint32_t Address, const uint8_t *pData, uint32_t Size)
{
  if (hextmem == NULL || pData == NULL || Size == 0) return EXTMEM_INVALID_PARAM;

  /* If Memory Mapped, PSRAM and FMC can be written directly */
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    if (!hextmem->Geometry.IsNonVolatile)
    {
      volatile uint8_t *pDst = (volatile uint8_t *)(uintptr_t)(hextmem->MemoryMappedBase + Address);
      memcpy((void *)pDst, pData, Size);
      ExtMem_CacheCleanInvalidate((uint32_t)(uintptr_t)pDst, Size);
      return EXTMEM_OK;
    }
    else
    {
      /* Flash cannot be programmed in read-only Memory Mapped mode without returning to indirect */
      ExtMem_DisableMemoryMapped(hextmem);
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
    else if (hextmem->Geometry.Type == EXTMEM_TYPE_PSRAM_PARALLEL_FMC)
    {
      return IS66WV_FMC_Write(hextmem->MemoryMappedBase, Address, pData, Size);
    }
  }

  /* Flash Page Program with page boundary splitting */
  uint32_t pageSize = (hextmem->Geometry.PageSizeBytes > 0) ? hextmem->Geometry.PageSizeBytes : 256;
  uint32_t currAddr = Address;
  uint32_t endAddr  = Address + Size;
  const uint8_t *pCurrData = pData;

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
      case EXTMEM_TYPE_HYPERFLASH_INFINEON:
        status = S26KS512S_ProgramBuffer(&hextmem->hxspi, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_QUAD_INFINEON:
        status = S25HL512T_PageProgramQuad(&hextmem->hxspi, currAddr, pCurrData, chunk);
        break;
      case EXTMEM_TYPE_NOR_QUAD_ISSI:
        status = IS25LP256_PageProgramQuad(&hextmem->hxspi, currAddr, pCurrData, chunk);
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
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK; /* RAM needs no erase */
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED) ExtMem_DisableMemoryMapped(hextmem);

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_EraseSector4K(&hextmem->hxspi, hextmem->ActiveMode, SectorAddress);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_EraseSector4K(&hextmem->hxspi, hextmem->ActiveMode, SectorAddress);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
      return S26KS512S_EraseSector(&hextmem->hxspi, SectorAddress);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_EraseSector4K(&hextmem->hxspi, SectorAddress);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_EraseSector4K(&hextmem->hxspi, SectorAddress);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EraseBlock(ExtMem_HandleTypeDef *hextmem, uint32_t BlockAddress)
{
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED) ExtMem_DisableMemoryMapped(hextmem);

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_EraseBlock256K(&hextmem->hxspi, hextmem->ActiveMode, BlockAddress);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_EraseBlock64K(&hextmem->hxspi, hextmem->ActiveMode, BlockAddress);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
      return S26KS512S_EraseSector(&hextmem->hxspi, BlockAddress);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_EraseBlock64K(&hextmem->hxspi, BlockAddress);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_EraseBlock64K(&hextmem->hxspi, BlockAddress);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EraseChip(ExtMem_HandleTypeDef *hextmem)
{
  if (!hextmem->Geometry.IsNonVolatile) return EXTMEM_OK;
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED) ExtMem_DisableMemoryMapped(hextmem);

  switch (hextmem->Geometry.Type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
      return S28HS512T_ChipErase(&hextmem->hxspi, hextmem->ActiveMode);
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
      return IS25LX256_ChipErase(&hextmem->hxspi, hextmem->ActiveMode);
    case EXTMEM_TYPE_HYPERFLASH_INFINEON:
      return S26KS512S_EraseChip(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
      return S25HL512T_ChipErase(&hextmem->hxspi);
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
      return IS25LP256_ChipErase(&hextmem->hxspi);
    default:
      return EXTMEM_NOT_SUPPORTED;
  }
}

int32_t ExtMem_EnterDeepPowerDown(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_EnterDeepPowerDown(&hextmem->hxspi);
  }
  return EXTMEM_OK;
}

int32_t ExtMem_LeaveDeepPowerDown(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem->Geometry.Type == EXTMEM_TYPE_HYPERRAM_INFINEON)
  {
    return S27KS0641_LeaveDeepPowerDown(&hextmem->hxspi);
  }
  return EXTMEM_OK;
}

int32_t ExtMem_Reset(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem->Config.Bus != EXTMEM_BUS_FMC_SRAM_BANK1_1)
  {
    return S28HS512T_Reset(&hextmem->hxspi);
  }
  return EXTMEM_OK;
}

int32_t ExtMem_DeInit(ExtMem_HandleTypeDef *hextmem)
{
  if (hextmem->State == EXTMEM_STATE_MEMORY_MAPPED)
  {
    ExtMem_DisableMemoryMapped(hextmem);
  }
  if (hextmem->Config.Bus != EXTMEM_BUS_FMC_SRAM_BANK1_1)
  {
    HAL_XSPI_DeInit(&hextmem->hxspi);
  }
  else
  {
    HAL_SRAM_DeInit(&hextmem->hsram);
  }
  hextmem->State = EXTMEM_STATE_UNINITIALIZED;
  return EXTMEM_OK;
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
