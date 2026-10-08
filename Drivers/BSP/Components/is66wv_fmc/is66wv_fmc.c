/**
  ******************************************************************************
  * @file    is66wv_fmc.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for Parallel Asynchronous PSRAM/SRAM via FMC.
  ******************************************************************************
  */

#include "is66wv_fmc.h"
#include <string.h>

int32_t IS66WV_FMC_Init(SRAM_HandleTypeDef *hsram, uint32_t Bank, const IS66WV_FMC_Timing_t *pTiming)
{
  FMC_NORSRAM_TimingTypeDef Timing = {0};

  hsram->Instance  = FMC_NORSRAM_DEVICE;
  hsram->Extended  = FMC_NORSRAM_EXTENDED_DEVICE;

  hsram->Init.NSBank             = Bank;
  hsram->Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram->Init.MemoryType         = FMC_MEMORY_TYPE_PSRAM;
  hsram->Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram->Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram->Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram->Init.WaitTiming         = FMC_WAIT_TIMING_BEFORE_WS;
  hsram->Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
  hsram->Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;
  hsram->Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
  hsram->Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram->Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
  hsram->Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram->Init.PageSize           = FMC_PAGE_SIZE_NONE;

  /* Default conservative timings if not provided */
  if (pTiming != NULL)
  {
    Timing.AddressSetupTime      = pTiming->AddressSetupTime;
    Timing.AddressHoldTime       = pTiming->AddressHoldTime;
    Timing.DataSetupTime         = pTiming->DataSetupTime;
    Timing.BusTurnAroundDuration = pTiming->BusTurnAroundDuration;
  }
  else
  {
    Timing.AddressSetupTime      = 3;
    Timing.AddressHoldTime       = 1;
    Timing.DataSetupTime         = 5;
    Timing.BusTurnAroundDuration = 1;
  }

  Timing.CLKDivision             = 2;
  Timing.DataLatency             = 2;
  Timing.AccessMode              = FMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(hsram, &Timing, &Timing) != HAL_OK)
  {
    return IS66WV_FMC_ERROR;
  }

  return IS66WV_FMC_OK;
}

/* CPU accesses to the FMC window. Host unit tests route them to the emulated array. */
static void IS66WV_FMC_CopyIn(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)BaseAddr;
  MockHAL_RamRead(Offset, pData, Size);
#else
  const volatile uint8_t *pSrc = (const volatile uint8_t *)(uintptr_t)(BaseAddr + Offset);
  for (uint32_t i = 0; i < Size; i++)
  {
    pData[i] = pSrc[i];
  }
#endif
}

static void IS66WV_FMC_CopyOut(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)BaseAddr;
  MockHAL_RamWrite(Offset, pData, Size);
#else
  volatile uint8_t *pDst = (volatile uint8_t *)(uintptr_t)(BaseAddr + Offset);
  for (uint32_t i = 0; i < Size; i++)
  {
    pDst[i] = pData[i];
  }
#endif
}

int32_t IS66WV_FMC_Read(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size)
{
  if (pData == NULL) return IS66WV_FMC_ERROR;
  IS66WV_FMC_CopyIn(BaseAddr, Offset, pData, Size);
  return IS66WV_FMC_OK;
}

int32_t IS66WV_FMC_Write(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size)
{
  if (pData == NULL) return IS66WV_FMC_ERROR;
  IS66WV_FMC_CopyOut(BaseAddr, Offset, pData, Size);
  return IS66WV_FMC_OK;
}

int32_t IS66WV_FMC_TestPattern(uint32_t BaseAddr, uint32_t TestSizeBytes)
{
  uint32_t words = TestSizeBytes / 4U;

  /* Write an address dependent pattern so stuck and shorted address lines are caught too */
  for (uint32_t i = 0; i < words; i++)
  {
    uint32_t v = 0xAA550000U ^ i;
    uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
    IS66WV_FMC_CopyOut(BaseAddr, i * 4U, b, 4U);
  }

  /* Verify test pattern */
  for (uint32_t i = 0; i < words; i++)
  {
    uint8_t b[4];
    IS66WV_FMC_CopyIn(BaseAddr, i * 4U, b, 4U);
    uint32_t v = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    if (v != (0xAA550000U ^ i))
    {
      return IS66WV_FMC_ERROR;
    }
  }

  return IS66WV_FMC_OK;
}
