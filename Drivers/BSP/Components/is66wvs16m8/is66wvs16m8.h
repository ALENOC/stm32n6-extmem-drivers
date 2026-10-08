/**
  ******************************************************************************
  * @file    is66wvs16m8.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI Quad SPI PSRAM (IS66WVS / IS67WVS series).
  *          Supports IS66WVS1M8, IS66WVS2M8, IS66WVS4M8, IS66WVS16M8.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS66WVS16M8_H
#define IS66WVS16M8_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS66WVS_RAM_SIZE_8MBIT           (1U * 1024U * 1024U)  /* 8 Mbits = 1 MByte    */
#define IS66WVS_RAM_SIZE_16MBIT          (2U * 1024U * 1024U)  /* 16 Mbits = 2 MBytes  */
#define IS66WVS_RAM_SIZE_32MBIT          (4U * 1024U * 1024U)  /* 32 Mbits = 4 MBytes  */
#define IS66WVS_RAM_SIZE_64MBIT          (8U * 1024U * 1024U)  /* 64 Mbits = 8 MBytes  */
#define IS66WVS_RAM_SIZE_128MBIT         (16U * 1024U * 1024U) /* 128 Mbits = 16 MBytes */

#define IS66WVS_OK                       (0)
#define IS66WVS_ERROR                    (-1)
#define IS66WVS_TIMEOUT                  (-3)

/* ISSI Vendor Identification */
#define IS66WVS_PAGE_SIZE                1024U  /* Reads and writes wrap inside a 1 KB page */
#define IS66WVS_MANUFACTURER_ID          0x9DU
#define IS66WVS_KGD                      0x5DU

/* Commands */
#define IS66WVS_CMD_READ                 0x03U
#define IS66WVS_CMD_FAST_READ            0x0BU
#define IS66WVS_CMD_FAST_READ_QUAD       0xEBU
#define IS66WVS_CMD_WRITE                0x02U
#define IS66WVS_CMD_WRITE_QUAD           0x38U
#define IS66WVS_CMD_ENTER_QUAD_MODE      0x35U
#define IS66WVS_CMD_EXIT_QUAD_MODE       0xF5U
#define IS66WVS_CMD_RESET_ENABLE         0x66U
#define IS66WVS_CMD_RESET                0x99U
#define IS66WVS_CMD_READ_ID              0x9FU

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WVS16M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS66WVS16M8_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t IS66WVS16M8_EnterQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t IS66WVS16M8_ExitQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t IS66WVS16M8_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t IS66WVS16M8_WriteQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS66WVS16M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS66WVS16M8_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS66WVS16M8_H */
