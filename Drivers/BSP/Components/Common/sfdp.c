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
    return EXTMEM_NOT_SUPPORTED; /* No SFDP table */
  }

  /* 2. Read First Parameter Header (offset 0x08) */
  if (SFDP_RawRead(hxspi, 0x000008, (uint8_t *)&paramHdr, sizeof(paramHdr)) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Verify JEDEC Parameter ID (LSB: 0x00, MSB: 0xFF) */
  if (paramHdr.ParamID_LSB != 0x00 || paramHdr.ParamID_MSB != 0xFF)
  {
    return EXTMEM_NOT_SUPPORTED;
  }

  uint32_t tablePtr = paramHdr.TablePointer;
  uint32_t readLen = (paramHdr.TableLengthDwords > 16) ? (16 * 4) : (paramHdr.TableLengthDwords * 4);

  /* JESD216 requires at least 9 DWORDs in the Basic Flash Parameter Table */
  if (paramHdr.TableLengthDwords < 9U)
  {
    return EXTMEM_NOT_SUPPORTED;
  }

  if (SFDP_RawRead(hxspi, tablePtr, (uint8_t *)bpt, readLen) != EXTMEM_OK)
  {
    return EXTMEM_ERROR;
  }

  /* Never trust fields beyond the advertised table length */
  if (readLen < sizeof(bpt))
  {
    memset((uint8_t *)bpt + readLen, 0, sizeof(bpt) - readLen);
  }

  /* DWORD 1 (JESD216 BFPT) */
  pParams->Supports4KBErase = ((bpt[0] & 0x03U) == 0x01U);
  pParams->Opcode4KBErase   = (uint8_t)((bpt[0] >> 8) & 0xFFU);
  /* Bits [18:17]: 00 = 3-byte only, 01 = 3 or 4-byte, 10 = 4-byte only */
  pParams->AddressBytes = (((bpt[0] >> 17) & 0x03U) == 0x00U) ? 3U : 4U;

  /* DWORD 2: Flash Memory Density */
  uint32_t dw2 = bpt[1];
  if (dw2 & 0x80000000U)
  {
    /* Density is 2^N bits; anything at or above 2^35 bits does not fit in 32 bits of bytes */
    uint32_t power = dw2 & 0x7FFFFFFFU;
    pParams->DensityBytes = ((power >= 3U) && (power < 35U)) ? (uint32_t)(1ULL << (power - 3U)) : 0U;
  }
  else
  {
    pParams->DensityBytes = (uint32_t)(((uint64_t)dw2 + 1ULL) / 8ULL);
  }

  /* DWORD 1 bit 21 = 1-4-4 supported, bit 22 = 1-1-4 supported.
   * DWORD 3: [4:0] 1-4-4 wait states, [7:5] 1-4-4 mode clocks, [15:8] 1-4-4 opcode,
   *          [20:16] 1-1-4 wait states, [23:21] 1-1-4 mode clocks, [31:24] 1-1-4 opcode */
  if (bpt[0] & (1U << 21))
  {
    pParams->SupportsQuad_1_4_4 = true;
    pParams->OpcodeQuad_1_4_4   = (uint8_t)((bpt[2] >> 8) & 0xFFU);
    pParams->DummyQuad_1_4_4    = (uint8_t)((bpt[2] & 0x1FU) + ((bpt[2] >> 5) & 0x07U));
  }

  if (bpt[0] & (1U << 22))
  {
    pParams->SupportsQuad_1_1_4 = true;
    pParams->OpcodeQuad_1_1_4   = (uint8_t)((bpt[2] >> 24) & 0xFFU);
    pParams->DummyQuad_1_1_4    = (uint8_t)(((bpt[2] >> 16) & 0x1FU) + ((bpt[2] >> 21) & 0x07U));
  }

  /* DWORD 8: Erase Type 1 size [7:0] / opcode [15:8], Erase Type 2 size [23:16] / opcode [31:24].
   * Report the largest erase type below 1 MB as the block erase. */
  pParams->SectorSizeBytes     = 0;
  pParams->SupportsSectorErase = false;
  for (uint32_t i = 0; i < 4U; i++)
  {
    uint32_t dw    = (i < 2U) ? bpt[7] : bpt[8];
    uint32_t shift = (i & 1U) ? 16U : 0U;
    uint8_t  sizeExp = (uint8_t)((dw >> shift) & 0xFFU);
    uint8_t  opcode  = (uint8_t)((dw >> (shift + 8U)) & 0xFFU);
    if ((sizeExp != 0U) && (sizeExp < 20U) && ((1UL << sizeExp) > pParams->SectorSizeBytes))
    {
      pParams->SupportsSectorErase = true;
      pParams->OpcodeSectorErase   = opcode;
      pParams->SectorSizeBytes     = (1UL << sizeExp);
    }
  }
  if (!pParams->SupportsSectorErase)
  {
    pParams->SectorSizeBytes = 65536U;
  }

  /* DWORD 11: Page size (2^N) */
  if (paramHdr.TableLengthDwords >= 11U)
  {
    uint8_t pageExp = (uint8_t)((bpt[10] >> 4) & 0x0FU);
    pParams->PageSizeBytes = (1U << pageExp);
  }
  else
  {
    pParams->PageSizeBytes = 256;
  }

  /* 8D-8D-8D support is advertised in the xSPI Profile 1.0 table, which is not parsed here.
   * Report it as unsupported so a quad-only part is never driven with octal commands. */
  pParams->SupportsOctal_8D_8D_8D = false;
  pParams->OpcodeOctalDTRRead     = 0;
  pParams->DummyCyclesOctalDTR    = 0;

  return EXTMEM_OK;
}
