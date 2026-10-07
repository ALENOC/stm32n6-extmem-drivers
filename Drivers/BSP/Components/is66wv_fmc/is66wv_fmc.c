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

int32_t IS66WV_FMC_Read(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)BaseAddr;
  uint8_t *pSrc = MockHAL_GetMemoryBuffer() + (Offset % MockHAL_GetMemoryBufferSize());
  memcpy(pData, (const void *)pSrc, Size);
  return IS66WV_FMC_OK;
#else
  volatile uint8_t *pSrc = (volatile uint8_t *)(uintptr_t)(BaseAddr + Offset);
  memcpy(pData, (const void *)pSrc, Size);
  return IS66WV_FMC_OK;
#endif
}

int32_t IS66WV_FMC_Write(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size)
{
#ifdef EXTMEM_UNIT_TEST
  (void)BaseAddr;
  uint8_t *pDst = MockHAL_GetMemoryBuffer() + (Offset % MockHAL_GetMemoryBufferSize());
  memcpy((void *)pDst, pData, Size);
  return IS66WV_FMC_OK;
#else
  volatile uint8_t *pDst = (volatile uint8_t *)(uintptr_t)(BaseAddr + Offset);
  memcpy((void *)pDst, pData, Size);
  return IS66WV_FMC_OK;
#endif
}

int32_t IS66WV_FMC_TestPattern(uint32_t BaseAddr, uint32_t TestSizeBytes)
{
#ifdef EXTMEM_UNIT_TEST
  (void)BaseAddr;
  volatile uint32_t *pMem = (volatile uint32_t *)(void *)MockHAL_GetMemoryBuffer();
#else
  volatile uint32_t *pMem = (volatile uint32_t *)(uintptr_t)BaseAddr;
#endif
  uint32_t words = TestSizeBytes / 4;

  /* Write test pattern */
  for (uint32_t i = 0; i < words; i++)
  {
    pMem[i] = (uint32_t)(0xAA550000U ^ i);
  }

  /* Verify test pattern */
  for (uint32_t i = 0; i < words; i++)
  {
    if (pMem[i] != (uint32_t)(0xAA550000U ^ i))
    {
      return IS66WV_FMC_ERROR;
    }
  }

  return IS66WV_FMC_OK;
}
