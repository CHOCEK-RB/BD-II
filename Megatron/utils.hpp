#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

class utils {
public:
  utils() = delete;
  static std::string trim(const std::string &);

  static std::vector<std::string> split(const std::string &, char);
};
#endif
