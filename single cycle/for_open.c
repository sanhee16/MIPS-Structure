#include <stdio.h>

void main(int argc, char *argv[]){

	FILE* fp = NULL;
	char* path = "simple.bin";
	int val = 0;
	int res;

	if(argc == 2){
		path=argv[1];
}

	fp = fopen(path, "rb");
	if(fp==NULL){
	printf("invalid input file: %s\n",path);
	return ;
}
while(1){
	res = fread(&val, sizeof(val), 1, fp);
	printf("0x%08x \n",val);
	break;
}
fclose(fp);
}
