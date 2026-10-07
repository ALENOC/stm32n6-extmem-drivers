/**
  ******************************************************************************
  * @file    s27ks0641.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon HyperRAM(TM) (S27KS / S27KL / S27HS / HL series).
  *          Compatible with S27KS0641, S27KL0641, S27KS128, S27KL128,
  *          S27KS256, S27KL256, S27KS512, S27HS064/128/256.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef S27KS0641_H
#define S27KS0641_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define S27KS0641_RAM_SIZE_64MBIT        (8U * 1024U * 1024U)   /* 64 Mbits = 8 MBytes   */
#define S27KS128_RAM_SIZE_128MBIT        (16U * 1024U * 1024U)  /* 128 Mbits = 16 MBytes */
#define S27KS256_RAM_SIZE_256MBIT        (32U * 1024U * 1024U)  /* 256 Mbits = 32 MBytes */
#define S27KS512_RAM_SIZE_512MBIT        (64U * 1024U * 1024U)  /* 512 Mbits = 64 MBytes */

#define S27KS_OK                         (0)
#define S27KS_ERROR                      (-1)
#define S27KS_TIMEOUT                    (-3)

/* Register Space Addresses */
#define S27KS_REG_ID0                    0x00000000U /* Identification Register 0 */
#define S27KS_REG_ID1                    0x00000002U /* Identification Register 1 */
#define S27KS_REG_CR0                    0x00001000U /* Configuration Register 0  */
#define S27KS_REG_CR1                    0x00001002U /* Configuration Register 1  */

/* CR0 Bit Definitions */
#define S27KS_CR0_BURST_128B             (0x00U << 0)
#define S27KS_CR0_BURST_64B              (0x01U << 0)
#define S27KS_CR0_BURST_16B              (0x02U << 0)
#define S27KS_CR0_BURST_32B              (0x03U << 0)

#define S27KS_CR0_DRIVE_STRENGTH_FULL    (0x00U << 2)
#define S27KS_CR0_DRIVE_STRENGTH_HALF    (0x01U << 2)

#define S27KS_CR0_VARIABLE_LATENCY       (0x00U << 3)
#define S27KS_CR0_FIXED_LATENCY          (0x01U << 3)

#define S27KS_CR0_LATENCY_5_CYCLES       (0x00U << 4) /* Up to 166 MHz */
#define S27KS_CR0_LATENCY_6_CYCLES       (0x01U << 4) /* Up to 200 MHz */
#define S27KS_CR0_LATENCY_4_CYCLES       (0x0EU << 4) /* Up to 133 MHz */
#define S27KS_CR0_LATENCY_3_CYCLES       (0x0FU << 4) /* Up to 83 MHz  */

/* CR1 Bit Definitions */
#define S27KS_CR1_DEEP_POWER_DOWN        (0x01U << 5)

/* Exported Functions --------------------------------------------------------*/
int32_t S27KS0641_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t S27KS0641_ReadRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue);
int32_t S27KS0641_WriteRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value);
int32_t S27KS0641_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t S27KS0641_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S27KS0641_Read_DMA(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t S27KS0641_Write_DMA(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S27KS0641_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx);
int32_t S27KS0641_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx);
int32_t S27KS0641_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* S27KS0641_H */
