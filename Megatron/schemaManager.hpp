#ifndef SCHEMAMANAGER_HPP
#define SCHEMAMANAGER_HPP

#include "const.cpp"
#include "diskManager.hpp"
#include <string>

class SchemaManager {
public:
  SchemaManager(DiskManager *);
  ~SchemaManager() {};

  bool checkSchema(const std::string &file, const std::string &schema,
                   bool load = true, const std::string &saveFile = TMP_SCHEMAS);

  std::string getSchema(const std::string &, const std::string &);

  std::string searchInEsquema(const std::string &, short int);

  bool createSchema(std::string &fileName);

private:
  DiskManager *diskManager;
};

#endif
