#ifndef DMA_H
#define DMA_H

#include <stdint.h>

void      ci_writeToMemory(uint32_t address, uint32_t value, uint32_t offset);
uint32_t  ci_readFromMemory(uint32_t address, uint32_t offset);

void      dma_writeBusAddress(uint32_t address);
uint32_t  dma_readBusAddress();
void      dma_writeMemoryStart(uint32_t address);
uint32_t  dma_readMemoryStart();
void      dma_writeBlockSize(uint32_t blockSize);
uint32_t  dma_readBlockSize();
void      dma_writeBurstSize(uint32_t burstSize);
uint32_t  dma_readBurstSize();
void      dma_startWriteTransfer();
void      dma_startReadTransfer();
uint32_t  dma_readStatus();
void      dma_waitTransferComplete();
uint32_t  dma_switchBuffer(int buffer);

#endif // DMA_H
