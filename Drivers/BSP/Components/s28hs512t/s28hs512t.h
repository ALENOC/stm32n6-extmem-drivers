/**
  ******************************************************************************
  * @file    s28hs512t.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon SEMPER(TM) Octal NOR Flash
  *          Compatible with S28HS512T, S28HL512T, S28HS256T, S28HL256T,
  *          S28HS01GT, S28HL01GT, S28HS02GT.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef S28HS512T_H
#define S28HS512T_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

/** @defgroup S28HS512T_Exported_Constants
  * @{
  */
#define S28HS512T_FLASH_SIZE             (64U * 1024U * 1024U) /* 512 Mbits = 64 MBytes */
#define S28HS512T_PAGE_SIZE              256U                  /* 256 or 512 Bytes */
#define S28HS512T_SECTOR_4K              (4U * 1024U)          /* 4 KBytes */
#define S28HS512T_BLOCK_256K             (256U * 1024U)        /* 256 KBytes */

#define S28HS512T_OK                     (0)
#define S28HS512T_ERROR                  (-1)
#define S28HS512T_TIMEOUT                (-3)

/* Device Identification */
#define S28HS_MANUFACTURER_ID            0x34U
#define S28HS_DEVICE_ID_512MB            0x5BU

/* Registers Addresses (Volatile, single die: base 0x00800000) */
#define S28HS_REG_STATUS1_V              0x00800000U /* Status Register 1 Volatile        */
#define S28HS_REG_CFR1_V                 0x00800002U /* Configuration Register 1 Volatile */
#define S28HS_REG_CFR2_V                 0x00800003U /* Configuration Register 2 Volatile */
#define S28HS_REG_CFR3_V                 0x00800004U /* Configuration Register 3 Volatile */
#define S28HS_REG_CFR4_V                 0x00800005U /* Configuration Register 4 Volatile */
#define S28HS_REG_CFR5_V                 0x00800006U /* Configuration Register 5 Volatile */
#define S28HS_REG_STATUS1                S28HS_REG_STATUS1_V

/* CFR2V Bit Fields */
#define S28HS_CFR2V_MEMLAT_MASK          0x0FU     /* Memory array read latency code */
#define S28HS_CFR2V_MEMLAT_24_CYCLES     0x0BU     /* 24 dummy cycles, valid up to 200 MHz in 8D-8D-8D */
#define S28HS_CFR2V_ADRBYT_4BYTE         (1U << 7) /* 4-byte address mode */

/* CFR5V Bit Fields */
#define S28HS_CFR5V_OPI_ENABLE           (1U << 0) /* Octal interface enable  */
#define S28HS_CFR5V_DDR_ENABLE           (1U << 1) /* Double data rate enable */
#define S28HS_CFR5V_RESERVED_BIT6        (1U << 6) /* Must be written as 1    */
#define S28HS_CFR5V_OCTAL_DTR            (S28HS_CFR5V_RESERVED_BIT6 | S28HS_CFR5V_DDR_ENABLE | S28HS_CFR5V_OPI_ENABLE)
#define S28HS_CFR5V_SPI                  (S28HS_CFR5V_RESERVED_BIT6)

/* Dummy cycles used for 8D-8D-8D memory array reads (matches MEMLAT = 0xB) */
#define S28HS_OCTAL_DTR_READ_DUMMY       24U
/* Dummy cycles used for 8D-8D-8D Read Any Register */
#define S28HS_OCTAL_DTR_REG_DUMMY        8U

/* Status Register 1 Masks */
#define S28HS_SR1_WIP                    (1U << 0) /* Write in Progress */
#define S28HS_SR1_WEL                    (1U << 1) /* Write Enable Latch */
#define S28HS_SR1_PRG_ERR                (1U << 5) /* Program Error */
#define S28HS_SR1_ERS_ERR                (1U << 6) /* Erase Error */

/* Commands (1-1-1 Single SPI) */
#define S28HS_CMD_READ_ID                0x9FU
#define S28HS_CMD_READ_SFDP              0x5AU
#define S28HS_CMD_READ_FAST_4B           0x0CU
#define S28HS_CMD_PAGE_PROG_4B           0x12U
#define S28HS_CMD_SECTOR_ERASE_4K_4B     0x21U
#define S28HS_CMD_BLOCK_ERASE_256K_4B    0xDCU
#define S28HS_CMD_CHIP_ERASE             0x60U
#define S28HS_CMD_WRITE_ENABLE           0x06U
#define S28HS_CMD_WRITE_DISABLE          0x04U
#define S28HS_CMD_READ_STATUS1           0x05U
#define S28HS_CMD_ENTER_4BYTE_ADDR       0xB7U
#define S28HS_CMD_READ_REG               0x65U /* Read Any Register */
#define S28HS_CMD_WRITE_REG              0x71U /* Write Any Register */
#define S28HS_CMD_RESET_ENABLE           0x66U
#define S28HS_CMD_RESET                  0x99U
#define S28HS_CMD_ENTER_DEEP_POWER_DOWN  0xB9U

/* Commands (8D-8D-8D Octal DTR - 16-bit Opcode where 2nd byte is bitwise inverse) */
#define S28HS_DTR_CMD_READ               0xEE11U
#define S28HS_DTR_CMD_PAGE_PROG          0x12EDU
#define S28HS_DTR_CMD_SECTOR_ERASE_4K    0x21DEU
#define S28HS_DTR_CMD_BLOCK_ERASE_256K   0xDC23U
#define S28HS_DTR_CMD_CHIP_ERASE         0x609FU
#define S28HS_DTR_CMD_WRITE_ENABLE       0x06F9U
#define S28HS_DTR_CMD_WRITE_DISABLE      0x04FBU
#define S28HS_DTR_CMD_READ_REG           0x659AU
#define S28HS_DTR_CMD_WRITE_REG          0x718EU
#define S28HS_DTR_CMD_READ_STATUS        0x05FAU

/**
  * @}
  */

/* Exported Types ------------------------------------------------------------*/
typedef struct {
  uint32_t FlashSize;
  uint32_t PageSize;
  uint32_t Sector4KSize;
  uint32_t Block256KSize;
  uint8_t  ManufacturerID;
  uint8_t  DeviceID;
} S28HS512T_Info_t;

/* Exported Functions --------------------------------------------------------*/
int32_t S28HS512T_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t S28HS512T_WriteEnable(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t S28HS512T_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout);
int32_t S28HS512T_ReadAnyReg(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t *pValue);
int32_t S28HS512T_WriteAnyReg(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t Value);
int32_t S28HS512T_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t S28HS512T_ExitOctalDTRMode(XSPI_HandleTypeDef *Ctx);
int32_t S28HS512T_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t S28HS512T_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t S28HS512T_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t S28HS512T_EraseBlock256K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t S28HS512T_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t S28HS512T_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t S28HS512T_EnterDeepPowerDown(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t S28HS512T_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx);
int32_t S28HS512T_Reset(XSPI_HandleTypeDef *Ctx);
int32_t S28HS512T_GetInfo(S28HS512T_Info_t *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* S28HS512T_H */
