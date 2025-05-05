#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "diskManager.hpp"
#include "megatron.hpp"
#include "schemaManager.hpp"
#include "utils.hpp"

Megatron::Megatron() {
  this->size = 500 * 1024 * 1024;
  this->diskManager = new DiskManager(PATH);
  this->schemaManager = new SchemaManager(diskManager);
}

Megatron::~Megatron() { delete schemaManager; }

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

bool Megatron::checkAttribute(const std::vector<std::string> &tables,
                              const std::string &attr, const std::string &file,
                              bool load = true,
                              const std::string &saveFile = TMP_ATTRIBUTES) {
  if (load && !diskManager->isOpen(saveFile)) {
    diskManager->openFile(saveFile, READ_WRITE_FLAGS);
  }

  int dotPos = attr.find('.');
  if (dotPos != std::string::npos) {
    std::string table = attr.substr(0, dotPos);
    std::string attribute = attr.substr(dotPos + 1);

    auto it = std::find(tables.begin(), tables.end(), table);
    if (it == tables.end()) {
      flag += ERR_NPOS_SCHEMA_ATTR + attr + "\n";
      return false;
    }

    std::string schemaLine = schemaManager->getSchema(file, table);
    short pos = positionAttribute(schemaLine, attribute);
    if (pos == -1) {
      flag += ERR_NPOS_ATTR + attr + "\n";
      return false;
    }

    if (load && schemaManager->getSchema(saveFile, attr) == "") {
      std::string type =
          schemaManager->searchInEsquema(schemaLine, (pos + 1) * 2);
      return diskManager->writeFileLine(saveFile, attr + "#" + type + "#" +
                                                      std::to_string(pos));
    }

    return true;

  } else {
    bool found = false;

    for (size_t i = 0; i < tables.size(); ++i) {
      std::string schemaLine = schemaManager->getSchema(file, tables[i]);
      short pos = positionAttribute(schemaLine, attr);
      if (pos != -1) {
        if (found) {
          flag += ERR_REPEAT_ATTR + attr + "\n";
          return false;
        }

        if (load && (!diskManager->isOpen(saveFile) ||
                     schemaManager->getSchema(saveFile, attr) == "")) {
          std::string type =
              schemaManager->searchInEsquema(schemaLine, (pos + 1) * 2);
          diskManager->writeFileLine(saveFile, tables[i] + "." + attr + "#" +
                                                   type + "#" +
                                                   std::to_string(pos));
        }

        found = true;
      }
    }

    if (!found) {
      flag += ERR_NPOS_ATTR + attr + "\n";
      return false;
    }
  }

  return true;
}

bool Megatron::existAttributes(const std::vector<std::string> &tables,
                               std::vector<std::string> &attributes,
                               const std::string &file) {
  if (attributes.size() == 1 && attributes[0] == "*") {
    attributes.clear();
    for (const std::string &table : tables) {
      std::string schema = schemaManager->getSchema(file, table);
      std::vector<std::string> newAttributes = utils::split(schema, '#');
      for (size_t i = 1; i < newAttributes.size(); i += 2) {
        attributes.push_back(newAttributes[0] + "." + newAttributes[i]);
      }
    }
  }

  for (size_t i = 0; i < attributes.size(); ++i) {
    if (!checkAttribute(tables, attributes[i], file)) {
      return false;
    }
  }

  return true;
}

bool Megatron::existTables(const std::vector<std::string> &tables,
                           const std::string &file) {
  for (const std::string &table : tables) {
    if (!schemaManager->checkSchema(file, table)) {
      flag += ERR_NOT_SCHEMA + table + "\n";
      return false;
    }
  }

  return true;
}

bool Megatron::loadSchemaFiles(std::vector<std::string> &files) {
  if (!diskManager->isOpen(TMP_ATTRIBUTES)) {
    diskManager->openFile(TMP_ATTRIBUTES, READ_FLAGS);
  }

  std::string line;

  while (diskManager->getLine(TMP_ATTRIBUTES, line) != END) {
    std::string file = line.substr(0, line.find(".")) + ".txt";

    if (!diskManager->isOpen(file)) {
      if (diskManager->openFile(file, READ_FLAGS) == NOT_OPEN)
        return false;

      files.push_back(file);
    }
  }
  return true;
};

