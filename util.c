#include <ctype.h>
#include <stdbool.h>
#include "util.h"

bool isNumeric(const char* string){
	while(*string){
		if(!isdigit(*string))
			return false;
		string++;
	}
	return true;
}