#include "const.cpp"
#include "megatron.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

Megatron::~Megatron() { delete diskManager; }

bool Megatron::loadSchema(const std::string &file, const std::string &schema) {
  std::string line;

  while (diskManager->getLine(file, line) != END) {
    size_t pos = line.find('#');
    if (pos != std::string::npos) {
      std::string _schema = line.substr(0, pos);
      if (_schema == schema && diskManager->writeFileLine(TMP_SCHEMAS, line)) {
        diskManager->setPosition(file, 0, SEEK_SET);
        return true;
      }
    }
  }

  return false;
}

bool Megatron::loadSchemaFiles(std::vector<std::string> &files) {
  if (!diskManager->isOpen(TMP_ATTRIBUTES)) {
    diskManager->openFile(TMP_ATTRIBUTES, READ_FLAGS);
  }

  std::string line;

  while (diskManager->getLine(TMP_ATTRIBUTES, line) != END) {
    auto file = line.substr(0, line.find(".")) + ".txt";

    if (!diskManager->isOpen(file)) {
      if (diskManager->openFile(file, READ_FLAGS) == NOT_OPEN)
        return false;

      files.push_back(file);
    }
  }
  return true;
};

std::string Megatron::getSchema(const std::string &file,
                                const std::string &schema) {
  std::string line;

  while (diskManager->getLine(file, line) != END) {
    size_t pos = line.find('#');

    if (pos != std::string::npos) {
      std::string _schema = line.substr(0, pos);

      if (_schema == schema) {
        diskManager->setPosition(file, 0, SEEK_SET);
        return line;
      }
    }
  }

  return "";
}

void Megatron::recorrerCartesian(int nivel, std::vector<std::string> &files,
                                 const std::string &tmp) {
  if (!diskManager->isOpen(tmp)) {
    diskManager->setPosition(TMP_ATTRIBUTES, 0, SEEK_SET);

    std::string attributesLine;

    for (std::string line; diskManager->getLine(TMP_ATTRIBUTES, line) != END;) {
      auto schemaAttribute = searchInEsquema(line, 0);
      auto attribute = schemaAttribute.substr(schemaAttribute.find(".") + 1);
      attributesLine += attribute + "#";

      diskManager->writeFileLine(TMP_SIZE_ATTRIBUTES,
                                 schemaAttribute + "#" +
                                     std::to_string(attribute.size()));
    }

    attributesLine.pop_back();
    diskManager->writeFileLine(tmp, attributesLine);
  }

  if (nivel == files.size()) {
    std::string outputLine;
    std::string attr;

    diskManager->setPosition(TMP_ATTRIBUTES, 0, SEEK_SET);
    diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

    int i = 0;
    while (diskManager->getLine(TMP_ATTRIBUTES, attr) != END) {

      auto schemaAttribute = searchInEsquema(attr, 0);
      auto schema = attr.substr(0, schemaAttribute.find("."));

      auto position = searchInEsquema(attr, 2);

      auto line = getSchema(TMP_LINES, schema);
      auto part = searchInEsquema(line, std::stoi(position) + 1);

      line = getSchema(TMP_SIZE_ATTRIBUTES, schemaAttribute);
      auto sizeAttr = searchInEsquema(line, 1);

      if (part.size() > std::stoi(sizeAttr)) {
        line = schemaAttribute + "#" + std::to_string(part.size());

        if (!diskManager->replaceLine(TMP_SIZE_ATTRIBUTES, i, line))
          return;
      }

      outputLine += part + "#";
      ++i;
    }

    outputLine.pop_back();
    diskManager->writeFileLine(tmp, outputLine);
    return;
  }

  diskManager->setPosition(files[nivel], 0, SEEK_SET);

  std::string line;
  while (diskManager->getLine(files[nivel], line) != END) {
    line = files[nivel].substr(0, files[nivel].find(".")) + "#" + line;
    if (!diskManager->replaceLine(TMP_LINES, nivel, line)) {
      return;
    }
    recorrerCartesian(nivel + 1, files, tmp);
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

bool Megatron::existTables(const std::vector<std::string> &tables,
                           const std::string &file) {
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

      diskManager->writeFileLine(TMP_ATTRIBUTES,
                                 attr + "#" + type + "#" + std::to_string(pos));

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

          diskManager->writeFileLine(TMP_ATTRIBUTES,
                                     tables[j] + "." + attributes[i] + "#" +
                                         type + "#" + std::to_string(pos));

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

void Megatron::showResult() {
  std::string line;
  std::string lineVertical;

  diskManager->setPosition(TMP_RESULT, 0, SEEK_SET);
  diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

  if (diskManager->getLine(TMP_RESULT, line) != END) {
    int i = 0;

    std::string lineSize;
    std::string outputLine;

    while (diskManager->getLine(TMP_SIZE_ATTRIBUTES, lineSize) != END) {
      int size = std::stoi(searchInEsquema(lineSize, 1));

      auto part = searchInEsquema(line, i);

      if (i == 0) {
        outputLine += "|";
        lineVertical += "+";
      }

      auto partFinal = " " + part + std::string((size - part.size() + 3), ' ');
      outputLine += partFinal + "|";
      lineVertical += std::string(partFinal.size(), '-') + "+";

      ++i;
    }

    std::cout << lineVertical << "\n"
              << outputLine << "\n"
              << lineVertical << "\n";
  }

  while (diskManager->getLine(TMP_RESULT, line) != END) {
    int i = 0;

    std::string lineSize;
    std::string outputLine;

    diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

    while (diskManager->getLine(TMP_SIZE_ATTRIBUTES, lineSize) != END) {
      int size = std::stoi(searchInEsquema(lineSize, 1));

      auto part = searchInEsquema(line, i);

      if (i == 0) 
        outputLine += "|";

      auto partFinal = " " + part + std::string((size - part.size() + 3), ' ');
      outputLine += partFinal + "|";

      ++i;
    }

    std::cout << outputLine << "\n";
  }

  std::cout << lineVertical << "\n";
}

void Megatron::clearCache() {
  diskManager->deleteFile(TMP_SCHEMAS);
  diskManager->deleteFile(TMP_ATTRIBUTES);
  diskManager->deleteFile(TMP_LINES);
  diskManager->deleteFile(TMP_RESULT);
  diskManager->deleteFile(TMP_SIZE_ATTRIBUTES);
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

  clearCache();

  diskManager->openFile(SCHEMA, READ_FLAGS);

  if (!diskManager->isOpen(SCHEMA)) {
    return;
  }

  auto tables = utils::split(from, ',');
  if (!existTables(tables, SCHEMA))
    return;

  auto atributos = utils::split(select, ',');
  if (!existAttributes(tables, atributos, TMP_SCHEMAS))
    return;

  std::vector<std::string> filePaths;
  if (!loadSchemaFiles(filePaths))
    return;

  for (int i = 0; i < filePaths.size(); ++i) {
    diskManager->writeFileLine(TMP_LINES, " ");
  }

  recorrerCartesian(0, filePaths, TMP_RESULT);
  
  showResult();

  delete diskManager;
}
