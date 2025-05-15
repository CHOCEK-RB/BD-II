#ifndef MEGATRON_HPP
#define MEGATRON_HPP

enum State {START, IS_INT, IS_FLOAT, IS_VARCHAR, UNKNOWN};

// Utils
int openFile(const char *fileName, int flags, bool registrar = false, int saveFd = -1);
void readInput(const int &fd);
void writeInt(const int &fd, int number);

int trimFile(const int &fd);
void splitLineFile(const int &fd, const int &fdSplit, char delimiter);
int findAndMove(const int &fd, char delimiter);
bool findInLine(const int &fd, char found, char delimiter);

bool copyLine(const int &from, const int &to);
bool removeLastLine(const int &fd);

void registerFile(const int &saveFd, const int &fd, const char *fileName);
bool searchFd(const int &saveFd, const int &fd);
int nextFd(const int &saveFd);
int prevFd(int saveFd);

// Read TSV
int openTSV(const int &input, const int &saveFd);
int openTxtFromTsv(const int &saveFd);
void createSchemaTSV(const int &fdTSV, const int &saveFd);
void readRegistersTsv(const int &fdTSV, const int &fdTXT);
bool readTsv(const int &fd, const int &saveFd);

// SELECT 


void insertRegisters();
void selectMenu();

void MegatronInit();

#endif
