#pragma once
#ifndef MYTOP_H
#define MYTOP_H

#include <sys/types.h>

void showErrorDialog(char* error);
void updateTreeView();
pid_t getSelectedPID();
void updateRow();

#endif //MYTOP_H