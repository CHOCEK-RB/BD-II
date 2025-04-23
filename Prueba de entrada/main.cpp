#include "disc.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  disc *disco = new disc(2 * 1024 * 1024);
  disco->load_db(argv[1], argv[2]);
  std::cout << "Archivo creado" << std::endl;
  std::cout << "Espacio total : " << disco->total_space() << std::endl;
  std::cout << "Espacio ocupado : " << disco->space_used() << std::endl;
  std::cout << "Espacio disponible : " << disco->available_space() << std::endl;

  return 0;
}
