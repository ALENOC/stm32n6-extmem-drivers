/**
  ******************************************************************************
  * @file    sfdp.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   JEDEC JESD216 Serial Flash Discoverable Parameter (SFDP) Parser.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef SFDP_H
#define SFDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extmem_common.h"
#include "stm32n6xx_hal.h"

/* SFDP Signatures & Defines -------------------------------------------------*/
#define SFDP_SIGNATURE                0x50444653U /* "SFDP" in little endian */
#define SFDP_CMD                      0x5AU
#define SFDP_DUMMY_CYCLES_1LINE       8U

/* SFDP Header structure */
typedef struct {
  uint32_t Signature;       /*!< Must be "SFDP" (0x50444653) */
  uint8_t  MinorRev;
  uint8_t  MajorRev;
  uint8_t  NumParameterHdrs;/*!< 0-based: 0 means 1 header */
  uint8_t  AccessProtocol;
} __attribute__((packed)) SFDP_Header_t;

/* SFDP Parameter Header */
typedef struct {
  uint8_t  ParamID_LSB;     /*!< 0x00 = JEDEC Basic Flash Parameter Table */
  uint8_t  MinorRev;
  uint8_t  MajorRev;
  uint8_t  TableLengthDwords;
  uint32_t TablePointer:24;
  uint32_t ParamID_MSB:8;   /*!< 0xFF = JEDEC */
} __attribute__((packed)) SFDP_ParamHeader_t;

/* Extracted SFDP Flash Capabilities */
typedef struct {
  uint32_t DensityBytes;
  uint32_t PageSizeBytes;
  uint8_t  AddressBytes;           /* 3 or 4 */
  bool     Supports4KBErase;
  uint8_t  Opcode4KBErase;
  bool     SupportsSectorErase;
  uint8_t  OpcodeSectorErase;
  uint32_t SectorSizeBytes;
  bool     SupportsQuad_1_1_4;
  uint8_t  OpcodeQuad_1_1_4;
  uint8_t  DummyQuad_1_1_4;
  bool     SupportsQuad_1_4_4;
  uint8_t  OpcodeQuad_1_4_4;
  uint8_t  DummyQuad_1_4_4;
  bool     SupportsOctal_8D_8D_8D; /* JEDEC Profile 1.0 (xSPI) */
  uint8_t  OpcodeOctalDTRRead;
  uint8_t  DummyCyclesOctalDTR;
} SFDP_FlashParams_t;

/* Function Prototypes -------------------------------------------------------*/
int32_t SFDP_ReadAndParse(XSPI_HandleTypeDef *hxspi, SFDP_FlashParams_t *pParams);

#ifdef __cplusplus
}
#endif

#endif /* SFDP_H */
