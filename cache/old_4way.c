#include <stdio.h>
#include <stdlib.h>
#include "inst.h"
#include <string.h>
#include "calc.h"

struct ifid{ // decode
	int valid, pc, pc4;
	int inst;
};

struct idex{ //execute
	int inst, valid, pc, pc4;
	int opcode, funct, rs, rt, imm;
	int shamt, alu1, alu2, v2, wreg;
	int aluSrc, regDst, jump, branch, memtoReg, memRead, memWrite, regWrite;
};

struct exmem{ // memory
	int inst;
	int valid, pc, pc4;
	int result, v2, wreg;
	int opcode, funct, rs, rt;
	int branch, memtoReg, memRead, memWrite, regWrite;
};

struct memwb{ //write back
	int inst;
    int valid, pc;
	int wdata, wreg;
	int regWrite;

};

struct cache{
	int valid;
	int tag; // 256 lines
	int data[16];
	int dirty;
	int sca;
	int first;
};


int mem[0x100000];
int reg[32];
int pc;
//float hit_inst, miss_inst, hit_data, miss_data;

int pp;

int jump, jr_taken, br_taken;
int jumpAddr, branchAddr, jrjump;
int endloop, cycle;

int r2,n_cycle,n_inst,n_instEx,n_mem,n_reg,n_branch,n_T,n_NT,n_jump=0;
int readmem(int addr, struct cache (* cacheline)[256], int num, int diff, int (*oldest)[256]);
void writemem(int addr, int value, struct cache (* cacheline)[256]);

struct ifid lat1[2];
struct idex lat2[2];
struct exmem lat3[2];
struct memwb lat4[2];
struct cache instca[4][256];
struct cache dataca[4][256];
int oldest_data[4][256];
int oldest_inst[4][256];
//struct cache cacheline[256];

void fetch();
void decode();
void execute();
void memory();
void writeback();

int cc(int addr);

void control(int opcode, int funct);
void openfile();

void main(int argc, char *argv[]){
	
	openfile();	
	
	memset(reg,0,32);
	reg[29] = 0x400000;
	reg[31] = 0xffffffff;
	
	memset(lat1,0,sizeof(struct ifid)*2);
	memset(lat2,0,sizeof(struct idex)*2);
	memset(lat3,0,sizeof(struct exmem)*2);
	memset(lat4,0,sizeof(struct memwb)*2);
	memset(instca,0,sizeof(struct cache)*4*256);
	memset(dataca,0,sizeof(struct cache)*4*256);
	memset(oldest_inst,0,sizeof(int)*4*256);
	memset(oldest_data,0,sizeof(int)*4*256);

	cycle=0;
	
	while(1){
		cycle++;
		if(endloop) break;	
		if(pp)printf("\n----------[cycle %d]--------\n",cycle);
		
		writeback();
		memory();
		execute();
		decode();
		fetch();

		lat1[1]=lat1[0];
		lat2[1]=lat2[0];
		lat3[1]=lat3[0];
		lat4[1]=lat4[0];


	}
	
	printf("\n--------------result--------------\n");
	r2=reg[2];
	n_cycle=cycle;
	printf("\nr[2]       \t %d \t cycle     \t %d \n", reg[2],cycle);
	printf("inst execute \t %d \t total     \t %d \n",n_instEx,n_inst);
	printf("mem ops      \t %d \t reg ops   \t %d \n",n_mem,n_reg);
	printf("branch       \t %d \t nt take   \t %d \t taken \t %d \n",n_branch,n_NT,n_T);
	printf("jump         \t %d \n\n",n_jump);

	calc(0);
	return ;
}

void fetch(){


	if(pc==0xffffffff){
		pc=0xffffffff;
	}
	else if(jump){
		pc=jumpAddr;
		n_jump++;
	}
	else if(jr_taken){
		pc=jrjump;
		n_jump++;
	}
	else if(br_taken){
		n_T++;
		pc=branchAddr;
	}
	else{
		pc=pc+4;
		if(cycle==1) pc=0;
	}
	jr_taken=0; jump=0; br_taken=0;	jumpAddr=0; branchAddr=0; jrjump=0;	

	lat1[0].inst=readmem(pc, instca, 0, 0, oldest_inst);

	lat1[0].pc=pc;
	lat1[0].pc4=pc+4;
	lat1[0].valid=1;
	
	if(pc==-1){
	lat1[0].valid=0;
	lat1[0].inst=0;
	}
	if(pp)printf("[fetch] \t%08x \t[pc] : 0x%08x\n",lat1[0].inst,pc);

	return ;
}

