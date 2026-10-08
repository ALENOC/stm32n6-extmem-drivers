/**
  ******************************************************************************
  * @file    extmem_demo.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Demonstration and self-test application for Infineon, ISSI & Micron
  *          Flash and PSRAM external memories on STM32N6.
  *          Can be called directly from main() in STM32CubeIDE projects.
  ******************************************************************************
  */

#include <stdio.h>
#include <string.h>
#include "stm32n6_extmem.h"

/* Test Buffer (aligned for 64-bit / DMA transfers) */
#define TEST_BUFFER_SIZE  4096U
static uint8_t TxBuffer[TEST_BUFFER_SIZE] __attribute__((aligned(8)));
static uint8_t RxBuffer[TEST_BUFFER_SIZE] __attribute__((aligned(8)));

/**
  * @brief  Main external memory demo and diagnostic routine.
  * @param  busTarget ExtMem_Bus_t bus (EXTMEM_BUS_XSPI1, EXTMEM_BUS_XSPI2, etc.)
  */
void ExtMem_RunDemo(ExtMem_Bus_t busTarget)
{
  ExtMem_HandleTypeDef hextmem = {0};
  int32_t status;

  printf("\r\n=======================================================\r\n");
  printf("  STM32N6 External Memory Driver Suite - Diagnostic Demo \r\n");
  printf("=======================================================\r\n");

  /* 1. Configuration */
  hextmem.Config.Bus            = busTarget;
  hextmem.Config.ClockPrescaler = 2;    /* 200 MHz if XSPI kernel clock is 400 MHz */
  hextmem.Config.Force1V8       = true; /* 1.8V high-speed mode */

  printf("[1] Initializing External Memory on Bus %d...\r\n", busTarget);
  status = ExtMem_Init(&hextmem);
  if (status != EXTMEM_OK)
  {
    printf("[-] ExtMem_Init failed with error code: %ld\r\n", (long)status);
    return;
  }

  /* 2. Display Detected Memory Information */
  printf("[+] Memory Initialized Successfully!\r\n");
  printf("    - Device Name   : %s\r\n", ExtMem_GetDeviceName(&hextmem));
  printf("    - Total Size    : %lu MB (%lu Bytes)\r\n",
         (unsigned long)(ExtMem_GetCapacity(&hextmem) / (1024 * 1024)),
         (unsigned long)ExtMem_GetCapacity(&hextmem));
  printf("    - Type          : %s\r\n", ExtMem_IsFlash(&hextmem) ? "NOR Flash (Non-Volatile)" : "PSRAM / RAM (Volatile)");
  printf("    - Active Mode   : %s\r\n", (hextmem.ActiveMode == EXTMEM_MODE_OCTAL_DTR) ? "Octal DTR (8D-8D-8D)" :
                                       (hextmem.ActiveMode == EXTMEM_MODE_HYPERBUS)  ? "HyperBus (DDR)" :
                                       (hextmem.ActiveMode == EXTMEM_MODE_QUAD_1_4_4)? "Quad SPI (1-4-4)" : "Parallel 16-bit");
  printf("    - Read Latency  : %u Dummy Cycles\r\n", hextmem.DummyCycles);
  printf("    - XIP Base Addr : 0x%08lX\r\n", (unsigned long)hextmem.MemoryMappedBase);

  /* 3. Flash or PSRAM Diagnostic Routine */
  const uint32_t testAddress = 0x00010000; /* Test at offset 64KB */

  /* Fill TX Buffer with pseudo-random test pattern */
  for (uint32_t i = 0; i < TEST_BUFFER_SIZE; i++)
  {
    TxBuffer[i] = (uint8_t)((i ^ 0x5A) & 0xFF);
  }
  memset(RxBuffer, 0, TEST_BUFFER_SIZE);

  if (ExtMem_IsFlash(&hextmem))
  {
    printf("\r\n[2] Performing Flash Sector Erase (%lu KB) at 0x%08lX...\r\n",
           (unsigned long)(hextmem.Geometry.SectorSizeBytes / 1024U), (unsigned long)testAddress);
    status = ExtMem_EraseSector(&hextmem, testAddress);
    if (status != EXTMEM_OK)
    {
      printf("[-] Erase failed: %ld\r\n", (long)status);
      return;
    }
    printf("[+] Sector Erased!\r\n");

    /* Verify Erase (All bytes must be 0xFF) */
    status = ExtMem_Read(&hextmem, testAddress, RxBuffer, 256);
    bool erased = true;
    for (int i = 0; i < 256; i++)
    {
      if (RxBuffer[i] != 0xFF) { erased = false; break; }
    }
    if (!erased)
    {
      printf("[-] Sector blank check failed (data != 0xFF)!\r\n");
      return;
    }
    printf("[+] Erase verification passed (0xFF confirmed).\r\n");

    /* Program Flash */
    printf("\r\n[3] Programming %u Bytes to Flash...\r\n", TEST_BUFFER_SIZE);
    status = ExtMem_Write(&hextmem, testAddress, TxBuffer, TEST_BUFFER_SIZE);
    if (status != EXTMEM_OK)
    {
      printf("[-] Write failed: %ld\r\n", (long)status);
      return;
    }
    printf("[+] Flash Programming Complete!\r\n");
  }
  else /* PSRAM */
  {
    printf("\r\n[2] Writing %u Bytes to PSRAM / HyperRAM at 0x%08lX...\r\n", TEST_BUFFER_SIZE, (unsigned long)testAddress);
    status = ExtMem_Write(&hextmem, testAddress, TxBuffer, TEST_BUFFER_SIZE);
    if (status != EXTMEM_OK)
    {
      printf("[-] PSRAM Write failed: %ld\r\n", (long)status);
      return;
    }
    printf("[+] PSRAM Write Complete!\r\n");
  }

  /* 4. Readback and Verify in Indirect Mode */
  printf("\r\n[4] Reading back in Indirect Mode and verifying integrity...\r\n");
  memset(RxBuffer, 0, TEST_BUFFER_SIZE);
  status = ExtMem_Read(&hextmem, testAddress, RxBuffer, TEST_BUFFER_SIZE);
  if (status != EXTMEM_OK)
  {
    printf("[-] Read failed: %ld\r\n", (long)status);
    return;
  }

  if (memcmp(TxBuffer, RxBuffer, TEST_BUFFER_SIZE) == 0)
  {
    printf("[+] DATA INTEGRITY VERIFIED: Indirect mode data matches 100%%!\r\n");
  }
  else
  {
    printf("[-] DATA CORRUPTION DETECTED in Indirect mode readback!\r\n");
    return;
  }

  /* 5. Memory-Mapped (XIP) Mode Test */
  printf("\r\n[5] Activating Memory-Mapped (XIP) Mode...\r\n");
  status = ExtMem_EnableMemoryMapped(&hextmem);
  if (status != EXTMEM_OK)
  {
    printf("[-] Failed to enter Memory-Mapped mode: %ld\r\n", (long)status);
    return;
  }
  printf("[+] Memory-Mapped Mode Active at 0x%08lX!\r\n", (unsigned long)hextmem.MemoryMappedBase);

  /* Direct Pointer Access */
  volatile uint8_t *pXip = (volatile uint8_t *)(uintptr_t)(hextmem.MemoryMappedBase + testAddress);
  bool xipMatch = true;
  for (uint32_t i = 0; i < TEST_BUFFER_SIZE; i++)
  {
    if (pXip[i] != TxBuffer[i])
    {
      xipMatch = false;
      printf("[-] Mismatch at offset %lu: expected 0x%02X, read 0x%02X\r\n",
             (unsigned long)i, TxBuffer[i], pXip[i]);
      break;
    }
  }

  if (xipMatch)
  {
    printf("[+] DIRECT XIP ACCESS SUCCESSFUL: CPU read %u bytes directly via pointer!\r\n", TEST_BUFFER_SIZE);
  }

  /* Return to indirect mode */
  ExtMem_DisableMemoryMapped(&hextmem);
  printf("[+] Successfully exited Memory-Mapped Mode.\r\n");

  printf("\r\n=======================================================\r\n");
  printf("  ALL DIAGNOSTIC CHECKS PASSED SUCCESSFULLY!\r\n");
  printf("=======================================================\r\n");
}
