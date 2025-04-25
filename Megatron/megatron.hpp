#ifndef MEGATRON_HPP
#define MEGATRON_HPP

#include "diskManager.hpp"
#include <string>
#include <vector>

struct AttributeInfo {
  std::string type;
  size_t tableIndex;
  size_t attributeIndex;
  size_t position;
};

class Megatron {
private:
  long int size;

  DiskManager *diskManager;

  bool loadSchema(const std::string &, const std::string &);
  bool loadSchemaFiles(std::vector<std::string> &);
  std::string getSchema(const std::string &, const std::string &);

  void recorrerCartesian(int, std::vector<std::string> &, const std::string &);

  bool existTables(const std::vector<std::string> &, const std::string &);
  bool existAttributes(const std::vector<std::string> &,
                       const std::vector<std::string> &, const std::string &);

  short int positionAttribute(const std::string &, const std::string &);

  std::string searchInEsquema(const std::string &, short int);
  
  void showResult();

  void clearCache();

public:
  Megatron() : size(500 * 1024 * 1024), diskManager(nullptr) {};
  ~Megatron();
  void init();
  int showMenu();
  void selectMenu();
  void whereMenu(const std::string &, const std::string &);
  void saveMenu(const std::string &, const std::string &, const std::string[]);
  void selectFuntion(const std::string &, const std::string &,
                     const std::string[], bool);
};

#endif
