/**
  ******************************************************************************
  * @file    extmem_unit_tests.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Unit test definitions for all Infineon, ISSI & Micron memory drivers.
  ******************************************************************************
  */

#ifndef EXTMEM_UNIT_TESTS_H
#define EXTMEM_UNIT_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct {
  const char *TestName;
  bool (*TestFunc)(void);
} TestCase_t;

/* Unit Test Declarations */
bool test_sfdp_parser(void);
bool test_device_database_consistency(void);
bool test_infineon_s28hs512t_octal_flash(void);
bool test_infineon_s26ks512s_hyperflash(void);
bool test_infineon_s27ks0641_hyperram(void);
bool test_infineon_s25hl512t_quad_flash(void);
bool test_issi_is25lx256_octal_flash(void);
bool test_issi_is25lp256_quad_flash(void);
bool test_issi_is66wvo32m8_octal_psram(void);
bool test_issi_is66wvh16m8_hyperram(void);
bool test_issi_is66wvs16m8_quad_psram(void);
bool test_issi_is66wv_fmc_parallel_psram(void);
bool test_issi_is29gl_fmc_parallel_nor_flash(void);
bool test_issi_is62wvs_serial_sram(void);
bool test_micron_mt35xu512a_octal_flash(void);
bool test_micron_mt25qu512a_quad_flash(void);
bool test_extmem_manager_all_devices(void);
bool test_extmem_manager_unified_autodetect(void);
bool test_multi_density_shared_drivers(void);
bool test_boundary_protection_and_bounds_checking(void);
bool test_rm0486_devsize_register_calculation(void);

#ifdef __cplusplus
}
#endif

#endif /* EXTMEM_UNIT_TESTS_H */
