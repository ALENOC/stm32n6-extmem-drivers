/**
  ******************************************************************************
  * @file    extmem_benchmark.c
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Performance benchmark tool for measuring Read/Write throughput
  *          and latency of external memories on STM32N6.
  ******************************************************************************
  */

#include <stdio.h>
#include "stm32n6_extmem.h"

#define BENCHMARK_BUFFER_SIZE  (64 * 1024) /* 64 KB benchmark buffer */
static uint8_t BenchBuffer[BENCHMARK_BUFFER_SIZE] __attribute__((aligned(32)));

void ExtMem_RunBenchmark(ExtMem_HandleTypeDef *hextmem, uint32_t TargetAddress, uint32_t TotalTransferBytes)
{
  uint32_t startTick, endTick, elapsedMs;
  float speedMBs;

  printf("\r\n--- Performance Benchmark: %s ---\r\n", ExtMem_GetDeviceName(hextmem));
  printf("Target Address: 0x%08lX, Test Size: %lu KB\r\n",
         (unsigned long)TargetAddress, (unsigned long)(TotalTransferBytes / 1024));

  /* 1. Indirect Read Benchmark */
  startTick = HAL_GetTick();
  uint32_t bytesRead = 0;
  while (bytesRead < TotalTransferBytes)
  {
    uint32_t chunkSize = (TotalTransferBytes - bytesRead > BENCHMARK_BUFFER_SIZE) ?
                          BENCHMARK_BUFFER_SIZE : (TotalTransferBytes - bytesRead);
    ExtMem_Read(hextmem, TargetAddress + bytesRead, BenchBuffer, chunkSize);
    bytesRead += chunkSize;
  }
  endTick = HAL_GetTick();
  elapsedMs = (endTick > startTick) ? (endTick - startTick) : 1;
  speedMBs = ((float)TotalTransferBytes / (1024.0f * 1024.0f)) / ((float)elapsedMs / 1000.0f);
  printf("[Indirect Read] Time: %lu ms -> Speed: %.2f MB/s\r\n", (unsigned long)elapsedMs, speedMBs);

  /* 2. Memory Mapped (XIP) Read Benchmark */
  if (ExtMem_EnableMemoryMapped(hextmem) == EXTMEM_OK)
  {
    volatile uint32_t *pSrc = (volatile uint32_t *)(hextmem->MemoryMappedBase + TargetAddress);
    volatile uint32_t dummy = 0;
    uint32_t words = TotalTransferBytes / 4;

    startTick = HAL_GetTick();
    for (uint32_t i = 0; i < words; i++)
    {
      dummy += pSrc[i];
    }
    endTick = HAL_GetTick();
    (void)dummy;

    elapsedMs = (endTick > startTick) ? (endTick - startTick) : 1;
    speedMBs = ((float)TotalTransferBytes / (1024.0f * 1024.0f)) / ((float)elapsedMs / 1000.0f);
    printf("[XIP Direct Read] Time: %lu ms -> Speed: %.2f MB/s\r\n", (unsigned long)elapsedMs, speedMBs);

    ExtMem_DisableMemoryMapped(hextmem);
  }

  /* 3. PSRAM Write Benchmark (if RAM) */
  if (ExtMem_IsRAM(hextmem))
  {
    startTick = HAL_GetTick();
    uint32_t bytesWritten = 0;
    while (bytesWritten < TotalTransferBytes)
    {
      uint32_t chunkSize = (TotalTransferBytes - bytesWritten > BENCHMARK_BUFFER_SIZE) ?
                            BENCHMARK_BUFFER_SIZE : (TotalTransferBytes - bytesWritten);
      ExtMem_Write(hextmem, TargetAddress + bytesWritten, BenchBuffer, chunkSize);
      bytesWritten += chunkSize;
    }
    endTick = HAL_GetTick();
    elapsedMs = (endTick > startTick) ? (endTick - startTick) : 1;
    speedMBs = ((float)TotalTransferBytes / (1024.0f * 1024.0f)) / ((float)elapsedMs / 1000.0f);
    printf("[PSRAM Burst Write] Time: %lu ms -> Speed: %.2f MB/s\r\n", (unsigned long)elapsedMs, speedMBs);
  }

  printf("--------------------------------------------------\r\n");
}
