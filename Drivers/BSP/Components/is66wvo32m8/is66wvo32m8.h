/**
  ******************************************************************************
  * @file    is66wvo32m8.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI OctalRAM (IS66WVO / IS67WVO series).
  *          Compatible with IS66WVO8M8, IS66WVO16M8, IS66WVO32M8, IS66WVO64M8.
  *
  *          Protocol (ISSI datasheet IS66/67WVO8M8DALL/BLL, Rev. A1):
  *          - 8D-8D-8D, 3 command/address clocks: command byte + 00h, then the
  *            row address and the column address (1 KB rows, word granularity).
  *          - The STM32 XSPI builds the row/column address phase in hardware in
  *            "Macronix RAM" mode (DCR1.MTYP = 011), which also matches the
  *            Q1/Q0 data byte order of the OctalRAM.
  *          - Register access: ID register at 0x00000000, configuration register
  *            at 0x00040000, 16-bit registers, zero latency on register writes.
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

/* Commands: 16-bit 8D instruction, command byte followed by 00h */
#define IS66WVO_CMD_READ_LINEAR          0xA000U /* Memory read, continuous burst  */
#define IS66WVO_CMD_READ_WRAPPED         0x8000U /* Memory read, wrapped burst     */
#define IS66WVO_CMD_WRITE_LINEAR         0x2000U /* Memory write, continuous burst */
#define IS66WVO_CMD_WRITE_WRAPPED        0x0000U /* Memory write, wrapped burst    */
#define IS66WVO_CMD_READ_REG             0xC000U /* ID / configuration register read */
#define IS66WVO_CMD_WRITE_REG            0x4000U /* Configuration register write     */

/* Register addresses (row/column fields of the address phase) */
#define IS66WVO_REG_ID                   0x00000000U
#define IS66WVO_REG_CR                   0x00040000U

/* Configuration Register (CR) */
#define IS66WVO_CR_DPD_NORMAL            (1U << 15)       /* 1 = normal, 0 = deep power down       */
#define IS66WVO_CR_ODS_MASK              (7U << 12)
#define IS66WVO_CR_ODS_24_OHM            (7U << 12)       /* Default drive strength                */
#define IS66WVO_CR_DQSM_PRECYCLE         (1U << 8)        /* 1 = one DQSM pre-cycle before data    */
#define IS66WVO_CR_LC_MASK               (0xFU << 4)
#define IS66WVO_CR_LC_3_CLOCKS           (0x0U << 4)      /*  83 MHz */
#define IS66WVO_CR_LC_4_CLOCKS           (0x1U << 4)      /* 100 MHz */
#define IS66WVO_CR_LC_5_CLOCKS           (0x2U << 4)      /* 133/166 MHz, default at 3.0 V */
#define IS66WVO_CR_LC_6_CLOCKS           (0x3U << 4)      /* 166 MHz */
#define IS66WVO_CR_LC_7_CLOCKS           (0x4U << 4)      /* 200 MHz */
#define IS66WVO_CR_LC_8_CLOCKS           (0x5U << 4)      /* 200 MHz, default at 1.8 V */
#define IS66WVO_CR_FIXED_LATENCY         (1U << 3)        /* Initial latency always 2 x LC         */
#define IS66WVO_CR_BURST_32B             (0x2U << 0)      /* Wrap length (default)                 */

/* Latency used by this driver: LC = 7 (200 MHz), fixed latency. The STM32N6 erratum
 * "variable latency is not supported when a refresh collision occurs during a write access
 * to some OctaRAM memories" (ES0620) requires fixed latency. */
#define IS66WVO_LATENCY_CLOCKS           7U
#define IS66WVO_CR_INIT_VALUE            (IS66WVO_CR_DPD_NORMAL | IS66WVO_CR_ODS_24_OHM | IS66WVO_CR_LC_7_CLOCKS | \
                                          IS66WVO_CR_FIXED_LATENCY | IS66WVO_CR_BURST_32B)
/* XSPI dummy cycles: the memory latency (2 x LC with fixed latency) minus the clock that
 * overlaps the end of the address phase */
#define IS66WVO_DUMMY_CYCLES             ((2U * IS66WVO_LATENCY_CLOCKS) - 1U)

/* ID register */
#define IS66WVO_ID_MANUFACTURER_MASK     0x000FU
#define IS66WVO_ID_MANUFACTURER_ISSI     0x0003U
#define IS66WVO_ID_ROW_BITS_POS          8U               /* [12:8] row address bits - 1    */
#define IS66WVO_ID_COL_BITS_POS          4U               /* [7:4]  column address bits - 1 */

/* Row length: transfers are split so that a single access never crosses it */
#define IS66WVO_ROW_SIZE                 1024U

/* Exported Functions --------------------------------------------------------*/
int32_t IS66WVO32M8_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS66WVO32M8_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t *pValue, uint32_t DummyCycles);
int32_t IS66WVO32M8_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t RegAddr, uint16_t Value);
int32_t IS66WVO32M8_ReadID(XSPI_HandleTypeDef *Ctx, uint16_t *pId, uint32_t *pCapacityBytes);
int32_t IS66WVO32M8_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Write(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_Write_DMA(XSPI_HandleTypeDef *Ctx, const uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t DummyCycles);
int32_t IS66WVO32M8_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint32_t ReadDummyCycles, uint32_t WriteDummyCycles);
int32_t IS66WVO32M8_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx);
int32_t IS66WVO32M8_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS66WVO32M8_H */
