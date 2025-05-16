#ifndef MEGATRON_HPP
#define MEGATRON_HPP

bool openRelationFiles(const int &relations, const int &saveFd);

void insertRegisters();
void selectMenu();
void cartesianProduct(int saveFd, int attributesTmpFd, int relationsTmpFd, int linesTmpFd, int resultTmpFd, bool conditions = false, int conditionsTmp = -1);
void MegatronInit();

#endif
