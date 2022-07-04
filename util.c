#include <ctype.h>
#include <string.h>
#include <assert.h>
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

	strncpy(res, string0, len0);
	strncat(res, string1, len1);
	strncat(res, string2, len2);
	return res;
}