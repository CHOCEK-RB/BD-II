#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include "attribute.hpp"
#include <string>
#include <vector>

class dataBase {
private:
  const char *name;
  std::vector<attributeBase *> attributes;
  long int size_m;

  void create_register(const char *name, const char *type);
  std::vector<std::string> split(const std::string &linea);
  void case_titanic();

public:
  dataBase(const char *n);
  ~dataBase();
  void read_document(const char *filename);
  void write_text();
  void calculate_bytes();
  long int space() { return size_m; };
};

#endif
