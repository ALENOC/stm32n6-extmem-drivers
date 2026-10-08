/**
  ******************************************************************************
  * @file    is66wvh16m8.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI HyperRAM(TM) PSRAM (IS66WVH / IS67WVH series).
  *          Compatible with IS66WVH8M8 and IS66WVH16M8.
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

/* CR0 Bit Definitions (HyperRAM CR0, default 0x8F1F) */
#define IS66WVH_CR0_DPD_NORMAL           (1U << 15)       /* 1 = normal operation, 0 = enter deep power down */
#define IS66WVH_CR0_DRIVE_STRENGTH_MASK  (0x7U << 12)
#define IS66WVH_CR0_DRIVE_STRENGTH_DEF   (0x0U << 12)     /* Default output impedance */
#define IS66WVH_CR0_RESERVED_ONES        (0xFU << 8)      /* Bits [11:8] must be written as 1 */
#define IS66WVH_CR0_LATENCY_MASK         (0xFU << 4)
#define IS66WVH_CR0_LATENCY_5_CYCLES     (0x0U << 4)      /* Up to 133 MHz */
#define IS66WVH_CR0_LATENCY_6_CYCLES     (0x1U << 4)      /* Up to 166 MHz (default) */
#define IS66WVH_CR0_LATENCY_7_CYCLES     (0x2U << 4)      /* Up to 200 MHz (default on 200 MHz parts) */
#define IS66WVH_CR0_LATENCY_3_CYCLES     (0xEU << 4)      /* Up to 85 MHz  */
#define IS66WVH_CR0_LATENCY_4_CYCLES     (0xFU << 4)      /* Up to 104 MHz */
#define IS66WVH_CR0_FIXED_LATENCY        (1U << 3)        /* 1 = fixed 2x latency, 0 = variable */
#define IS66WVH_CR0_VARIABLE_LATENCY     (0U << 3)
#define IS66WVH_CR0_LEGACY_WRAP          (1U << 2)        /* 1 = legacy wrapped burst, 0 = hybrid burst */
#define IS66WVH_CR0_BURST_128B           (0x0U << 0)
#define IS66WVH_CR0_BURST_64B            (0x1U << 0)
#define IS66WVH_CR0_BURST_16B            (0x2U << 0)
#define IS66WVH_CR0_BURST_32B            (0x3U << 0)

/* Initial latency used by the driver: 7 clocks is valid at every frequency up to 200 MHz
 * (CR0[7:4] = 0010b on Infineon S27KS/S27KL and ISSI IS66WVH/IS67WVH) */
#define IS66WVH_LATENCY_CLOCKS           7U

/* Initial configuration: normal operation, 7 clock variable latency, legacy 32 byte wrap */
#define IS66WVH_CR0_INIT_VALUE           (IS66WVH_CR0_DPD_NORMAL | IS66WVH_CR0_DRIVE_STRENGTH_DEF | IS66WVH_CR0_RESERVED_ONES | \
                                          IS66WVH_CR0_LATENCY_7_CYCLES | IS66WVH_CR0_VARIABLE_LATENCY | \
                                          IS66WVH_CR0_LEGACY_WRAP | IS66WVH_CR0_BURST_32B)

/* Wake-up time after deep power down exit (tEXTDPD, 150 us max) */
#define IS66WVH_DPD_EXIT_TIME_MS         1U

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WVH16M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS66WVH16M8_ReadRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue);
int32_t IS66WVH16M8_WriteRegister(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value);
int32_t IS66WVH16M8_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t IS66WVH16M8_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS66WVH16M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx);
int32_t IS66WVH16M8_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx);
int32_t IS66WVH16M8_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS66WVH16M8_H */
