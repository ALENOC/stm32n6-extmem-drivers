/**
  ******************************************************************************
  * @file    is29gl_fmc.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI IS29GL Parallel NOR Flash via FMC (16-bit).
  *          Supports IS29GL256, IS29GL128, IS29GL064, IS29GL032 and Micron MT28EW.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 Alessandro Nocentini (ALENOC) & Community Contributors.
  * All rights reserved.
  *
  ******************************************************************************
  */

#ifndef IS29GL_FMC_H
#define IS29GL_FMC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS29GL_FMC_OK                  (0)
#define IS29GL_FMC_ERROR              (-1)
#define IS29GL_FMC_TIMEOUT            (-3)

/* Standard CFI / AMD Command Definitions for 16-bit word operations */
#define IS29GL_UNLOCK_ADDR1           0x00000555U
#define IS29GL_UNLOCK_ADDR2           0x000002AAU

#define IS29GL_CMD_UNLOCK_DATA1       0x00AAU
#define IS29GL_CMD_UNLOCK_DATA2       0x0055U
#define IS29GL_CMD_AUTOSELECT         0x0090U
#define IS29GL_CMD_PROGRAM            0x00A0U
#define IS29GL_CMD_ERASE_SETUP        0x0080U
#define IS29GL_CMD_SECTOR_ERASE       0x0030U
#define IS29GL_CMD_CHIP_ERASE         0x0010U
#define IS29GL_CMD_RESET              0x00F0U

/* Status Register Bit Masks */
#define IS29GL_SR_DQ7_POLL            0x0080U
#define IS29GL_SR_DQ5_EXCEEDED        0x0020U
#define IS29GL_SR_DQ3_SECTOR_TIMEOUT  0x0008U
#define IS29GL_SR_DQ1_WRITE_BUFFER_ABT 0x0002U

typedef struct {
  uint32_t AddressSetupTime;      /*!< Setup time in HCLK cycles */
  uint32_t AddressHoldTime;       /*!< Hold time in HCLK cycles */
  uint32_t DataSetupTime;          /*!< Data setup time in HCLK cycles */
  uint32_t BusTurnAroundDuration; /*!< Bus turn-around in HCLK cycles */
} IS29GL_FMC_Timing_t;

/* Driver API */
int32_t IS29GL_FMC_Init(SRAM_HandleTypeDef *hsram, uint32_t Bank, const IS29GL_FMC_Timing_t *pTiming);
int32_t IS29GL_FMC_Reset(uint32_t BaseAddr);
int32_t IS29GL_FMC_ReadID(uint32_t BaseAddr, uint16_t *pMfgId, uint16_t *pDevId);
int32_t IS29GL_FMC_Read(uint32_t BaseAddr, uint32_t Offset, uint8_t *pData, uint32_t Size);
int32_t IS29GL_FMC_ProgramWord(uint32_t BaseAddr, uint32_t Offset, uint16_t Data);
int32_t IS29GL_FMC_ProgramBuffer(uint32_t BaseAddr, uint32_t Offset, const uint8_t *pData, uint32_t Size);
int32_t IS29GL_FMC_EraseSector(uint32_t BaseAddr, uint32_t SectorOffset);
int32_t IS29GL_FMC_EraseChip(uint32_t BaseAddr);

#ifdef __cplusplus
}
#endif

#endif /* IS29GL_FMC_H */
