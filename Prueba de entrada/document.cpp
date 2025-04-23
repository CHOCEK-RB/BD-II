#include "document.hpp"
#include "attribute.hpp"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

dataBase::dataBase(const char *n) : name(n), size_m(0) {}

dataBase::~dataBase() {
  for (auto ptr : attributes) {
    delete ptr;
  }
}

void dataBase::case_titanic() {
  create_register("PassengerId", "long int");
  create_register("Survived", "long int");
  create_register("Pclass", "long int");
  create_register("Name", "string");
  create_register("Sex", "string");
  create_register("Age", "double");
  create_register("SibSp", "long int");
  create_register("Parch", "long int");
  create_register("Ticket", "string");
  create_register("Fare", "double");
  create_register("Cabin", "string");
  create_register("Embarked", "string");
}

void dataBase::create_register(const char *name, const char *type) {
  if (strcmp(type, "int") == 0)
    attributes.push_back(new attribute<int>(name));
  else if (strcmp(type, "long int") == 0)
    attributes.push_back(new attribute<long int>(name));
  else if (strcmp(type, "float") == 0)
    attributes.push_back(new attribute<float>(name));
  else if (strcmp(type, "double") == 0)
    attributes.push_back(new attribute<double>(name));
  else if (strcmp(type, "string") == 0)
    attributes.push_back(new attribute<std::string>(name));
  else
    std::cerr << "Tipo desconocido: " << type << std::endl;
}

std::vector<std::string> dataBase::split(const std::string &linea) {
  std::vector<std::string> campos;
  std::stringstream ss(linea);
  std::string campo;

  while (std::getline(ss, campo, '\t')) {
    campos.push_back(campo);
  }
  return campos;
}

void dataBase::read_document(const char *filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
    return;
  }

  std::string linea;
  bool initialized = false;

  while (std::getline(file, linea)) {
    if (linea.empty())
      continue;

    std::vector<std::string> valores = split(linea);

    if (!initialized) {
      case_titanic();
      initialized = true;
      continue;
    }

    if (valores.size() != attributes.size()) {
      std::cerr << "Línea con número incorrecto de campos:\n"
                << linea << std::endl;
      continue;
    }

    for (size_t i = 0; i < attributes.size(); ++i) {
      auto *base = attributes[i];
      if (auto *atr = dynamic_cast<attribute<int> *>(base))
        atr->insert(valores[i]);
      else if (auto *atr = dynamic_cast<attribute<long int> *>(base))
        atr->insert(valores[i]);
      else if (auto *atr = dynamic_cast<attribute<float> *>(base))
        atr->insert(valores[i]);
      else if (auto *atr = dynamic_cast<attribute<double> *>(base))
        atr->insert(valores[i]);
      else if (auto *atr = dynamic_cast<attribute<std::string> *>(base))
        atr->insert(valores[i]);
    }
  }
}

void dataBase::write_text() {
  std::ofstream file(std::string(name) + ".txt");
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo para escribir." << std::endl;
    return;
  }

  if (attributes.empty())
    return;

  for (size_t i = 0; i < attributes.size(); ++i) {
    file << attributes[i]->get_name();
    if (i < attributes.size() - 1)
      file << ",";
  }

  file << "\n";

  size_t total_registros = attributes[0]->size();

  for (size_t i = 0; i < total_registros; ++i) {
    for (size_t j = 0; j < attributes.size(); ++j) {
      file << attributes[j]->get_as_string(i);
      if (j < attributes.size() - 1)
        file << ",";
    }
    file << "\n";
  }

  file.close();
}

void dataBase::calculate_bytes() {
  for (auto *attribute : attributes) {
    size_m += attribute->space();
  }
}