int windex, woffset, wnum;

int readmem(int addr, struct cache (* cacheline)[256], int num, int diff, int (*oldest)[256]){
	int tag, index, offset;
	tag=(addr&0xFFFFC000)>>14;
	index=(addr&0x00003FC0)>>6;
	offset=(addr&0x0000003F);

	windex, woffset, wnum=0;
	if(cacheline[num][index].valid==0){ // cold miss : mem->cache 

		if(diff)
			miss_data++;
		else if(diff==0)
			miss_inst++;

		int addr_for_data=addr&0xFFFFFFC0;
		for(int x=0;x<16;x++){
			cacheline[num][index].data[x]=mem[addr_for_data/4];
			addr_for_data=addr_for_data+4;
		}
		
		if(oldest[num][index]==0){
			for(int x=0;x<4;x++){
				if(oldest[x][index]!=0){
					--oldest[x][index];
				}
			oldest[num][index]=4;
			}
		}

		cacheline[num][index].tag=tag;
		cacheline[num][index].valid=1;	
		cacheline[num][index].dirty=0;
		cacheline[num][index].sca=0;
		cacheline[num][index].first=1;
		return readmem(addr, cacheline, num, diff, oldest);
	}

	else if(cacheline[num][index].valid==1){ // cache exist
		if(cacheline[num][index].tag!=tag){
			if(num==3){ //select oldest
				int old1, old2, old3, old4, erase = 0;
				for(int x=0;x<4;x++){
					if(oldest[x][index]==1)
						old1=x;
					else if(oldest[x][index]==2)
						old2=x;
					else if(oldest[x][index]==3)
						old3=x;
					else if(oldest[x][index]==4)
						old4=x;
				}
				int change=old1;
				while(1){
					if(cacheline[change][index].sca==0){
						erase=change;
						for(int x=0;x<4;x++){
							if(change==old1){
								--oldest[x][index];
							}
							else{
								if(oldest[x][index]>oldest[change][index]){
									--oldest[x][index];
								}
							}
						}
						oldest[change][index]=4;
						break;
					}
					else{
						cacheline[change][index].sca=0;
					}
					if(change==old4)
						change=old1;
					else if(change==old3)
						change=old4;
					else if(change==old2)
						change=old3;
					else if(change==old1)
						change=old2;
				}
			num=erase;
			if(diff)
				miss_data++;
			else if(diff==0)
				miss_inst++;

			if(cacheline[num][index].dirty){ // dirty->store to memory
				printf("change\n\n");
				int addr_for_data=(cacheline[num][index].tag<<14)|(index<<6);
				for(int x=0; x<16; x++){
					mem[addr_for_data/4]=cacheline[num][index].data[x];
					addr_for_data=addr_for_data+4;
					printf("%d \n",cacheline[num][index].data[x]);
				}
			}
			//erase
			cacheline[num][index].valid=0;
			return readmem(addr, cacheline, num, diff, oldest);
		}//if num==3
		
		
		else{//if num!=3
			num++;
			return readmem(addr, cacheline, num, diff, oldest);
		}

	}//tag!=tag

		else if(cacheline[num][index].tag==tag){ // hit
			if(cacheline[num][index].first==0){
				cacheline[num][index].sca=1;
			}
			else{
				cacheline[num][index].first=0;
			}
			if(diff)
				hit_data++;
			else if(diff==0)
				hit_inst++;

			windex=index;
			wnum=num;
			woffset=offset;
			return cacheline[num][index].data[offset/4];			
		}//valid==1
	}	
}

void writemem(int addr, int value, struct cache (*cacheline)[256]){
	
	readmem(addr, cacheline, 0, 1, oldest_data);
	cacheline[wnum][windex].data[woffset/4]=value;
	cacheline[wnum][windex].dirty=1;
	return ;
}

