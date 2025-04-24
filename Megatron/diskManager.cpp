#include "const.cpp"
#include "diskManager.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>

DiskManager::DiskManager(const std::string &path) : path(path) {}

DiskManager::~DiskManager() {
  for (auto &[name, fd] : files) {
    close(fd);
  }

  files.clear();
}

int DiskManager::openFile(const std::string &filename) {
  std::string fullPath = path + filename;
  int fd = open(fullPath.c_str(), OPEN_MODES);
  if (fd == -1) {
    perror(("Error al abrir archivo: " + fullPath).c_str());
    return -1;
  }
  files[filename] = fd;
  return fd;
}

bool DiskManager::closeFile(const std::string &filename) {
  if (files.find(filename) == files.end())
    return false;
  close(files[filename]);
  files.erase(filename);
  return true;
}

bool DiskManager::isOpen(const std::string &file) {
  return files.find(file) != files.end();
}

off_t DiskManager::getLine(const std::string &filename, std::string &line) {
  line.clear();
  if (files.find(filename) == files.end())
    return -1;

  int fd = files[filename];
  char buffer[BUFFER_SIZE];
  ssize_t bytesRead;
  off_t startPos = lseek(fd, 0, SEEK_CUR);

  if (startPos == -1) {
    perror("lseek failed");
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
    perror(("Error al leer el archivo " + filename).c_str());
    return -1;
  }

  if (!line.empty() || foundNewline) {
    lseek(fd, currentPos, SEEK_SET);
    return currentPos;
  }

  if (bytesRead == 0) {
    return -1;
  }

  perror(("No se pudo obtener la línea de " + filename).c_str());
  return -1;
}

off_t DiskManager::setPosition(const std::string &filename, off_t offset,
                               int whence) {
  if (files.find(filename) == files.end())
    return -1;
  return lseek(files[filename], offset, whence);
}

bool DiskManager::writeFileLine(const std::string &filename,
                                const std::string &line) {
  int fd;
  if (files.find(filename) == files.end()) {
    std::string fullPath = path + filename;
    fd = open(fullPath.c_str(), WRITE_FLAGS, WRITE_MODE);
    if (fd == -1) {
      perror(("Error al abrir archivo para escribir: " + fullPath).c_str());
      return false;
    }
    files[filename] = fd;
  } else {
    fd = files[filename];
    lseek(fd, 0, SEEK_END);
  }

  std::string finalLine = line + "\n";
  ssize_t written = write(fd, finalLine.c_str(), finalLine.size());

  if (written != (ssize_t)finalLine.size()) {
    perror(("Error al intentar escribir en el archivo " + filename).c_str());
    return false;
  }

  lseek(fd, 0, SEEK_SET);
  return true;
}

void DiskManager::deleteFile(const std::string &file) {
  std::string fullPath = path + file;
  const char *pointer = fullPath.c_str();
  remove(pointer);
}
