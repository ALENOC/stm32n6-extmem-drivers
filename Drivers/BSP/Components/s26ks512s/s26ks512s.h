/**
  ******************************************************************************
  * @file    s26ks512s.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon HyperFlash(TM) (S26KS / S26KL series).
  *          Supports S26KS128S, S26KL128S, S26KS256S, S26KL256S, S26KS512S, S26KL512S.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef S26KS512S_H
#define S26KS512S_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define S26KS512S_FLASH_SIZE             (64U * 1024U * 1024U) /* 512 Mbits = 64 MBytes */
#define S26KS512S_SECTOR_SIZE            (256U * 1024U)        /* 256 KBytes */
#define S26KS512S_PAGE_SIZE              512U                  /* 512 Bytes write buffer */

#define S26KS512S_OK                     (0)
#define S26KS512S_ERROR                  (-1)
#define S26KS512S_TIMEOUT                (-3)

/* HyperFlash Unlock & Command byte addresses (Word addr << 1) */
#define S26KS_UNLOCK_ADDR1               0x00000AAAU /* Word 0x555 */
#define S26KS_UNLOCK_ADDR2               0x00000554U /* Word 0x2AA */

/* Command Opcodes (16-bit word data) */
#define S26KS_CMD_UNLOCK_DATA1           0x00AAU
#define S26KS_CMD_UNLOCK_DATA2           0x0055U
#define S26KS_CMD_WORD_PROGRAM           0x00A0U
#define S26KS_CMD_BUFFER_PROGRAM         0x0025U
#define S26KS_CMD_BUFFER_TO_FLASH        0x0029U
#define S26KS_CMD_SECTOR_ERASE_SETUP     0x0080U
#define S26KS_CMD_SECTOR_ERASE_CONFIRM   0x0030U
#define S26KS_CMD_CHIP_ERASE_CONFIRM     0x0010U
#define S26KS_CMD_RESET_CFI_EXIT         0x00F0U
#define S26KS_CMD_READ_STATUS            0x0070U
#define S26KS_CMD_CLEAR_STATUS           0x0071U
#define S26KS_CMD_ENTER_CFI              0x0098U

/* Status Register Bit Masks */
#define S26KS_SR_DEVICE_READY            (1U << 7)
#define S26KS_SR_ERASE_SUSPEND           (1U << 6)
#define S26KS_SR_ERASE_ERROR             (1U << 5)
#define S26KS_SR_PROGRAM_ERROR           (1U << 4)
#define S26KS_SR_PROGRAM_SUSPEND         (1U << 2)
#define S26KS_SR_SECTOR_LOCKED           (1U << 1)

/* Timing & Latency Configuration */
#define S26KS_INITIAL_LATENCY_CYCLES     16U

/* Exported Functions --------------------------------------------------------*/
int32_t S26KS512S_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler);
int32_t S26KS512S_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t S26KS512S_ProgramWord(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint16_t Data);
int32_t S26KS512S_ProgramBuffer(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S26KS512S_EraseSector(XSPI_HandleTypeDef *Ctx, uint32_t SectorAddress);
int32_t S26KS512S_EraseChip(XSPI_HandleTypeDef *Ctx);
int32_t S26KS512S_ReadStatus(XSPI_HandleTypeDef *Ctx, uint16_t *pStatus);
int32_t S26KS512S_ClearStatus(XSPI_HandleTypeDef *Ctx);
int32_t S26KS512S_WaitUntilReady(XSPI_HandleTypeDef *Ctx, uint32_t TimeoutMs);
int32_t S26KS512S_ReadCFI(XSPI_HandleTypeDef *Ctx, uint32_t WordOffset, uint16_t *pData);
int32_t S26KS512S_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx);
int32_t S26KS512S_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* S26KS512S_H */