bool Megatron::shuntingYard(const std::string &expr,
                            const std::vector<std::string> &tables) {
  std::stack<std::string> operators;

  std::vector<std::string> tokens = utils::tokenize(expr);

  for (const auto &token : tokens) {
    if (utils::isOperator(token)) {
      while (!operators.empty() && utils::isOperator(operators.top())) {
        const auto &top = operators.top();
        if ((utils::isLeftAssociative(token) &&
             utils::getPrecedence(token) <= utils::getPrecedence(top)) ||
            (!utils::isLeftAssociative(token) &&
             utils::getPrecedence(token) < utils::getPrecedence(top))) {
          diskManager->writeFileLine(TMP_CONDITIONS, top);
          operators.pop();
        } else {
          break;
        }
      }
      operators.push(token);
    } else if (token == "(") {
      operators.push(token);
    } else if (token == ")") {
      while (!operators.empty() && operators.top() != "(") {
        diskManager->writeFileLine(TMP_CONDITIONS, operators.top());
        operators.pop();
      }
      if (!operators.empty() && operators.top() == "(") {
        operators.pop();
      } else {
        flag += "Error: paréntesis desbalanceados.\n";
      }
    } else {
      if (!utils::isNumber(token) && !utils::isStr(token) &&
          !checkAttribute(tables, token, TMP_SCHEMAS, true,
                          TMP_CONDITIONAL_ATTRIBUTES)) {
        return false;
      }
      diskManager->writeFileLine(TMP_CONDITIONS, token);
    }
  }

  while (!operators.empty()) {
    if (operators.top() == "(" || operators.top() == ")") {
      flag += "Error: paréntesis desbalanceados.\n";
      operators.pop();
    } else {
      diskManager->writeFileLine(TMP_CONDITIONS, operators.top());
      operators.pop();
    }
  }

  return true;
}

