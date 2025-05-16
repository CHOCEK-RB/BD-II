#include "const.cpp"
#include "readCSV.hpp"
#include "utils.hpp"

#include <cctype>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

bool openRelationFiles(const int &relations, const int &saveFd) {
  lseek(relations, 0, SEEK_SET);

  char ch;
  int sizeFile = 0;

  while (read(relations, &ch, 1) == 1) {

    if (ch == '\n') {

      char fileName[sizeFile + 5];
      lseek(relations, -(sizeFile + 1), SEEK_CUR);

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
  if (read(relationsTmp, &ch1, 1) != 1 || ch1 != '*'){
    lseek(relationsTmp, 0, SEEK_SET);
    return false;
  }

  if (read(relationsTmp, &ch2, 1) != 1 || ch2 != '\n'){
    lseek(relationsTmp, 0, SEEK_SET);
    return false;
  }

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


void cartesianProduct(int saveFd,
                      int attributesTmpFd,
                      int relationsTmpFd,
                      int linesTmpFd,
                      int resultTmpFd) {
  int nivel = 0;

  lseek(saveFd, 0, SEEK_SET);

  // Contar cantidad de relaciones
  int cantidadRelaciones = 0;
  {
    char ch;
    while (read(saveFd, &ch, 1) == 1) {
      if (ch == '\n') cantidadRelaciones++;
    }
    lseek(saveFd, 0, SEEK_SET);
  }

  while (true) {
    if (nivel == cantidadRelaciones) {
      lseek(attributesTmpFd, 0, SEEK_SET);

      int totalAtributos = 0;
      char ch;
      while (read(attributesTmpFd, &ch, 1) == 1) {
        if (ch == '\n') totalAtributos++;
      }
      lseek(attributesTmpFd, 0, SEEK_SET);

      int atributoIndex = 0;
      while (atributoIndex < totalAtributos) {
        char chLine, chAttributes;
        off_t relStart = lseek(attributesTmpFd, 0, SEEK_CUR);

        if (findAndMove(attributesTmpFd, '.') == 0)
          break;

        int relSize = lseek(attributesTmpFd, 0, SEEK_CUR) - relStart - 1;
        lseek(attributesTmpFd, relStart, SEEK_SET);

        lseek(linesTmpFd, 0, SEEK_SET);
        off_t lineStart = lseek(linesTmpFd, 0, SEEK_CUR);

        while (true) {
          if (read(attributesTmpFd, &chAttributes, 1) != 1) break;
          if (read(linesTmpFd, &chLine, 1) != 1) return;

          if (chAttributes == '.' && chLine == '#') {
            findAndMove(attributesTmpFd, '#', 2);

            int colPos = 0;
            while (read(attributesTmpFd, &chAttributes, 1) == 1 && isdigit(chAttributes)) {
              colPos = colPos * 10 + (chAttributes - '0');
            }

            lseek(linesTmpFd, lineStart, SEEK_SET);
            findAndMove(linesTmpFd, '#', colPos + 1);

            while (read(linesTmpFd, &chLine, 1) == 1 && chLine != '\n' && chLine != '#') {
              write(resultTmpFd, &chLine, 1);
            }

            if (atributoIndex < totalAtributos - 1) {
              char sep = '#';
              write(resultTmpFd, &sep, 1);
            }

            atributoIndex++;
            lineStart = lseek(linesTmpFd, 0, SEEK_SET);
            break;
          }

          if (chAttributes != chLine) {
            lseek(attributesTmpFd, relStart, SEEK_SET);
            findAndMove(linesTmpFd, '\n');
            lineStart = lseek(linesTmpFd, 0, SEEK_CUR);
          }
        }
      }

      char nl = '\n';
      write(resultTmpFd, &nl, 1);

      nivel--;
      if (nivel < 0) break;

      removeLastLine(linesTmpFd);
      int fdAnterior = prevFd(saveFd);
      if (fdAnterior == -1) continue;

      lseek(fdAnterior, 0, SEEK_SET);
      continue;
    }

    int fdActual = nextFd(saveFd);

    char ch;
    bool hasLine = false;
  
    while (read(saveFd, &ch, 1) == 1 && ch != '.') {
      write(linesTmpFd, &ch, 1);
    }

    ch = '#';
    write(linesTmpFd, &ch, 1);

    while (read(fdActual, &ch, 1) == 1) {
      hasLine = true;
      write(linesTmpFd, &ch, 1);
      if (ch == '\n') break;
    }

    if (hasLine) {
      nivel++;
    } else {
      // No hay más líneas: retroceder de nivel
      nivel--;
      if (nivel < 0) break;
      removeLastLine(linesTmpFd);

      int fdAnterior = prevFd(saveFd);
      if (fdAnterior == -1) break;

      lseek(fdAnterior, 0, SEEK_SET);
    }
  }
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
  
  int linesTmp = openFile(LINES_TMP, READ_WRITE_TRUNC_FLAGS);
  int resultTmp = openFile(RESULT_TMP, READ_WRITE_TRUNC_FLAGS);

  cartesianProduct(saveFd, attributesTmp, relations, linesTmp, resultTmp);

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
