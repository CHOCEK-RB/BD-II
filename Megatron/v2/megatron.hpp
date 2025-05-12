#ifndef MEGATRON_HPP
#define MEGATRON_HPP

int findInLineFile(const int &fd, char delimiter);
bool searchFd(const int &saveFd, const int &fd);
void registerFile(const int &saveFd, const int &fd, const char *fileName);
int openFile(const char* path, int flags);
int openTSV(const int &input, const int &saveFd);
int openTxtFromTsv(const int &saveFd);
void readInput(const int &fd);
int trimFile(const int &fd);
void createSchemaTSV(const int &fdTSV, const int &saveFd);
void readRegistersTsv(const int &fdTSV, const int &fdTXT);
bool readTsv(const int &fd, const int &saveFd);
void insertRegisters();
void MegatronInit();

#endif
