#include <ctype.h>
#include "util.h"

int isNumeric(const char* string){
	int len = 0;
	while(*string){
		if(!isdigit(*string))
			return 0;
		string++;
		len++;
	}
	return len;
}