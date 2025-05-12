#include "const.cpp"
#include "megatron.hpp"
#include <cctype>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int findInLineFile(const int &fd, char delimiter) {
  int startPos = lseek(fd, 0, SEEK_CUR);
  char ch;
  while (read(fd, &ch, 1) == 1) {
    if (ch == delimiter) {
      return lseek(fd, 0, SEEK_CUR);
    }

    if (ch == '\n') {
      break;
    }
  }

  lseek(fd, startPos, SEEK_SET);
  return -1;
}

bool searchFd(const int &saveFd, const int &fd) {
  lseek(saveFd, 0, SEEK_SET);

  while (true) {
    int startPos = lseek(saveFd, 0, SEEK_CUR);
    int number = 0;
    char ch;
    while (true) {
      if (read(saveFd, &ch, 1) != 1)
        return false;

      if (isdigit(ch))
        number = number * 10 + ch - '0';
      else
        break;
    }

    if (number == fd) {
      lseek(saveFd, startPos, SEEK_SET);
      return true;
    }

    while (true) {
      if (read(saveFd, &ch, 1) != 1)
        return false;

      if (ch == '\n'){
        break;
      }else
        return false;
    }
  }

  return true;
}

void registerFile(const int &saveFd, const int &fd, const char *fileName) {
  lseek(saveFd, 0, SEEK_END);
  int copy = fd;
  char chNumber;
  while (copy > 0) {
    int number = copy % 10;
    chNumber = '0' + number;
    write(saveFd, &chNumber, 1);
    copy /= 10;
  }

  chNumber = '=';
  write(saveFd, &chNumber, 1);

  int i = 0;
  while (fileName[i]) {
    write(saveFd, &fileName[i], 1);
    ++i;
  }

  chNumber = '\n';
  write(saveFd, &chNumber, 1);
}

int openFile(const char *fileName, int flags) {
  int sizePath = 0, sizeFile = 0;

  while (PATH[sizePath] != '\0')
    sizePath++;
  while (fileName[sizeFile] != '\0')
    sizeFile++;

  char fullPath[sizePath + sizeFile + 1];

  for (int i = 0; i < sizePath; ++i)
    fullPath[i] = PATH[i];

  for (int i = 0; i < sizeFile; ++i)
    fullPath[sizePath + i] = fileName[i];

  fullPath[sizePath + sizeFile] = '\0';

  int fd = open(fullPath, flags, 0644);
  if (fd == -1) {
    std::cerr << "Error al abrir el archivo " << fileName << '\n';
  }
  return fd;
}

int openTSV(const int &input, const int &saveFd) {
  int sizeTSV = trimFile(input);

  if (sizeTSV == -1 || sizeTSV < 5) {
    std::cerr << "No se ingreso un archivo correcto";
    return -1;
  }

  char fileTSV[sizeTSV + 1];
  char ch;

  for (int i = 0; i < sizeTSV; ++i) {
    if (read(input, &ch, 1) == 1) {
      fileTSV[i] = ch;
    }
  }
  
  fileTSV[sizeTSV] = '\0';

  if (fileTSV[sizeTSV - 4] != '.' || fileTSV[sizeTSV - 3] != 't' ||
      fileTSV[sizeTSV - 2] != 's' || fileTSV[sizeTSV - 1] != 'v') {
    std::cerr << "No se ingreso un archivo tsv";
    return -1;
  }

  int fdTSV = openFile(fileTSV, O_RDONLY);
  registerFile(saveFd, fdTSV, fileTSV);
  return fdTSV;
}

int openTxtFromTsv(const int &saveFd) {
  int start = lseek(saveFd, 0, SEEK_CUR);
  int sizeFile = 0;
  char ch;

  while (read(saveFd, &ch, 1) == 1 && ch != '\n') {
    sizeFile++;
  }

  lseek(saveFd, start, SEEK_SET);
  char fileName[sizeFile + 1];

  for (int i = 0; i < sizeFile; ++i) {
    read(saveFd, &fileName[i], 1);
  }

  fileName[sizeFile] = '\0';

  fileName[sizeFile - 2] = 'x';
  fileName[sizeFile - 1] = 't';

  int fd = openFile(fileName, READ_WRITE_TRUNC_FLAGS);
  return fd;
}

