#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

int isNumber(const char* string){
	while(*string){
		if(!isdigit(*string))
			return 0;
		string++;
	}
	return 1;
}

char* cuncatenateStrings(const char *string0, const char *string1, const char *string2){
	size_t lenght, len0, len1, len2;
	len0 = strlen(string0);
	len1 = strlen(string1);
	len2 = strlen(string2);
	lenght = len0 + len1 + len2 + 1; // +1 = null terminator!!
	
	char* res = malloc(lenght*sizeof(char));
	if(!res){
		perror("Errore allocazione stringa");
		return NULL;
	}

	snprintf(res, lenght, "%s%s%s", string0, string1, string2);
	return res;
}