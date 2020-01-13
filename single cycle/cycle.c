#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct r_type{
	int count_r;
	int rs;
	int rt;
	int rd;
	int opcode;
	int shamt;
	int funct;
};

struct i_type{
	int count_i;
	int opcode;
	int rs;
	int rt;
	int imm;
};

struct j_type{
	int count_j;
	int opcode;
	int imm;
};
struct completion{
	int return_value;
	int num_instruction;
	int num_R;
	int num_I;
	int num_J;
	int memory_access;
	int branches;

};
unsigned char mem[0x8000];//memory
int reg[32];
int pc;


int fetch();
void decode();



void main(){
//initialization
	FILE* stream;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	char buffer[1024];
	unsigned int value;
	stream = fopen("simple.bin","rb");

	while(1){
	if(-1==fread(buffer,&value,sizeof(value),stream)){
	break;}
	
	
	}	

	pc = 0;
	memset(reg,0,32);//all of register initialize to 0
	reg[29] = 0x800000;
	reg[31] = 0xffffffff;
	//reg[31]=0xffffffff; //r[31]=ra


	if(fetch()==-1){}
		
	
}

int fetch(){	//move inst memory to CPU
			
	
	 
}







