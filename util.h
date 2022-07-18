#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

//Controlla se la stringa passata come argomento è composta esclusivamente da cifre. Se la stringa è numerica restituisce 1 altrimenti ritorna 0.
bool isNumeric(const char* string);
#endif //UTIL_H