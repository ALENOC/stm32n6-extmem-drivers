/**
  ******************************************************************************
  * @file    stm32n6xx_hal.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Shim header providing HAL definitions for host unit testing.
  ******************************************************************************
  */

#ifndef STM32N6XX_HAL_H
#define STM32N6XX_HAL_H

#ifdef EXTMEM_UNIT_TEST
#include "mock_hal.h"
#else
#error "When building on target hardware, include your STM32CubeN6 HAL drivers."
#endif

#endif /* STM32N6XX_HAL_H */
