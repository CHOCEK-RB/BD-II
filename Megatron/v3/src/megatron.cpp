#include <cstddef>
#include <cstring>
#include <diskController.hpp>
#include <megatron.hpp>
#include <utils.hpp>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "const.cpp"
#include <iostream>

Megatron::~Megatron() { delete diskController; }

void Megatron::buildStructure() {
  unsigned int numberDisks, numberTracks, numberSectors;

  std::cout << "& Cantidad de discos : ";
  std::cin >> numberDisks;

  std::cout << "& Cantidad de pistas por superficie : ";
  std::cin >> numberTracks;

  std::cout << "& Cantidad de sectores por pista : ";
  std::cin >> numberSectors;

  // Eliminar estructura anterior
  rmdir(PATH);
  // Crear nuevo PATH
  mkdir(PATH, 0777);

  // Calcular el tamaño maximo de la ruta
  // La cantidad maxima de digitos para un entero positivo normal es 10
  // (4.294.967.295)
  char path[sizeof(PATH) + 3 * (1 + 10) + 7];

  for (size_t d = 0; d < numberDisks; ++d) {
    int pos = 0;

    for (int i = 0; PATH[i]; ++i)
      path[pos++] = PATH[i];
    path[pos++] = '/';

    utils::writeInt(d, path, pos);
    path[pos] = '\0';

    mkdir(path, 0777);

    for (size_t s = 0; s < 2; ++s) {
      int posS = pos;
      path[posS++] = '/';

      utils::writeInt(s, path, posS);
      path[posS] = '\0';

      mkdir(path, 0777);

      for (size_t t = 0; t < numberTracks; ++t) {
        int posT = posS;
        path[posT++] = '/';

        utils::writeInt(t, path, posT);
        path[posT] = '\0';

        mkdir(path, 0777);

        for (size_t sec = 0; sec < numberSectors; ++sec) {
          int posF = posT;
          path[posF++] = '/';

          utils::writeInt(sec, path, posF);
          const char *ext = ".dat";

          for (int i = 0; ext[i]; ++i)
            path[posF++] = ext[i];
          path[posF] = '\0';

          int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (fd == -1) {
            std::cerr << "Error al crear archivo: " << path << "\n";
            return;
          }

          char vacio = 0;
          write(fd, &vacio, 1);
          close(fd);
        }
      }
    }
  }

  initializeBootSector(numberDisks, numberTracks, numberSectors);
}

void Megatron::initializeBootSector(int numberdisks,
                                    int numbertracks,
                                    int numbersectors) {
  diskController = new DiskController(numberdisks, numbertracks, numbersectors);
  diskController->head->resetPosition();
  diskController->head->openCurrentSectorFD();
  diskController->writeInt(numberdisks);
  diskController->writeInt(numbertracks);
  diskController->writeInt(numbersectors);
}
