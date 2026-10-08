/**
  ******************************************************************************
  * @file    s25hl512t.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon SEMPER(TM) / FL Quad SPI NOR Flash
  *          Compatible with S25HL512T, S25HS512T (SEMPER Quad, 002-23660) and
  *          S25FL256L / S25FL128L (FL-L, 002-00124).
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
#define S25HL512T_SECTOR_256K            (256U * 1024U)        /* 256 KBytes uniform sector (S25HL-T / S25HS-T) */
#define S25FL_L_BLOCK_64K                (64U * 1024U)         /* 64 KBytes block (S25FL-L) */

#define S25HL512T_OK                     (0)
#define S25HL512T_ERROR                  (-1)
#define S25HL512T_TIMEOUT                (-3)

/* Commands */
#define S25HL_CMD_READ_ID                0x9FU
#define S25HL_CMD_READ_SFDP              0x5AU
#define S25HL_CMD_READ_FAST_4B           0x0CU
#define S25HL_CMD_READ_QUAD_IO_4B        0xECU /* 4-byte address Quad I/O Fast Read */
#define S25HL_CMD_READ_QUAD_OUT_4B       0x6CU /* 4-byte address Quad Output Read   */
#define S25HL_CMD_PAGE_PROG_4B           0x12U /* PRPGE_4_1, 1S-1S-1S */
#define S25HL_CMD_QUAD_PAGE_PROG_4B      0x34U /* 4QPP: S25FL-L only, not implemented by S25Hx-T */
#define S25HL_CMD_SECTOR_ERASE_4K_4B     0x21U
#define S25HL_CMD_BLOCK_ERASE_4B         0xDCU /* 256 KB sector on S25Hx-T, 64 KB block on S25FL-L */
#define S25HL_CMD_CHIP_ERASE             0x60U
#define S25HL_CMD_WRITE_ENABLE           0x06U
#define S25HL_CMD_WRITE_DISABLE          0x04U
#define S25HL_CMD_READ_STATUS1           0x05U
#define S25HL_CMD_READ_CONFIG1           0x35U
#define S25HL_CMD_WRITE_STATUS1          0x01U
#define S25HL_CMD_CLEAR_ERRORS           0x82U /* CLPEF: clear program / erase failure flags (SEMPER) */
#define S25FL_CMD_CLEAR_STATUS           0x30U /* CLSR: clear failure flags and WIP (S25FL-L only)    */
#define S25HL_CMD_READ_STATUS2           0x07U /* RDSR2 */
#define S25HL_CMD_RESET_ENABLE           0x66U
#define S25HL_CMD_RESET                  0x99U

/* Status / Config Bits */
#define S25HL_SR1_WIP                    (1U << 0)
#define S25HL_SR1_WEL                    (1U << 1)
#define S25HL_SR1_ERS_ERR                (1U << 5) /* SEMPER STR1V[5] ERSERR */
#define S25HL_SR1_PRG_ERR                (1U << 6) /* SEMPER STR1V[6] PRGERR */
#define S25FL_SR2_PRG_ERR                (1U << 5) /* S25FL-L SR2V[5] P_ERR  */
#define S25FL_SR2_ERS_ERR                (1U << 6) /* S25FL-L SR2V[6] E_ERR  */
/* Maximum erase times: S25Hx-T 002-23660 (256 KB sector 5869 ms with Endurance Flex, 512 Mb chip 696 s),
 * S25FL-L 002-00124 (64 KB block 725 ms, 256 Mb chip 360 s) */
#define S25HL_TIMEOUT_BLOCK_ERASE_MS     6000U
#define S25HL_TIMEOUT_CHIP_ERASE_MS      700000U

#define S25HL_MANUFACTURER_SEMPER        0x34U     /* S25Hx-T JEDEC manufacturer ID; S25FL-L reports 01h */
#define S25HL_CR1_QUAD_ENABLE            (1U << 1) /* Bit 1 of Configuration Register 1 is QUAD bit */

/* Quad I/O read: 2 continuous-read mode cycles follow the address and are NOT part of the latency
 * (datasheet 002-12345 note to the latency table). Mode bits other than Axh keep normal read mode. */
#define S25HL_MODE_BITS_NO_CONTINUOUS    0x00U
#define S25HL_DEFAULT_READ_LATENCY       8U    /* MEMLAT factory value: 1-4-4 up to 118 MHz */

/* Exported Functions --------------------------------------------------------*/
int32_t S25HL512T_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t S25HL512T_WriteEnable(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout);
int32_t S25HL512T_EnableQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t S25HL512T_PageProgram(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S25HL512T_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S25HL512T_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t S25HL512T_EraseBlock(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t S25HL512T_ChipErase(XSPI_HandleTypeDef *Ctx);
int32_t S25HL512T_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t S25HL512T_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* S25HL512T_H */