void decode(){

	if(lat1[1].valid==0) {
		if(pp)printf("[decode] : - \n");
		lat2[0].valid=0;
		lat2[0].pc=lat1[1].pc;
		return;
	}

	int inst=lat1[1].inst;
	int opcode, rs, rt, rd, shamt, funct, imm, address = 0;
	
	opcode = (inst&0xFC000000) >> 26;
	rs = (inst&0x03E00000) >> 21;
	rt = (inst&0x001F0000) >> 16;
	rd = (inst&0x0000F800) >> 11;
	shamt = (inst&0x000007C0) >> 6;
	funct = inst&0x0000003F;
	address = inst&0x03FFFFFF;
	imm = inst&0x0000FFFF;
	
	if(pp)printf("[decode]\t%08x\t",inst);
	if((opcode==andi)||(opcode==ori)){
		imm = imm&0x0000FFFF;
	} 
	else{
		if((imm>>15)==1)
			imm = imm|0xFFFF0000;
		else 
			imm = imm&0x0000FFFF;
	}

	if(opcode==0){
		if(pp)printf("opcode : 0x%x , rs : %d, rt : %d, rd : %d,shamt : %d, funct: 0x%x \n", opcode, rs, rt, rd, shamt, funct);
		address=0; imm=0;
	}
	else if((opcode==j)||(opcode==jal)){
		if(pp)printf("opcode : 0x%x , addr : 0x%x \n", opcode, address);
		rs=0; rt=0; rd=0; shamt=0; funct=0; imm=0;
	}
	else{
		if(pp)printf("opcode : 0x%x , rs : %d, rt : %d, imm : 0x%08x\n",opcode, rs, rt, imm);
		rd=0; shamt=0; funct=0; address=0;
	}

	control(opcode, funct);

	if(jump){
		jumpAddr=((address<<2)&0x0FFFFFFF)|((lat1[1].pc+4)&0xF0000000);
	}
	if(opcode==j){
		lat2[0].valid=0;
	}	
	else
		lat2[0].valid=lat1[1].valid;
	
	lat2[0].pc=lat1[1].pc;
	lat2[0].pc4=lat1[1].pc4;

	lat2[0].opcode=opcode;
	lat2[0].funct=funct;
	lat2[0].rs=rs;
	lat2[0].rt=rt;
	if(opcode==jal)
			 lat2[0].rt=31;
	lat2[0].shamt=shamt;
	lat2[0].inst=inst;
	
	lat2[0].alu1=reg[rs];
	lat2[0].v2=reg[rt];
	lat2[0].imm=imm;
		
	if(lat2[0].regDst)
		lat2[0].wreg=rd;
	else if(opcode==jal)
		lat2[0].wreg=31;
	else
		lat2[0].wreg=rt;
	
	lat2[0].alu2=reg[rt];

	return ;
}

