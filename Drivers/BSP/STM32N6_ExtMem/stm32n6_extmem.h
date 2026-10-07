/**
  ******************************************************************************
  * @file    stm32n6_extmem.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Unified High-Level External Memory Driver Suite for STM32N6.
  *          Provides seamless initialization, auto-detection, memory mapping (XIP),
  *          read, write, erase, and power management for all Infineon, ISSI & Micron
  *          Flash and PSRAM devices.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef STM32N6_EXTMEM_H
#define STM32N6_EXTMEM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6_extmem_conf.h"
#include "stm32n6_extmem_devices.h"
#include "stm32n6xx_hal.h"

/* Component Headers */
#include "../Components/s28hs512t/s28hs512t.h"
#include "../Components/s26ks512s/s26ks512s.h"
#include "../Components/s27ks0641/s27ks0641.h"
#include "../Components/s25hl512t/s25hl512t.h"
#include "../Components/is25lx256/is25lx256.h"
#include "../Components/is25lp256/is25lp256.h"
#include "../Components/is66wvo32m8/is66wvo32m8.h"
#include "../Components/is66wvh16m8/is66wvh16m8.h"
#include "../Components/is66wvs16m8/is66wvs16m8.h"
#include "../Components/is62wvs/is62wvs.h"
#include "../Components/is66wv_fmc/is66wv_fmc.h"
#include "../Components/is29gl_fmc/is29gl_fmc.h"
#include "../Components/mt35xu512a/mt35xu512a.h"
#include "../Components/mt25qu512a/mt25qu512a.h"
#include "../Components/Common/sfdp.h"

/* Configuration Structure ---------------------------------------------------*/
typedef struct {
  ExtMem_Bus_t            Bus;                 /*!< Target Bus: XSPI1, XSPI2, XSPI3, or FMC   */
  uint32_t                ClockPrescaler;      /*!< Clock prescaler (2 for 200MHz, 3 for 133) */
  bool                    Force1V8;            /*!< Force 1.8V VDDIO domain configuration     */
  ExtMem_Type_t           ForcedDeviceType;    /*!< Optional: EXTMEM_TYPE_UNKNOWN for auto   */
  ExtMem_Mode_t           DesiredMode;         /*!< Desired protocol mode (e.g. OCTAL_DTR)    */
  uint32_t                IOPort;              /*!< Optional XSPIM Port: 0 (auto), 1, or 2    */
} ExtMem_Config_t;

/* Master External Memory Handle ---------------------------------------------*/
typedef struct {
  ExtMem_Config_t         Config;              /*!< User configuration parameters             */
  ExtMem_State_t          State;               /*!< Current driver state                      */
  ExtMem_Mode_t           ActiveMode;          /*!< Current active communication mode         */
  ExtMem_Geometry_t       Geometry;            /*!< Detected/configured geometry details      */
  const ExtMem_DeviceDescriptor_t *pDevice;    /*!< Matched database device entry             */
  uint32_t                MemoryMappedBase;    /*!< Base pointer for memory-mapped XIP        */

  /* Underlying Hardware Peripheral Handles */
  XSPI_HandleTypeDef      hxspi;               /*!< STM32N6 HAL XSPI handle                   */
  SRAM_HandleTypeDef      hsram;               /*!< STM32N6 HAL FMC SRAM handle               */

  /* Runtime Info */
  uint8_t                 RawID[8];            /*!< Raw device ID read during probe           */
  uint8_t                 DummyCycles;         /*!< Active read dummy cycles                  */
} ExtMem_HandleTypeDef;

/* High-Level API Functions --------------------------------------------------*/

/**
  * @brief  Initialize the external memory subsystem and configure pins/clocks.
  * @param  hextmem Pointer to ExtMem handle with Config filled in.
  * @retval EXTMEM_OK on success, or negative error code.
  */
int32_t ExtMem_Init(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  De-initialize the external memory interface.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_DeInit(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Auto-detect and probe the connected memory (Infineon, ISSI, or SFDP).
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on successful detection.
  */
int32_t ExtMem_AutoDetect(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Enable Memory-Mapped (Execute-In-Place / XIP) mode.
  *         Allows direct CPU read access at the base address (e.g. 0x90000000).
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_EnableMemoryMapped(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Disable Memory-Mapped mode and return to indirect command mode.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_DisableMemoryMapped(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Read data from external memory in indirect mode.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  Address Byte address to read from.
  * @param  pData   Buffer to receive read data.
  * @param  Size    Number of bytes to read.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_Read(ExtMem_HandleTypeDef *hextmem, uint32_t Address, uint8_t *pData, uint32_t Size);

/**
  * @brief  Write data to external memory. Handles Flash page splitting and PSRAM burst.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  Address Byte address to write to.
  * @param  pData   Buffer with data to write.
  * @param  Size    Number of bytes to write.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_Write(ExtMem_HandleTypeDef *hextmem, uint32_t Address, const uint8_t *pData, uint32_t Size);

/**
  * @brief  Read data using DMA.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  Address Byte address.
  * @param  pData   Destination buffer.
  * @param  Size    Byte count.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_ReadDMA(ExtMem_HandleTypeDef *hextmem, uint32_t Address, uint8_t *pData, uint32_t Size);

/**
  * @brief  Write data using DMA.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  Address Byte address.
  * @param  pData   Source buffer.
  * @param  Size    Byte count.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_WriteDMA(ExtMem_HandleTypeDef *hextmem, uint32_t Address, const uint8_t *pData, uint32_t Size);

/**
  * @brief  Erase a small sector (typically 4KB) in Flash memory.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  SectorAddress Byte address aligned to sector boundary.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_EraseSector(ExtMem_HandleTypeDef *hextmem, uint32_t SectorAddress);

/**
  * @brief  Erase a standard block (typically 64KB or 256KB) in Flash memory.
  * @param  hextmem Pointer to ExtMem handle.
  * @param  BlockAddress Byte address aligned to block boundary.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_EraseBlock(ExtMem_HandleTypeDef *hextmem, uint32_t BlockAddress);

/**
  * @brief  Erase the entire Flash chip.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_EraseChip(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Put the memory into ultra-low power Deep Power Down mode.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_EnterDeepPowerDown(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Wake up the memory from Deep Power Down mode.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_LeaveDeepPowerDown(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Perform software reset of the memory device.
  * @param  hextmem Pointer to ExtMem handle.
  * @retval EXTMEM_OK on success.
  */
int32_t ExtMem_Reset(ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Returns null-terminated device part number string.
  */
const char* ExtMem_GetDeviceName(const ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Returns memory total capacity in bytes.
  */
uint32_t ExtMem_GetCapacity(const ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Returns true if the memory is Non-Volatile Flash.
  */
bool ExtMem_IsFlash(const ExtMem_HandleTypeDef *hextmem);

/**
  * @brief  Returns true if the memory is Volatile RAM (PSRAM or HyperRAM).
  */
bool ExtMem_IsRAM(const ExtMem_HandleTypeDef *hextmem);

#ifdef __cplusplus
}
#endif

#endif /* STM32N6_EXTMEM_H */
