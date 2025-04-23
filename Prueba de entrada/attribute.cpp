#include "attribute.hpp"
#include "type_traits"
#include <iostream>

template class attribute<int>;
template class attribute<float>;
template class attribute<long int>;
template class attribute<double>;
template class attribute<std::string>;

template <typename T>
std::string attribute<T>::get_as_string(size_t index) const {
  if (index >= registers.size() || registers[index] == nullptr)
    return "";
  if constexpr (std::is_same_v<T, std::string>)
    return *registers[index];
  else
    return std::to_string(*registers[index]);
}
template <typename T> size_t attribute<T>::size() const {
  return registers.size();
}

template <typename T> long int attribute<T>::space() const { return size_m; }

template <typename T>
attribute<T>::attribute(const char *n) : attributeBase(n), size_m(0) {
  if constexpr (std::is_same_v<T, int>)
    type = "int";
  else if constexpr (std::is_same_v<T, long int>)
    type = "long int";
  else if constexpr (std::is_same_v<T, float>)
    type = "float";
  else if constexpr (std::is_same_v<T, double>)
    type = "double";
  else if constexpr (std::is_same_v<T, std::string>)
    type = "string";
  else
    type = "unknown";
}
template <typename T>
void attribute<T>::insert(const std::string &str_register) {
  /*if (str_register.empty()) {
    registers.push_back(nullptr);
    return;
  }*/

  T new_register = convert_str(str_register);
  registers.push_back(std::make_unique<T>(new_register));

  if constexpr (std::is_same_v<T, std::string>) {
    size_m += str_register.size();
  } else {
    size_m += sizeof(new_register);
  }
}

template <typename T> T attribute<T>::convert_str(const std::string &r) {

  try {
    if constexpr (std::is_same_v<T, long int> || std::is_same_v<T, int>) {
      return std::stoi(r);
    } else if constexpr (std::is_same_v<T, float>) {
      return std::stof(r);
    } else if constexpr (std::is_same_v<T, double>) {
      return std::stod(r);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return r;
    } else {
      return T();
    }
  } catch (const std::exception &e) {
    return T();
  }
}