bool Megatron::checkConditions() {
  if (!diskManager->copyFile(TMP_CONDITIONS, TMP_COPY_CONDITIONS)) {
    return false;
  }

  while (true) {
    bool result;
    std::string parameterA;
    std::string parameterB;
    std::string op;
    std::string line;
    int currentLine = -1;
    diskManager->setPosition(TMP_COPY_CONDITIONS, 0, SEEK_SET);
    while (diskManager->getLine(TMP_COPY_CONDITIONS, line) != END) {
      ++currentLine;

      if (utils::isOperator(line)) {
        op = line;
        break;
      }

      if (parameterA.empty())
        parameterA = line;
      else if (parameterB.empty())
        parameterB = line;
      else {
        parameterA = parameterB;
        parameterB = line;
      }
    }

    if (currentLine == 0 && utils::isBool(parameterA)) {
      diskManager->deleteFile(TMP_COPY_CONDITIONS);
      return parameterA == "true";
    }

    if (parameterA.empty() && parameterB.empty() || op.empty() ||
        currentLine == -1) {
      flag += "Error al trata de obtener el operador o los parametros\n";
      diskManager->deleteFile(TMP_COPY_CONDITIONS);
      return false;
    }

    if (op == "NOT") {
      bool val;
      if (utils::isBool(parameterA)) {
        val = parameterA == "true";
      } else {
        flag += "NOT solo puede aplicarse a valores booleanos\n";
        diskManager->deleteFile(TMP_COPY_CONDITIONS);
        return false;
      }

      result = !val;

      if (!diskManager->replaceLines(TMP_COPY_CONDITIONS, currentLine - 1, 2,
                                     result ? "true" : "false"))
        return false;

      continue;
    }

    diskManager->setPosition(TMP_CONDITIONAL_ATTRIBUTES, 0, SEEK_SET);
    bool isAttributeA = false;
    bool isAttributeB = false;

    if (!utils::isStr(parameterA) && !utils::isFloat(parameterA) &&
        !utils::isNumber(parameterA))
      isAttributeA = true;

    if (!utils::isStr(parameterB) && !utils::isFloat(parameterB) &&
        !utils::isNumber(parameterB))
      isAttributeB = true;

    std::string tableA, typeA, tableB, typeB;
    short posA, posB;

    while (diskManager->getLine(TMP_CONDITIONAL_ATTRIBUTES, line) != END) {
      std::string schemaAttribute = schemaManager->searchInEsquema(line, 0);
      int dotPos = schemaAttribute.find(".");

      if (isAttributeA && parameterA == schemaAttribute ||
          parameterA == schemaAttribute.substr(dotPos + 1)) {
        tableA = schemaAttribute.substr(0, dotPos);
        typeA = schemaManager->searchInEsquema(line, 1);
        posA = std::stoi(schemaManager->searchInEsquema(line, 2));
      }

      if (isAttributeB && parameterB == schemaAttribute ||
          parameterB == schemaAttribute.substr(dotPos + 1)) {
        tableB = schemaAttribute.substr(0, dotPos);
        typeB = schemaManager->searchInEsquema(line, 1);
        posB = std::stoi(schemaManager->searchInEsquema(line, 2));
      }
    }

    if (isAttributeA && isAttributeB && !typeA.empty() && !typeB.empty()) {
      if (typeA == typeB) {
        std::string lineA = schemaManager->getSchema(TMP_LINES, tableA);
        std::string lineB = schemaManager->getSchema(TMP_LINES, tableB);

        std::string newParameterA =
            schemaManager->searchInEsquema(lineA, posA + 1);
        std::string newParameterB =
            schemaManager->searchInEsquema(lineB, posB + 1);

        result =
            utils::convertAndVerify(op, newParameterA, newParameterB, typeA);
      } else {
        flag += "Los atributos " + parameterA + " y " + parameterB +
                " no coinciden";
        diskManager->deleteFile(TMP_COPY_CONDITIONS);
        return false;
      }

    } else if (isAttributeA && !typeA.empty()) {
      std::string lineA = schemaManager->getSchema(TMP_LINES, tableA);
      std::string newParameterA =
          schemaManager->searchInEsquema(lineA, posA + 1);
      result = utils::convertAndVerify(op, newParameterA, parameterB, typeA);

    } else if (isAttributeB && !typeB.empty()) {
      std::string lineB = schemaManager->getSchema(TMP_LINES, tableB);
      std::string newParameterB =
          schemaManager->searchInEsquema(lineB, posB + 1);
      result = utils::convertAndVerify(op, parameterA, newParameterB, typeB);

    } else if (utils::isBool(parameterA) && utils::isBool(parameterB)) {
      result = utils::convertAndVerify(op, parameterA, parameterB, "bool");

    } else {
      flag += ERR_WHERE;
      diskManager->deleteFile(TMP_COPY_CONDITIONS);
      return false;
    }

    if (!diskManager->replaceLines(TMP_COPY_CONDITIONS, currentLine - 2, 3,
                                   result ? "true" : "false")) {
      return false;
    }
  }
}

