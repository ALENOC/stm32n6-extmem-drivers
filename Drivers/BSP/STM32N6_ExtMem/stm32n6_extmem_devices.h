/**
  ******************************************************************************
  * @file    stm32n6_extmem_devices.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Comprehensive device registry and database for Infineon, ISSI & Micron
  *          Flash and PSRAM external memories compatible with STM32N6.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef STM32N6_EXTMEM_DEVICES_H
#define STM32N6_EXTMEM_DEVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extmem_common.h"
#include <string.h>

/* Memory Device Descriptor */
typedef struct {
  const char        *PartNumber;
  ExtMem_Type_t     Type;
  uint8_t           ManufacturerID;
  uint8_t           MemoryTypeID;
  uint8_t           DensityID;
  uint32_t          CapacityBytes;
  uint32_t          PageSizeBytes;
  uint32_t          SectorSizeBytes;
  uint32_t          BlockSizeBytes;
  float             VoltageNominal;  /* 1.8V or 3.0V / 3.3V */
  uint32_t          MaxClockFreqMHz;
  ExtMem_Mode_t     PreferredMode;
  uint8_t           DefaultReadDummyCycles;
  uint8_t           DefaultWriteDummyCycles;
} ExtMem_DeviceDescriptor_t;

/* Database of Supported Infineon, ISSI & Micron Devices */
static const ExtMem_DeviceDescriptor_t ExtMem_DeviceDatabase[] = {
  /* ========================================================================= */
  /* INFINEON SEMPER(TM) OCTAL NOR FLASH (S28HS / S28HL)                      */
  /* ========================================================================= */
  {
    .PartNumber              = "S28HS512T",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S28HL512T",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S28HS256T",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S28HL256T",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S28HS01GT",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1B,
    .CapacityBytes           = 128 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S28HS02GT",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_SEMPER,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1C,
    .CapacityBytes           = 256 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* INFINEON HYPERFLASH(TM) (S26KS / S26KL)                                   */
  /* ========================================================================= */
  {
    .PartNumber              = "S26KS512S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S26KL512S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 14,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S26KS256S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S26KS128S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* INFINEON HYPERRAM(TM) (S27KS / S27KL / S27HS / S27HL)                     */
  /* ========================================================================= */
  {
    .PartNumber              = "S27KS0641",
    .Type                    = EXTMEM_TYPE_HYPERRAM_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x01,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S27KL0641",
    .Type                    = EXTMEM_TYPE_HYPERRAM_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x01,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S27KS128",
    .Type                    = EXTMEM_TYPE_HYPERRAM_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x02,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S27KS256",
    .Type                    = EXTMEM_TYPE_HYPERRAM_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x03,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S27HS256",
    .Type                    = EXTMEM_TYPE_HYPERRAM_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x03,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* INFINEON SEMPER / FL QUAD SPI FLASH (S25HL / S25HS / S25FL)               */
  /* ========================================================================= */
  {
    .PartNumber              = "S25HL512T",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x2A,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S25HS512T",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_INFINEON,
    .ManufacturerID          = 0x34,
    .MemoryTypeID            = 0x2B,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "S25FL256L",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_INFINEON,
    .ManufacturerID          = 0x01,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 108,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI OCTAL NOR FLASH (IS25LX / IS25WX)                                    */
  /* ========================================================================= */
  /* ISSI OCTAL NOR FLASH (IS25LX / IS25WX - 1.8V & 3.0V)                     */
  /* ========================================================================= */
  {
    .PartNumber              = "IS25LX512",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WX512",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LX256",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WX256",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LX128",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WX128",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LX064",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WX064",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 20,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI HYPERFLASH(TM) (IS26KS / IS26KL - 1.8V & 3.0V)                      */
  /* ========================================================================= */
  {
    .PartNumber              = "IS26KS512S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS26KL512S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS26KS256S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS26KL256S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS26KS128S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS26KL128S",
    .Type                    = EXTMEM_TYPE_HYPERFLASH_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7E,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 512,
    .SectorSizeBytes         = 256 * 1024,
    .BlockSizeBytes          = 256 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI QUAD SPI NOR FLASH (IS25LP / IS25WP / IS25LE / IS25WE / IS25LQ / WQ) */
  /* ========================================================================= */
  {
    .PartNumber              = "IS25LP512M",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP512M",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP256D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP256D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP128F",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP128F",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP064D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP064D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP032D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP032D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP016D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x15,
    .CapacityBytes           = 2 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP016D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x15,
    .CapacityBytes           = 2 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LP080D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x60,
    .DensityID               = 0x14,
    .CapacityBytes           = 1 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WP080D",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x70,
    .DensityID               = 0x14,
    .CapacityBytes           = 1 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LQ032B",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x40,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 104,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WQ032",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x40,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 104,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25LE128",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7B,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS25WE128",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x7B,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI PARALLEL NOR FLASH VIA FMC (IS29GL - 16-BIT PARALLEL CFI)           */
  /* ========================================================================= */
  {
    .PartNumber              = "IS29GL512",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x22,
    .DensityID               = 0x23,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS29GL256",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x22,
    .DensityID               = 0x22,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS29GL128",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x22,
    .DensityID               = 0x21,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS29GL064",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x22,
    .DensityID               = 0x10,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 64 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS29GL032",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x22,
    .DensityID               = 0x1A,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 64 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI OCTAL PSRAM (IS66WVO / IS67WVO)                                      */
  /* ========================================================================= */
  {
    .PartNumber              = "IS66WVO32M8",
    .Type                    = EXTMEM_TYPE_PSRAM_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x07,
    .CapacityBytes           = 32 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 5
  },
  {
    .PartNumber              = "IS66WVO16M8",
    .Type                    = EXTMEM_TYPE_PSRAM_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x05,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 5
  },
  {
    .PartNumber              = "IS66WVO64M8",
    .Type                    = EXTMEM_TYPE_PSRAM_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x06,
    .CapacityBytes           = 64 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 5
  },
  {
    .PartNumber              = "IS66WVO8M8",
    .Type                    = EXTMEM_TYPE_PSRAM_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x04,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 5
  },
  {
    .PartNumber              = "IS67WVO8M8",
    .Type                    = EXTMEM_TYPE_PSRAM_OCTAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x04,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 5,
    .DefaultWriteDummyCycles = 5
  },

  /* ========================================================================= */
  /* ISSI HYPERRAM PSRAM (IS66WVH / IS67WVH)                                   */
  /* ========================================================================= */
  {
    .PartNumber              = "IS66WVH16M8",
    .Type                    = EXTMEM_TYPE_HYPERRAM_ISSI,
    .ManufacturerID          = 0x0F,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x02,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WVH8M8",
    .Type                    = EXTMEM_TYPE_HYPERRAM_ISSI,
    .ManufacturerID          = 0x0F,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x01,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 166,
    .PreferredMode           = EXTMEM_MODE_HYPERBUS,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI QUAD SPI PSRAM (IS66WVS / IS67WVS)                                   */
  /* ========================================================================= */
  {
    .PartNumber              = "IS66WVS1M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x00,
    .CapacityBytes           = 1 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WVS2M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x01,
    .CapacityBytes           = 2 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WVS4M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x02,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WVS8M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x03,
    .CapacityBytes           = 8 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WVS16M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x04,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS67WVS4M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x02,
    .CapacityBytes           = 4 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 104,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS67WVS16M8",
    .Type                    = EXTMEM_TYPE_PSRAM_QUAD_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x5D,
    .DensityID               = 0x04,
    .CapacityBytes           = 16 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 104,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 6,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* ISSI SERIAL SRAM (IS62WVS / IS65WVS)                                      */
  /* ========================================================================= */
  {
    .PartNumber              = "IS62WVS5128",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x62,
    .DensityID               = 0x04,
    .CapacityBytes           = 512 * 1024,         /* 4 Mbits = 512 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 45,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS62WVS2568",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x62,
    .DensityID               = 0x02,
    .CapacityBytes           = 256 * 1024,         /* 2 Mbits = 256 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 45,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS62WVS1288",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x62,
    .DensityID               = 0x01,
    .CapacityBytes           = 128 * 1024,         /* 1 Mbits = 128 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 45,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS62WVS0648",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x62,
    .DensityID               = 0x00,
    .CapacityBytes           = 64 * 1024,          /* 512 Kbits = 64 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 45,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS65WVS5128",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x65,
    .DensityID               = 0x04,
    .CapacityBytes           = 512 * 1024,         /* 4 Mbits = 512 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 30,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS65WVS2568",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x65,
    .DensityID               = 0x02,
    .CapacityBytes           = 256 * 1024,         /* 2 Mbits = 256 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 30,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS65WVS1288",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x65,
    .DensityID               = 0x01,
    .CapacityBytes           = 128 * 1024,         /* 1 Mbits = 128 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 30,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS65WVS0648",
    .Type                    = EXTMEM_TYPE_SRAM_SERIAL_ISSI,
    .ManufacturerID          = 0x9D,
    .MemoryTypeID            = 0x65,
    .DensityID               = 0x00,
    .CapacityBytes           = 64 * 1024,          /* 512 Kbits = 64 KBytes */
    .PageSizeBytes           = 32,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 30,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 2,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* PARALLEL ASYNCHRONOUS PSRAM / SRAM (FMC)                                  */
  /* ========================================================================= */
  {
    .PartNumber              = "IS66WV409616",
    .Type                    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 8 * 1024 * 1024,    /* 64 Mbits = 8 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WV204816",
    .Type                    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 4 * 1024 * 1024,    /* 32 Mbits = 4 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WV102416",
    .Type                    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 2 * 1024 * 1024,    /* 16 Mbits = 2 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "IS66WV51216",
    .Type                    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 1 * 1024 * 1024,    /* 8 Mbits = 1 MByte */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "CY62167EV30",
    .Type                    = EXTMEM_TYPE_PSRAM_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 2 * 1024 * 1024,
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 0,
    .BlockSizeBytes          = 0,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* MICRON XCCELA(TM) OCTAL NOR FLASH (MT35XU / MT35XL)                       */
  /* ========================================================================= */
  {
    .PartNumber              = "MT35XU02G",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1C,
    .CapacityBytes           = 256 * 1024 * 1024,  /* 2 Gbits = 256 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XU01GBBA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1B,
    .CapacityBytes           = 128 * 1024 * 1024,  /* 1 Gbit = 128 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XL01GBBA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x1B,
    .CapacityBytes           = 128 * 1024 * 1024,  /* 1 Gbit = 128 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XU512ABA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,   /* 512 Mbits = 64 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XL512ABA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x1A,
    .CapacityBytes           = 64 * 1024 * 1024,   /* 512 Mbits = 64 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XU256ABA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5B,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,   /* 256 Mbits = 32 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 200,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT35XL256ABA",
    .Type                    = EXTMEM_TYPE_NOR_OCTAL_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0x5A,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,   /* 256 Mbits = 32 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_OCTAL_DTR,
    .DefaultReadDummyCycles  = 16,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* MICRON MT25Q / N25Q QUAD SPI NOR FLASH (MT25QU / MT25QL)                  */
  /* ========================================================================= */
  {
    .PartNumber              = "MT25QU01GBBB",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x21,
    .CapacityBytes           = 128 * 1024 * 1024,  /* 1 Gbit = 128 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL01GBBB",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x21,
    .CapacityBytes           = 128 * 1024 * 1024,  /* 1 Gbit = 128 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QU512ABB",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x20,
    .CapacityBytes           = 64 * 1024 * 1024,   /* 512 Mbits = 64 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL512ABB",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x20,
    .CapacityBytes           = 64 * 1024 * 1024,   /* 512 Mbits = 64 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QU256ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,   /* 256 Mbits = 32 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL256ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x19,
    .CapacityBytes           = 32 * 1024 * 1024,   /* 256 Mbits = 32 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QU128ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,   /* 128 Mbits = 16 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL128ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x18,
    .CapacityBytes           = 16 * 1024 * 1024,   /* 128 Mbits = 16 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QU064ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,    /* 64 Mbits = 8 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL064ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x17,
    .CapacityBytes           = 8 * 1024 * 1024,    /* 64 Mbits = 8 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QU032ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBB,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,    /* 32 Mbits = 4 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 1.8f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT25QL032ABA",
    .Type                    = EXTMEM_TYPE_NOR_QUAD_MICRON,
    .ManufacturerID          = 0x2C,
    .MemoryTypeID            = 0xBA,
    .DensityID               = 0x16,
    .CapacityBytes           = 4 * 1024 * 1024,    /* 32 Mbits = 4 MBytes */
    .PageSizeBytes           = 256,
    .SectorSizeBytes         = 4 * 1024,
    .BlockSizeBytes          = 64 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 133,
    .PreferredMode           = EXTMEM_MODE_QUAD_1_4_4,
    .DefaultReadDummyCycles  = 10,
    .DefaultWriteDummyCycles = 0
  },

  /* ========================================================================= */
  /* MICRON MT28EW PARALLEL NOR FLASH (FMC BANK 1)                             */
  /* ========================================================================= */
  {
    .PartNumber              = "MT28EW01GABA",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 128 * 1024 * 1024,  /* 1 Gbit = 128 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT28EW512ABA",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 64 * 1024 * 1024,   /* 512 Mbits = 64 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT28EW256ABA",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 32 * 1024 * 1024,   /* 256 Mbits = 32 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  },
  {
    .PartNumber              = "MT28EW128ABA",
    .Type                    = EXTMEM_TYPE_NOR_PARALLEL_FMC,
    .ManufacturerID          = 0x00,
    .MemoryTypeID            = 0x00,
    .DensityID               = 0x00,
    .CapacityBytes           = 16 * 1024 * 1024,   /* 128 Mbits = 16 MBytes */
    .PageSizeBytes           = 0,
    .SectorSizeBytes         = 128 * 1024,
    .BlockSizeBytes          = 128 * 1024,
    .VoltageNominal          = 3.0f,
    .MaxClockFreqMHz         = 100,
    .PreferredMode           = EXTMEM_MODE_PARALLEL_16BIT,
    .DefaultReadDummyCycles  = 0,
    .DefaultWriteDummyCycles = 0
  }
};

#define EXTMEM_DEVICE_DATABASE_SIZE  (sizeof(ExtMem_DeviceDatabase) / sizeof(ExtMem_DeviceDatabase[0]))

/* Lookup helper with exact match priority */
static inline const ExtMem_DeviceDescriptor_t* ExtMem_FindDevice(uint8_t mfg, uint8_t memType, uint8_t density)
{
  /* Pass 1: Exact match on all three identification bytes */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (ExtMem_DeviceDatabase[i].ManufacturerID == mfg &&
        ExtMem_DeviceDatabase[i].MemoryTypeID == memType &&
        ExtMem_DeviceDatabase[i].DensityID == density)
    {
      return &ExtMem_DeviceDatabase[i];
    }
  }

  /* Pass 2: Wildcard match if manufacturer matches and generic descriptors exist */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (ExtMem_DeviceDatabase[i].ManufacturerID == mfg)
    {
      if (ExtMem_DeviceDatabase[i].MemoryTypeID == 0 || ExtMem_DeviceDatabase[i].MemoryTypeID == memType)
      {
        if (ExtMem_DeviceDatabase[i].DensityID == 0 || ExtMem_DeviceDatabase[i].DensityID == density)
        {
          return &ExtMem_DeviceDatabase[i];
        }
      }
    }
  }
  return NULL;
}

/* Lookup helper by exact or partial part number */
static inline const ExtMem_DeviceDescriptor_t* ExtMem_FindDeviceByPartNumber(const char *partNumber)
{
  if (partNumber == NULL) return NULL;

  /* Pass 1: Exact match */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (strcmp(ExtMem_DeviceDatabase[i].PartNumber, partNumber) == 0)
    {
      return &ExtMem_DeviceDatabase[i];
    }
  }

  /* Pass 2: Substring / prefix match */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (strstr(ExtMem_DeviceDatabase[i].PartNumber, partNumber) != NULL ||
        strstr(partNumber, ExtMem_DeviceDatabase[i].PartNumber) != NULL)
    {
      return &ExtMem_DeviceDatabase[i];
    }
  }
  return NULL;
}

/* Lookup helper by device type and explicit capacity in bytes */
static inline const ExtMem_DeviceDescriptor_t* ExtMem_FindDeviceByTypeAndCapacity(ExtMem_Type_t type, uint32_t capacityBytes)
{
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (ExtMem_DeviceDatabase[i].Type == type)
    {
      if (capacityBytes == 0 || ExtMem_DeviceDatabase[i].CapacityBytes == capacityBytes)
      {
        return &ExtMem_DeviceDatabase[i];
      }
    }
  }
  return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* STM32N6_EXTMEM_DEVICES_H */
