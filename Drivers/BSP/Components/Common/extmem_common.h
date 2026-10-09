/**
  ******************************************************************************
  * @file    extmem_common.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Common definitions, error codes, and memory geometry structures
  *          for Infineon and ISSI Flash and PSRAM memories on STM32N6.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file.
  *
  ******************************************************************************
  */

#ifndef EXTMEM_COMMON_H
#define EXTMEM_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Exported Constants & Error Codes ------------------------------------------*/
#define EXTMEM_OK                   (0)
#define EXTMEM_ERROR               (-1)
#define EXTMEM_BUSY                (-2)
#define EXTMEM_TIMEOUT             (-3)
#define EXTMEM_NOT_SUPPORTED       (-4)
#define EXTMEM_INVALID_PARAM       (-5)
#define EXTMEM_AUTH_FAIL           (-6)
#define EXTMEM_NOT_INITIALIZED     (-7)

/* JEDEC / Vendor Manufacturer IDs -------------------------------------------*/
#define EXTMEM_MFG_INFINEON_SPANSION  0x34U  /*!< Infineon / Spansion Manufacturer ID */
#define EXTMEM_MFG_CYPRESS_LEGACY     0x01U  /*!< Legacy Cypress Manufacturer ID      */
#define EXTMEM_MFG_ISSI               0x9DU  /*!< ISSI Manufacturer ID                */
#define EXTMEM_MFG_APMEM              0x0DU  /*!< AP Memory ID                        */
#define EXTMEM_MFG_MICRON             0x2CU  /*!< Micron Technology Manufacturer ID   */
#define EXTMEM_MFG_NUMONYX_LEGACY     0x20U  /*!< Legacy Numonyx/ST Manufacturer ID   */
#define EXTMEM_MFG_MACRONIX           0xC2U  /*!< Macronix Manufacturer ID            */
#define EXTMEM_MFG_WINBOND            0xEFU  /*!< Winbond Manufacturer ID             */

/* HyperRAM ID0[3:0] manufacturer codes */
#define EXTMEM_HYPERRAM_MFG_CYPRESS   0x01U  /*!< Cypress / Infineon HyperRAM         */
#define EXTMEM_HYPERRAM_MFG_ISSI      0x03U  /*!< ISSI HyperRAM                       */
#define EXTMEM_HYPERRAM_MFG_INFINEON  0x06U  /*!< Infineon HyperRAM 2.0 (S80KS2562)   */

/* Memory Classification -----------------------------------------------------*/
typedef enum {
  EXTMEM_TYPE_UNKNOWN = 0,
  EXTMEM_TYPE_NOR_OCTAL_SEMPER,    /*!< Infineon SEMPER Octal NOR Flash (S28HS / S28HL) */
  EXTMEM_TYPE_NOR_OCTAL_ISSI,      /*!< ISSI Octal NOR Flash (IS25LX / IS25WX)          */
  EXTMEM_TYPE_NOR_OCTAL_MICRON,    /*!< Micron Xccela Octal NOR Flash (MT35XU / MT35XL) */
  EXTMEM_TYPE_NOR_QUAD_INFINEON,   /*!< Infineon SEMPER / FL Quad SPI Flash             */
  EXTMEM_TYPE_NOR_QUAD_ISSI,       /*!< ISSI Quad SPI Flash (IS25LP / IS25WP / IS25LE / IS25WE) */
  EXTMEM_TYPE_NOR_QUAD_MICRON,     /*!< Micron Quad SPI Flash (MT25QU / MT25QL)         */
  EXTMEM_TYPE_HYPERFLASH_INFINEON, /*!< Infineon HyperFlash (S26KS / S26KL)             */
  EXTMEM_TYPE_HYPERFLASH_ISSI,     /*!< ISSI HyperFlash (IS26KS / IS26KL)               */
  EXTMEM_TYPE_HYPERRAM_INFINEON,   /*!< Infineon HyperRAM (S80KS)                       */
  EXTMEM_TYPE_HYPERRAM_ISSI,       /*!< ISSI HyperRAM (IS66WVH / IS67WVH)               */
  EXTMEM_TYPE_PSRAM_OCTAL_ISSI,    /*!< ISSI Octal PSRAM (IS66WVO / IS67WVO)            */
  EXTMEM_TYPE_PSRAM_QUAD_ISSI,     /*!< ISSI Quad PSRAM (IS66WVS / IS67WVS)             */
  EXTMEM_TYPE_SRAM_SERIAL_ISSI,    /*!< ISSI Serial SRAM (IS62WVS / IS65WVS)            */
  EXTMEM_TYPE_PSRAM_PARALLEL_FMC,  /*!< ISSI / Infineon Parallel Asynchronous PSRAM/SRAM*/
  EXTMEM_TYPE_NOR_PARALLEL_FMC     /*!< Parallel Asynchronous NOR Flash via FMC         */
} ExtMem_Type_t;