bool Megatron::recorrerCartesian(std::vector<std::string> &files,
                                 const std::string &saveFile, bool conditions) {
  for (const std::string &file : files) {
    if (diskManager->isEmpty(file))
      return false;
  }

  if (!diskManager->isOpen(saveFile))
    diskManager->openFile(saveFile, READ_WRITE_FLAGS);

  diskManager->setPosition(TMP_ATTRIBUTES, 0, SEEK_SET);

  std::string attributesLine;

  for (std::string line; diskManager->getLine(TMP_ATTRIBUTES, line) != END;) {
    auto schemaAttribute = schemaManager->searchInEsquema(line, 0);
    auto attribute = schemaAttribute.substr(schemaAttribute.find(".") + 1);
    attributesLine += attribute + "#";

    diskManager->writeFileLine(TMP_SIZE_ATTRIBUTES,
                               schemaAttribute + "#" +
                                   std::to_string(attribute.size()));
  }

  int nivel = 0;
  diskManager->setPosition(files[nivel], 0, SEEK_SET);
  while (true) {
    if (nivel == files.size()) {
      if (conditions && !checkConditions()) {
        if (!flag.empty()) {
          return false;
        }
        --nivel;
        continue;
      }

      std::string outputLine;
      std::string attr;

      diskManager->setPosition(TMP_ATTRIBUTES, 0, SEEK_SET);
      diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

      int i = 0;
      while (diskManager->getLine(TMP_ATTRIBUTES, attr) != END) {
        std::string schemaAttribute = schemaManager->searchInEsquema(attr, 0);
        std::string schema = attr.substr(0, schemaAttribute.find("."));

        std::string position = schemaManager->searchInEsquema(attr, 2);

        std::string line = schemaManager->getSchema(TMP_LINES, schema);
        std::string part =
            schemaManager->searchInEsquema(line, std::stoi(position) + 1);

        line = schemaManager->getSchema(TMP_SIZE_ATTRIBUTES, schemaAttribute);
        std::string sizeAttr = schemaManager->searchInEsquema(line, 1);

        if (part.size() > std::stoi(sizeAttr)) {
          line = schemaAttribute + "#" + std::to_string(part.size());

          if (!diskManager->replaceLines(TMP_SIZE_ATTRIBUTES, i, 1, line))
            return false;
        }
        if (!outputLine.empty())
          outputLine += "#";

        outputLine += part;
        ++i;
      }

      diskManager->writeFileLine(saveFile, outputLine);
      --nivel;
      continue;
    }

    if (nivel < files.size() - 1)
      diskManager->setPosition(files[nivel + 1], 0, SEEK_SET);

    std::string line;
    if (diskManager->getLine(files[nivel], line) != END) {
      line = files[nivel].substr(0, files[nivel].find(".")) + "#" + line;

      if (!diskManager->replaceLines(TMP_LINES, nivel, 1, line))
        return false;

      ++nivel;
    } else {
      if (nivel == 0)
        return true;

      --nivel;
    }
  }
};

