/**
  ******************************************************************************
  * @file    mock_hal.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Mock Hardware Abstraction Layer implementation for host unit testing.
  ******************************************************************************
  */

#include "mock_hal.h"
#include <stdio.h>
#include <stdlib.h>

#define MOCK_MEM_BUFFER_SIZE  (256 * 1024) /* 256 KB virtual array */
static uint8_t s_MemoryBuffer[MOCK_MEM_BUFFER_SIZE];

static SCB_Type s_SCB = { .CCR = SCB_CCR_DC_Msk };
SCB_Type *SCB = &s_SCB;

/* Mock Device Registers */
static uint8_t  s_MfgID       = 0x34; /* Default: Infineon */
static uint8_t  s_MemTypeID   = 0x5B; /* Default: SEMPER Octal */
static uint8_t  s_DensityID   = 0x1A; /* 512Mb */

static uint16_t s_HyperID0    = 0x0001; /* Infineon HyperRAM */
static uint16_t s_HyperID1    = 0x0000;
static uint16_t s_HyperCR0    = 0x0000;
static uint16_t s_HyperCR1    = 0x0000;

static uint8_t  s_StatusReg1  = 0x00;
static uint8_t  s_ConfigReg1  = 0x00;
static uint8_t  s_CFR2V       = 0x00;
static uint8_t  s_CFR3V       = 0x00;
static uint8_t  s_VCR[8]      = {0};
static uint8_t  s_MR[16]      = {0};

static XSPI_RegularCmdTypeDef  s_LastCmd = {0};
static XSPI_HyperbusCmdTypeDef s_LastHyperCmd = {0};
static bool                    s_IsHyperbus = false;
static bool                    s_HyperFlashSectorErasePending = false;

/* Mock Control API */
void MockHAL_Reset(void)
{
  memset(s_MemoryBuffer, 0xFF, MOCK_MEM_BUFFER_SIZE);
  s_MfgID      = 0x34;
  s_MemTypeID  = 0x5B;
  s_DensityID  = 0x1A;
  s_HyperID0   = 0x0001;
  s_HyperID1   = 0x0000;
  s_HyperCR0   = 0x0000;
  s_HyperCR1   = 0x0000;
  s_StatusReg1 = 0x00;
  s_ConfigReg1 = 0x00;
  s_CFR2V      = 0x00;
  s_CFR3V      = 0x00;
  memset(s_VCR, 0, sizeof(s_VCR));
  memset(s_MR, 0, sizeof(s_MR));
  s_IsHyperbus = false;
  s_HyperFlashSectorErasePending = false;
}

void MockHAL_SetEmulatedChip(uint8_t mfg, uint8_t memType, uint8_t density)
{
  s_MfgID     = mfg;
  s_MemTypeID = memType;
  s_DensityID = density;
}

void MockHAL_SetHyperBusID(uint16_t id0, uint16_t id1)
{
  s_HyperID0 = id0;
  s_HyperID1 = id1;
}

uint8_t* MockHAL_GetMemoryBuffer(void)
{
  return s_MemoryBuffer;
}

uint32_t MockHAL_GetMemoryBufferSize(void)
{
  return MOCK_MEM_BUFFER_SIZE;
}

