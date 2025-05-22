#ifndef DISKCONTROLLER_HPP
#define DISKCONTROLLER_HPP

#include <head.hpp>

class DiskController {
public:
  int numberDisks;
  int numberTracks;
  int numberSectors;

  Head *head;

  DiskController(int numberDisks, int numberTracks, int numberSectors);
  ~DiskController();

  void moveHeadTo(int disk, int surface, int track, int sector);
  void advanceHead();

  void initializeFAT();
  void writeFATEntry(int sectorID, int nextSectorID);
  int readFATEntry(int sectorID);

  void createRelation(const char *name, int sizeBytes);
  void listRelations();

  void writeCharToCurrentSector(char c);
  char readCharFromCurrentSector();
};

#endif // !DISKCONTROLLER_HPP