void Megatron::showResult(const std::string &file) {
  std::string line;
  std::string lineVertical;

  diskManager->setPosition(file, 0, SEEK_SET);
  diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

  if (!diskManager->isEmpty(file)) {
    int i = 0;

    std::string lineSize;
    std::string outputLine;

    while (diskManager->getLine(TMP_SIZE_ATTRIBUTES, lineSize) != END) {
      int size = std::stoi(schemaManager->searchInEsquema(lineSize, 1));

      std::string part = schemaManager->searchInEsquema(lineSize, 0);
      part = part.substr(part.find(".") + 1);

      if (i == 0) {
        outputLine += "|";
        lineVertical += "+";
      }

      std::string partFinal =
          " " + part + std::string((size - part.size() + 3), ' ');
      outputLine += partFinal + "|";
      lineVertical += std::string(partFinal.size(), '-') + "+";

      ++i;
    }

    std::cout << lineVertical << "\n"
              << outputLine << "\n"
              << lineVertical << "\n";
  } else {
    return;
  }

  while (diskManager->getLine(file, line) != END) {
    int i = 0;

    std::string lineSize;
    std::string outputLine;

    diskManager->setPosition(TMP_SIZE_ATTRIBUTES, 0, SEEK_SET);

    while (diskManager->getLine(TMP_SIZE_ATTRIBUTES, lineSize) != END) {
      int size = std::stoi(schemaManager->searchInEsquema(lineSize, 1));

      std::string part = schemaManager->searchInEsquema(line, i);

      if (i == 0)
        outputLine += "|";

      std::string partFinal =
          " " + part + std::string((size - part.size() + 3), ' ');
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
  diskManager->deleteFile(TMP_CONDITIONAL_ATTRIBUTES);
  diskManager->deleteFile(TMP_CONDITIONS);
}

void Megatron::init() {
  clearCache();

  std::cout << "% Megatron 3000\n";
  std::cout << "\tWelcome to Megatron 3000!\n\n";
  std::cout << "% Opciones:\n";
  std::cout << "1) SELECT\n";
  std::cout << "2) Salir\n\n";

  int choice;
  std::cout << "Opcion : ";
  std::cin >> choice;

  switch (choice) {
  case 1:
    selectMenu();
    break;
  case 2:
    std::cout << "Hasta luego.\n";
    return;
  default:
    std::cout << "Opcion invalida.\n";
    break;
  }

  if (!flag.empty()) {
    std::cerr << flag;
  }
}

void Megatron::selectMenu() {
  std::string select, from, conditions, saveFile;
  std::cin.ignore();

  std::cout << "& SELECT : ";
  std::getline(std::cin, select);
  std::cout << "& FROM : ";
  std::getline(std::cin, from);

  std::cout << "\n& SELECT " << select << " FROM " << from << "\n";

  std::cout << "\n% Opciones:\n";
  std::cout << "1) WHERE\n";
  std::cout << "2) Guardar resultado (|)\n";
  std::cout << "3) Ejecutar (#)\n\n";

  int choice;
  std::cout << "Opcion : ";
  std::cin >> choice;

  switch (choice) {
  case 1:
    whereMenu(conditions, saveFile);
    break;
  case 2:
    pipeMenu(saveFile);
    break;
  case 3:
    from = "titanic, barco";
    select = "PassengerId, barco.ID, titanic.Name, Sex, Price";
    conditions = "Sex=\'female\' AND (titanic.PassengerId > 10 OR "
                 "PassengerId=ID) AND Age <= 30";
    saveFile = "prueba";
    break;
  default:
    std::cout << "Opcion invalida.\n";
    return;
  }

  selectFuntion(select, from, conditions, saveFile);
}

void Megatron::whereMenu(std::string &conditions, std::string &saveFile) {
  std::cin.ignore();
  std::cout << "& WHERE : ";
  std::getline(std::cin, conditions);

  std::cout << "\n& SELECT ... WHERE " << conditions << "\n";

  std::cout << "\n% Opciones:\n";
  std::cout << "1) Guardar resultado (|)\n";
  std::cout << "2) Ejecutar (#)\n\n";

  int choice;
  std::cout << "Opcion : ";
  std::cin >> choice;

  switch (choice) {
  case 1:
    pipeMenu(saveFile);
    break;
  case 2:
    break;
  default:
    std::cout << "Opcion invalida.\n";
    return;
  }
}

void Megatron::pipeMenu(std::string &saveFile) {
  std::cin.ignore();
  std::cout << "& Archivo para guardar resultado: ";
  std::getline(std::cin, saveFile);

  std::cout << "\n% Guardando en: " << saveFile << "\n";
}

void Megatron::selectFuntion(const std::string &select, const std::string &from,
                             const std::string &conditions,
                             const std::string &saveFile) {

  diskManager->openFile(SCHEMA, READ_FLAGS);
  if (!diskManager->isOpen(SCHEMA))
    return;

  std::vector<std::string> tables = utils::split(from, ',');
  if (!existTables(tables, SCHEMA))
    return;

  std::vector<std::string> atributos = utils::split(select, ',');
  if (!existAttributes(tables, atributos, TMP_SCHEMAS))
    return;

  std::vector<std::string> filePaths;
  if (!loadSchemaFiles(filePaths))
    return;

  for (int i = 0; i < filePaths.size(); ++i) {
    diskManager->writeFileLine(TMP_LINES, " ");
  }

  if (!conditions.empty() && !shuntingYard(conditions, tables)) {
    return;
  }

  std::string svFile = (!saveFile.empty()) ? saveFile : TMP_RESULT;

  if (!saveFile.empty() && !schemaManager->createSchema(svFile)) {
    return;
  }

  if (recorrerCartesian(filePaths, svFile, !conditions.empty()))
    showResult(svFile);
}
