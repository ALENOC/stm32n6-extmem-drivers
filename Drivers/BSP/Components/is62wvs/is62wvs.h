/**
  ******************************************************************************
  * @file    is62wvs.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI Serial SRAM (IS62WVS / IS65WVS series).
  *          Supports IS62WVS0648 (512Kb), IS62WVS1288 (1Mb), IS62WVS2568 (2Mb),
  *          IS62WVS5128 (4Mb) and automotive IS65WVS counterparts.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS62WVS_H
#define IS62WVS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

/* Density Definitions -------------------------------------------------------*/
#define IS62WVS_SRAM_SIZE_512KBIT        (64U * 1024U)         /* 512 Kbits = 64 KBytes  */
#define IS62WVS_SRAM_SIZE_1MBIT          (128U * 1024U)        /* 1 Mbits  = 128 KBytes  */
#define IS62WVS_SRAM_SIZE_2MBIT          (256U * 1024U)        /* 2 Mbits  = 256 KBytes  */
#define IS62WVS_SRAM_SIZE_4MBIT          (512U * 1024U)        /* 4 Mbits  = 512 KBytes  */

/* Return Codes --------------------------------------------------------------*/
#define IS62WVS_DIE_SIZE                 (256U * 1024U)  /* 2 Mbit die (IS62WVS5128 = 2 dice) */

#define IS62WVS_OK                       (0)
#define IS62WVS_ERROR                    (-1)
#define IS62WVS_TIMEOUT                  (-3)

/* Commands ------------------------------------------------------------------*/
#define IS62WVS_CMD_READ                 0x03U   /*!< Read Memory Array (Byte/Sequential)  */
#define IS62WVS_CMD_FAST_READ            0x0BU   /*!< Fast Read with Dummy Cycles          */
#define IS62WVS_CMD_WRITE                0x02U   /*!< Write Memory Array (Byte/Sequential) */
#define IS62WVS_CMD_RDMR                 0x05U   /*!< Read Mode Register                   */
#define IS62WVS_CMD_WRMR                 0x01U   /*!< Write Mode Register                  */
#define IS62WVS_CMD_ENTER_QUAD           0x38U   /*!< Enter Serial Quad Interface (SQI)    */
#define IS62WVS_CMD_EXIT_QUAD            0xFFU   /*!< Reset / Exit SQI to SPI Mode         */
#define IS62WVS_CMD_ENTER_DUAL           0x3BU   /*!< Enter Serial Dual Interface (SDI)    */

/* Mode Register Bits --------------------------------------------------------*/
#define IS62WVS_MODE_BYTE                0x00U   /*!< Byte Operation Mode                  */
#define IS62WVS_MODE_PAGE                0x80U   /*!< Page Operation Mode (32-byte page)   */
#define IS62WVS_MODE_SEQUENTIAL          0x40U   /*!< Sequential Mode (continuous burst)   */
#define IS62WVS_MODE_MASK                0xC0U   /*!< Mode Bitmask                         */

/* Exported Functions --------------------------------------------------------*/
int32_t IS62WVS_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t IS62WVS_ReadModeRegister(XSPI_HandleTypeDef *Ctx, uint8_t *pMode);
int32_t IS62WVS_WriteModeRegister(XSPI_HandleTypeDef *Ctx, uint8_t Mode);
int32_t IS62WVS_EnterQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t IS62WVS_ExitQuadMode(XSPI_HandleTypeDef *Ctx);
int32_t IS62WVS_Read(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size);
int32_t IS62WVS_Write(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS62WVS_ReadQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t IS62WVS_WriteQuad(XSPI_HandleTypeDef *Ctx, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS62WVS_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS62WVS_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS62WVS_H */
