#ifndef DISKMANAGER_HPP
#define DISKMANAGER_HPP

#include <string>
#include <sys/types.h>
#include <unordered_map>
class DiskManager {
private:
  const std::string path;
  std::unordered_map<std::string, int> files;

public:
  DiskManager(const std::string &);
  ~DiskManager();

  int openFile(const std::string &, int);
  bool closeFile(const std::string &);
  bool isOpen(const std::string &);

  off_t getLine(const std::string &, std::string &);
  off_t setPosition(const std::string &, off_t, int);

  bool writeFileLine(const std::string &, const std::string &);
  bool replaceLine(const std::string &, size_t, const std::string &);
  void deleteFile(const std::string &);
};

#endif
