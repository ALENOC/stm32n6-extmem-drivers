/**
  ******************************************************************************
  * @file    s25hl512t.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon SEMPER(TM) / FL Quad SPI NOR Flash
  *          Compatible with S25HL512T, S25HS512T, S25FL256L, S25FL128L,
  *          S25FL512S, S25FL256S, S25FL128S.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef S25HL512T_H
#define S25HL512T_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define S25HL512T_FLASH_SIZE             (64U * 1024U * 1024U) /* 512 Mbits = 64 MBytes */
#define S25HL512T_PAGE_SIZE              256U                  /* 256 Bytes */
#define S25HL512T_SECTOR_4K              (4U * 1024U)          /* 4 KBytes  */
#define S25HL512T_BLOCK_64K              (64U * 1024U)         /* 64 KBytes */

#define S25HL512T_OK                     (0)
#define S25HL512T_ERROR                  (-1)
#define S25HL512T_TIMEOUT                (-3)

/* Commands */
#define S25HL_CMD_READ_ID                0x9FU
#define S25HL_CMD_READ_SFDP              0x5AU
#define S25HL_CMD_READ_FAST_4B           0x0CU
#define S25HL_CMD_READ_QUAD_IO_4B        0xECU /* 4-byte address Quad I/O Fast Read */
#define S25HL_CMD_READ_QUAD_OUT_4B       0x6CU /* 4-byte address Quad Output Read   */
#define S25HL_CMD_PAGE_PROG_4B           0x12U
#define S25HL_CMD_QUAD_PAGE_PROG_4B      0x34U
#define S25HL_CMD_SECTOR_ERASE_4K_4B     0x21U
#define S25HL_CMD_BLOCK_ERASE_64K_4B     0xDCU
#define S25HL_CMD_CHIP_ERASE             0x60U
#define S25HL_CMD_WRITE_ENABLE           0x06U
#define S25HL_CMD_WRITE_DISABLE          0x04U
#define S25HL_CMD_READ_STATUS1           0x05U
#define S25HL_CMD_READ_CONFIG1           0x35U
#define S25HL_CMD_WRITE_STATUS1          0x01U
#define S25HL_CMD_RESET_ENABLE           0x66U
#define S25HL_CMD_RESET                  0x99U

/* Status / Config Bits */
#define S25HL_SR1_WIP                    (1U << 0)
#define S25HL_SR1_WEL                    (1U << 1)
#define S25HL_CR1_QUAD_ENABLE            (1U << 1) /* Bit 1 of Configuration Register 1 is QUAD bit */

/* Exported Functions --------------------------------------------------------*/
int32_t S25HL512T_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t S25HL512T_WriteEnable(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout);
int32_t S25HL512T_EnableQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t S25HL512T_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S25HL512T_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t S25HL512T_EraseBlock64K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t S25HL512T_ChipErase(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t S25HL512T_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* S25HL512T_H */
