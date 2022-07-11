#include <ctype.h>
#include "util.h"

int isNumeric(const char* string){
	while(*string){
		if(!isdigit(*string))
			return 0;
		string++;
	}
	return 1;
}