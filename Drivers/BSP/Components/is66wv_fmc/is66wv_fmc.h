/**
  ******************************************************************************
  * @file    is66wv_fmc.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Parallel Asynchronous PSRAM/SRAM via STM32N6 FMC.
  *          Supports ISSI IS66WV/IS67WV and Infineon MoBL (CY62xxx) series.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS66WV_FMC_H
#define IS66WV_FMC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS66WV_FMC_OK                    (0)
#define IS66WV_FMC_ERROR                 (-1)

/* Base Addresses for FMC Bank 1 Sub-Banks */
#define FMC_BANK1_1_BASE_ADDR            0x60000000U /* NE1 */
#define FMC_BANK1_2_BASE_ADDR            0x64000000U /* NE2 */
#define FMC_BANK1_3_BASE_ADDR            0x68000000U /* NE3 */
#define FMC_BANK1_4_BASE_ADDR            0x6C000000U /* NE4 */

/* Asynchronous Timings in FMC Clock Cycles (for 45ns / 55ns / 70ns PSRAM) */
typedef struct {
  uint32_t AddressSetupTime;     /* 1..15 HCLK cycles */
  uint32_t AddressHoldTime;      /* 1..15 HCLK cycles */
  uint32_t DataSetupTime;        /* 1..255 HCLK cycles */
  uint32_t BusTurnAroundDuration;/* 0..15 HCLK cycles */
} IS66WV_FMC_Timing_t;

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WV_FMC_Init(SRAM_HandleTypeDef *hsram, uint32_t Bank, const IS66WV_FMC_Timing_t *pTiming);
int32_t IS66WV_FMC_Read(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size);
int32_t IS66WV_FMC_Write(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size);
int32_t IS66WV_FMC_TestPattern(uint32_t BaseAddr, uint32_t TestSizeBytes);

#ifdef __cplusplus
}
#endif

#endif /* IS66WV_FMC_H */
