#include "megatron.hpp"
#include "const.cpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


Megatron::~Megatron() {
  delete diskManager;
}
bool Megatron::loadSchema(const std::string &file, const std::string &schema) {
  std::string line;

  while (diskManager->getLine(file, line) != END) {
    size_t pos = line.find('#');
    if (pos != std::string::npos) {
      std::string _schema = line.substr(0, pos);
      if (_schema == schema){
        if(diskManager->writeFileLine(TMP_SCHEMAS, line)){
          diskManager->setPosition(file, 0, SEEK_SET);
          return true;
        };
      }
    }
  }

  return false;
}

std::string Megatron::getSchema(const std::string &file, const std::string &schema){
  std::string line;

  while(diskManager ->getLine(file, line) != END){
    size_t pos = line.find('#');
    if (pos != std::string::npos) {
      std::string _schema = line.substr(0, pos);
      if (_schema == schema){
        diskManager->setPosition(file, 0, SEEK_SET);
        return line;
      }
    }
  }

  return "";
}

std::string trim(const std::string &string) {
  size_t start = string.find_first_not_of(" \t\n\r");
  size_t end = string.find_last_not_of(" \t\n\r");
  return (start == std::string::npos) ? ""
                                      : string.substr(start, end - start + 1);
}

std::vector<std::string> Megatron::split(const std::string &text,
                                         char delimiter) {
  std::vector<std::string> result;
  std::stringstream ss(text);
  std::string item;

  while (getline(ss, item, delimiter)) {
    item = trim(item);
    result.push_back(item);
  }
  return result;
}

void Megatron::recorrerCartesian(int nivel, std::vector<int> &tablesOrder,
                                 std::vector<std::string> &files,
                                 std::vector<std::string> &currentLines,
                                 const std::string &tmp) {
  if (nivel == files.size()) {
    // Tenemos una combinación completa
    std::string outputLine;
    for (const auto &attr : this->attributesInfo) {
      const std::string &line = currentLines[attr.tableIndex];
      auto part = searchInEsquema(line, attr.position) + "#";
      outputLine += part;
    }
    outputLine.pop_back();
    diskManager -> writeFileLine(tmp, outputLine);
    return;
  }

  diskManager->setPosition(files[nivel], 0, SEEK_SET);

  std::string line;
  while (diskManager->getLine(files[nivel], line) != END) {
    currentLines[tablesOrder[nivel]] = line;
    recorrerCartesian(nivel + 1, tablesOrder, files, currentLines, tmp);
  }
};

short Megatron::positionAttribute(const std::string &line,
                                  const std::string &attribute) {
  size_t pos = 0;
  size_t start = line.find('#') + 1;
  size_t end = line.find('#', start);

  while (end != std::string::npos) {
    if (line.substr(start, end - start) == attribute) {
      return pos / 2;
    }
    start = end + 1;
    end = line.find('#', start);
    ++pos;
  }

  return -1;
}

std::string Megatron::searchInEsquema(const std::string &line, short position) {
  int count = 0;
  size_t start = 0, end = line.find('#');
  while (count < position && end != std::string::npos) {
    start = end + 1;
    end = line.find('#', start);
    ++count;
  }

  if (end == std::string::npos)
    end = line.size();
  return line.substr(start, end - start);
}

bool Megatron::existTables(const std::vector<std::string> &tables, const std::string &file) {
  for (const auto &table : tables) {
    if (!loadSchema(file, table)) {
      std::cerr << "La tabla '" << table << "' no existe en esquemas.\n";
      return false;
    }
  }

  return true;
}

