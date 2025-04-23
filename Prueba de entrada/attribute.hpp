#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include <cstring>
#include <memory>
#include <string>
#include <vector>

class attributeBase {
private:
  const char *name;

public:
  attributeBase(const char *name) : name(name) {};
  const char *get_name() { return name; };
  virtual std::string get_as_string(size_t index) const = 0;
  virtual size_t size() const = 0;
  virtual long int space() const = 0;
  virtual ~attributeBase() {}
};

template <typename T> class attribute : public attributeBase {
private:
  const char *type;
  long int size_m;
  std::vector<std::unique_ptr<T>> registers;

  T convert_str(const std::string &r);

public:
  attribute(const char *);
  void insert(const std::string &);
  std::string get_as_string(size_t) const override;
  size_t size() const override;
  long int space() const override;
};

#endif
