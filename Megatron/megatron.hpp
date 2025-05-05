#ifndef MEGATRON_HPP
#define MEGATRON_HPP

#include <string>
#include <vector>

#include "diskManager.hpp"
#include "schemaManager.hpp"

class Megatron {
private:
  long int size;

  DiskManager *diskManager;
  SchemaManager *schemaManager;
  std::string flag;

  bool checkAttribute(const std::vector<std::string> &, const std::string &, const std::string &, bool, const std::string &);
  bool loadSchemaFiles(std::vector<std::string> &);

  bool recorrerCartesian(std::vector<std::string> &, const std::string &, bool);

  bool existTables(const std::vector<std::string> &, const std::string &);
  bool existAttributes(const std::vector<std::string> &, std::vector<std::string> &, const std::string &);

  short positionAttribute(const std::string &, const std::string &);

  bool shuntingYard(const std::string &, const std::vector<std::string> &);

  bool checkConditions();
  void showResult(const std::string &file);

  void clearCache();

public:
  Megatron();
  ~Megatron();
  void init();
  int showMenu();
  void selectMenu();
  void whereMenu(std::string &conditions, std::string &saveFile);
  void pipeMenu(std::string &saveFile);
  void selectFuntion(const std::string &select, const std::string &from, const std::string &conditions, const std::string &saveFile);
};

#endif
