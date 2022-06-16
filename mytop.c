#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

void getCPUinfo() {
	FILE *file = fopen("/proc/cpuinfo", "r");
	assert(file && "Errore apertura file");

	char c;
	while (fread(&c, sizeof(char), 1, file)){
		printf("%c", c);
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
			case DT_DIR:
				printf("%s\n", entry->d_name);
				break;
			default:
				break;
		}
		entry = readdir(dir);
	}

	closedir(dir);
}

int main(int argc, char *argv[]) {
	//getCPUinfo();
	getProcessesList();
}