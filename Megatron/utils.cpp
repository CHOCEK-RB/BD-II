#include "utils.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_set>

std::string utils::trim(const std::string &string) {
  size_t start = string.find_first_not_of(" \t\n\r");
  size_t end = string.find_last_not_of(" \t\n\r");
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

bool isOperator(const std::string &token) {
  static const std::unordered_set<std::string> operators = {
      "AND", "OR", "NOT", "<=", ">=", "=", "<", ">"};
  return operators.find(token) != operators.end();
}

int getPrecedence(const std::string &op) {
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

bool isLeftAssociative(const std::string &op) { return op != "NOT"; }

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

  if (!current.empty()) {
    tokens.push_back(current);
  }

  return tokens;
}

std::vector<std::string> utils::shuntingYard(const std::string &expr) {
  std::vector<std::string> output;
  std::stack<std::string> operators;
  
  auto tokens = tokenize(expr);

  for (const auto &token : tokens) {
    if (isOperator(token)) {
      while (!operators.empty() && isOperator(operators.top())) {
        const auto &top = operators.top();
        if ((isLeftAssociative(token) &&
             getPrecedence(token) <= getPrecedence(top)) ||
            (!isLeftAssociative(token) &&
             getPrecedence(token) < getPrecedence(top))) {
          output.push_back(top);
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
        output.push_back(operators.top());
        operators.pop();
      }
      if (!operators.empty() && operators.top() == "(") {
        operators.pop();
      } else {
        std::cerr << "Error: paréntesis desbalanceados.\n";
      }
    } else {
      output.push_back(token);
    }
  }

  while (!operators.empty()) {
    if (operators.top() == "(" || operators.top() == ")") {
      std::cerr << "Error: paréntesis desbalanceados.\n";
      operators.pop();
    } else {
      output.push_back(operators.top());
      operators.pop();
    }
  }

  return output;
}
