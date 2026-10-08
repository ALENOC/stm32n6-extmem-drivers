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
  uint8_t           DieCount;        /* Stacked dice sharing one chip select (0 or 1: single die) */
} ExtMem_DeviceDescriptor_t;

/* Database of Supported Infineon, ISSI & Micron Devices (stm32n6_extmem_devices.c) */
extern const ExtMem_DeviceDescriptor_t ExtMem_DeviceDatabase[];
extern const size_t ExtMem_DeviceDatabaseSize;
#define EXTMEM_DEVICE_DATABASE_SIZE  (ExtMem_DeviceDatabaseSize)

/* True for memory types identified through the SPI 0x9F READ ID command */
static inline bool ExtMem_TypeAnswersJedecId(ExtMem_Type_t type)
{
  switch (type)
  {
    case EXTMEM_TYPE_NOR_OCTAL_SEMPER:
    case EXTMEM_TYPE_NOR_OCTAL_ISSI:
    case EXTMEM_TYPE_NOR_OCTAL_MICRON:
    case EXTMEM_TYPE_NOR_QUAD_INFINEON:
    case EXTMEM_TYPE_NOR_QUAD_ISSI:
    case EXTMEM_TYPE_NOR_QUAD_MICRON:
    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
      return true;
    default:
      return false;
  }
}

/* True for volatile memories (no erase, direct writes) */
static inline bool ExtMem_TypeIsVolatile(ExtMem_Type_t type)
{
  switch (type)
  {
    case EXTMEM_TYPE_HYPERRAM_INFINEON:
    case EXTMEM_TYPE_HYPERRAM_ISSI:
    case EXTMEM_TYPE_PSRAM_OCTAL_ISSI:
    case EXTMEM_TYPE_PSRAM_QUAD_ISSI:
    case EXTMEM_TYPE_SRAM_SERIAL_ISSI:
    case EXTMEM_TYPE_PSRAM_PARALLEL_FMC:
      return true;
    default:
      return false;
  }
}

/* Lookup helper with exact match priority */
static inline const ExtMem_DeviceDescriptor_t* ExtMem_FindDevice(uint8_t mfg, uint8_t memType, uint8_t density)
{
  /* Pass 1: Exact match on all three identification bytes */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (ExtMem_TypeAnswersJedecId(ExtMem_DeviceDatabase[i].Type) &&
        ExtMem_DeviceDatabase[i].ManufacturerID == mfg &&
        ExtMem_DeviceDatabase[i].MemoryTypeID == memType &&
        ExtMem_DeviceDatabase[i].DensityID == density)
    {
      return &ExtMem_DeviceDatabase[i];
    }
  }

  /* Pass 2: Wildcard match if manufacturer matches and generic descriptors exist.
   * Only devices that answer the SPI 0x9F READ ID command take part: HyperBus, octal PSRAM,
   * serial SRAM and FMC parts are identified by other means and must never match here. */
  for (size_t i = 0; i < EXTMEM_DEVICE_DATABASE_SIZE; i++)
  {
    if (!ExtMem_TypeAnswersJedecId(ExtMem_DeviceDatabase[i].Type))
    {
      continue;
    }
    if (ExtMem_DeviceDatabase[i].ManufacturerID == mfg)
    {
      if (ExtMem_DeviceDatabase[i].MemoryTypeID == 0U || ExtMem_DeviceDatabase[i].MemoryTypeID == memType)
      {
        if (ExtMem_DeviceDatabase[i].DensityID == 0U || ExtMem_DeviceDatabase[i].DensityID == density)
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
  if (partNumber == NULL) { return NULL; }

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
      if (capacityBytes == 0U || ExtMem_DeviceDatabase[i].CapacityBytes == capacityBytes)
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
