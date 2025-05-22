#ifndef MEGATRON_HPP
#define MEGATRON_HPP

#include <diskController.hpp>

class Megatron {

public:

  DiskController *diskController;
  
  Megatron() = default;
  ~Megatron();
  void buildStructure();
  void initializeBootSector(int numberdisks, int numbertracks, int numbersectors);
};

#endif
