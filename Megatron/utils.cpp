#include "utils.hpp"
#include <sstream>

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
