/**
  ******************************************************************************
  * @file    test_common.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Assertion and protocol checking helpers shared by the unit tests.
  ******************************************************************************
  */

#include "test_common.h"

static bool Test_Field(const char *name, uint32_t expected, uint32_t actual, const char *file, int line, uint32_t instr)
{
  if (expected == TEST_ANY || expected == actual)
  {
    return true;
  }
  printf("       [FAIL] %s:%d: command 0x%04lX field %s = 0x%lX, expected 0x%lX\r\n",
         file, line, (unsigned long)instr, name, (unsigned long)actual, (unsigned long)expected);
  return false;
}

bool Test_CheckCmd(const MockEvent_t *ev, const CmdSpec_t *spec, const char *file, int line)
{
  if (ev == NULL || ev->Type != MOCK_EV_CMD)
  {
    printf("       [FAIL] %s:%d: expected command 0x%04lX was not issued\r\n", file, line, (unsigned long)spec->Instruction);
    return false;
  }
  const XSPI_RegularCmdTypeDef *c = &ev->Cmd;
  bool ok = true;
  ok &= Test_Field("Instruction",        spec->Instruction,        c->Instruction,        file, line, c->Instruction);
  ok &= Test_Field("InstructionMode",    spec->InstructionMode,    c->InstructionMode,    file, line, c->Instruction);
  ok &= Test_Field("InstructionWidth",   spec->InstructionWidth,   c->InstructionWidth,   file, line, c->Instruction);
  ok &= Test_Field("InstructionDTRMode", spec->InstructionDTRMode, c->InstructionDTRMode, file, line, c->Instruction);
  ok &= Test_Field("AddressMode",        spec->AddressMode,        c->AddressMode,        file, line, c->Instruction);
  if (c->AddressMode != HAL_XSPI_ADDRESS_NONE)
  {
    ok &= Test_Field("AddressWidth",     spec->AddressWidth,       c->AddressWidth,       file, line, c->Instruction);
    ok &= Test_Field("AddressDTRMode",   spec->AddressDTRMode,     c->AddressDTRMode,     file, line, c->Instruction);
    ok &= Test_Field("Address",          spec->Address,            c->Address,            file, line, c->Instruction);
  }
  ok &= Test_Field("AlternateBytesMode", spec->AlternateBytesMode, c->AlternateBytesMode, file, line, c->Instruction);
  ok &= Test_Field("DataMode",           spec->DataMode,           c->DataMode,           file, line, c->Instruction);
  if (c->DataMode != HAL_XSPI_DATA_NONE)
  {
    ok &= Test_Field("DataDTRMode",      spec->DataDTRMode,        c->DataDTRMode,        file, line, c->Instruction);
    ok &= Test_Field("DataLength",       spec->DataLength,         c->DataLength,         file, line, c->Instruction);
  }
  ok &= Test_Field("DummyCycles",        spec->DummyCycles,        c->DummyCycles,        file, line, c->Instruction);
  ok &= Test_Field("DQSMode",            spec->DQSMode,            c->DQSMode,            file, line, c->Instruction);
  ok &= Test_Field("OperationType",      spec->OperationType,      c->OperationType,      file, line, c->Instruction);

  /* Protocol sanity: 8D instructions are 16-bit with an inverted or repeated extension byte */
  if (c->InstructionDTRMode == HAL_XSPI_INSTRUCTION_DTR_ENABLE)
  {
    uint8_t hi = (uint8_t)(c->Instruction >> 8), lo = (uint8_t)(c->Instruction & 0xFFU);
    if (c->InstructionWidth != HAL_XSPI_INSTRUCTION_16_BITS || !(lo == hi || lo == (uint8_t)~hi))
    {
      printf("       [FAIL] %s:%d: malformed 8D instruction 0x%04lX\r\n", file, line, (unsigned long)c->Instruction);
      ok = false;
    }
  }
  /* DTR data phases always move an even number of bytes */
  if (c->DataMode != HAL_XSPI_DATA_NONE && c->DataDTRMode == HAL_XSPI_DATA_DTR_ENABLE &&
      c->OperationType == HAL_XSPI_OPTYPE_COMMON_CFG && (c->DataLength & 1U) != 0U)
  {
    printf("       [FAIL] %s:%d: odd DTR data length %lu\r\n", file, line, (unsigned long)c->DataLength);
    ok = false;
  }
  return ok;
}

const MockEvent_t *Test_NthCommand(uint32_t n)
{
  return MockHAL_FindEvent(MOCK_EV_CMD, n);
}

uint32_t Test_CommandCount(void)
{
  uint32_t n = 0;
  while (MockHAL_FindEvent(MOCK_EV_CMD, n) != NULL) n++;
  return n;
}

bool Test_CheckSequence(uint32_t first, const uint32_t *instr, uint32_t count, const char *file, int line)
{
  for (uint32_t i = 0; i < count; i++)
  {
    const MockEvent_t *ev = Test_NthCommand(first + i);
    if (ev == NULL || ev->Cmd.Instruction != instr[i])
    {
      printf("       [FAIL] %s:%d: command #%lu is 0x%04lX, expected 0x%04lX\r\n", file, line,
             (unsigned long)(first + i), ev ? (unsigned long)ev->Cmd.Instruction : 0xFFFFFFFFUL, (unsigned long)instr[i]);
      return false;
    }
  }
  return true;
}

void Test_Pattern(uint8_t *buf, uint32_t size, uint8_t seed)
{
  for (uint32_t i = 0; i < size; i++)
  {
    buf[i] = (uint8_t)((i * 7U) ^ (i >> 8) ^ seed);
  }
}
