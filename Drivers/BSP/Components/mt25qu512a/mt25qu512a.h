/**
  ******************************************************************************
  * @file    mt25qu512a.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Micron Quad SPI NOR Flash memory
  *          (MT25QU / MT25QL and N25Q series: 32Mb up to 1Gb).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef MT25QU512A_H
#define MT25QU512A_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

/* Memory Constants ----------------------------------------------------------*/
#define MT25Q_FLASH_SIZE_32MBIT          (4U * 1024U * 1024U)    /* 4 MBytes   */
#define MT25Q_FLASH_SIZE_64MBIT          (8U * 1024U * 1024U)    /* 8 MBytes   */
#define MT25Q_FLASH_SIZE_128MBIT         (16U * 1024U * 1024U)   /* 16 MBytes  */
#define MT25Q_FLASH_SIZE_256MBIT         (32U * 1024U * 1024U)   /* 32 MBytes  */
#define MT25Q_FLASH_SIZE_512MBIT         (64U * 1024U * 1024U)   /* 64 MBytes  */
#define MT25Q_FLASH_SIZE_1GBIT           (128U * 1024U * 1024U)  /* 128 MBytes */

#define MT25Q_PAGE_SIZE                  256U                    /* 256 Bytes  */
#define MT25Q_SUBSECTOR_4K_SIZE          (4U * 1024U)            /* 4 KBytes   */
#define MT25Q_SECTOR_64K_SIZE            (64U * 1024U)           /* 64 KBytes  */

#define MT25Q_OK                         (0)
#define MT25Q_ERROR                      (-1)
#define MT25Q_BUSY                       (-2)
#define MT25Q_TIMEOUT                    (-3)

/* JEDEC Identification */
#define MT25Q_MANUFACTURER_ID            0x2CU
#define MT25Q_MEMORY_TYPE_3V0            0xBAU                   /* MT25QL (3.0V) */
#define MT25Q_MEMORY_TYPE_1V8            0xBBU                   /* MT25QU (1.8V) */

/* Commands */
#define MT25Q_CMD_READ_ID                0x9FU
#define MT25Q_CMD_READ_STATUS_REG        0x05U
#define MT25Q_CMD_WRITE_STATUS_REG       0x01U
#define MT25Q_CMD_READ_FLAG_STATUS_REG   0x70U
#define MT25Q_CMD_CLEAR_FLAG_STATUS_REG  0x50U
#define MT25Q_CMD_WRITE_ENABLE           0x06U
#define MT25Q_CMD_WRITE_DISABLE          0x04U
#define MT25Q_CMD_READ_EVCR              0x65U                   /* Enhanced Volatile Configuration Register */
#define MT25Q_CMD_WRITE_EVCR             0x61U
#define MT25Q_CMD_READ_VCR               0x85U                   /* Volatile Configuration Register */
#define MT25Q_CMD_WRITE_VCR              0x81U
#define MT25Q_CMD_FAST_READ_QUAD         0xEBU                   /* 1-4-4 Fast Read */
#define MT25Q_CMD_FAST_READ_QUAD_4B      0xECU                   /* 4-byte 1-4-4 Fast Read */
#define MT25Q_CMD_PAGE_PROGRAM_QUAD      0x32U                   /* 1-1-4 Quad Page Program */
#define MT25Q_CMD_PAGE_PROGRAM_QUAD_4B   0x34U                   /* 4-byte Quad Page Program */
#define MT25Q_CMD_SUBSECTOR_ERASE_4K     0x20U                   /* 4KB Subsector Erase */
#define MT25Q_CMD_SUBSECTOR_ERASE_4K_4B  0x21U
#define MT25Q_CMD_SECTOR_ERASE_64K       0xD8U                   /* 64KB Sector Erase */
#define MT25Q_CMD_SECTOR_ERASE_64K_4B    0xDCU
#define MT25Q_CMD_CHIP_ERASE             0xC7U
#define MT25Q_CMD_ENTER_4BYTE_ADDR       0xB7U
#define MT25Q_CMD_EXIT_4BYTE_ADDR        0xE9U
#define MT25Q_CMD_RESET_ENABLE           0x66U
#define MT25Q_CMD_RESET                  0x99U

/* Status & Flag Status Register Bits */
#define MT25Q_SR_WIP                     (1U << 0)
#define MT25Q_SR_WEL                     (1U << 1)
#define MT25Q_FSR_READY                  (1U << 7)
#define MT25Q_FSR_ERASE_ERROR            (1U << 5)
#define MT25Q_FSR_PROGRAM_ERROR          (1U << 4)

/* Exported Functions --------------------------------------------------------*/
int32_t MT25QU_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t MT25QU_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t MT25QU_ReadStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pStatus);
int32_t MT25QU_ReadFlagStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pFlagStatus);
int32_t MT25QU_WriteEnable(XSPI_HandleTypeDef *Ctx);
int32_t MT25QU_Enter4ByteAddressMode(XSPI_HandleTypeDef *Ctx);
int32_t MT25QU_Exit4ByteAddressMode(XSPI_HandleTypeDef *Ctx);
int32_t MT25QU_ReadQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles, uint32_t AddressWidth);
int32_t MT25QU_PageProgramQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size, uint32_t AddressWidth);
int32_t MT25QU_EraseSector4KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth);
int32_t MT25QU_EraseBlock64KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth);
int32_t MT25QU_EnableMemoryMappedModeEx(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint32_t AddressWidth);
int32_t MT25QU_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t MT25QU_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t MT25QU_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t MT25QU_EraseBlock64K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t MT25QU_EraseChip(XSPI_HandleTypeDef *Ctx);
int32_t MT25QU_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t MT25QU_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* MT25QU512A_H */