/* Mock HAL Functions */
HAL_StatusTypeDef HAL_XSPI_Init(XSPI_HandleTypeDef *hxspi)
{
  (void)hxspi;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_DeInit(XSPI_HandleTypeDef *hxspi)
{
  (void)hxspi;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Command(XSPI_HandleTypeDef *hxspi, const XSPI_RegularCmdTypeDef *pCmd, uint32_t Timeout)
{
  (void)hxspi;
  (void)Timeout;
  s_IsHyperbus = false;
  s_LastCmd = *pCmd;

  /* Handle immediate commands */
  if (pCmd->Instruction == 0x06 || pCmd->Instruction == 0x06F9) /* Write Enable */
  {
    s_StatusReg1 |= 0x02; /* WEL */
  }
  else if (pCmd->Instruction == 0x04 || pCmd->Instruction == 0x04FB) /* Write Disable */
  {
    s_StatusReg1 &= ~0x02;
  }
  else if (pCmd->Instruction == 0x20 || pCmd->Instruction == 0x21 || pCmd->Instruction == 0x21DE) /* Sector Erase 4KB */
  {
    uint32_t offset = pCmd->Address % MOCK_MEM_BUFFER_SIZE;
    uint32_t len = 4096;
    if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
    memset(&s_MemoryBuffer[offset], 0xFF, len);
    s_StatusReg1 |= 0x01; /* WIP */
  }
  else if (pCmd->Instruction == 0xD8 || pCmd->Instruction == 0xDC || pCmd->Instruction == 0xDC23) /* Block Erase */
  {
    uint32_t offset = pCmd->Address % MOCK_MEM_BUFFER_SIZE;
    uint32_t len = 65536;
    if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
    memset(&s_MemoryBuffer[offset], 0xFF, len);
    s_StatusReg1 |= 0x01; /* WIP */
  }
  else if (pCmd->Instruction == 0x60 || pCmd->Instruction == 0xC7 || pCmd->Instruction == 0x609F) /* Chip Erase */
  {
    memset(s_MemoryBuffer, 0xFF, MOCK_MEM_BUFFER_SIZE);
    s_StatusReg1 |= 0x01; /* WIP */
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Transmit(XSPI_HandleTypeDef *hxspi, const uint8_t *pData, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;

  if (s_IsHyperbus)
  {
    if (s_LastHyperCmd.AddressSpace == HAL_XSPI_REGISTER_ADDRESS_SPACE)
    {
      uint16_t val = (uint16_t)(pData[0] | ((uint16_t)pData[1] << 8));
      if (s_LastHyperCmd.Address == 0x00001000) s_HyperCR0 = val;
      else if (s_LastHyperCmd.Address == 0x00001002) s_HyperCR1 = val;
    }
    else
    {
      uint16_t wordVal = (s_LastHyperCmd.DataLength >= 2) ? (uint16_t)(pData[0] | ((uint16_t)pData[1] << 8)) : pData[0];

      if (wordVal == 0x0080)
      {
        s_HyperFlashSectorErasePending = true;
        return HAL_OK;
      }
      else if (s_HyperFlashSectorErasePending && wordVal == 0x0030)
      {
        uint32_t offset = s_LastHyperCmd.Address % MOCK_MEM_BUFFER_SIZE;
        uint32_t secStart = (offset / 4096) * 4096;
        uint32_t len = 65536;
        if (secStart + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - secStart;
        memset(&s_MemoryBuffer[secStart], 0xFF, len);
        s_HyperFlashSectorErasePending = false;
        return HAL_OK;
      }
      else if (wordVal == 0x00AA || wordVal == 0x0055 || wordVal == 0x00A0 || wordVal == 0x0070 || wordVal == 0x0071 || wordVal == 0x00F0)
      {
        return HAL_OK;
      }
      else
      {
        uint32_t offset = s_LastHyperCmd.Address % MOCK_MEM_BUFFER_SIZE;
        uint32_t len = s_LastHyperCmd.DataLength;
        if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
        memcpy(&s_MemoryBuffer[offset], pData, len);
      }
    }
    return HAL_OK;
  }

  /* Regular SPI / Octal / Quad Transmit */
  uint32_t cmd = s_LastCmd.Instruction;

  if (cmd == 0x71 || cmd == 0x718E) /* Write Any Register (SEMPER) */
  {
    if (s_LastCmd.Address == 0x00800003) s_CFR2V = pData[0];
    else if (s_LastCmd.Address == 0x00800004) s_CFR3V = pData[0];
  }
  else if (cmd == 0x81 || cmd == 0x817E) /* Write VCR (ISSI) */
  {
    if (s_LastCmd.Address < sizeof(s_VCR)) s_VCR[s_LastCmd.Address] = pData[0];
  }
  else if (cmd == 0xC0) /* Write Mode Register (PSRAM) */
  {
    if (s_LastCmd.Address < sizeof(s_MR)) s_MR[s_LastCmd.Address] = pData[0];
  }
  else if (cmd == 0x01) /* Write Status / Config */
  {
    s_StatusReg1 = pData[0];
    if (s_LastCmd.DataLength > 1) s_ConfigReg1 = pData[1];
  }
  else /* Page Program / Write */
  {
    uint32_t offset = s_LastCmd.Address % MOCK_MEM_BUFFER_SIZE;
    uint32_t len = s_LastCmd.DataLength;
    if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
    memcpy(&s_MemoryBuffer[offset], pData, len);
    s_StatusReg1 |= 0x01; /* Set WIP */
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Receive(XSPI_HandleTypeDef *hxspi, uint8_t *pData, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;

  if (s_IsHyperbus)
  {
    if (s_LastHyperCmd.AddressSpace == HAL_XSPI_REGISTER_ADDRESS_SPACE)
    {
      uint16_t val = 0;
      if (s_LastHyperCmd.Address == 0x00000000) val = s_HyperID0;
      else if (s_LastHyperCmd.Address == 0x00000002) val = s_HyperID1;
      else if (s_LastHyperCmd.Address == 0x00001000) val = s_HyperCR0;
      else if (s_LastHyperCmd.Address == 0x00001002) val = s_HyperCR1;

      pData[0] = (uint8_t)(val & 0xFF);
      pData[1] = (uint8_t)((val >> 8) & 0xFF);
    }
    else
    {
      /* Check HyperFlash Status register query */
      if (s_LastHyperCmd.Address == 0x00000000 && s_LastHyperCmd.DataLength == 2)
      {
        /* Return Device Ready (0x0080) */
        pData[0] = 0x80;
        pData[1] = 0x00;
      }
      else
      {
        uint32_t offset = s_LastHyperCmd.Address % MOCK_MEM_BUFFER_SIZE;
        uint32_t len = s_LastHyperCmd.DataLength;
        if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
        memcpy(pData, &s_MemoryBuffer[offset], len);
      }
    }
    return HAL_OK;
  }

  /* Regular SPI / Octal / Quad Receive */
  uint32_t cmd = s_LastCmd.Instruction;

  if (cmd == 0x9F) /* Read ID */
  {
    pData[0] = s_MfgID;
    pData[1] = s_MemTypeID;
    pData[2] = s_DensityID;
  }
  else if (cmd == 0x5A) /* Read SFDP */
  {
    /* Emulate JESD216 SFDP Table */
    memset(pData, 0, s_LastCmd.DataLength);
    if (s_LastCmd.Address == 0x00)
    {
      /* Header: "SFDP", v1.6, 1 parameter header */
      pData[0] = 'S'; pData[1] = 'F'; pData[2] = 'D'; pData[3] = 'P';
      pData[4] = 0x06; pData[5] = 0x01; pData[6] = 0x00; pData[7] = 0xFF;
    }
    else if (s_LastCmd.Address == 0x08)
    {
      /* Parameter Header: JEDEC ID (0xFF00), 16 DWORDS at offset 0x30 */
      pData[0] = 0x00; pData[1] = 0x06; pData[2] = 0x01; pData[3] = 16;
      pData[4] = 0x30; pData[5] = 0x00; pData[6] = 0x00; pData[7] = 0xFF;
    }
    else if (s_LastCmd.Address == 0x30)
    {
      /* Basic Parameter Table DWORDs */
      uint32_t *dw = (uint32_t *)pData;
      dw[0] = 0x00000005; /* 4KB erase supported, 3 or 4-byte address */
      dw[1] = (64 * 1024 * 1024 * 8) - 1; /* 512 Mbits density */
      dw[2] = 0x00000040; /* 1-4-4 Fast read supported */
      dw[7] = 0x00200000 | 0x20; /* 4KB erase opcode 0x20 */
      dw[10] = 0x00000080; /* Page size 256B (2^8) */
    }
  }
  else if (cmd == 0x05 || cmd == 0x05FA) /* Read Status Register 1 */
  {
    pData[0] = s_StatusReg1;
  }
  else if (cmd == 0x35) /* Read Config 1 / Function Register */
  {
    pData[0] = s_ConfigReg1;
  }
  else if (cmd == 0x65 || cmd == 0x659A) /* Read Any Register (SEMPER) */
  {
    if (s_LastCmd.Address == 0x00800003) pData[0] = s_CFR2V;
    else if (s_LastCmd.Address == 0x00800004) pData[0] = s_CFR3V;
    else pData[0] = 0x00;
  }
  else if (cmd == 0x85 || cmd == 0x857A) /* Read VCR (ISSI) */
  {
    if (s_LastCmd.Address < sizeof(s_VCR)) pData[0] = s_VCR[s_LastCmd.Address];
    else pData[0] = 0x00;
  }
  else if (cmd == 0x40) /* Read Mode Register (PSRAM) */
  {
    if (s_LastCmd.Address < sizeof(s_MR)) pData[0] = s_MR[s_LastCmd.Address];
    else pData[0] = 0x00;
  }
  else /* Array Read */
  {
    uint32_t offset = s_LastCmd.Address % MOCK_MEM_BUFFER_SIZE;
    uint32_t len = s_LastCmd.DataLength;
    if (offset + len > MOCK_MEM_BUFFER_SIZE) len = MOCK_MEM_BUFFER_SIZE - offset;
    memcpy(pData, &s_MemoryBuffer[offset], len);
  }

  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Transmit_DMA(XSPI_HandleTypeDef *hxspi, const uint8_t *pData)
{
  return HAL_XSPI_Transmit(hxspi, pData, 1000);
}

HAL_StatusTypeDef HAL_XSPI_Receive_DMA(XSPI_HandleTypeDef *hxspi, uint8_t *pData)
{
  return HAL_XSPI_Receive(hxspi, pData, 1000);
}

HAL_StatusTypeDef HAL_XSPI_AutoPolling(XSPI_HandleTypeDef *hxspi, const XSPI_AutoPollingTypeDef *pCfg, uint32_t Timeout)
{
  (void)hxspi; (void)pCfg; (void)Timeout;
  /* Automatically clear WIP bit to simulate completion */
  s_StatusReg1 &= ~0x01;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_MemoryMapped(XSPI_HandleTypeDef *hxspi, const XSPI_MemoryMappedTypeDef *pCfg)
{
  (void)hxspi; (void)pCfg;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_Abort(XSPI_HandleTypeDef *hxspi)
{
  (void)hxspi;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_HyperbusCfg(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCfgTypeDef *pCfg, uint32_t Timeout)
{
  (void)hxspi; (void)pCfg; (void)Timeout;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_XSPI_HyperbusCmd(XSPI_HandleTypeDef *hxspi, const XSPI_HyperbusCmdTypeDef *pCmd, uint32_t Timeout)
{
  (void)hxspi; (void)Timeout;
  s_IsHyperbus = true;
  s_LastHyperCmd = *pCmd;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_SRAM_Init(SRAM_HandleTypeDef *hsram, const FMC_NORSRAM_TimingTypeDef *pTiming, const FMC_NORSRAM_TimingTypeDef *pExtTiming)
{
  (void)hsram; (void)pTiming; (void)pExtTiming;
  return HAL_OK;
}

HAL_StatusTypeDef HAL_SRAM_DeInit(SRAM_HandleTypeDef *hsram)
{
  (void)hsram;
  return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
  static uint32_t tick = 0;
  return ++tick;
}

void HAL_Delay(uint32_t Delay)
{
  (void)Delay;
}

void HAL_PWREx_EnableVddIO2(void) {}
void HAL_PWREx_ConfigVddIORange(uint32_t Domain, uint32_t Range) {
  (void)Domain; (void)Range;
}
