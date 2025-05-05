#include "utils.hpp"
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_set>

std::string utils::trim(const std::string &string) {
  size_t start = string.find_first_not_of(" \t\n\r\'");
  size_t end = string.find_last_not_of(" \t\n\r\'");
  return (start == std::string::npos) ? ""
                                      : string.substr(start, end - start + 1);
}

std::vector<std::string> utils::split(const std::string &text, char delimiter) {
  std::vector<std::string> result;
  std::stringstream ss(text);
  std::string item;

  while (getline(ss, item, delimiter)) {
    item = trim(item);
    result.push_back(item);
  }
  return result;
}

bool utils::isOperator(const std::string &token) {
  static const std::unordered_set<std::string> operators = {
      "AND", "OR", "NOT", "<=", ">=", "=", "<", ">"};
  return operators.find(token) != operators.end();
}

int utils::getPrecedence(const std::string &op) {
  if (op == "OR")
    return 1;
  if (op == "AND")
    return 2;
  if (op == "NOT")
    return 3;
  if (op == "<=" || op == ">=" || op == "=" || op == "<" || op == ">")
    return 4;
  return 0;
}

bool utils::isLeftAssociative(const std::string &op) { return op != "NOT"; }

std::vector<std::string> utils::tokenize(const std::string &expr) {
  std::vector<std::string> tokens;
  std::string current;

  for (size_t i = 0; i < expr.size(); ++i) {
    char ch = expr[i];

    if (std::isspace(ch)) {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }

    if (ch == '(' || ch == ')') {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      tokens.push_back(std::string(1, ch));
      continue;
    }

    std::string op(1, ch);
    if (i + 1 < expr.size()) {
      std::string twoCharOp = op + expr[i + 1];
      if (isOperator(twoCharOp)) {
        if (!current.empty()) {
          tokens.push_back(current);
          current.clear();
        }
        tokens.push_back(twoCharOp);
        i++;
        continue;
      }
    }

    if (isOperator(op)) {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      tokens.push_back(op);
      continue;
    }

    current += ch;
  }

  if (!current.empty())
    tokens.push_back(current);

  return tokens;
}

bool utils::isNumber(const std::string &string) {
  if (string.empty())
    return false;

  size_t start = 0;

  if (string[0] == '+' || string[0] == '-') {
    if (string.size() == 1)
      return false;
    start = 1;
  }

  return std::all_of(string.begin() + start, string.end(), ::isdigit);
}

bool utils::isStr(const std::string &string) {
  return !string.empty() && string[0] == '\'' &&
         string[string.size() - 1] == '\'';
}

bool utils::isFloat(const std::string &string) {
  if (string.empty())
    return false;

  if (string[0] == '+' || string[0] == '-') {
    if (string.size() == 1)
      return false;
  }
  char *endptr = nullptr;
  std::strtod(string.c_str(), &endptr);
  return endptr == string.c_str() + string.size();
}

bool utils::isBool(const std::string &string) {
  return string == "true" || string == "false";
}

bool utils::convertAndVerify(const std::string &op,
                             const std::string &parameterA,
                             const std::string &parameterB,
                             const std::string &type) {
  if (parameterA.empty() || parameterB.empty())
    return false;

  if (type == "str") {
    std::string a = trim(parameterA);
    std::string b = trim(parameterB);
    return verifyCondition(op, a, b);
  }

  if (type == "int" && isNumber(parameterA) && isNumber(parameterB)) {
    int a = std::stoi(parameterA);
    int b = std::stoi(parameterB);
    return verifyCondition(op, a, b);
  }

  if (type == "float" && isFloat(parameterA) && isFloat(parameterB)) {
    float a = std::stof(parameterA);
    float b = std::stof(parameterB);
    return verifyCondition(op, a, b);
  }

  if (type == "bool" && isBool(parameterA) && isBool(parameterB)) {
    bool a = parameterA == "true";
    bool b = parameterB == "true";
    return verifyCondition(op, a, b);
  }

  return false;
}
