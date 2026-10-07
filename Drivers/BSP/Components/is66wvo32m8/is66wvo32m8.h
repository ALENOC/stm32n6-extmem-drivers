/**
  ******************************************************************************
  * @file    is66wvo32m8.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI Octal PSRAM (IS66WVO / IS67WVO series).
  *          Compatible with IS66WVO8M8, IS66WVO16M8, IS66WVO32M8, IS66WVO64M8.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS66WVO32M8_H
#define IS66WVO32M8_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS66WVO_RAM_SIZE_256MBIT         (32U * 1024U * 1024U) /* 256 Mbits = 32 MBytes */

#define IS66WVO_OK                       (0)
#define IS66WVO_ERROR                    (-1)
#define IS66WVO_TIMEOUT                  (-3)

/* Mode Registers Addresses */
#define IS66WVO_MR0_ADDR                 0x00000000U
#define IS66WVO_MR1_ADDR                 0x00000001U
#define IS66WVO_MR2_ADDR                 0x00000002U
#define IS66WVO_MR3_ADDR                 0x00000003U
#define IS66WVO_MR4_ADDR                 0x00000004U
#define IS66WVO_MR8_ADDR                 0x00000008U

/* MR0 Bit Definitions */
#define IS66WVO_MR0_DRIVE_STRENGTH_FULL  0x00U
#define IS66WVO_MR0_DRIVE_STRENGTH_HALF  0x01U
#define IS66WVO_MR0_READ_LATENCY_3       (0x00U << 2)
#define IS66WVO_MR0_READ_LATENCY_4       (0x01U << 2)
#define IS66WVO_MR0_READ_LATENCY_5       (0x02U << 2)
#define IS66WVO_MR0_READ_LATENCY_6       (0x03U << 2)
#define IS66WVO_MR0_VARIABLE_LATENCY     (0x00U << 5)
#define IS66WVO_MR0_FIXED_LATENCY        (0x01U << 5)

/* MR4 Bit Definitions */
#define IS66WVO_MR4_WRITE_LATENCY_3      (0x00U << 5)
#define IS66WVO_MR4_WRITE_LATENCY_4      (0x04U << 5)
#define IS66WVO_MR4_WRITE_LATENCY_5      (0x02U << 5)
#define IS66WVO_MR4_WRITE_LATENCY_6      (0x06U << 5)

/* MR8 Bit Definitions */
#define IS66WVO_MR8_BURST_16B            0x00U
#define IS66WVO_MR8_BURST_32B            0x01U
#define IS66WVO_MR8_BURST_64B            0x02U
#define IS66WVO_MR8_BURST_2KB            0x03U
#define IS66WVO_MR8_BURST_WRAPPED        (0x00U << 2)
#define IS66WVO_MR8_BURST_LINEAR         (0x01U << 2)

/* Commands */
#define IS66WVO_CMD_READ_SYNC            0x00U
#define IS66WVO_CMD_READ_LINEAR          0x20U
#define IS66WVO_CMD_WRITE_SYNC           0x80U
#define IS66WVO_CMD_WRITE_LINEAR         0xA0U
#define IS66WVO_CMD_READ_REG             0x40U
#define IS66WVO_CMD_WRITE_REG            0xC0U
#define IS66WVO_CMD_RESET                0xFFU

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WVO32M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS66WVO32M8_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint8_t *pValue, uint32_t DummyCycles);
int32_t IS66WVO32M8_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint8_t Value);
int32_t IS66WVO32M8_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Write(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Write_DMA(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint32_t ReadDummyCycles, uint32_t WriteDummyCycles);
int32_t IS66WVO32M8_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS66WVO32M8_H */
