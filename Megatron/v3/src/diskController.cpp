#include <diskController.hpp>
#include <head.hpp>

DiskController::DiskController(int numberDisks, int numberTracks, int numberSectors) : numberDisks(numberDisks), numberTracks(numberTracks), numberSectors(numberSectors) {
  head = new Head;
};

DiskController::~DiskController(){
  delete head;
}

void DiskController::moveHeadTo(int disk, int surface, int track, int sector){
  
}

