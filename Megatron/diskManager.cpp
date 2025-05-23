#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

#include "const.cpp"
#include "diskManager.hpp"

DiskManager::DiskManager(const std::string &path) : path(path) {}

DiskManager::~DiskManager() {
  for (auto &[name, fd] : files) {
    close(fd);
  }

  files.clear();
}

int DiskManager::openFile(const std::string &file, int flags) {
  int fd;

  if (isOpen(file)) {
    close(files[file]);
  }

  if (flags == READ_FLAGS)
    fd = open((path + file).c_str(), flags);

  else
    fd = open((path + file).c_str(), flags, WRITE_MODE);

  if (fd == NOT_OPEN) {
    perror((ERR_OPEN_FILE + file).c_str());
    return NOT_OPEN;
  }

  files[file] = fd;
  return fd;
}

bool DiskManager::closeFile(const std::string &file) {
  if (!isOpen(file))
    return false;

  close(files[file]);
  files.erase(file);
  return true;
}

bool DiskManager::isOpen(const std::string &file) {
  return files.find(file) != files.end();
}

bool DiskManager::isEmpty(const std::string &file) {
  struct stat st;

  std::string fullpath = path + file;
  const char *fd = fullpath.c_str();

  if (stat(fd, &st) != 0)
    return true;

  return st.st_size == 0;
}

bool DiskManager::exist(const std::string &file) {
  return std::filesystem::exists(path + file);
}

off_t DiskManager::getLine(const std::string &file, std::string &line) {
  line.clear();

  if (!isOpen(file)) {
    perror((ERR_GETLINE_NOT_OPEN_FILE + file).c_str());
    return -1;
  }

  int fd = files[file];
  char buffer[BUFFER_SIZE];
  ssize_t bytesRead;
  off_t startPos = lseek(fd, 0, SEEK_CUR);

  if (startPos == -1) {
    perror("Error al usar la funcion lseek");
    return -1;
  }

  off_t currentPos = startPos;
  bool foundNewline = false;

  while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
    for (ssize_t i = 0; i < bytesRead; ++i) {
      ++currentPos;
      if (buffer[i] == '\n') {
        foundNewline = true;
        break;
      }
      line += buffer[i];
    }
    if (foundNewline)
      break;
  }

  if (bytesRead == -1) {
    perror((ERR_GETLINE_NOT_READ + file).c_str());
    return -1;
  }

  if (!line.empty() || foundNewline) {
    lseek(fd, currentPos, SEEK_SET);
    return currentPos;
  }

  if (bytesRead == 0) {
    return -1;
  }

  return -1;
}

off_t DiskManager::setPosition(const std::string &file,
                               off_t offset,
                               int whence) {
  if (!isOpen(file)) {
    perror((ERR_NOT_OPEN + file).c_str());
    return -1;
  }

  return lseek(files[file], offset, whence);
}

bool DiskManager::writeFileLine(const std::string &file,
                                const std::string &line) {
  int fd;

  if (!isOpen(file)) {
    if ((fd = openFile(file, READ_WRITE_FLAGS)) == NOT_OPEN) {
      return false;
    }

  } else {
    fd = files[file];
    lseek(fd, 0, SEEK_END);
  }

  std::string finalLine = line + "\n";
  ssize_t written = write(fd, finalLine.c_str(), finalLine.size());

  if (written != (ssize_t)finalLine.size()) {
    perror((ERR_NOT_WRIT_NEW_LINE + file).c_str());
    return false;
  }

  lseek(fd, 0, SEEK_SET);
  return true;
}

bool DiskManager::replaceLines(const std::string &file,
                               size_t start,
                               size_t numberLines,
                               const std::string &newText) {
  std::string tempPath = path + file + ".tmp";

  if (!isOpen(file)) {
    perror((ERR_NOT_OPEN + file).c_str());
    return false;
  }

  int fdOrig = files[file];
  int fdTemp = open(tempPath.c_str(), WRITE_FLAGS, 0644);

  if (fdTemp == NOT_OPEN) {
    perror(("Error al crear archivo temporal: " + tempPath).c_str());
    return false;
  }

  char buffer[BUFFER_SIZE];
  std::string line;
  ssize_t bytesRead;
  size_t currentLine = 0;
  off_t pos = 0;

  while ((bytesRead = pread(fdOrig, buffer, BUFFER_SIZE, pos)) > 0) {
    for (ssize_t i = 0; i < bytesRead; ++i) {
      pos++;
      if (buffer[i] == '\n') {
        if (currentLine == start + numberLines - 1 && !newText.empty()) {
          std::string replaced = newText + "\n";
          if (write(fdTemp, replaced.c_str(), replaced.size()) == -1) {
            perror("Error escribiendo línea reemplazada");
            close(fdTemp);
            return false;
          }

        } else if (currentLine < start || currentLine >= start + numberLines) {
          line += '\n';
          if (write(fdTemp, line.c_str(), line.size()) == -1) {
            perror("Error escribiendo línea normal");
            close(fdTemp);
            return false;
          }
        }
        line.clear();
        currentLine++;
      } else {
        line += buffer[i];
      }
    }
  }

  if (!line.empty() && currentLine - 1 == start + numberLines) {
    std::string replaced = newText + "\n";
    write(fdTemp, replaced.c_str(), replaced.size());

  } else if (!line.empty()) {
    line += "\n";
    write(fdTemp, line.c_str(), line.size());
  }

  closeFile(file);
  close(fdTemp);

  if (rename(tempPath.c_str(), (path + file).c_str()) == -1) {
    perror("Error al reemplazar archivo original");
    return false;
  }

  if (openFile(file, READ_FLAGS) == NOT_OPEN)
    return false;

  return true;
}

bool DiskManager::copyFile(const std::string &file,
                           const std::string &copyFile) {
  if (!isOpen(file)) {
    perror((ERR_NOT_OPEN + file).c_str());
    return false;
  }

  int fd = files[file];
  int fdCopy;

  if ((fdCopy = openFile(copyFile, READ_WRITE_FLAGS)) == NOT_OPEN)
    return false;

  char buffer[BUFFER_SIZE];
  std::string line;
  ssize_t bytesRead;

  while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
    ssize_t written = write(fdCopy, buffer, bytesRead);
    if (written != bytesRead) {
      perror("Error al escribir en el archivo destino");
      return false;
    }
  }

  lseek(fd, 0, SEEK_SET);
  lseek(fdCopy, 0, SEEK_SET);

  return true;
}

void DiskManager::deleteFile(const std::string &file) {
  std::string fullPath = path + file;
  const char *pointer = fullPath.c_str();
  remove(pointer);
}
