/**
  ******************************************************************************
  * @file    test_common.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Assertion and protocol checking helpers shared by the unit tests.
  ******************************************************************************
  */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "mock_hal.h"
#include "stm32n6_extmem.h"

#define ASSERT_TRUE(cond) \
  do { if (!(cond)) { printf("       [FAIL] %s:%d: %s\r\n", __FILE__, __LINE__, #cond); return false; } } while (0)

#define ASSERT_EQ(a, b) \
  do { long long _va = (long long)(a), _vb = (long long)(b); \
       if (_va != _vb) { printf("       [FAIL] %s:%d: %s != %s (0x%llX != 0x%llX)\r\n", __FILE__, __LINE__, #a, #b, \
                                (unsigned long long)_va, (unsigned long long)_vb); return false; } } while (0)

#define ASSERT_NOT_NULL(p) ASSERT_TRUE((p) != NULL)

/* Expected shape of one XSPI regular command. Fields set to TEST_ANY are not checked. */
#define TEST_ANY 0xFFFFFFFFU

typedef struct {
  uint32_t Instruction;
  uint32_t InstructionMode;
  uint32_t InstructionWidth;
  uint32_t InstructionDTRMode;
  uint32_t AddressMode;
  uint32_t AddressWidth;
  uint32_t AddressDTRMode;
  uint32_t Address;
  uint32_t AlternateBytesMode;
  uint32_t DataMode;
  uint32_t DataDTRMode;
  uint32_t DataLength;
  uint32_t DummyCycles;
  uint32_t DQSMode;
  uint32_t OperationType;
} CmdSpec_t;

/* Command spec builder: every field defaults to TEST_ANY, the listed phases override it */
#define SPEC(...) { .Instruction = TEST_ANY, .InstructionMode = TEST_ANY, .InstructionWidth = TEST_ANY, \
                    .InstructionDTRMode = TEST_ANY, .AddressMode = TEST_ANY, .AddressWidth = TEST_ANY, \
                    .AddressDTRMode = TEST_ANY, .Address = TEST_ANY, .AlternateBytesMode = TEST_ANY, \
                    .DataMode = TEST_ANY, .DataDTRMode = TEST_ANY, .DataLength = TEST_ANY, \
                    .DummyCycles = TEST_ANY, .DQSMode = TEST_ANY, .OperationType = TEST_ANY, __VA_ARGS__ }

#define I1S(op)  .Instruction = (op), .InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE, \
                 .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS, .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE
#define I4S(op)  .Instruction = (op), .InstructionMode = HAL_XSPI_INSTRUCTION_4_LINES, \
                 .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS, .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE
#define I8S(op)  .Instruction = (op), .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES, \
                 .InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS, .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE
#define I8D(op)  .Instruction = (op), .InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES, \
                 .InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS, .InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE
#define NOADDR   .AddressMode = HAL_XSPI_ADDRESS_NONE
#define A1S(w, a) .AddressMode = HAL_XSPI_ADDRESS_1_LINE, .AddressWidth = (w), .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE, .Address = (a)
#define A4S(w, a) .AddressMode = HAL_XSPI_ADDRESS_4_LINES, .AddressWidth = (w), .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE, .Address = (a)
#define A8D(a)    .AddressMode = HAL_XSPI_ADDRESS_8_LINES, .AddressWidth = HAL_XSPI_ADDRESS_32_BITS, .AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE, .Address = (a)
#define NOALT    .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE
#define NODATA   .DataMode = HAL_XSPI_DATA_NONE
#define D1S(n)   .DataMode = HAL_XSPI_DATA_1_LINE, .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE, .DataLength = (n)
#define D4S(n)   .DataMode = HAL_XSPI_DATA_4_LINES, .DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE, .DataLength = (n)
#define D8D(n)   .DataMode = HAL_XSPI_DATA_8_LINES, .DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE, .DataLength = (n)
#define DUMMY(n) .DummyCycles = (n)
#define NODQS    .DQSMode = HAL_XSPI_DQS_DISABLE
#define WITHDQS  .DQSMode = HAL_XSPI_DQS_ENABLE
#define OPREAD   .OperationType = HAL_XSPI_OPTYPE_READ_CFG
#define OPWRITE  .OperationType = HAL_XSPI_OPTYPE_WRITE_CFG
#define OPCOMMON .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG

bool Test_CheckCmd(const MockEvent_t *ev, const CmdSpec_t *spec, const char *file, int line);

#define ASSERT_CMD(ev, ...) \
  do { const CmdSpec_t _spec = SPEC(__VA_ARGS__); if (!Test_CheckCmd((ev), &_spec, __FILE__, __LINE__)) return false; } while (0)

/* Returns the n-th logged event of type MOCK_EV_CMD */
const MockEvent_t *Test_NthCommand(uint32_t n);
uint32_t Test_CommandCount(void);

/* Checks that the logged command sequence starts at command index 'first' with the listed instructions */
bool Test_CheckSequence(uint32_t first, const uint32_t *instr, uint32_t count, const char *file, int line);
#define ASSERT_SEQUENCE(first, ...) \
  do { const uint32_t _seq[] = { __VA_ARGS__ }; \
       if (!Test_CheckSequence((first), _seq, (uint32_t)(sizeof(_seq) / sizeof(_seq[0])), __FILE__, __LINE__)) return false; } while (0)

/*
 * Fault sweep: runs SETUP then CALL with the k-th HAL call forced to fail, for k = 0, 1, 2 ...
 * Every run that hits the injected fault must return a non-zero status. The sweep ends with
 * the first run that completes without reaching the fault, which must return EXPECT_OK.
 */
#define FAULT_SWEEP_EXPECT(SETUP, CALL, FINAL) \
  do { \
    for (int32_t _k = 0; ; _k++) { \
      SETUP; \
      MockHAL_FailCall(_k); \
      long long _r = (long long)(CALL); \
      bool _hit = MockHAL_FaultTriggered(); \
      MockHAL_FailNone(); \
      if (!_hit) { \
        if (_r != (long long)(FINAL)) { printf("       [FAIL] %s:%d: %s returned %lld without fault\r\n", __FILE__, __LINE__, #CALL, _r); return false; } \
        break; \
      } \
      if (_r == 0) { printf("       [FAIL] %s:%d: %s ignored HAL failure at call %ld\r\n", __FILE__, __LINE__, #CALL, (long)_k); return false; } \
      if (_k > 4096) { printf("       [FAIL] %s:%d: fault sweep did not converge\r\n", __FILE__, __LINE__); return false; } \
    } \
  } while (0)

#define FAULT_SWEEP(SETUP, CALL) FAULT_SWEEP_EXPECT(SETUP, CALL, 0)

/* Fills a buffer with a position dependent pattern */
void Test_Pattern(uint8_t *buf, uint32_t size, uint8_t seed);

#endif /* TEST_COMMON_H */
