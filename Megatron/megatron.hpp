#ifndef MEGATRON_HPP
#define MEGATRON_HPP

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#define PATH "/home/CHOCEK//tmp/megatron"

struct AttributeInfo {
  std::string type;
  size_t tableIndex;
  size_t attributeIndex;
  size_t position;
};

class Megatron {
private:
  long int size;
  std::unordered_map<std::string, std::string> schemas;
  std::vector<AttributeInfo> attributesInfo;

  void loadSchemas(std::ifstream &);
  void recorrerCartesian(int, std::vector<int> &, std::vector<std::ifstream> &,
                         std::vector<std::string> &, std::ofstream &);

  std::vector<std::string> split(const std::string &, const char);

  bool existTables(const std::vector<std::string> &);
  bool existAttributes(const std::vector<std::string> &,
                       const std::vector<std::string> &);

  short int positionAttribute(const std::string &, const std::string &);

  std::string searchTable(const std::string &);
  std::string searchInEsquema(const std::string &, short int);

public:
  Megatron() : size(500 * 1024 * 1024) {};
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
