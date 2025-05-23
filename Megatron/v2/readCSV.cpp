#include "utils.hpp"
#include "const.cpp"
#include "readCSV.hpp"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int openCSV(const int &input, const int &saveFd) {
  int sizeCSV = trimFile(input);

  if (sizeCSV == -1 || sizeCSV < 5) {
    std::cerr << "No se ingreso un archivo correcto";
    return -1;
  }

  char fileCSV[sizeCSV + 1];
  char ch;

  for (int i = 0; i < sizeCSV; ++i) {
    if (read(input, &ch, 1) == 1) {
      fileCSV[i] = ch;
    }
  }

  fileCSV[sizeCSV] = '\0';

  if (fileCSV[sizeCSV - 4] != '.' || fileCSV[sizeCSV - 3] != 'c' ||
      fileCSV[sizeCSV - 2] != 's' || fileCSV[sizeCSV - 1] != 'v') {
    std::cerr << "No se ingreso un archivo csv";
    return -1;
  }

  int fdCSV = openFile(fileCSV, O_RDONLY);
  registerFile(saveFd, fdCSV, fileCSV);
  return fdCSV;
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

  fileName[sizeFile - 3] = 't';
  fileName[sizeFile - 2] = 'x';
  fileName[sizeFile - 1] = 't';

  int fd = openFile(fileName, READ_WRITE_TRUNC_FLAGS);
  return fd;
}

void createSchemaCSV(const int &fdCSV, const int &saveFd, const int &fdSchema) {
  searchFd(saveFd, fdCSV);
  findAndMove(saveFd, '=');
  
  lseek(fdSchema, 0, SEEK_END);
  int startPos = lseek(fdSchema, 0, SEEK_CUR);

  char ch;
  while (read(saveFd, &ch, 1) == 1 && ch != '.') {
    write(fdSchema, &ch, 1);
  }

  ch = '#';
  write(fdSchema, &ch, 1);
  int parenth = 0;
  while (read(fdCSV, &ch, 1) == 1) {
    if (ch == '\r') {
      continue;
    }

    if (ch == ',') {
      ch = '#';
      write(fdSchema, &ch, 1);

      for (int i = 0; i < BYTES_FOR_ATTRIBUTES; ++i) {
        ch = '\0';
        write(fdSchema, &ch, 1);
      }

      ch = '#';
      write(fdSchema, &ch, 1);
    } else if (ch == '\n') {
      ch = '#';
      write(fdSchema, &ch, 1);

      for (int i = 0; i < BYTES_FOR_ATTRIBUTES; ++i) {
        ch = '\0';
        write(fdSchema, &ch, 1);
      }

      ch = '\n';
      write(fdSchema, &ch, 1);

      lseek(fdSchema, startPos, SEEK_SET);
      return;
    } else {
      write(fdSchema, &ch, 1);
    }
  }

  ch = '\n';
  write(fdSchema, &ch, 1);

  close(fdSchema);
}

State checkState(const int &relations) {
  char ch;
  if (read(relations, &ch, 1) != 1) {
    return UNKNOWN;
  }

  lseek(relations, -1, SEEK_CUR);
  switch (ch) {
  case '\0':
    return START;
  case 'V':
    return IS_VARCHAR;
  case 'I':
    return IS_INT;
  case 'F':
    return IS_FLOAT;
  default:
    return UNKNOWN;
  }
}

void comparateTypes(const int &relations, State state, int &lengthAttribute) {

  if (state == START)
    return;

  int startPos = lseek(relations, 0, SEEK_CUR);
  bool changeType = false, changeSize = false;

  char ch;

  if (!changeType) {
    findAndMove(relations, '(');
    int lenght = 0;

    while (read(relations, &ch, 1) == 1 && isdigit(ch))
      lenght = lenght * 10 + (ch - '0');

    if (lenght < lengthAttribute) {
      changeSize = true;
    } else {
      lengthAttribute = lenght;
    }

    lseek(relations, startPos, SEEK_SET);
  }

  State actualyState = checkState(relations);
  if (actualyState != state && actualyState != IS_VARCHAR) {
    if (state == IS_VARCHAR) {
      changeType = true;
    } else if (state == IS_FLOAT && actualyState != IS_FLOAT) {
      changeType = true;
    } else if (state == IS_INT && actualyState != IS_INT) {
      changeType = true;
    }
  }

  if (changeType) {

    if (state == IS_VARCHAR) {
      write(relations, "VARCHAR", 7);

    } else if (state == IS_FLOAT) {

      write(relations, "FLOAT", 5);
    } else if (state == IS_INT) {

      write(relations, "INT", 3);
    } else {
      return;
    }

    ch = '(';
    write(relations, &ch, 1);
    writeInt(relations, lengthAttribute);
    ch = ')';
    write(relations, &ch, 1);

    while (read(relations, &ch, 1) == 1 && ch != '\0' && ch != '#' &&
           ch != '\n') {
      ch = '\0';
      lseek(relations, -1, SEEK_CUR);
      write(relations, &ch, 1);
    }
    lseek(relations, -1, SEEK_CUR);

  } else if (changeSize) {

    findAndMove(relations, '(');

    writeInt(relations, lengthAttribute);
    ch = ')';
    write(relations, &ch, 1);

    while (read(relations, &ch, 1) == 1 && ch != '\0' && ch != '#' &&
           ch != '\n') {
      ch = '\0';
      lseek(relations, -1, SEEK_CUR);
      write(relations, &ch, 1);
    }
    lseek(relations, -1, SEEK_CUR);
  }
}

