#ifndef DISKMANAGER_HPP
#define DISKMANAGER_HPP

#include <sys/types.h>

#include <string>
#include <unordered_map>
class DiskManager {
private:
  const std::string path;
  std::unordered_map<std::string, int> files;

public:
  DiskManager(const std::string &path);
  ~DiskManager();

  int openFile(const std::string &file, int flags);
  bool closeFile(const std::string &file);
  bool isOpen(const std::string &file);
  bool isEmpty(const std::string &file);
  bool exist(const std::string &file);

  off_t getLine(const std::string &file, std::string &line);
  off_t setPosition(const std::string &file, off_t position, int whence);

  bool writeFileLine(const std::string &file, const std::string &line);
  bool replaceLines(const std::string &file, size_t start = 0, size_t numberLines = 1, const std::string &newText = "");
  bool copyFile(const std::string &file, const std::string &copyFile);
  void deleteFile(const std::string &file);
};

#endif
