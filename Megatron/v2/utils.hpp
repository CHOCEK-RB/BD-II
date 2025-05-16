#ifndef UTILS_HPP
#define UTILS_HPP

int openFile(const char *fileName, int flags, bool registrar = false, int saveFd = -1, int fdSize = -1);
void renameFile(const char *fileName, const char *newFileName, int sizeFileName = -1, int sizeNewFileName = -1);
void readInput(const int &fd);
void clearFile(int saveFd, int fd);
void writeInt(const int &fd, int number);

int trimFile(const int &fd);
void splitLineFile(const int &fd, const int &fdSplit, char delimiter);
int findAndMove(const int &fd, char delimiter, int times = 1); 
bool findInLine(const int &fd, char found, char delimiter);

bool copyLine(const int &from, const int &to);
bool removeLastLine(const int &fd);

void registerFile(const int &saveFd, const int &fd, const char *fileName);
bool searchFd(const int &saveFd, const int &fd);
int nextFd(const int &saveFd);
int prevFd(int saveFd);

#endif
