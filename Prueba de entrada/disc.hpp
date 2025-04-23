#ifndef DISC_HPP
#define DISC_HPP

#include "document.hpp"

class disc {
private:
  dataBase *db;
  unsigned long int size_m;

public:
  disc(unsigned long int s) : size_m(s), db(nullptr) {};
  ~disc() { delete db; };
  void load_db(const char *name, const char *file) {
    db = new dataBase(name);
    db->read_document(file);
    db->write_text();
    db->calculate_bytes();
  };
  long int total_space() { return size_m; };
  long int space_used() { return db != nullptr ? db->space() : 0; };
  long int available_space() {
    return db != nullptr ? size_m - db->space() : size_m;
  };
};

#endif
