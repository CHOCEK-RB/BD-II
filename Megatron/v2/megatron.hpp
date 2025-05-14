#ifndef MEGATRON_HPP
#define MEGATRON_HPP

// Utils
int openFile(const char *fileName, int flags, bool registrar = false, int saveFd = -1);
void readInput(const int &fd);

int trimFile(const int &fd);
int findAndMove(const int &fd, char delimiter);

void registerFile(const int &saveFd, const int &fd, const char *fileName);
bool searchFd(const int &saveFd, const int &fd);

// Read TSV
int openTSV(const int &input, const int &saveFd);
int openTxtFromTsv(const int &saveFd);
void createSchemaTSV(const int &fdTSV, const int &saveFd);
void readRegistersTsv(const int &fdTSV, const int &fdTXT);
bool readTsv(const int &fd, const int &saveFd);

void insertRegisters();
void selectMenu();

void MegatronInit();

#endif
