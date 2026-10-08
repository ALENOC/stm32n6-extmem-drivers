/**
  ******************************************************************************
  * @file    is25lp256.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI Quad SPI NOR Flash (IS25LP / IS25WP series).
  *          Supports IS25LP064, IS25WP064, IS25LP128, IS25WP128,
  *          IS25LP256, IS25WP256, IS25LP512, IS25WP512.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS25LP256_H
#define IS25LP256_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS25LP256_FLASH_SIZE             (32U * 1024U * 1024U) /* 256 Mbits = 32 MBytes */
#define IS25LP256_PAGE_SIZE              256U                  /* 256 Bytes */
#define IS25LP256_SECTOR_4K              (4U * 1024U)          /* 4 KBytes  */
#define IS25LP256_BLOCK_64K              (64U * 1024U)         /* 64 KBytes */

#define IS25LP_OK                        (0)
#define IS25LP_ERROR                     (-1)
#define IS25LP_TIMEOUT                   (-3)

/* Commands - Standard 3-byte / 24-bit addressing */
#define IS25LP_CMD_READ_ID               0x9FU
#define IS25LP_CMD_READ_SFDP             0x5AU
#define IS25LP_CMD_READ_FAST             0x0BU
#define IS25LP_CMD_READ_QUAD_IO          0xEBU
#define IS25LP_CMD_PAGE_PROG_QUAD        0x32U
#define IS25LP_CMD_SECTOR_ERASE_4K       0x20U
#define IS25LP_CMD_BLOCK_ERASE_64K       0xD8U

/* Commands - Dedicated 4-byte / 32-bit addressing */
#define IS25LP_CMD_READ_FAST_4B          0x0CU
#define IS25LP_CMD_READ_QUAD_IO_4B       0xECU
#define IS25LP_CMD_READ_QUAD_OUT_4B      0x6CU
#define IS25LP_CMD_PAGE_PROG_4B          0x12U
#define IS25LP_CMD_QUAD_PAGE_PROG_4B     0x34U
#define IS25LP_CMD_SECTOR_ERASE_4K_4B    0x21U
#define IS25LP_CMD_BLOCK_ERASE_64K_4B    0xDCU
#define IS25LP_CMD_CHIP_ERASE            0x60U
#define IS25LP_CMD_WRITE_ENABLE          0x06U
#define IS25LP_CMD_WRITE_DISABLE         0x04U
#define IS25LP_CMD_READ_STATUS           0x05U
#define IS25LP_CMD_READ_FUNCTION_REG     0x48U
#define IS25LP_CMD_WRITE_STATUS          0x01U
#define IS25LP_CMD_ENTER_4BYTE_ADDR      0xB7U
#define IS25LP_CMD_EXIT_4BYTE_ADDR       0xE9U
#define IS25LP_CMD_SET_READ_PARAMS_V     0xC0U /* SRPV: volatile Read Register (P7..P0)     */
#define IS25LP_CMD_READ_READ_PARAMS      0x61U /* RDRP: read the Read Register              */
#define IS25LP_CMD_RESET_ENABLE          0x66U
#define IS25LP_CMD_RESET                 0x99U

/* Status / Config Masks */
#define IS25LP_SR_WIP                    (1U << 0)
#define IS25LP_SR_WEL                    (1U << 1)
#define IS25LP_SR_QE                     (1U << 6) /* Bit 6 of Status Register is QE */

/* Read Register P[6:3]: dummy cycles (mode bit cycles included), 0 = factory default (6 for EBh, 81 MHz) */
#define IS25LP_READ_PARAMS_DUMMY_POS     3U
#define IS25LP_READ_PARAMS_DUMMY_MASK    (0x0FU << IS25LP_READ_PARAMS_DUMMY_POS)
#define IS25LP_DEFAULT_QUAD_IO_DUMMY     6U
#define IS25LP_FAST_QUAD_IO_DUMMY        11U   /* EBh/ECh up to 139 MHz on IS25LP (3.0 V) */

/* Quad I/O read mode bits: anything other than AXh keeps the device out of XIP mode */
#define IS25LP_MODE_BITS_NO_XIP          0x00U

/* Exported Functions --------------------------------------------------------*/
int32_t IS25LP256_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t IS25LP256_WriteEnable(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP256_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, uint32_t Timeout);
int32_t IS25LP256_EnableQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP_Enter4ByteAddressMode(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP_Exit4ByteAddressMode(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP_ReadQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles, uint32_t AddressWidth);
int32_t IS25LP_PageProgramQuadEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size, uint32_t AddressWidth);
int32_t IS25LP_EraseSector4KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth);
int32_t IS25LP_EraseBlock64KEx(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint32_t AddressWidth);
int32_t IS25LP_EnableMemoryMappedModeEx(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint32_t AddressWidth);
int32_t IS25LP256_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t IS25LP256_PageProgramQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS25LP256_EraseSector4K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t IS25LP256_EraseBlock64K(XSPI_HandleTypeDef *Ctx, uint32_t Address);
int32_t IS25LP256_ChipErase(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP256_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS25LP256_Reset(XSPI_HandleTypeDef *Ctx);
int32_t IS25LP256_SetReadDummyCycles(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles, uint8_t *pApplied);

#ifdef __cplusplus
}
#endif

#endif /* IS25LP256_H */