void execute(){
	if(lat2[1].valid==0){
		lat3[0].valid=0;
		lat3[0].pc=lat2[1].pc;
		if(pp)printf("[execute] : - \n");
		return ;
	}
	n_instEx++;
	if((lat2[1].rs==lat4[1].wreg)&&(lat2[1].rs!=0)&&(lat4[1].regWrite)){
		lat2[1].alu1=lat4[1].wdata;
	}
	
	if((lat2[1].rs==lat3[1].wreg)&&(lat2[1].rs!=0)&&(lat3[1].regWrite)){
		lat2[1].alu1=lat3[1].result;
	}
		
	if((lat2[1].rt==lat4[1].wreg)&&(lat2[1].rt!=0)&&(lat4[1].regWrite)){
		lat2[1].alu2=lat4[1].wdata;
	}	
			
	if((lat2[1].rt==lat3[1].wreg)&&(lat2[1].rt!=0)&&(lat3[1].regWrite)){
		lat2[1].alu2=lat3[1].result;
	}	

	int shamt,result, a1, a2, bcond;
	a1=lat2[1].alu1;

	if(lat2[1].aluSrc==1){
		a2=lat2[1].imm;
	}
	else{
		a2=lat2[1].alu2;
	}

	shamt=lat2[1].shamt;
	result=0; bcond=0;	
	
	switch(lat2[1].opcode){
		case addi:
			result=a1+a2;
			break;
		
		case addiu:
			result=a1+a2;
			break;

		case andi:
			result=a1&a2;
			break;

		case beq:
			n_branch++;
			if(a1==a2)	bcond=1;
			else{
				n_NT++;
				bcond=0;
			}
			break;

		case bne:
			n_branch++;
			if(a1!=a2)	bcond=1;
			else{
				n_NT++;
				bcond=0;
			}
			break;

		case lbu:
			result=a1+a2;
			break;
		
		case lhu:
			result=a1+a2;
			break;
		
		case ll:
			result=a1+a2;
			break;
		
		case lui:
			result=a2<<16;
			break;
		
		case lw:
			result=a1+a2;
			break;

		case ori:
			result=a1|a2;
			break;
	
		case slti:
			if(a1<a2) result=1;
			else result=0;
			break;

		case sltiu:
			if(a1<a2) result=1;
			else result=0;
			break;

		case sb:
			result=a1+a2;
			break;
	
		case sc:
			result=a1+a2;
			break;

		case sh:
			result=a1+a2;
			break;

		case sw:
			result=a1+a2;
			break;

		case j:
			break;

		case jal:
			result=(lat2[1].pc)+8;
			break;

		case 0:
			switch(lat2[1].funct){
				case add:
					result=a1+a2;
					break;

				case addu:
					result=a1+a2;
					break;

				case and:
					result=a1&a2;
					break;

				case jr:
					jr_taken=1;
					jrjump=a1;
					break;
			
				case nor:
					result=~(a1|a2);
					break;
			
				case or:
					result=a1|a2;
					break;
	
				case slt:
					if(a1<a2) result=1;
					else	result=0;
					break;

				case sltu:
					if(a1<a2) result=1;
					else    result=0;
					break;
		
				case sll:
					result=a2<<shamt;
					break;

				case srl:
					result=a2>>shamt;
					break;
				
				case sub:
					result=a1-a2;
					break;

				case subu:
					result=a1-a2;
					break;

			}
		}

	br_taken=lat2[1].branch&bcond;
	if(lat2[1].branch){
		lat2[1].valid=0;
	}
	if(br_taken){
		branchAddr=(lat2[1].imm<<2)+(lat2[1].pc+4);
	}
	if((lat2[1].funct==jr)||(lat2[1].opcode==j)){
		lat2[1].valid=0;
	}

	if(pp)printf("[execute]\t%08x\tresult : %d(%x)\n",lat2[1].inst, result, result);
	
	lat3[0].valid=lat2[1].valid;
	lat3[0].pc=lat2[1].pc;
	lat3[0].pc4=lat2[1].pc4;

	lat3[0].result=result;
	lat3[0].wreg=lat2[1].wreg;
	lat3[0].v2=lat2[1].alu2;
		
	lat3[0].opcode=lat2[1].opcode;
	lat3[0].funct=lat2[1].funct;
	lat3[0].rs=lat2[1].rs;
	lat3[0].rt=lat2[1].rt;
	lat3[0].inst=lat2[1].inst;

	lat3[0].memtoReg=lat2[1].memtoReg;
	lat3[0].memRead=lat2[1].memRead;
	lat3[0].memWrite=lat2[1].memWrite;
	lat3[0].regWrite=lat2[1].regWrite;

	bcond=0;

	return ;
}

void memory(){
	if(lat3[1].valid==0){
		lat4[0].valid=0;
		lat4[0].pc=lat3[1].pc;
		if(pp)printf("[memory] : -\n");
		return ;
	}


	int memtoResult;
	int addr=lat3[1].result;
	
	if(lat3[1].memRead){
		memtoResult=readmem(addr, dataca, 0, 1, oldest_data);
		n_mem++;
		if(pp)printf("[memory]\t%08x\t[LW]:reg[%d] = mem[%x] \n",lat3[1].inst,memtoResult,addr);	
	}

	else if(lat3[1].memWrite){
		writemem(addr, lat3[1].v2, dataca);
		n_mem++;
		
		if(pp)printf("[memory]\t%08x\t[SW]:mem[%x] = %x \n", lat3[1].inst, addr, lat3[1].v2);
	}
	
	else{
		if(pp)printf("[memory]\t%08x\t\n",lat3[1].inst);
	}

	if(lat3[1].memtoReg)
		lat4[0].wdata=memtoResult;
	else 
		lat4[0].wdata=lat3[1].result;

	lat4[0].inst=lat3[1].inst;
	lat4[0].valid=lat3[1].valid;
	lat4[0].pc=lat3[1].pc;
	lat4[0].wreg=lat3[1].wreg;
	lat4[0].regWrite=lat3[1].regWrite;

	return;
}

