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

#define SFDP_PARAM_ID_BFPT            0xFF00U
#define SFDP_PARAM_ID_SCCR            0xFF87U /* Status, Control and Configuration Register map        */
#define SFDP_PARAM_ID_SCCR_MC         0xFF88U /* SCCR map offsets for multi-chip (stacked die) devices */
#define SFDP_MAX_DICE                 4U

/* Function Prototypes -------------------------------------------------------*/
/**
  * @brief  Reads and decodes the JEDEC Basic Flash Parameter Table.
  * @retval EXTMEM_OK on success, EXTMEM_NOT_SUPPORTED when the device has no valid SFDP table,
  *         EXTMEM_ERROR when the XSPI transfer failed, EXTMEM_INVALID_PARAM for NULL arguments.
  */
int32_t SFDP_ReadAndParse(XSPI_HandleTypeDef *hxspi, SFDP_FlashParams_t *pParams);

/**
  * @brief  Reads the volatile register base address of every die of a stacked-die flash from the
  *         SCCR (die 0) and SCCR multi-chip (other dice) parameter tables, as JESD216 defines them.
  * @param  pVregBase  Array of SFDP_MAX_DICE entries filled with the per-die base addresses.
  * @param  pDice      Number of dice described (1 when the device has no multi-chip table).
  * @retval EXTMEM_OK, EXTMEM_NOT_SUPPORTED when the tables are missing, EXTMEM_ERROR on transfer errors.
  */
int32_t SFDP_ReadDieRegisterMap(XSPI_HandleTypeDef *hxspi, uint32_t *pVregBase, uint8_t *pDice);

#ifdef __cplusplus
}
#endif

#endif /* SFDP_H */
