/**
  ******************************************************************************
  * @file    stm32n6_extmem_conf.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Configuration header for STM32N6 External Memory Driver Suite.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef STM32N6_EXTMEM_CONF_H
#define STM32N6_EXTMEM_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration Defines -----------------------------------------------------*/

/**
  * @brief Default XSPI Bus instance to use if not specified.
  *        Can be 1 (XSPI1), 2 (XSPI2), or 3 (XSPI3).
  */
#ifndef EXTMEM_DEFAULT_XSPI_INSTANCE
#define EXTMEM_DEFAULT_XSPI_INSTANCE         1
#endif

/**
  * @brief I/O Voltage Domain for External Memory
  *        0 = 3.3V / 3.0V (PWR_VDDIO_RANGE_3V3)
  *        1 = 1.8V        (PWR_VDDIO_RANGE_1V8) - Required for >133MHz Octal/HyperBus
  */
#ifndef EXTMEM_USE_1V8_IO
#define EXTMEM_USE_1V8_IO                    1
#endif

/**
  * @brief Enable SFDP (JESD216) automated probe during initialization
  */
#ifndef EXTMEM_ENABLE_SFDP_PROBE
#define EXTMEM_ENABLE_SFDP_PROBE             1
#endif

/**
  * @brief XSPI clock divider applied to the XSPI kernel clock (ExtMem_Config_t.ClockPrescaler = 0 selects it).
  *        The memory clock is Fkernel / divider, e.g. with a 400 MHz XSPI kernel clock:
  *        Divider = 2 -> 200 MHz
  *        Divider = 3 -> 133 MHz
  *        Divider = 4 -> 100 MHz
  *        The driver programs DCR2.PRESCALER = divider - 1 (RM0486: Fclk = Fkernel / (PRESCALER + 1)).
  */
#ifndef EXTMEM_DEFAULT_CLOCK_PRESCALER
#define EXTMEM_DEFAULT_CLOCK_PRESCALER       2
#endif

/**
  * @brief Longest time CS# may stay LOW on PSRAM / HyperRAM / OctalRAM (tCSM / tCEM), in ns.
  *        1 us covers every supported part up to 105/125 degC (4 us is allowed up to 85 degC).
  *        The XSPI refresh counter releases CS# before this limit so the memory can refresh.
  */
#ifndef EXTMEM_PSRAM_MAX_CS_LOW_NS
#define EXTMEM_PSRAM_MAX_CS_LOW_NS           1000U
#endif

/**
  * @brief XSPI bus clock assumed for the refresh counter when the RCC cannot report the kernel clock.
  */
#ifndef EXTMEM_XSPI_FALLBACK_BUS_CLOCK_HZ
#define EXTMEM_XSPI_FALLBACK_BUS_CLOCK_HZ    200000000U
#endif

/**
  * @brief Maximum Timeouts (in milliseconds)
  */
#define EXTMEM_DEFAULT_TIMEOUT_MS            5000U
#define EXTMEM_CHIP_ERASE_TIMEOUT_MS         300000U /* 5 minutes */

/**
  * @brief Cortex-M55 D-Cache Management
  *        Set to 1 to automatically invalidate / clean D-Cache around memory mapped and DMA transfers.
  */
#ifndef EXTMEM_ENABLE_DCACHE_MAINTENANCE
#define EXTMEM_ENABLE_DCACHE_MAINTENANCE     1
#endif

/**
  * @brief Memory Mapped Base Address
  *        On STM32N6:
  *        XSPI1 Memory-Mapped Base: 0x90000000
  *        XSPI2 Memory-Mapped Base: 0x70000000
  *        XSPI3 Memory-Mapped Base: 0x80000000
  *        FMC Bank 1 Base:         0x60000000
  */
#define EXTMEM_XSPI1_BASE_ADDR               0x90000000U
#define EXTMEM_XSPI2_BASE_ADDR               0x70000000U
#define EXTMEM_XSPI3_BASE_ADDR               0x80000000U
#define EXTMEM_FMC_BANK1_BASE_ADDR           0x60000000U

#ifdef __cplusplus
}
#endif

#endif /* STM32N6_EXTMEM_CONF_H */
