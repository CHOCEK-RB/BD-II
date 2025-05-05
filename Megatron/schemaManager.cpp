#include "schemaManager.hpp"
#include "utils.hpp"
#include <cstdio>
#include <iostream>
#include <string>

SchemaManager::SchemaManager(DiskManager *diskManager) {
  this->diskManager = diskManager;
}

bool SchemaManager::checkSchema(const std::string &file,
                                const std::string &schema,
                                bool load,
                                const std::string &saveFile) {
  std::string line;

  while (diskManager->getLine(file, line) != END) {
    int pos = line.find('#');
    if (pos != std::string::npos) {
      std::string _schema = line.substr(0, pos);
      if (_schema == schema) {
        diskManager->setPosition(file, 0, SEEK_SET);

        if (load)
          return diskManager->writeFileLine(saveFile, line);

        return true;
      }
    }
  }

  return false;
}

std::string SchemaManager::getSchema(const std::string &file,
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

std::string SchemaManager::searchInEsquema(const std::string &line,
                                           short position) {
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

bool SchemaManager::createSchema(std::string &fileName) {
  if (fileName.empty()) {
    std::cerr << "No se ingreso el nombre para la nueva relacion\n";
    return false;
  }
  std::string newFile = utils::trim(fileName);

  int dotPos = newFile.find(".");

  if (dotPos == std::string::npos) {
    newFile += ".txt";
  } else if (newFile.substr(dotPos + 1) != "txt") {
    newFile.replace(dotPos + 1, 3, "txt");
  }

  bool reWrite = false;

  if (diskManager->exist(newFile)) {
    int choice;
    std::cout << "Ya existe el archivo : " << newFile << "\n\n";
    std::cout << "Desea sobreescribir?\n1) Si\n2) No\n";
    std::cout << "Opcion : ";
    std::cin >> choice;

    while (true) {
      if (choice == 1) {
        reWrite = true;
        break;
      } else if (choice == 2) {
        std::cerr << "No se pudo crear la nueva relacion porque ya existe\n";
        return false;
      }
    }

    diskManager->openFile(newFile, READ_WRITE_TRUNC_FLAGS);
  } else {
    diskManager->openFile(newFile, READ_WRITE_FLAGS);
  }
  std::string schemaName = newFile.substr(0, newFile.find("."));
  std::string newSchema = schemaName;

  diskManager->setPosition(TMP_ATTRIBUTES, 0, SEEK_SET);

  std::string line;
  while (diskManager->getLine(TMP_ATTRIBUTES, line) != END) {
    std::string schemaAttribute = searchInEsquema(line, 0);
    std::string type = searchInEsquema(line, 1);
    int newDotPos = schemaAttribute.find(".");
    std::string newAttribute = schemaAttribute.substr(newDotPos + 1);

    if (newSchema.find(newAttribute) != std::string::npos) {
      newAttribute = schemaAttribute.substr(0, newDotPos) + "-" + newAttribute;
    }

    newSchema += "#" + newAttribute + "#" + type;
  }

  diskManager->openFile(SCHEMA, READ_WRITE_FLAGS);

  fileName = newFile;
  if (reWrite) {
    int schemaPos = 0;
    diskManager->setPosition(SCHEMA, 0, SEEK_SET);

    std::string line;
    while (diskManager->getLine(SCHEMA, line) != END) {
      std::string schema = searchInEsquema(line, 0);
      if (schema == schemaName)
        break;

      ++schemaPos;
    }

    return diskManager->replaceLines(SCHEMA, schemaPos, 1, newSchema);
  }

  return diskManager->writeFileLine(SCHEMA, newSchema);
}
