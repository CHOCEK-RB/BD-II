#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

class utils {
public:
  utils() = delete;
  static std::string trim(const std::string &);

  static std::vector<std::string> split(const std::string &, char);
  static std::vector<std::string> tokenize(const std::string &);
  static std::vector<std::string> shuntingYard(const std::string &);

  static bool isNumber(const std::string &);
  static bool isStr(const std::string &);
};
#endif
