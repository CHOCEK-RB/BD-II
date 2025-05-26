#include <cstdint>
#include <diskController.hpp>
#include <head.hpp>
#include <unistd.h>

DiskController::DiskController(int numberDisks, int numberTracks, int numberSectors, int numberBytes, int sectorsBlock)
    : numberDisks(numberDisks), numberTracks(numberTracks),
      numberSectors(numberSectors), numberBytes(numberBytes), sectorsBlock(sectorsBlock) {
  head = new Head;
};

DiskController::~DiskController() { delete head; }

void DiskController::moveHeadTo(int disk, int surface, int track, int sector) {
  if (head ->currentFd != -1){
    close(head->currentFd);
    head->currentFd = -1;
  }
  
  head->moveTo(disk%numberDisks, surface%2, track%numberTracks, sector%numberSectors);
}

void DiskController::nextSector() {
    head->currentSector++;

    if (head->currentSector >= numberSectors) {
        head->currentSector = 0;
        nextTrack();
    }
}


void DiskController::nextTrack() {
    head->currentTrack++;

    if (head->currentTrack >= numberTracks) {
        head->currentTrack = 0;
        nextSurface();
    }
}

void DiskController::nextSurface() {
    head->currentSurface++;

    if (head->currentSurface >= 2) {
        head->currentSurface = 0;
        nextDisk();
    }
}

void DiskController::nextDisk() {
    head->currentDisk++;

    if (head->currentDisk >= numberDisks) {
        head->currentDisk = 0;
    }
}

void DiskController::moveToBlock(uint32_t blockID) {
    int blocksPerTrack = numberSectors / sectorsBlock;
    int blocksPerSurface = numberTracks * blocksPerTrack;
    int blocksPerDisk = 2 * blocksPerSurface;

    int disk = blockID / blocksPerDisk;
    blockID %= blocksPerDisk;

    int surface = blockID / blocksPerSurface;
    blockID %= blocksPerSurface;

    int track = blockID / blocksPerTrack;
    int blockInTrack = blockID % blocksPerTrack;

    int sector = blockInTrack * sectorsBlock;

    head->moveTo(disk, surface, track, sector);
    head->openCurrentSectorFD();
}


void DiskController::initializeFAT() {
    moveToBlock(1);

    uint32_t totalBlocks =
        numberDisks * 2 * numberTracks * (numberSectors / sectorsBlock);

    // 4 bytes en 0 para bloques libres
    uint32_t empty = 0; 

    for (uint32_t i = 0; i < totalBlocks; ++i) {
        writeBinary(empty);
    }

    head->resetPosition();
}

void DiskController::writeFATEntry(uint32_t blockID, uint32_t nextBlockID) {
    moveToBlock(1);

    uint32_t offset = blockID * sizeof(uint32_t);
    lseek(head->currentFd, offset, SEEK_CUR);

    writeBinary(nextBlockID);
}

uint32_t DiskController::readFATEntry(uint32_t blockID) {
    moveToBlock(1);

    uint32_t offset = blockID * sizeof(uint32_t);
    lseek(head->currentFd, offset, SEEK_CUR);

    uint32_t nextBlock;
    readBinary(nextBlock);
    return nextBlock;
}
