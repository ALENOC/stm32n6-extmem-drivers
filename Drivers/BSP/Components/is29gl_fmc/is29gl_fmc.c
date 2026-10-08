/**
  ******************************************************************************
  * @file    is29gl_fmc.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver implementation for ISSI IS29GL Parallel NOR Flash via FMC.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Alessandro Nocentini (ALENOC) & Community Contributors.
  * All rights reserved.
  *
  ******************************************************************************
  */

#include "is29gl_fmc.h"
#include <string.h>

#ifdef EXTMEM_UNIT_TEST
#include "mock_hal.h"
#endif

/* Low-level 16-bit word access helpers */
static inline void FMC_WriteWord(uint32_t BaseAddr, uint32_t WordOffset, uint16_t Data)
{
#ifdef EXTMEM_UNIT_TEST
  MockHAL_FmcWrite16(BaseAddr, WordOffset * 2, Data);
#else
  volatile uint16_t *pDst = (volatile uint16_t *)(uintptr_t)(BaseAddr + (WordOffset * 2));
  *pDst = Data;
#endif
}

static inline uint16_t FMC_ReadWord(uint32_t BaseAddr, uint32_t WordOffset)
{
#ifdef EXTMEM_UNIT_TEST
  return MockHAL_FmcRead16(BaseAddr, WordOffset * 2);
#else
  volatile uint16_t *pSrc = (volatile uint16_t *)(uintptr_t)(BaseAddr + (WordOffset * 2));
  return *pSrc;
#endif
}

int32_t IS29GL_FMC_Init(SRAM_HandleTypeDef *hsram, uint32_t Bank, const IS29GL_FMC_Timing_t *pTiming)
{
  FMC_NORSRAM_TimingTypeDef Timing = {0};

  hsram->Instance = FMC_NORSRAM_DEVICE;
  hsram->Extended = FMC_NORSRAM_EXTENDED_DEVICE;

  hsram->Init.NSBank             = Bank;
  hsram->Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram->Init.MemoryType         = FMC_MEMORY_TYPE_NOR;
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

  if (pTiming != NULL)
  {
    Timing.AddressSetupTime      = pTiming->AddressSetupTime;
    Timing.AddressHoldTime       = pTiming->AddressHoldTime;
    Timing.DataSetupTime         = pTiming->DataSetupTime;
    Timing.BusTurnAroundDuration = pTiming->BusTurnAroundDuration;
  }
  else
  {
    Timing.AddressSetupTime      = 4;
    Timing.AddressHoldTime       = 2;
    Timing.DataSetupTime         = 7;
    Timing.BusTurnAroundDuration = 2;
  }

  Timing.CLKDivision             = 2;
  Timing.DataLatency             = 2;
  Timing.AccessMode              = FMC_ACCESS_MODE_B; /* Mode B for NOR Flash */

  if (HAL_SRAM_Init(hsram, &Timing, &Timing) != HAL_OK)
  {
    return IS29GL_FMC_ERROR;
  }

  return IS29GL_FMC_OK;
}

int32_t IS29GL_FMC_Reset(uint32_t BaseAddr)
{
  FMC_WriteWord(BaseAddr, 0x0000, IS29GL_CMD_RESET);
  HAL_Delay(1);
  return IS29GL_FMC_OK;
}

int32_t IS29GL_FMC_ReadID(uint32_t BaseAddr, uint16_t *pMfgId, uint16_t *pDevId)
{
  if (pMfgId == NULL || pDevId == NULL) return IS29GL_FMC_ERROR;

  /* Autoselect Command Sequence */
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_AUTOSELECT);

  *pMfgId = FMC_ReadWord(BaseAddr, 0x0000);
  *pDevId = FMC_ReadWord(BaseAddr, 0x0001);

  /* Return to Read Array Mode */
  return IS29GL_FMC_Reset(BaseAddr);
}

