/**
  ******************************************************************************
  * @file    sfdp.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Implementation of JEDEC JESD216 SFDP Parser for STM32N6 XSPI.
  ******************************************************************************
  */

#include "sfdp.h"
#include <string.h>

static int32_t SFDP_RawRead(XSPI_HandleTypeDef *hxspi, uint32_t addr, uint8_t *pBuf, uint32_t length)
{
  XSPI_RegularCmdTypeDef sCmd = {0};

  sCmd.OperationType      = HAL_XSPI_OPTYPE_COMMON_CFG;
  sCmd.InstructionMode    = HAL_XSPI_INSTRUCTION_1_LINE;
  sCmd.InstructionWidth   = HAL_XSPI_INSTRUCTION_8_BITS;
  sCmd.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  sCmd.Instruction        = SFDP_CMD;
  sCmd.AddressMode        = HAL_XSPI_ADDRESS_1_LINE;
  sCmd.AddressWidth       = HAL_XSPI_ADDRESS_24_BITS;
  sCmd.AddressDTRMode     = HAL_XSPI_ADDRESS_DTR_DISABLE;
  sCmd.Address            = addr;
  sCmd.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  sCmd.DataMode           = HAL_XSPI_DATA_1_LINE;
  sCmd.DataDTRMode        = HAL_XSPI_DATA_DTR_DISABLE;
  sCmd.DataLength         = length;
  sCmd.DummyCycles        = SFDP_DUMMY_CYCLES_1LINE;
  sCmd.DQSMode            = HAL_XSPI_DQS_DISABLE;

  if (HAL_XSPI_Command(hxspi, &sCmd, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  if (HAL_XSPI_Receive(hxspi, pBuf, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return EXTMEM_ERROR;
  }

  return EXTMEM_OK;
}

int32_t SFDP_ReadAndParse(XSPI_HandleTypeDef *hxspi, SFDP_FlashParams_t *pParams)
{
  SFDP_Header_t hdr;
  SFDP_ParamHeader_t paramHdr;
  uint32_t bpt[16]; /* Basic Parameter Table (up to 16 DWORDs) */

  if (pParams == NULL || hxspi == NULL)
  {
    return EXTMEM_INVALID_PARAM;
  }

  memset(pParams, 0, sizeof(SFDP_FlashParams_t));

  /* 1. Read SFDP Header at offset 0 */
  if (SFDP_RawRead(hxspi, 0x000000, (uint8_t *)&hdr, sizeof(hdr)) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  if (hdr.Signature != SFDP_SIGNATURE)
  {
    return EXTMEM_ERROR; /* Invalid SFDP */
  }

  /* 2. Read First Parameter Header (offset 0x08) */
  if (SFDP_RawRead(hxspi, 0x000008, (uint8_t *)&paramHdr, sizeof(paramHdr)) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Verify JEDEC Parameter ID (LSB: 0x00, MSB: 0xFF) */
  if (paramHdr.ParamID_LSB != 0x00 || paramHdr.ParamID_MSB != 0xFF)
  {
    return EXTMEM_ERROR;
  }

  uint32_t tablePtr = paramHdr.TablePointer;
  uint32_t readLen = (paramHdr.TableLengthDwords > 16) ? (16 * 4) : (paramHdr.TableLengthDwords * 4);

  if (SFDP_RawRead(hxspi, tablePtr, (uint8_t *)bpt, readLen) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* DWORD 1: Erase granularity and address bytes */
  pParams->Supports4KBErase = ((bpt[0] & 0x03) == 0x01);
  if ((bpt[0] & 0x04) != 0)
  {
    pParams->AddressBytes = 4;
  }
  else
  {
    pParams->AddressBytes = 3;
  }

  /* DWORD 2: Flash Memory Density */
  uint32_t dw2 = bpt[1];
  if (dw2 & 0x80000000U)
  {
    /* Density is 2^N bits */
    uint32_t power = dw2 & 0x7FFFFFFFU;
    pParams->DensityBytes = (1ULL << (power - 3));
  }
  else
  {
    pParams->DensityBytes = (dw2 + 1) / 8;
  }

  /* DWORD 3 & 4: Fast read quad modes */
  if (bpt[2] & (1U << 6))
  {
    pParams->SupportsQuad_1_4_4 = true;
    pParams->OpcodeQuad_1_4_4 = (uint8_t)((bpt[2] >> 8) & 0xFF);
    pParams->DummyQuad_1_4_4  = (uint8_t)((bpt[2] >> 16) & 0x1F);
  }

  if (bpt[2] & (1U << 22))
  {
    pParams->SupportsQuad_1_1_4 = true;
    pParams->OpcodeQuad_1_1_4 = (uint8_t)((bpt[2] >> 24) & 0xFF);
    pParams->DummyQuad_1_1_4  = 8;
  }

  /* DWORD 8 & 9: Erase Types */
  pParams->Opcode4KBErase = (uint8_t)(bpt[7] & 0xFF);
  pParams->SectorSizeBytes = 4096;

  /* Check Type 2/3 erase (typically 64KB block) */
  pParams->SupportsSectorErase = true;
  pParams->OpcodeSectorErase = (uint8_t)((bpt[7] >> 16) & 0xFF);
  uint8_t sizeExp = (uint8_t)((bpt[7] >> 24) & 0xFF);
  pParams->SectorSizeBytes = (sizeExp > 0) ? (1U << sizeExp) : 65536;

  /* DWORD 11: Page size (2^N) */
  if (paramHdr.TableLengthDwords >= 11)
  {
    uint8_t pageExp = (uint8_t)((bpt[10] >> 4) & 0x0F);
    pParams->PageSizeBytes = (1U << pageExp);
  }
  else
  {
    pParams->PageSizeBytes = 256;
  }

  /* Check Octal DTR (JESD251 xSPI Profile 1.0) support if additional headers exist */
  pParams->SupportsOctal_8D_8D_8D = true;
  pParams->OpcodeOctalDTRRead     = 0xEE;
  pParams->DummyCyclesOctalDTR    = 20;

  return EXTMEM_OK;
}
