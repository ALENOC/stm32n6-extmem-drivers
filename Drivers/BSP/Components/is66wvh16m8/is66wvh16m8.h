/**
  ******************************************************************************
  * @file    is66wvh16m8.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI HyperRAM(TM) PSRAM (IS66WVH / IS67WVH series).
  *          Compatible with IS66WVH8M8, IS66WVH16M8, IS66WVH32M8.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS66WVH16M8_H
#define IS66WVH16M8_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS66WVH_RAM_SIZE_128MBIT         (16U * 1024U * 1024U) /* 128 Mbits = 16 MBytes */

#define IS66WVH_OK                       (0)
#define IS66WVH_ERROR                    (-1)
#define IS66WVH_TIMEOUT                  (-3)

/* Register Space Addresses */
#define IS66WVH_REG_ID0                  0x00000000U
#define IS66WVH_REG_ID1                  0x00000002U
#define IS66WVH_REG_CR0                  0x00001000U
#define IS66WVH_REG_CR1                  0x00001002U

/* Configuration Register 0 Bits */
#define IS66WVH_CR0_VARIABLE_LATENCY     (0x00U << 3)
#define IS66WVH_CR0_FIXED_LATENCY        (0x01U << 3)
#define IS66WVH_CR0_LATENCY_5_CYCLES     (0x00U << 4)
#define IS66WVH_CR0_LATENCY_6_CYCLES     (0x01U << 4)
#define IS66WVH_CR0_DRIVE_STRENGTH_FULL  (0x00U << 2)

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WVH16M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS66WVH16M8_ReadRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue);
int32_t IS66WVH16M8_WriteRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value);
int32_t IS66WVH16M8_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t IS66WVH16M8_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS66WVH16M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS66WVH16M8_H */
