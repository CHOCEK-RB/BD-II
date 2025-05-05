#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

class utils {
public:
  utils() = delete;
  static std::string trim(const std::string &);

  static std::vector<std::string> split(const std::string &, char);
  static std::vector<std::string> tokenize(const std::string &);

  static int getPrecedence(const std::string &);

  static bool isOperator(const std::string &token);
  static bool isLeftAssociative(const std::string &);
  static bool isNumber(const std::string &);
  static bool isStr(const std::string &);
  static bool isFloat(const std::string &);
  static bool isBool(const std::string &);
  static bool convertAndVerify(const std::string &, const std::string &, const std::string &, const std::string &);

  template <typename T> static bool verifyCondition(const std::string &, const T &, const T &);
};

template <typename T> bool utils::verifyCondition(const std::string &op, const T &parameterA, const T &parameterB) {
  if (op == "=")
    return parameterA == parameterB;
  if (op == "<")
    return parameterA < parameterB;
  if (op == ">")
    return parameterA > parameterB;
  if (op == "<=")
    return parameterA <= parameterB;
  if (op == ">=")
    return parameterA >= parameterB;
  if (op == "AND") {
    if constexpr (std::is_same_v<T, bool>)
      return parameterA && parameterB;
    else
      throw std::invalid_argument("AND only valid for bool types");
  }
  if (op == "OR") {
    if constexpr (std::is_same_v<T, bool>)
      return parameterA || parameterB;
    else
      throw std::invalid_argument("OR only valid for bool types");
  }

  return false;
}

#endif
