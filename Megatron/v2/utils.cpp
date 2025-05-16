#include "const.cpp"
#include "utils.hpp"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int openFile(const char *fileName,
             int flags,
             bool registrar,
             int saveFd,
             int fdSize) {
  int sizePath = 0, sizeFile = 0;

  while (PATH[sizePath] != '\0')
    sizePath++;
  if (fdSize == -1) {
    while (fileName[sizeFile] != '\0')
      sizeFile++;
  } else {
    sizeFile = fdSize;
  }

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

void renameFile(const char *fileName,
                const char *newFileName,
                int sizeFileName,
                int sizeNewFileName) {
  int sizePath = 0, sizeFile = 0, sizeNewFile = 0;

  while (PATH[sizePath] != '\0')
    sizePath++;

  if (sizeFileName == -1) {
    while (fileName[sizeFile] != '\0')
      sizeFile++;
  } else {
    sizeFile = sizeFileName;
  }

  if (sizeNewFileName == -1) {
    while (newFileName[sizeNewFile] != '\0')
      sizeNewFile++;
  } else {
    sizeNewFile = sizeNewFileName;
  }

  char fullPath[sizePath + sizeFile + 1];
  char newFullPath[sizePath + sizeNewFile + 1];

  for (int i = 0; i < sizePath; ++i) {
    fullPath[i] = PATH[i];
    newFullPath[i] = PATH[i];
  }

  for (int i = 0; i < sizeFile; ++i)
    fullPath[sizePath + i] = fileName[i];
  fullPath[sizePath + sizeFile] = '\0';

  for (int i = 0; i < sizeNewFile; ++i)
    newFullPath[sizePath + i] = newFileName[i];
  newFullPath[sizePath + sizeNewFile] = '\0';

  remove(newFullPath);
  rename(fullPath, newFullPath);
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

void clearFile(int saveFd, int fd) {
  if (!searchFd(saveFd, fd)) {
    write(STDERR_FILENO, "FD no registrado\n", 17);
    return;
  }

  lseek(fd, 0, SEEK_SET);

  char ch;

  while (read(saveFd, &ch, 1) == 1 && ch != '=')
    ;

  int nameLen = 0;
  off_t nameStart = lseek(saveFd, 0, SEEK_CUR);
  while (read(saveFd, &ch, 1) == 1 && ch != '\n') {
    nameLen++;
  }

  lseek(saveFd, nameStart, SEEK_SET);

  char fileName[nameLen + 1];
  read(saveFd, fileName, nameLen);
  fileName[nameLen] = '\0';

  char tmpFileName[nameLen + 5];
  int i = 0;
  while (i < nameLen) {
    tmpFileName[i] = fileName[i];
    i++;
  }
  tmpFileName[i++] = '.';
  tmpFileName[i++] = 't';
  tmpFileName[i++] = 'm';
  tmpFileName[i++] = 'p';
  tmpFileName[i] = '\0';

  int tmp =
      openFile(tmpFileName, READ_WRITE_TRUNC_FLAGS, false, -1, nameLen + 4);
  if (tmp == -1) {
    return;
  }

  while (read(fd, &ch, 1) == 1) {
    if (ch != '\0')
      write(tmp, &ch, 1);
  }

  close(tmp);

  renameFile(tmpFileName, fileName, nameLen + 4, nameLen);
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

int findAndMove(const int &fd, char delimiter, int times) {
  char ch;
  for (int i = 0; i < times; ++i) {
    int startPos = lseek(fd, 0, SEEK_CUR);
    while (read(fd, &ch, 1) == 1) {
      if (ch == delimiter) {
        break;
      }

      if (ch == '\n') {
        lseek(fd, startPos, SEEK_SET);
        return -1;
      }
    }
  }

  return lseek(fd, 0, SEEK_CUR);
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

bool copyLine(const int &from, const int &to) {
  char ch;
  bool copied = false;

  while (read(from, &ch, 1) == 1) {
    copied = true;
    write(to, &ch, 1);

    if (ch == '\n') {
      break;
    }
  }

  return copied;
}

bool removeLastLine(const int &fd) {
  off_t size = lseek(fd, 0, SEEK_END);
  if (size == 0)
    return false;

  char ch;
  off_t pos = size - 1;

  while (pos >= 0) {
    lseek(fd, pos, SEEK_SET);
    if (read(fd, &ch, 1) != 1)
      return false;

    if (ch == '\n' && pos != size - 1) {
      if (ftruncate(fd, pos + 1) == -1)
        return false;

      lseek(fd, pos, SEEK_SET);
      return true;
    }

    --pos;
  }
  
  if (ftruncate(fd, 0) == -1)
    return false;

  lseek(fd, 0, SEEK_SET);
  return true;
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
      }
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
  int startPos = lseek(saveFd, 0, SEEK_SET),
      fileSize = lseek(saveFd, 0, SEEK_END);
  char ch;
  bool found = false;

  for (off_t i = fileSize; i >= 0; --i) {
    lseek(saveFd, i, SEEK_SET);

    if (read(saveFd, &ch, 1) != 1) {
      lseek(saveFd, 0, SEEK_SET);
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
