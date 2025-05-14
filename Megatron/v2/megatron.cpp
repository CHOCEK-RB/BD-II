#include "const.cpp"
#include "megatron.hpp"

#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int openFile(const char *fileName, int flags, bool registrar, int saveFd) {
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
    return -1;
  }

  if (registrar && saveFd != -1) {
    registerFile(saveFd, fd, fileName);
  }

  return fd;
}

void readInput(const int &fdSave) {
  int ch;

  while ((ch = getchar()) != EOF) {
    char read = (char)ch;
    write(fdSave, &read, 1);

    if (ch == '\n')
      break;
  }
}

void writeInt(const int &fd, int number) {
  int div = 1;
  int temp = number;
  while (temp >= 10) {
    temp /= 10;
    div *= 10;
  }

  while (div > 0) {
    char digit = '0' + (number / div);
    write(fd, &digit, 1);
    number %= div;
    div /= 10;
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

void splitLineFile(const int &fd, const int &fdSplit, char delimiter) {
  char ch;
  while (read(fd, &ch, 1) == 1) {
    char input = ch;

    if (ch == delimiter) {
      input = '\n';
    }

    if (ch != ' ' && ch != '\t') {
      write(fdSplit, &input, 1);
    }

    if (ch == '\n') {
      break;
    }
  }
}

int findAndMove(const int &fd, char delimiter) {
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

bool findInLine(const int &fd, char found, char delimiter) {
  int startPos = lseek(fd, 0, SEEK_CUR);
  char ch;
  while (read(fd, &ch, 1) == 1) {
    if (ch == found) {
      lseek(fd, startPos, SEEK_SET);
      return true;
    }

    if (ch == '\n' || ch == delimiter) {
      break;
    }
  }

  lseek(fd, startPos, SEEK_SET);
  return false;
}

void registerFile(const int &saveFd, const int &fd, const char *fileName) {
  lseek(saveFd, 0, SEEK_END);
  writeInt(saveFd, fd);

  char chNumber = '=';
  write(saveFd, &chNumber, 1);

  int i = 0;
  while (fileName[i]) {
    write(saveFd, &fileName[i], 1);
    ++i;
  }

  chNumber = '\n';
  write(saveFd, &chNumber, 1);
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

      if (ch == '\n') {
        break;
      } else
        return false;
    }
  }

  return true;
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

bool openRelationFiles(const int &relations, const int &saveFd) {
  lseek(relations, 0, SEEK_SET);

  char ch;
  int sizeFile = 0;

  while (read(relations, &ch, 1) == 1) {

    if (ch == '\n') {
      char fileName[sizeFile + 5];
      lseek(relations, -sizeFile, SEEK_CUR);

      for (int i = 0; i < sizeFile; ++i) {
        read(relations, &fileName[i], 1);
      }

      fileName[sizeFile] = '.';
      fileName[sizeFile + 1] = 't';
      fileName[sizeFile + 2] = 'x';
      fileName[sizeFile + 3] = 't';
      fileName[sizeFile + 4] = '\0';

      if (openFile(fileName, O_RDONLY, true, saveFd) == -1)
        return false;

      sizeFile = 0;
      lseek(relations, 1, SEEK_CUR);

    } else {
      ++sizeFile;
    }
  }
  return true;
}

int nextFd(const int &saveFd) {
  int startPos = lseek(saveFd, 0, SEEK_SET);
  if (lseek(saveFd, 0, SEEK_CUR) == 0) {
    lseek(saveFd, 0, SEEK_SET);
    char ch;
    int fd = 0;
    while (read(saveFd, &ch, 1) == 1 && isdigit(ch))
      fd = fd * 10 + (ch - '0');
    return fd;
  }

  char ch;
  while (read(saveFd, &ch, 1) == 1) {
    if (ch == '\n') {
      int fd = 0;
      while (read(saveFd, &ch, 1) == 1 && isdigit(ch))
        fd = fd * 10 + (ch - '0');
      return fd;
    }
  }

  lseek(saveFd, startPos, SEEK_SET);
  return -1;
}

int prevFd(int saveFd) {
  int startPos = lseek(saveFd, 0, SEEK_SET), fileSize = lseek(saveFd, 0, SEEK_END);
  char ch;
  bool found = false;

  for (off_t i = fileSize; i >= 0; --i) {
    lseek(saveFd, i, SEEK_SET);

    if (read(saveFd, &ch, 1) != 1){
      lseek(saveFd, startPos, SEEK_SET);
      return -1;
    }

    if (found && ch == '=') {
      --i;
      lseek(saveFd, i, SEEK_SET);

      int fd = 0, mult = 1;
      while (i >= 0) {
        lseek(saveFd, i, SEEK_SET);
        if (read(saveFd, &ch, 1) != 1 || !isdigit(ch))
          break;

        fd += (ch - '0') * mult;
        mult *= 10;
        --i;
      }      

      return fd;
    }

    if (ch == '\n') {
      found = true;
    }
  }

  lseek(saveFd, startPos, SEEK_SET);
  return -1;
}

void createSchemaTSV(const int &fdTSV, const int &saveFd) {
  searchFd(saveFd, fdTSV);
  findAndMove(saveFd, '=');

  int fdSchema = openFile(SCHEMA, READ_WRITE_FLAGS);
  char ch;
  while (read(saveFd, &ch, 1) == 1 && ch != '.') {
    write(fdSchema, &ch, 1);
  }

  ch = '#';
  write(fdSchema, &ch, 1);
  int parenth = 0;
  while (read(fdTSV, &ch, 1) == 1 && ch != '\n') {
    if (ch == '\r') {
      continue;
    }

    if (ch == '(' || ch == ')') {
      parenth += (ch == '(') ? 1 : -1;

      if (ch == '(' && parenth == 1) {
        ch = '#';
      }

      if (ch != ')' || parenth != 0) {
        write(fdSchema, &ch, 1);
      }

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

void readRegistersTsv(const int &fdTSV, const int &fdTXT) {
  char ch;
  while (read(fdTSV, &ch, 1) == 1) {
    if (ch == '\r') {
      continue;
    }

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

  findAndMove(saveFd, '=');

  int fdTXT = openTxtFromTsv(saveFd);

  createSchemaTSV(fdTSV, saveFd);

  readRegistersTsv(fdTSV, fdTXT);

  close(fdTSV);
  close(fdTXT);
  return true;
}

bool checkFrom(const int &schema,
               const int &relations,
               const int &relationsTmp) {
  lseek(schema, 0, SEEK_SET);
  lseek(relations, 0, SEEK_SET);
  char chSchema, chFrom;

  int startRelation = 0;

  while (true) {
    if (read(relations, &chFrom, 1) != 1)
      return true;

    if (read(schema, &chSchema, 1) != 1)
      return false;

    if (chSchema == '#' && chFrom == '\n') {
      lseek(schema, startRelation - lseek(relations, 0, SEEK_CUR), SEEK_CUR);
      char ch;

      while (read(schema, &ch, 1) == 1) {
        write(relationsTmp, &ch, 1);

        if (ch == '\n')
          break;
      }

      startRelation = lseek(relations, 0, SEEK_CUR);
      lseek(schema, 0, SEEK_SET);
      continue;
    }

    if (chSchema != chFrom) {
      findAndMove(schema, '\n');
      lseek(relations, startRelation, SEEK_SET);
    }
  }

  return false;
}

bool findSchemaAttribute(const int &relationsTmp,
                         const int &attributes,
                         const int &attributesTmp) {

  int startPosAttributes = lseek(attributes, 0, SEEK_CUR);
  char chRelations, chAttributes;

  while (true) {
    if (read(relationsTmp, &chRelations, 1) != 1 ||
        read(attributes, &chAttributes, 1) != 1)
      return false;

    if (chRelations == '#' && chAttributes == '.') {
      lseek(attributes, startPosAttributes, SEEK_SET);

      while (read(attributes, &chAttributes, 1) == 1) {
        write(attributesTmp, &chAttributes, 1);

        if (chAttributes == '.')
          break;
      }

      startPosAttributes = lseek(attributes, 0, SEEK_CUR);
      break;
    }

    if (chRelations != chAttributes) {
      findAndMove(relationsTmp, '\n');
      lseek(attributes, startPosAttributes, SEEK_SET);
    }
  }

  int positionAttribute = 0;
  while (true) {
    if (read(relationsTmp, &chRelations, 1) != 1 ||
        read(attributes, &chAttributes, 1) != 1)
      return false;

    if (chRelations == '#' && chAttributes == '\n') {
      lseek(relationsTmp,
            startPosAttributes - lseek(attributes, 0, SEEK_CUR),
            SEEK_CUR);
      bool foundType = false;

      while (read(relationsTmp, &chRelations, 1) == 1) {
        if (chRelations == '#' && !foundType) {
          foundType = true;

        } else if ((chRelations == '#' || chRelations == '\n') && foundType) {
          chRelations = '#';
          write(attributesTmp, &chRelations, 1);
          writeInt(attributesTmp, positionAttribute);
          chRelations = '\n';
        }

        write(attributesTmp, &chRelations, 1);

        if (chRelations == '\n')
          break;
      }

      return true;
    } else if (chRelations == '#') {
      if (findAndMove(relationsTmp, '#') == -1)
        return false;

      ++positionAttribute;
    }

    if (chRelations != chAttributes) {
      findAndMove(relationsTmp, '#');
      findAndMove(relationsTmp, '#');

      lseek(attributes, startPosAttributes, SEEK_SET);
      ++positionAttribute;
    }
  }
  return true;
}

bool findAttribute(const int &relationsTmp,
                   const int &attributes,
                   const int &attributesTmp) {

  int startPosAttributes = lseek(attributes, 0, SEEK_CUR);
  char chRelations, chAttributes;
  bool found = false;

  while (true) {
    int positionAttribute = 0;
    int startPosRelation = lseek(relationsTmp, 0, SEEK_CUR);

    if (findAndMove(relationsTmp, '#') == -1)
      return false;

    while (true) {
      if (read(relationsTmp, &chRelations, 1) != 1 ||
          read(attributes, &chAttributes, 1) != 1)
        return false;

      if (chRelations == '#' && chAttributes == '\n') {
        if (found)
          return false;

        found = true;
        int actualyPos = lseek(relationsTmp, 0, SEEK_CUR);

        lseek(relationsTmp, startPosRelation, SEEK_SET);

        while (read(relationsTmp, &chRelations, 1) == 1) {
          if (chRelations == '#')
            chRelations = '.';

          write(attributesTmp, &chRelations, 1);

          if (chRelations == '.')
            break;
        }

        lseek(relationsTmp,
              actualyPos -
                  (lseek(attributes, 0, SEEK_CUR) - startPosAttributes),
              SEEK_SET);

        bool foundType = false;
        while (read(relationsTmp, &chRelations, 1) == 1) {
          if (chRelations == '#' && !foundType) {
            foundType = true;

          } else if ((chRelations == '#' || chRelations == '\n') && foundType) {
            chRelations = '#';
            write(attributesTmp, &chRelations, 1);
            writeInt(attributesTmp, positionAttribute);
            chRelations = '\n';
          }

          write(attributesTmp, &chRelations, 1);

          if (chRelations == '\n')
            break;
        }

        return true;
      } else if (chRelations == '#') {
        if (findAndMove(relationsTmp, '#') == -1)
          break;
        else
          ++positionAttribute;
      }

      if (chRelations != chAttributes) {
        findAndMove(relationsTmp, '#');
        findAndMove(relationsTmp, '#');

        lseek(attributes, startPosAttributes, SEEK_SET);
        ++positionAttribute;
      }
    }

    findAndMove(relationsTmp, '\n');
  }
}

void addAllAttributes(const int &relationsTmp, const int &attributesTmp) {
  char ch;

  while (true) {
    int startPos = lseek(relationsTmp, 0, SEEK_CUR);
    int actualyPos = 0, positionAttribute = 0;
    bool foundType = false, init = false;
    while (true) {
      if (read(relationsTmp, &ch, 1) != 1)
        return;

      if (ch == '\n') {
        ch = '#';
        write(attributesTmp, &ch, 1);
        writeInt(attributesTmp, positionAttribute);

        ++positionAttribute;

        ch = '\n';
        write(attributesTmp, &ch, 1);
        break;
      }

      if (ch == '#') {
        if (lseek(relationsTmp, 0, SEEK_CUR) < actualyPos) {
          ch = '.';
          lseek(relationsTmp, actualyPos, SEEK_SET);
        } else if (!init) {
          init = true;
          ch = '.';
        } else if (!foundType) {
          foundType = true;
        } else if (foundType) {
          foundType = false;
          actualyPos = lseek(relationsTmp, 0, SEEK_CUR);

          ch = '#';
          write(attributesTmp, &ch, 1);
          writeInt(attributesTmp, positionAttribute);

          ++positionAttribute;

          ch = '\n';
          lseek(relationsTmp, startPos, SEEK_SET);
        }

        write(attributesTmp, &ch, 1);
      } else {
        write(attributesTmp, &ch, 1);
      }
    }
  }
}

bool isAsterisk(const int &relationsTmp) {
  char ch1, ch2;
  if (read(relationsTmp, &ch1, 1) != 1 || ch1 != '*')
    return false;

  if (read(relationsTmp, &ch2, 1) != 1 || ch2 != '\n')
    return false;

  if (lseek(relationsTmp, 0, SEEK_CUR) != lseek(relationsTmp, 0, SEEK_END))
    return false;

  lseek(relationsTmp, 0, SEEK_SET);
  return true;
}

bool checkAttributes(const int &relationsTmp,
                     const int &attributes,
                     const int &attributesTmp) {
  lseek(relationsTmp, 0, SEEK_SET);
  lseek(attributes, 0, SEEK_SET);

  if (isAsterisk(attributes)) {
    addAllAttributes(relationsTmp, attributesTmp);
    return true;
  }

  while (true) {
    int actualyPosAttributes = lseek(attributes, 0, SEEK_CUR);
    if (actualyPosAttributes == lseek(attributes, 0, SEEK_END)) {
      break;
    }

    lseek(attributes, actualyPosAttributes, SEEK_SET);

    if (findInLine(attributes, '.', '#')) {
      if (!findSchemaAttribute(relationsTmp, attributes, attributesTmp)) {
        return false;
      }

      lseek(relationsTmp, 0, SEEK_SET);
    } else {
      if (!findAttribute(relationsTmp, attributes, attributesTmp)) {
        return false;
      }

      lseek(relationsTmp, 0, SEEK_SET);
    }
  }
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

void selectMenu() {
  std::cin.ignore();
  int saveFd = openFile(SAVE_FD_TMP, READ_WRITE_TRUNC_FLAGS);

  int input = openFile(INPUT_TMP, READ_WRITE_TRUNC_FLAGS);
  int schema = openFile(SCHEMA, READ_WRITE_FLAGS);

  int attributes = openFile(SELECT_TMP, READ_WRITE_TRUNC_FLAGS);
  int relations = openFile(FROM_TMP, READ_WRITE_TRUNC_FLAGS);

  int attributesTmp = openFile(ATTRIBUTES_TMP, READ_WRITE_TRUNC_FLAGS);
  int relationsTmp = openFile(RELATIONS_TMP, READ_WRITE_TRUNC_FLAGS);

  if (input == -1 || schema == -1) {
    return;
  }

  std::cout << "& SELECT : ";
  readInput(input);
  std::cout << "& FROM : ";
  readInput(input);

  lseek(input, 0, SEEK_SET);
  splitLineFile(input, attributes, ',');
  splitLineFile(input, relations, ',');

  if (!checkFrom(schema, relations, relationsTmp)) {
    std::cerr << "Error al comprobar las relaciones en el FROM\n";
    return;
  }

  if (!checkAttributes(relationsTmp, attributes, attributesTmp)) {
    std::cerr << "Error al comprobar los atributos el SELECT\n";
    return;
  }

  if (!openRelationFiles(relations, saveFd)) {
    std::cerr << "Error al buscar los registros de las relaciones\n";
    return;
  }

  close(saveFd);

  close(input);
  close(schema);

  close(attributes);
  close(relations);

  close(attributesTmp);
  close(relationsTmp);
}

void MegatronInit() {

  std::cout << "% Megatron 3000\n";
  std::cout << "\tWelcome to Megatron 3000!\n\n";
  std::cout << "% Opciones:\n";
  std::cout << "1) Insertar registros\n";
  std::cout << "2) SELECT\n";
  std::cout << "3) Salir\n\n";

  int choice;
  std::cout << "Opcion : ";
  std::cin >> choice;

  switch (choice) {
  case 1:
    insertRegisters();
    break;
  case 2:
    selectMenu();
  case 3:
    std::cout << "Hasta luego.\n";
    return;
  default:
    std::cout << "Opcion invalida.\n";
    break;
  }
}
