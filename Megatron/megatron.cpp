#include "megatron.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

Megatron::~Megatron() {}

void Megatron::loadSchemas(std::ifstream &file) {
  if (!this->schemas.empty()) {
    this->schemas.clear();
  }

  std::string line;

  while (getline(file, line)) {
    size_t pos = line.find('#');
    if (pos != std::string::npos) {
      std::string table = line.substr(0, pos);
      this->schemas[table] = line;
    }
  }
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
                                 std::vector<std::ifstream> &files,
                                 std::vector<std::string> &currentLines,
                                 std::ofstream &tmp) {
  if (nivel == files.size()) {
    // Tenemos una combinación completa
    std::string outputLine;
    for (const auto &attr : this->attributesInfo) {
      const std::string &line = currentLines[attr.tableIndex];
      /*
      auto parts = split(line, '#'); // suponiendo TSV
      if (attr.attributeIndex < parts.size()) {
        outputLine += parts[attr.position] + "#";
      } else {
        outputLine += "#";
      }
      */
      auto part = searchInEsquema(line, attr.position) + "#";
      outputLine += part;
    }
    outputLine.pop_back(); // quitar último #
    tmp << outputLine << "\n";
    return;
  }

  std::ifstream &file = files[nivel];
  file.clear();
  file.seekg(0);

  std::string line;
  while (getline(file, line)) {
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

std::string Megatron::searchTable(const std::string &table) {

  if (this->schemas.find(table) == this->schemas.end()) {
    return "";
  }

  return this->schemas[table];
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

bool Megatron::existTables(const std::vector<std::string> &tables) {
  for (const auto &table : tables) {
    if (this->schemas.find(table) == this->schemas.end()) {
      std::cerr << "La tabla '" << table << "' no existe en esquemas.\n";
      return false;
    }
  }

  return true;
}

bool Megatron::existAttributes(const std::vector<std::string> &tables,
                               const std::vector<std::string> &attributes) {
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

      std::string schemaLine = searchTable(table);
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
        std::string schemaLine = searchTable(tables[j]);
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
  std::filesystem::path path = PATH;
  if (!std::filesystem::is_directory(path))
    std::filesystem::create_directory(PATH);

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
  std::ifstream esquemas(PATH "/db/esquema.txt");

  if (!esquemas.is_open()) {
    return;
  }

  loadSchemas(esquemas);

  auto tables = split(from, ',');
  if (!existTables(tables))
    return;

  auto atributos = split(select, ',');
  if (!existAttributes(tables, atributos))
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

  // Cargar paths de los archivos
  std::vector<std::string> filePaths;
  for (int index : tablesOrder) {
    filePaths.push_back(PATH "/db/" + tables[index] + ".txt");
  }

  // Abrir los archivos
  std::vector<std::ifstream> files;
  for (const auto &path : filePaths) {
    std::ifstream file(path);
    if (!file.is_open()) {
      std::cerr << "Error al abrir el archivo: " << path << std::endl;
      return;
    }
    files.push_back(std::move(file));
  }

  std::ofstream tmp(PATH "/tmp.txt");
  if (!tmp.is_open()) {
    std::cerr << "No se pudo crear el archivo temporal.\n";
    return;
  }

  std::vector<std::string> currentLines(tablesOrder.size());

  // Función recursiva para recorrer el producto cartesiano
  recorrerCartesian(0, tablesOrder, files, currentLines, tmp);

  // Cerrar archivos
  for (auto &f : files) {
    f.close();
  }

  tmp.close();
}
