#include <diskController.hpp>
#include <head.hpp>
#include <unistd.h>

DiskController::DiskController(int numberDisks,
                               int numberTracks,
                               int numberSectors)
    : numberDisks(numberDisks), numberTracks(numberTracks),
      numberSectors(numberSectors) {
  head = new Head;
};

DiskController::~DiskController() { delete head; }

void DiskController::moveHeadTo(int disk, int surface, int track, int sector) {}

int DiskController::writeChar(char ch) {
  return write(head->currentFd, &ch, 1);
}
int DiskController::readChar(char &ch) { return read(head->currentFd, &ch, 1); }

int DiskController::writeInt(int number) {
  return write(head->currentFd, &number, sizeof(number));
}
int DiskController::readInt(int &number) {
  return read(head->currentFd, &number, sizeof(number));
}