void readInput(const int &fdSave) {
  int ch;

  while ((ch = getchar()) != EOF && ch != '\n') {
    char read = (char)ch;
    write(fdSave, &read, 1);
  }
}

int trimFile(const int &file) {
  char ch;
  int startPos = -1, endPos = -1;

  lseek(file, 0, SEEK_SET);
  while (read(file, &ch, 1) == 1) {
    if (ch != ' ' && ch != '\n' && ch != '\t' && ch != '\'') {
      startPos = lseek(file, 0, SEEK_CUR) - 1;
      break;
    }
  }

  if (startPos == -1)
    return -1;

  off_t fileSize = lseek(file, 0, SEEK_END);
  for (off_t i = fileSize - 1; i >= 0; --i) {
    lseek(file, i, SEEK_SET);
    if (read(file, &ch, 1) != 1)
      break;

    if (ch != ' ' && ch != '\n' && ch != '\t' && ch != '\'') {
      endPos = i;
      break;
    }
  }

  lseek(file, startPos, SEEK_SET);

  return endPos - startPos + 1;
}

void createSchemaTSV(const int &fdTSV, const int &saveFd) {
  searchFd(saveFd, fdTSV);
  findInLineFile(saveFd, '=');

  int fdSchema = openFile(SCHEMA, READ_WRITE_FLAGS);
  char ch;
  while (read(saveFd, &ch, 1) == 1 && ch != '.') {
    write(fdSchema, &ch, 1);
  }

  ch = '#';
  write(fdSchema, &ch, 1);
  int parenth = 0;
  while (read(fdTSV, &ch, 1) == 1 && ch != '\n') {
    if (ch == '(' || ch == ')') {
      if ((ch == '(' && parenth == 0) || (ch == ')' && parenth == 1)){
        parenth += (ch == '(') ? 1 : -1;
        ch = '#';
      }else{
        parenth += (ch == '(') ? 1 : -1;
      }

      write(fdSchema, &ch, 1);
    } else if (ch == '\t') {
      ch = '#';
      write(fdSchema, &ch, 1);
    } else {
      write(fdSchema, &ch, 1);
    }
  }
  
  ch = '\n';
  write(fdSchema, &ch, 1);

  close(fdSchema);
}

void readRegistersTsv(const int &fdTSV, const int &fdTXT){
  char ch;
  while (read(fdTSV, &ch, 1) == 1) {
    if (ch == '\t') {
      ch = '#';
      write(fdTXT, &ch, 1);
    } else {
      write(fdTXT, &ch, 1);
    }
  }
}

bool readTsv(const int &input, const int &saveFd) {
  int fdTSV = openTSV(input, saveFd);

  if (fdTSV == -1) {
    return false;
  }

  searchFd(saveFd, fdTSV);

  findInLineFile(saveFd, '=');

  int fdTXT = openTxtFromTsv(saveFd);

  createSchemaTSV(fdTSV, saveFd);
  
  readRegistersTsv(fdTSV, fdTXT);

  close(fdTSV);
  close(fdTXT);
  return true;
}

void insertRegisters() {
  std::cin.ignore();
  int saveFd = openFile(SAVE_FD_TMP, READ_WRITE_TRUNC_FLAGS);
  int input = openFile(INPUT_TMP, READ_WRITE_TRUNC_FLAGS);

  if (input == -1 || saveFd == -1) {
    return;
  }

  std::cout << "& Nombre del archivo TSV : ";
  readInput(input);

  readTsv(input, saveFd);
  close(saveFd);
  close(input);
}

void MegatronInit() {

  std::cout << "% Megatron 3000\n";
  std::cout << "\tWelcome to Megatron 3000!\n\n";
  std::cout << "% Opciones:\n";
  std::cout << "1) Insertar registros\n";
  std::cout << "2) Salir\n\n";

  int choice;
  std::cout << "Opcion : ";
  std::cin >> choice;

  switch (choice) {
  case 1:
    insertRegisters();
    break;
  case 2:
    std::cout << "Hasta luego.\n";
    return;
  default:
    std::cout << "Opcion invalida.\n";
    break;
  }
}