int32_t IS29GL_FMC_Read(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size)
{
  if (pData == NULL) return IS29GL_FMC_ERROR;
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
  return IS29GL_FMC_OK;
}

/* Embedded algorithm completion: DQ7 equals the expected value, DQ5 set means the operation timed out */
static int32_t IS29GL_FMC_PollDQ7(uint32_t BaseAddr, uint32_t WordOffset, uint16_t Expected, uint32_t TimeoutMs, bool Sleep)
{
  uint32_t tickstart = HAL_GetTick();

  do
  {
    uint16_t status = FMC_ReadWord(BaseAddr, WordOffset);
    if ((status & IS29GL_SR_DQ7_POLL) == (Expected & IS29GL_SR_DQ7_POLL))
    {
      return IS29GL_FMC_OK;
    }
    if (status & IS29GL_SR_DQ5_EXCEEDED)
    {
      /* DQ7 must be read again after DQ5 goes high */
      status = FMC_ReadWord(BaseAddr, WordOffset);
      if ((status & IS29GL_SR_DQ7_POLL) == (Expected & IS29GL_SR_DQ7_POLL))
      {
        return IS29GL_FMC_OK;
      }
      (void)IS29GL_FMC_Reset(BaseAddr);
      return IS29GL_FMC_ERROR;
    }
    if (Sleep)
    {
      HAL_Delay(1);
    }
  } while ((HAL_GetTick() - tickstart) < TimeoutMs);

  (void)IS29GL_FMC_Reset(BaseAddr);
  return IS29GL_FMC_TIMEOUT;
}

int32_t IS29GL_FMC_ProgramWord(uint32_t BaseAddr, uint32_t Offset, uint16_t Data)
{
  uint32_t wordOffset = Offset / 2;

  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_PROGRAM);
  FMC_WriteWord(BaseAddr, wordOffset, Data);

  return IS29GL_FMC_PollDQ7(BaseAddr, wordOffset, Data, 20, false);
}

int32_t IS29GL_FMC_ProgramBuffer(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size)
{
  uint32_t idx = 0;

  while (idx < Size)
  {
    uint32_t addr = Offset + idx;
    /* Programming 0xFF leaves a NOR byte unchanged: pad unaligned head and odd tail.
     * The FMC maps the byte at the even address to D[7:0]. */
    uint8_t lo = 0xFF, hi = 0xFF;

    if ((addr & 1U) == 0U)
    {
      lo = pData[idx++];
      if (idx < Size)
      {
        hi = pData[idx++];
      }
    }
    else
    {
      hi = pData[idx++];
    }

    int32_t ret = IS29GL_FMC_ProgramWord(BaseAddr, addr & ~1U, (uint16_t)(lo | ((uint16_t)hi << 8)));
    if (ret != IS29GL_FMC_OK)
    {
      return ret;
    }
  }
  return IS29GL_FMC_OK;
}

int32_t IS29GL_FMC_EraseSector(uint32_t BaseAddr, uint32_t SectorOffset)
{
  uint32_t wordOffset = SectorOffset / 2;

  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_ERASE_SETUP);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, wordOffset, IS29GL_CMD_SECTOR_ERASE);

  /* Sector erase: up to several seconds on 128 KB sectors */
  return IS29GL_FMC_PollDQ7(BaseAddr, wordOffset, 0xFFFFU, 6000, true);
}

int32_t IS29GL_FMC_EraseChip(uint32_t BaseAddr)
{
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_ERASE_SETUP);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_UNLOCK_DATA1);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR2, IS29GL_CMD_UNLOCK_DATA2);
  FMC_WriteWord(BaseAddr, IS29GL_UNLOCK_ADDR1, IS29GL_CMD_CHIP_ERASE);

  /* Chip erase runs for minutes on 512 Mbit parts: poll until the array reads erased */
  return IS29GL_FMC_PollDQ7(BaseAddr, 0, 0xFFFFU, 600000, true);
}