bool Megatron::existAttributes(const std::vector<std::string> &tables,
                               const std::vector<std::string> &attributes,
                               const std::string &file) {
  if (!this->attributesInfo.empty()) {
    this->attributesInfo.clear();
  }

  for (size_t i = 0; i < attributes.size(); ++i) {
    const auto &attr = attributes[i];
    bool found = false;

    size_t dotPos = attr.find('.');
    if (dotPos != std::string::npos) {
      std::string table = attr.substr(0, dotPos);
      std::string attribute = attr.substr(dotPos + 1);

      auto it = std::find(tables.begin(), tables.end(), table);
      if (it == tables.end()) {
        std::cerr << "La tabla del atributo " << attr
                  << " no se ha encontrado\n";
        return false;
      }

      std::string schemaLine = getSchema(file, table);
      short pos = positionAttribute(schemaLine, attribute);
      if (pos == -1) {
        std::cerr << "El atributo " << attr << " no se ha encontrado\n";
        return false;
      }

      std::string type = searchInEsquema(schemaLine, (pos + 1) * 2);
      AttributeInfo attrInfo;
      attrInfo.type = type;
      attrInfo.attributeIndex = i;
      attrInfo.tableIndex = std::distance(tables.begin(), it);
      attrInfo.position = pos;
      attributesInfo.push_back(attrInfo);

    } else {
      for (size_t j = 0; j < tables.size(); ++j) {
        std::string schemaLine = getSchema(file, tables[j]);
        short pos = positionAttribute(schemaLine, attr);
        if (pos != -1) {
          if (found) {
            std::cerr << "Especifique la tabla del atributo " << attr << "\n";
            return false;
          }
          std::string type = searchInEsquema(schemaLine, (pos + 1) * 2);
          AttributeInfo attrInfo;
          attrInfo.type = type;
          attrInfo.attributeIndex = i;
          attrInfo.tableIndex = j;
          attrInfo.position = pos;
          attributesInfo.push_back(attrInfo);
          found = true;
        }
      }

      if (!found) {
        std::cerr << "El atributo " << attr << " no se ha encontrado\n";
        return false;
      }
    }
  }

  return true;
}

void Megatron::init() {
  std::cout << "% Megatron 3000\n\tWelcome to Megatron 3000!\n";
  switch (showMenu()) {
  case 1:
    selectMenu();
    break;
  case 2:
    return;
  }
}

int Megatron::showMenu() {
  std::cout << "% Opciones\n1) SELECT\n2) Salir\n\n";
  int choice;
  std::cin >> choice;
  return choice;
}

void Megatron::selectMenu() {
  std::string select, from;
  std::cout << "& SELECT : ";
  std::cin.ignore();
  std::getline(std::cin, select);
  std::cout << "& FROM : ";
  std::getline(std::cin, from);

  std::cout << "\n& SELECT " << select << " FROM " << from << "\n\n";
  std::cout << "% Opciones\n1) WHERE\n2) Ejecutar\n";

  int choice;
  std::cin >> choice;

  if (choice == 1)
    whereMenu(select, from);
  else
    selectFuntion(select, from, {}, false);
}

void Megatron::whereMenu(const std::string &select, const std::string &from) {
  // Implementación futura
}

void Megatron::saveMenu(const std::string &select, const std::string &from,
                        const std::string conditions[]) {
  // Implementación futura
}

void Megatron::selectFuntion(const std::string &select, const std::string &from,
                             const std::string conditions[], bool save) {
  diskManager = new DiskManager(PATH);

  diskManager->openFile(SCHEMA);

  if(!diskManager->isOpen(SCHEMA)){
    return;
  }

  auto tables = split(from, ',');
  if (!existTables(tables, SCHEMA))
    return;

  auto atributos = split(select, ',');
  if (!existAttributes(tables, atributos, TMP_SCHEMAS))
    return;

  // Muestra información de atributos (debug)
  for (auto &attr : this->attributesInfo) {
    std::cout << attr.type << "|" << attr.attributeIndex << "|"
              << attr.tableIndex << "|" << attr.position << std::endl;
  }

  // Orden único de tablas basado en los atributos
  std::vector<int> tablesOrder;
  for (const auto &attr : attributesInfo) {
    auto it =
        std::find(tablesOrder.begin(), tablesOrder.end(), attr.tableIndex);
    if (it == tablesOrder.end()) {
      tablesOrder.push_back(attr.tableIndex);
    }
  }

  std::vector<std::string> filePaths;
  for (int index : tablesOrder) {
    filePaths.push_back(tables[index] + ".txt");
  }

  for (const auto &path : filePaths) {
    diskManager -> openFile(path);

    if(!diskManager->isOpen(path)){
      std::cerr << "Error al abrir el archivo " << path << "\n";
    }
  }

  std::vector<std::string> currentLines(tablesOrder.size());

  recorrerCartesian(0, tablesOrder, filePaths, currentLines, TMP_RESULT);

  delete diskManager; 
}
