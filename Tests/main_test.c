/**
  ******************************************************************************
  * @file    main_test.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Automated Unit Test Suite Runner for STM32N6 External Memories
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 ALENOC.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "extmem_unit_tests.h"
#include "mock_hal.h"

#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_BOLD    "\033[1m"

static const TestCase_t s_test_cases[] = {
  { "SFDP Discovery Parser (JEDEC JESD216)",            test_sfdp_parser },
  { "Device Database Identification & Geometry",        test_device_database_consistency },
  { "Infineon SEMPER Octal NOR Flash (S28HS512T)",      test_infineon_s28hs512t_octal_flash },
  { "Infineon HyperFlash NOR Flash (S26KS512S)",        test_infineon_s26ks512s_hyperflash },
  { "Infineon HyperRAM PSRAM (S27KS0641)",              test_infineon_s27ks0641_hyperram },
  { "Infineon SEMPER/FL Quad NOR Flash (S25HL512T)",    test_infineon_s25hl512t_quad_flash },
  { "ISSI Octal NOR Flash (IS25LX256)",                 test_issi_is25lx256_octal_flash },
  { "ISSI Quad NOR Flash (IS25LP256)",                  test_issi_is25lp256_quad_flash },
  { "ISSI Octal PSRAM xSPI Profile 2.0 (IS66WVO32M8)",  test_issi_is66wvo32m8_octal_psram },
  { "ISSI HyperRAM PSRAM (IS66WVH16M8)",                test_issi_is66wvh16m8_hyperram },
  { "ISSI Quad SPI PSRAM (IS66WVS16M8)",                test_issi_is66wvs16m8_quad_psram },
  { "ISSI FMC 16-bit Parallel PSRAM (IS66WV51216)",     test_issi_is66wv_fmc_parallel_psram },
  { "ISSI FMC 16-bit Parallel NOR Flash (IS29GL512)",   test_issi_is29gl_fmc_parallel_nor_flash },
  { "ISSI Serial SRAM (IS62WVS / IS65WVS)",             test_issi_is62wvs_serial_sram },
  { "Micron Xccela Octal NOR Flash (MT35XU512ABA)",     test_micron_mt35xu512a_octal_flash },
  { "Micron Quad SPI NOR Flash (MT25QU512ABB)",         test_micron_mt25qu512a_quad_flash },
  { "STM32N6 ExtMem Manager: Every Database Device",    test_extmem_manager_all_devices },
  { "STM32N6 ExtMem Unified Manager & Auto-Detect",     test_extmem_manager_unified_autodetect },
  { "STM32N6 ExtMem Bus Routing, FMC Banks & Timings",    test_multi_density_shared_drivers },
  { "STM32N6 ExtMem Boundary Protection & Bounds Check",test_boundary_protection_and_bounds_checking },
  { "STM32N6 XSPI DEVSIZE Calculation (per RM0486)",    test_rm0486_devsize_register_calculation }
};

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  size_t total_tests = sizeof(s_test_cases) / sizeof(s_test_cases[0]);
  size_t passed_tests = 0;
  size_t failed_tests = 0;

  printf("\n");
  printf("====================================================================\n");
  printf("%s  STM32N6 External Memory Driver Suite - Host Unit Tests%s\n", COLOR_BOLD, COLOR_RESET);
  printf("  Target Architecture: STM32N6 (ARM Cortex-M55 + NPU)\n");
  printf("  Supported Peripherals: XSPI1, XSPI2, XSPI3, FMC\n");
  printf("====================================================================\n\n");
  printf("[INFO] Running %zu unit tests...\n\n", total_tests);

  for (size_t i = 0; i < total_tests; i++)
  {
    printf("[ RUN      ] [%2zu/%2zu] %s\n", i + 1, total_tests, s_test_cases[i].TestName);
    fflush(stdout);

    MockHAL_ClearAssertViolations();
    bool result = s_test_cases[i].TestFunc();
    if (result && MockHAL_GetAssertViolations() != 0U)
    {
      printf("       [FAIL] %lu HAL parameter check violation(s), last: %s\r\n",
             (unsigned long)MockHAL_GetAssertViolations(), MockHAL_GetLastViolation());
      result = false;
    }

    if (result)
    {
      printf("%s[       OK ]%s [%2zu/%2zu] %s\n", COLOR_GREEN, COLOR_RESET, i + 1, total_tests, s_test_cases[i].TestName);
      passed_tests++;
    }
    else
    {
      printf("%s[  FAILED  ]%s [%2zu/%2zu] %s\n", COLOR_RED, COLOR_RESET, i + 1, total_tests, s_test_cases[i].TestName);
      failed_tests++;
    }
  }

  printf("\n");
  printf("====================================================================\n");
  printf("%sTest Results Summary:%s\n", COLOR_BOLD, COLOR_RESET);
  printf("  Total:   %zu\n", total_tests);
  printf("  Passed:  %s%zu%s\n", COLOR_GREEN, passed_tests, COLOR_RESET);
  if (failed_tests > 0)
  {
    printf("  Failed:  %s%zu%s\n", COLOR_RED, failed_tests, COLOR_RESET);
  }
  else
  {
    printf("  Failed:  0\n");
  }
  printf("====================================================================\n");

  if (failed_tests == 0)
  {
    printf("%s>>> ALL TESTS PASSED SUCCESSFULLY! <<<%s\n\n", COLOR_GREEN, COLOR_RESET);
    return 0;
  }
  else
  {
    printf("%s>>> SOME TESTS FAILED! <<<%s\n\n", COLOR_RED, COLOR_RESET);
    return 1;
  }
}