void readRegistersTsv(const int &fdCSV,
                      const int &fdTXT,
                      const int &relations) {
  int startPos = lseek(relations, 0, SEEK_CUR);
  
  findAndMove(relations, '#');
  findAndMove(relations, '#');
  
  char ch;
  State state = START;
  
  int lengthAttribute = 0;
  bool inQuotes = false;

  while (read(fdCSV, &ch, 1) == 1) {
    if (ch == '\r')
      continue;

    if (state == START && ch == '\n'){
      write(fdTXT, &ch, 1);
      
      lseek(relations, startPos, SEEK_SET);
      findAndMove(relations, '#');
      findAndMove(relations, '#');
      continue;
    }

    switch (state) {
    case START:
      if (ch == '\"') {
        inQuotes = true;
        state = IS_VARCHAR;
        continue;
      } else if (isalpha(ch) || isspace(ch)) {
        state = IS_VARCHAR;
        write(fdTXT, &ch, 1);
      } else if (isdigit(ch)) {
        state = IS_INT;
        write(fdTXT, &ch, 1);
      } else if (ch == '-') {
        state = IS_INT;
        write(fdTXT, &ch, 1);
      } else if (ch == ',') {
        comparateTypes(relations, state, lengthAttribute);
        write(fdTXT, "#", 1);

        findAndMove(relations, '#');
        findAndMove(relations, '#');
        state = START;
      } else if (ch == '\n') {
        comparateTypes(relations, state, lengthAttribute);
        write(fdTXT, "\n", 1);

        lseek(relations, startPos, SEEK_SET);
        findAndMove(relations, '#');
        findAndMove(relations, '#');
        state = START;
      } else {

        state = IS_VARCHAR;
        write(fdTXT, &ch, 1);
      }
      ++lengthAttribute;
      continue;

    case IS_VARCHAR:
      if (ch == '\"') {

        if (read(fdCSV, &ch, 1) == 1) {
          if (ch == ',' || ch == '\n') {
            inQuotes = false;
            comparateTypes(relations, state, lengthAttribute);
            ch = (ch == ',') ? '#' : '\n';
            write(fdTXT, &ch, 1);

            if (ch == '\n') {
              lseek(relations, startPos, SEEK_SET);
            }

            findAndMove(relations, '#');
            findAndMove(relations, '#');

            state = START;
            lengthAttribute = 0;

          } else {
            write(fdTXT, &ch, 1);
            ++lengthAttribute;
          }
        } else {
          continue;
        }
      } else if (!inQuotes && (ch == ',' || ch == '\n')) {

        comparateTypes(relations, state, lengthAttribute);
        ch = (ch == ',') ? '#' : '\n';
        write(fdTXT, &ch, 1);

        if (ch == '\n') {
          lseek(relations, startPos, SEEK_SET);
        }
        findAndMove(relations, '#');
        findAndMove(relations, '#');

        state = START;
        lengthAttribute = 0;

      } else {
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      }
      continue;

    case IS_INT:
      if (isdigit(ch)) {
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      } else if (ch == '.') {
        state = IS_FLOAT;
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      } else if (ch == ',' || ch == '\n') {
        comparateTypes(relations, state, lengthAttribute);
        ch = (ch == ',') ? '#' : '\n';
        write(fdTXT, &ch, 1);

        if (ch == '\n') {
          lseek(relations, startPos, SEEK_SET);
        }

        findAndMove(relations, '#');
        findAndMove(relations, '#');

        state = START;
        lengthAttribute = 0;
      } else {

        state = IS_VARCHAR;
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      }
      continue;

    case IS_FLOAT:
      if (isdigit(ch)) {
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      } else if (ch == ',' || ch == '\n') {
        comparateTypes(relations, state, lengthAttribute);
        ch = (ch == ',') ? '#' : '\n';
        write(fdTXT, &ch, 1);

        if (ch == '\n') {
          lseek(relations, startPos, SEEK_SET);
        }

        findAndMove(relations, '#');
        findAndMove(relations, '#');

        state = START;
        lengthAttribute = 0;
      } else {

        state = IS_VARCHAR;
        write(fdTXT, &ch, 1);
        ++lengthAttribute;
      }
      continue;

    default:
      continue;
    }

    if (lseek(fdCSV, 0, SEEK_CUR) == lseek(fdCSV, 0, SEEK_END))
      break;
  }
}

bool readTsv(const int &input, const int &saveFd) {
  int fdCSV = openCSV(input, saveFd);
  int fdSchema = openFile(SCHEMA, READ_WRITE_FLAGS, true, saveFd);
  if (fdCSV == -1) {
    return false;
  }

  searchFd(saveFd, fdCSV);

  findAndMove(saveFd, '=');

  int fdTXT = openTxtFromTsv(saveFd);

  createSchemaCSV(fdCSV, saveFd, fdSchema);

  readRegistersTsv(fdCSV, fdTXT, fdSchema);

  clearFile(saveFd, fdSchema);

  close(fdCSV);
  close(fdTXT);
  return true;
}
