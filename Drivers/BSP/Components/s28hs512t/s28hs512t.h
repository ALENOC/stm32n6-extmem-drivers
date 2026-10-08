/**
  ******************************************************************************
  * @file    s28hs512t.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Infineon SEMPER(TM) Octal NOR Flash
  *          Compatible with S28HS512T, S28HL512T, S28HS256T, S28HL256T,
  *          S28HS01GT, S28HL01GT and the dual-die S28HS02GT / S28HL02GT
  *          (datasheet 002-23755: per-die registers at die base + 0x800000,
  *          see S28HS512T_SetDieLayout).
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

/* Volatile register map: base 0x00800000 on a single die; each die of a stacked part has its own
 * base, given by the SFDP SCCR / SCCR multi-chip tables (S28HS512T_SetDieLayout) */
#define S28HS_REG_VOLATILE_BASE          0x00800000U
#define S28HS_REG_OFS_STATUS1            0x0U
#define S28HS_REG_OFS_CFR2               0x3U
#define S28HS_REG_OFS_CFR3               0x4U
#define S28HS_REG_OFS_CFR5               0x6U
#define S28HS_MAX_DICE                   4U

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
/* CFR3V Bit Fields */
#define S28HS_CFR3V_VRGLAT_MASK          (3U << 6) /* Volatile register read latency */
#define S28HS_CFR3V_VRGLAT_CODE_11       (3U << 6) /* 6 cycles in 8D-8D-8D, valid up to 200 MHz */

/* Dummy cycles for 8D-8D-8D volatile register reads (matches VRGLAT = 11) */
#define S28HS_OCTAL_DTR_REG_DUMMY        6U

/* Status Register 1 Masks */
#define S28HS_SR1_WIP                    (1U << 0) /* Write in Progress (RDYBSY) */
#define S28HS_SR1_WEL                    (1U << 1) /* Write Enable Latch */
#define S28HS_SR1_ERS_ERR                (1U << 5) /* Erase Error (ERSERR)      */
#define S28HS_SR1_PRG_ERR                (1U << 6) /* Program Error (PRGERR)    */

/* Commands (1-1-1 Single SPI) */
#define S28HS_CMD_READ_ID                0x9FU
#define S28HS_CMD_READ_SFDP              0x5AU
#define S28HS_CMD_READ_FAST              0x0BU /* RDAY2_C_0: current address length (4 bytes after EN4BA); 0Ch is not supported */
#define S28HS_CMD_PAGE_PROG_4B           0x12U
#define S28HS_CMD_SECTOR_ERASE_4K_4B     0x21U
#define S28HS_CMD_BLOCK_ERASE_256K_4B    0xDCU
#define S28HS_CMD_CHIP_ERASE             0x60U
#define S28HS_CMD_DIE_ERASE              0x61U /* Erase one die of a stacked-die part */
#define S28HS_CMD_WRITE_ENABLE           0x06U
#define S28HS_CMD_WRITE_DISABLE          0x04U
#define S28HS_CMD_READ_STATUS1           0x05U
#define S28HS_CMD_ENTER_4BYTE_ADDR       0xB7U
#define S28HS_CMD_READ_REG               0x65U /* Read Any Register */
#define S28HS_CMD_WRITE_REG              0x71U /* Write Any Register */
#define S28HS_CMD_CLEAR_ERRORS           0x82U /* Clear Program and Erase Failure Flags (CLPEF) */
#define S28HS_CMD_RESET_ENABLE           0x66U
#define S28HS_CMD_RESET                  0x99U
#define S28HS_CMD_ENTER_DEEP_POWER_DOWN  0xB9U

/* Commands (8D-8D-8D Octal DTR): 16-bit instruction whose second byte repeats the opcode.
 * Datasheet 002-23755 (2 Gb SEMPER Octal): SFDP BFPT DWORD-18 bits 30:29 = 00b "command extension
 * is the same as the command", and the 8D transaction table sends the opcode on both CK edges. */
#define S28HS_DTR_CMD_READ               0xEEEEU
#define S28HS_DTR_CMD_PAGE_PROG          0x1212U
#define S28HS_DTR_CMD_SECTOR_ERASE_4K    0x2121U
#define S28HS_DTR_CMD_BLOCK_ERASE_256K   0xDCDCU
#define S28HS_DTR_CMD_CHIP_ERASE         0x6060U
#define S28HS_DTR_CMD_DIE_ERASE          0x6161U
#define S28HS_DTR_CMD_WRITE_ENABLE       0x0606U
#define S28HS_DTR_CMD_WRITE_DISABLE      0x0404U
#define S28HS_DTR_CMD_READ_REG           0x6565U
#define S28HS_DTR_CMD_WRITE_REG          0x7171U
#define S28HS_DTR_CMD_READ_STATUS        0x0505U
#define S28HS_DTR_CMD_CLEAR_ERRORS       0x8282U
#define S28HS_DTR_CMD_RESET_ENABLE       0x6666U
#define S28HS_DTR_CMD_RESET              0x9999U
#define S28HS_DTR_CMD_ENTER_DEEP_POWER_DOWN 0xB9B9U

/* Maximum embedded operation times (datasheet 002-23755, Table 86), used as polling timeouts */
#define S28HS_TIMEOUT_PAGE_PROG_MS       10U         /* tPP  max 2.175 ms                       */
#define S28HS_TIMEOUT_ERASE_4K_MS        400U        /* tSE  max 335 ms (4 KB sector)           */
#define S28HS_TIMEOUT_ERASE_256K_MS      6000U       /* tSE  max 5869 ms (256 KB, endurance flex) */
#define S28HS_TIMEOUT_CHIP_ERASE_MS      2800000U    /* tBE  max 2762 s                         */

/* Deep power down timings (datasheet 002-23755, Table 86) */
#define S28HS_DPD_ENTER_MS               1U          /* tENTDPD max 3 us                        */
#define S28HS_DPD_EXIT_MS                1U          /* tEXTDPD max 430 us                      */

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

/* Stacked-die layout */
typedef struct {
  uint8_t  Dice;                          /*!< Number of dice (1 for monolithic parts)       */
  uint32_t DieSize;                       /*!< Bytes per die                                 */
  uint32_t VregBase[S28HS_MAX_DICE];      /*!< Volatile register base address of each die    */
} S28HS512T_DieLayout_t;

/* Exported Functions --------------------------------------------------------*/
int32_t S28HS512T_SetDieLayout(XSPI_HandleTypeDef *Ctx, const S28HS512T_DieLayout_t *pLayout);
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
int32_t S28HS512T_LeaveDeepPowerDown(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t S28HS512T_Reset(XSPI_HandleTypeDef *Ctx);
int32_t S28HS512T_GetInfo(S28HS512T_Info_t *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* S28HS512T_H */
