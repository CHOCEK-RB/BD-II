#ifndef READCSV_HPP
#define READCSV_HPP

enum State { START, IS_INT, IS_FLOAT, IS_VARCHAR, UNKNOWN };

int openTSV(const int &input, const int &saveFd);
int openTxtFromTsv(const int &saveFd);
void createSchemaTSV(const int &fdTSV, const int &saveFd, const int &fdSchema);
State checkState(const int &relations);
void comparateTypes(const int &relations, State state, int &lengthAttribute);
void readRegistersTsv(const int &fdTSV, const int &fdTXT, const int &relations);
bool readTsv(const int &fd, const int &saveFd);


#endif
