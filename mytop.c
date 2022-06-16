#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#define CHUNK 1024

int isNumber(char* string){
	while(*string != '\0'){
		if(!isdigit(*string))
			return 0;
		string++;
	}
	return 1;
}

void getCPUinfo() {
	FILE *file = fopen("/proc/cpuinfo", "r");
	assert(file && "Errore apertura file");

	char c[CHUNK];
	while (fgets(c, CHUNK, file)){
		printf("%s", c);
	}

	fclose(file);
}

void getProcessesList(){
	DIR *dir = opendir("/proc/");
	assert(dir && "Errore apertura directory");

	struct dirent *entry;
	entry = readdir(dir);
	assert(entry && "Errore lettura directory");

	while(entry){
		switch (entry->d_type){
			//stampa solo le entry directory che corrispondono ai processi (ovvero che come nome hanno il pid numerico)
			case DT_DIR:
				char *name = entry->d_name;
				if(isNumber(name))
					printf("%s\n", entry->d_name);
				break;
			default:
				break;
		}
		entry = readdir(dir);
	}

	closedir(dir);
}

void getProcessData(){
	char path[] = "/proc/1/status";
	FILE *file = fopen(path, "r");
	assert(file && "Errore apertura file");

	char c[CHUNK];
	while (fgets(c, CHUNK, file)){
		printf("%s", c);
	}

	fclose(file);
}

int main(int argc, char *argv[]) {
	//getCPUinfo();
	//getProcessesList();
	//getProcessData();
}