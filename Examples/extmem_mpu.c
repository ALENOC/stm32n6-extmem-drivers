/**
  ******************************************************************************
  * @file    extmem_mpu.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Example MPU configuration (ARMv8.1-M PMSAv8) for external memory
  *          windows: a read-only, executable, cacheable region for a Flash on
  *          XSPI2 and a read-write, non-executable, cacheable region for a RAM
  *          on XSPI1. Base addresses and sizes must match your board.
  *          This file is compile-checked against the STM32CubeN6 HAL
  *          (make target-check); it has not been run on hardware.
  ******************************************************************************
  */

#include "stm32n6xx_hal.h"

#define EXTMEM_MPU_FLASH_BASE  0x70000000U          /* XSPI2 memory-mapped window */
#define EXTMEM_MPU_FLASH_SIZE  (128U * 1024U * 1024U)
#define EXTMEM_MPU_RAM_BASE    0x90000000U          /* XSPI1 memory-mapped window */
#define EXTMEM_MPU_RAM_SIZE    (32U * 1024U * 1024U)

void ExtMem_MPU_Config(void)
{
  MPU_Attributes_InitTypeDef attr   = {0};
  MPU_Region_InitTypeDef     region = {0};

  HAL_MPU_Disable();

  /* Attribute 0: normal memory, write-back, read allocate (Flash, read only) */
  attr.Number     = MPU_ATTRIBUTES_NUMBER0;
  attr.Attributes = INNER_OUTER(MPU_WRITE_BACK | MPU_NON_TRANSIENT | MPU_R_ALLOCATE);
  HAL_MPU_ConfigMemoryAttributes(&attr);

  /* Attribute 1: normal memory, write-back, read and write allocate (RAM) */
  attr.Number     = MPU_ATTRIBUTES_NUMBER1;
  attr.Attributes = INNER_OUTER(MPU_WRITE_BACK | MPU_NON_TRANSIENT | MPU_RW_ALLOCATE);
  HAL_MPU_ConfigMemoryAttributes(&attr);

  /* External Flash: read only, executable (XIP) */
  region.Enable           = MPU_REGION_ENABLE;
  region.Number           = MPU_REGION_NUMBER0;
  region.AttributesIndex  = MPU_ATTRIBUTES_NUMBER0;
  region.BaseAddress      = EXTMEM_MPU_FLASH_BASE;
  region.LimitAddress     = EXTMEM_MPU_FLASH_BASE + EXTMEM_MPU_FLASH_SIZE - 1U;
  region.AccessPermission = MPU_REGION_ALL_RO;
  region.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  region.DisablePrivExec  = MPU_PRIV_INSTRUCTION_ACCESS_ENABLE;
  region.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&region);

  /* External RAM: read write, not executable */
  region.Number           = MPU_REGION_NUMBER1;
  region.AttributesIndex  = MPU_ATTRIBUTES_NUMBER1;
  region.BaseAddress      = EXTMEM_MPU_RAM_BASE;
  region.LimitAddress     = EXTMEM_MPU_RAM_BASE + EXTMEM_MPU_RAM_SIZE - 1U;
  region.AccessPermission = MPU_REGION_ALL_RW;
  region.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  region.DisablePrivExec  = MPU_PRIV_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&region);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  SCB_EnableICache();
  SCB_EnableDCache();
}