void writeback(){
	if(lat4[1].pc==0xFFFFFFFF) endloop=1;
	if(lat4[1].valid==0){
		if(pp)printf("[WriteBack] : - \n");
		return ;
	}
		
	int wreg, wdata;
	wreg=lat4[1].wreg;
	wdata=lat4[1].wdata;

	if(lat4[1].regWrite){
		reg[wreg]=wdata;
		n_reg++;
		if(pp)printf("[WriteBack]\t%08x\tr[%d] : %x\n",lat4[1].inst,wreg,wdata);
	}
	else{
		if(pp)printf("[WriteBack]\t%08x\n",lat4[1].inst);
	}	
	return ;

}


void control(int opcode, int funct){
	int regDst, aluSrc, branch, memtoReg, memRead, memWrite, regWrite;


	if(opcode==0)	regDst=1;
	else	regDst=0;
	
	if((opcode!=0)&&(opcode!=beq)&&(opcode!=bne))	aluSrc=1;
	else	aluSrc=0;

	if((opcode==lw)||(opcode==ll))	memtoReg=1;
	else	memtoReg=0;

	if((opcode!=sw)&&(opcode!=sh)&&(opcode!=sb)&&(opcode!=beq)&&(opcode!=bne)&&(opcode!=j)&&(funct!=jr))	regWrite=1;
	else	regWrite=0;

	if((opcode==lw)||(opcode==ll)||(opcode==lbu)||(opcode==lhu))	memRead=1;
	else	memRead=0;

	if((opcode==sw)||(opcode==sh)||(opcode==sb))	memWrite=1;
	else	memWrite=0;
        
	if((opcode==j)||(opcode==jal))	jump=1;
	else	jump=0;

	if((opcode==bne)||(opcode==beq))	branch=1;	
	else	branch=0;

	lat2[0].regDst=regDst;
	lat2[0].aluSrc=aluSrc;
	lat2[0].memtoReg=memtoReg;
	lat2[0].memWrite=memWrite;
	lat2[0].memRead=memRead;
	lat2[0].regWrite=regWrite;
	lat2[0].branch=branch;
	lat2[0].jump=jump;


	return;
}


void openfile(){
	FILE* fp = NULL;
	int val = 0;
	int res;
	int inst;
	int num = 0;
	char* path;

	path = (char*)malloc(sizeof(char)*10);
	
	//if(argc == 2){path=argv[1];}
	
	printf("enter the bin file : ");
	scanf("%s",path);

	printf("enter the option [printout = 1 | no print = 0] : ");
	scanf("%d",&pp);

	if((strcmp(path,"s1"))==0) path="simple.bin";
	if((strcmp(path,"s2"))==0) path="simple2.bin";
	if((strcmp(path,"s3"))==0) path="simple3.bin";
	if((strcmp(path,"s4"))==0) path="simple4.bin";
	if((strcmp(path,"fib"))==0) path="fib.bin";
	if((strcmp(path,"gcd"))==0) path="gcd.bin";
	if((strcmp(path,"in"))==0) path="input4.bin";

	

	fp = fopen(path, "rb");

	if(fp==NULL){
		if(pp)printf("invalid input file: %s\n",path);
		return ;
	}

	while(1){
		res = fread(&val, 4,  1, fp);
		if(res == 0)break;
		
		inst = (val&0xFF) << 24
			| (val&0xFF00) << 8
			| (val&0xFF0000) >> 8
			| (val&0xFF000000) >> 24;
	mem[num] = inst;
	num++; n_inst++;
	}

	if(pp)printf("-------------------instruction-----------------\n");

	for(int x = 0; x < num ; x++)
		if(pp)printf("inst%d : 0x%08x\n", x , mem[x]);

	return ;
}