/* Physical Bus Interface ----------------------------------------------------*/
typedef enum {
  EXTMEM_BUS_XSPI1 = 0,            /*!< STM32N6 XSPI Port 1 (Octal / Hexa / HyperBus)   */
  EXTMEM_BUS_XSPI2,                /*!< STM32N6 XSPI Port 2                             */
  EXTMEM_BUS_XSPI3,                /*!< STM32N6 XSPI Port 3                             */
  EXTMEM_BUS_FMC_SRAM_BANK1_1,     /*!< FMC Bank 1 Sub-Bank 1 (0x60000000, NE1)         */
  EXTMEM_BUS_FMC_SRAM_BANK1_2,     /*!< FMC Bank 1 Sub-Bank 2 (0x64000000, NE2)         */
  EXTMEM_BUS_FMC_SRAM_BANK1_3,     /*!< FMC Bank 1 Sub-Bank 3 (0x68000000, NE3)         */
  EXTMEM_BUS_FMC_SRAM_BANK1_4      /*!< FMC Bank 1 Sub-Bank 4 (0x6C000000, NE4)         */
} ExtMem_Bus_t;

/* Protocol Mode -------------------------------------------------------------*/
typedef enum {
  EXTMEM_MODE_SPI = 0,             /*!< 1-1-1 Single SPI Mode                           */
  EXTMEM_MODE_QUAD_1_1_4,          /*!< 1-1-4 Quad Output Fast Read                     */
  EXTMEM_MODE_QUAD_1_4_4,          /*!< 1-4-4 Quad I/O                                  */
  EXTMEM_MODE_QUAD_4_4_4,          /*!< 4-4-4 QPI Mode                                  */
  EXTMEM_MODE_OCTAL_STR,           /*!< 8S-8S-8S Octal Single Transfer Rate             */
  EXTMEM_MODE_OCTAL_DTR,           /*!< 8D-8D-8D Octal Double Transfer Rate (DDR)       */
  EXTMEM_MODE_HYPERBUS,            /*!< HyperBus Protocol (Differential / DQS)          */
  EXTMEM_MODE_PARALLEL_16BIT       /*!< 16-bit Asynchronous Parallel Bus (FMC)          */
} ExtMem_Mode_t;

/* Transfer Rate -------------------------------------------------------------*/
typedef enum {
  EXTMEM_TRANSFER_STR = 0,         /*!< Single Transfer Rate                            */
  EXTMEM_TRANSFER_DTR = 1          /*!< Double Transfer Rate                            */
} ExtMem_TransferRate_t;

/* Memory Access State -------------------------------------------------------*/
typedef enum {
  EXTMEM_STATE_UNINITIALIZED = 0,
  EXTMEM_STATE_INDIRECT,           /*!< Regular Read / Write / Command mode             */
  EXTMEM_STATE_MEMORY_MAPPED,      /*!< Direct memory-mapped XIP execution mode         */
  EXTMEM_STATE_LOW_POWER           /*!< Deep Power Down / Standby mode                  */
} ExtMem_State_t;

/* Memory Geometry & Characteristics -----------------------------------------*/
typedef struct {
  char              DeviceName[32];      /*!< Null-terminated part number string       */
  ExtMem_Type_t     Type;                /*!< Memory technology & family               */
  uint32_t          TotalSizeBytes;      /*!< Total memory capacity in bytes           */
  uint32_t          PageSizeBytes;       /*!< Page program size (256B, 512B, or 0)     */
  uint32_t          SectorSizeBytes;     /*!< Smallest erasable sector (typically 4KB) */
  uint32_t          BlockSizeBytes;      /*!< Standard erase block (64KB or 256KB)     */
  uint32_t          MaxClockFreqMHz;     /*!< Maximum rated clock frequency in MHz     */
  uint8_t           DummyCyclesRead;     /*!< Configured dummy latency cycles for read */
  uint8_t           DummyCyclesWrite;    /*!< Latency cycles for write (PSRAM)         */
  bool              SupportsDTR;         /*!< Supports Double Transfer Rate            */
  bool              SupportsMemoryMapped;/*!< Supports STM32 Memory-Mapped Mode        */
  bool              IsNonVolatile;       /*!< True for Flash, False for PSRAM/HyperRAM */
} ExtMem_Geometry_t;

#ifdef __cplusplus
}
#endif

#endif /* EXTMEM_COMMON_H */
