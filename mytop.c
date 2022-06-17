#include "aux.c"

int main(int argc, char *argv[]) {
	//getCPUinfo();
	//getProcessesList();
	
	info* info = getProcessData();
	info_print(info);
	info_free(info);
}

