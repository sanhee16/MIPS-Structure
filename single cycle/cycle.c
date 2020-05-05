#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
char* path = "gcd.bin";

int opcode;
int rs,rt,rd;
int shamt;
int funct;
short imm;
int address;
int inst;
int branchAddr;

int regDst;
int jump;
int branch;
int memRead;
int memtoReg;
int memWrite;
int aluSrc;
int regWrite;

int mem_address;
int memory[0x100000];//memory
int reg[32];
int pc;
//for print out num
int R; // r inst
int I; // i inst
int J; // j inst
int N; // total inst
int v0; //return value
int B; // branch
int M; // memory access inst

//variable NAME

//REG
int readReg1; // rs
int readReg2; // rt
int writeReg; // rd
int writeData; // if mem_mux == 0 -> writeData = result
int readData1; // reg[rs]
int readData2; // reg[rt]
int inst_ext;

//ALU
int result; // alu result
int bcond; // bcond & branch -> pcsrc2
int aluData1;
int aluData2;

//branch_mux =
int address_mem;
int writeData_mem;
int readData_mem;

//pc
int pc0;
int pc1;
int pc2;
int pc_add1;
int pc_add2;
int ALUresult;
int jumpAddress;
int br_taken;

//MUX
int reg_mux; // regDst = 1 -> 1
int alu_mux; //ALUSrc = 1 -> 1
int mem_mux; // MemtoReg = 1 -> 1
int pc_mux; // pcSrc2 = br taken -> 1
int branch_mux; // pcSrc1 jump -> 1



void fetch();
void decode();
void execute();
void result_memory();
void write_back();

void signExtImm();
void zeroExtImm();
void setALUdata();
void branchAddress();
void set_reg();
void add_pc();
void set_MUX();
void control();
void shift_left();
void setting();
void print_inst();

void main(int argc, char *argv[]){
        FILE* fp = NULL;
        //  char* path = "simple4.bin";
        int val = 0;
        int res;
        int i = 0;

        memset(reg,0,32);//all of register initialize to 0
        //set basic registers
        pc = 0;
        reg[29] = 0x100000;
        reg[31] = 0xffffffff;

        if(argc == 2){ path=argv[1];}

        fp = fopen(path, "rb");
        if(fp==NULL){
                printf("invalid input file: %s\n",path);
                return ;
        }

        while(1){
                res = fread(&val, 4,  1, fp);
                if(res == 0)break;

                inst = (val&0xFF) << 24
                        | (val&0xFF00) << 8
                        | (val&0xFF0000) >> 8
                        | (val&0xFF000000) >> 24;
                //     printf("0x%08x --> 0x%08x \n", val, inst);
                printf("%08x \n",inst);
                memory[i] = inst;
                i++;
        }

        //int x = 0;
        int cycle = 0;

        while(1){

                if(pc==0xffffffff) break;
                cycle++;

                //if(cycle==100)break;
                //if pc meet 0xffffffff -> go out of loop

                printf("\n-----------------[cycle %d]---------------\n",cycle);
                if((pc/4)>i) break;

                //print pc value
                printf("[PC] \t0x%08x\n",pc);

                fetch();
                printf("[inst]\t0x%08x \n", inst);

                if(inst!=0x0){ // not nop
                        decode(inst);
                        setting();
                        execute();
                        result_memory(); // memory access
                        write_back();// update
                        N++;
                }
                else{
                        printf("nop \n"); pc = pc+4;
                        R++;
                        N++;}
        } // while(1)

        //end of file, print result(V0)
        v0 = reg[2];
        printf("\n-----------------------result------------------------\n");
        printf("\nnumber of total cycle %d \t total inst %d \nR-type %d\tI-type %d\tJ-type %d \nbranch %d\tmemory access %d\nreturn value %d\n",cycle,N,R,I,J,B,M,v0);
        printf("\n-----------------------------------------------------\n");

        fclose(fp);

        return ;
}

/////////////////////////////////////////////fetch///////////////////////////////
void fetch(){ // memory -> CPU
        //get inst from memory
        inst=memory[pc/4];
}

//set basic register value
void set_reg(){
        if(reg_mux==1){writeReg = rd;} // R type
        else{writeReg=rt;} //I type

        //common register set(R, I type)
        readReg1 = rs;
        readReg2 = rt;
        readData1 = reg[readReg1];
        readData2 = reg[readReg2];
        return ;
}

void decode(){

        opcode = (inst&0xFC000000) >> 26;

        bcond=0; // for br_taken, if beq or bne satisfy branch condition, bcond become 1
        rs,rt,rd=0;
        shamt = 0;
        funct = 0;
        imm = 0;
        address=0;

        if(opcode == 0){ // R-type

                rs = (inst&0x03E00000) >> 21;
                rt = (inst&0x001F0000) >> 16;
                rd = (inst&0x0000F800) >> 11;
                shamt = (inst&0x000007C0) >> 6;
                funct = inst&0x0000003F;
                R++;
        }
        else if(opcode == 2 || opcode == 3){ //J-type(j, jal)
                address = inst&0x03FFFFFF;
                J++;
        }

        else{ //I -type
                rs = (inst&0x03E00000) >> 21;
                rt = (inst&0x001F0000) >> 16;
                imm = inst&0x0000FFFF;
                I++;
        }
        return ;

}//end of decode

void execute(){ // ALU, result means ALU result
        switch(opcode){
                case addi: //addi overflow X
                        result=aluData1+aluData2;
                        break;

                case addiu: //addiu overflow O
                        result=aluData1+aluData2;
                        break;

                case andi: //and
                        result=aluData1&aluData2;
                        break;
                case beq: //beq
                        if(aluData1==aluData2){
                                bcond = 1;
                                B++;}
                        else{bcond = 0;}
                        break;

                case bne: //bne
                        if(aluData1!=aluData2){
                                bcond =1;
                                B++;
                        }
                        else{bcond = 0;}
                        break;

                case j: //j
                        result = address;
                        break;

                case jal: //jal
                        result = address;

                        break;
                case lbu: // ibu
                        result = ((aluData1+aluData2));
                        break;

                case lhu: //lhu
                        result = ((aluData1+aluData2));
                        break;

                case ll: //ll
                        result=((aluData1+aluData2));
                        break;

                case lui: //lui
                        result=(0xFFFF0000)&(aluData2<<16);
                        break;

                case lw://lw
                        result=((aluData1+aluData2));
                        break;

                case ori: //ori
                        result=aluData1|aluData2;
                        break;

                case slti:
                        if(aluData1<aluData2){
                                result=1;
                        }
                        else{
                                result=0;
                        }
                        break;


                case sb: //sb
                        result = ((aluData1+aluData2));
                        break;

                case sh: //sh
                        result = ((aluData1+aluData2));
                        break;

                case sw: //sw
                        result = ((aluData1+aluData2));
                        break;

                case 0: // R_type

                        switch(funct){
                                case add: //add
                                        result = aluData1+aluData2;
                                        break;

                                case addu:  //addu
                                        result = aluData1+aluData2;
                                        break;
                                case and: //and
                                        result=aluData1&aluData2;
                                        break;

                                case jr: // jr
                                        result = aluData1;
                                        break;

                                case nor://nor
                                        result=0xffffffff-(aluData1|aluData2);
                                        break;

                                case or: //or
                                        result=(aluData1|aluData2);
                                        break;

                                case slt: //slt
                                        if(aluData1<aluData2){
                                                result=1;
                                        }
                                        else{
                                                result=0;
                                        }
                                        break;

                                case sltu: //sltu
                                        if(aluData1<aluData2){
                                                result=1;
                                        }
                                        else{
                                                result=0;
                                        }
                                        break;

                                case sll: //sll
                                        result=aluData2<<shamt;
                                        break;

                                case srl: //srl
                                        result=aluData1>>shamt;
                                        break;


                                case sub: //sub
                                        result=aluData1-aluData2;
                                        break;

                                case subu: //subu
                                        result=aluData1-aluData2;
                                        break;
                                case jalr: // Rtype;
                                        result=aluData1;
                        }//R_type

        }//end of switch(opcode)
        print_inst(); // printf instruction
        br_taken = bcond&branch; // for branch
        set_MUX(); // because of branch, set again
        return ;
} // end of execute



void signExtImm(){
        inst_ext = imm; // short->int(16 bits -> 32 bits)
        if((imm >> 15)==1){ //negative
                inst_ext = imm|0xffff0000;}
        else{ // postive
                inst_ext = imm;}
        return ;
}

void zeroExtImm(){

        inst_ext=imm;
        inst_ext = (inst_ext)&0x0000ffff;

        return ;
}

void branchAddress(){
        signExtImm();
}

void setALUdata(){
        aluData1 = readData1;
        if(alu_mux==1){  // I-type
                if((opcode==andi)||(opcode==ori)){
                        zeroExtImm();}
                else{signExtImm();}

                aluData2 = inst_ext;
        }

        else{ //R-type
                aluData2 = readData2;
        }

        if((opcode==bne)||(opcode==beq)){branchAddress();}
}


void result_memory(){
        //basic set, after ALU
        address_mem = result;
        writeData_mem = readData2;

        if(memWrite==1){ // store
                readData_mem = address_mem;
        }

        if(memRead==1){ //load
                readData_mem = memory[address_mem];
        }

        ///////////////////mem to Reg////////////////////
        if(mem_mux == 1){ // load
                writeData = readData_mem;}
        else{  //R, I type(store = *)
                writeData = result;}

}


void write_back(){
        //register update
        if(regWrite==1){

                if((opcode==jal)){
                        reg[31]=pc+8; // r[31] update
                        printf("r[31] = 0x%x \n",reg[31]);}

                else if(funct==jalr){ // instead of 8, insert 4
                        reg[rd]=pc+4; // r[31] update
                        printf("r[31] = 0x%x \n",reg[31]);}

                else{ //register update
                        reg[writeReg] = writeData;
                        printf("r[%d] =(0x%08x)\n", writeReg,  writeData);
                }
        }
        //memory update//
        if(memWrite==1){//store
                memory[readData_mem]=writeData_mem;     M++;
                printf("store to memory, M[0x%08x] = r[%d] = %08x\n", readData_mem, readReg2 ,writeData_mem);
        }

        if(memRead==1){ //load
                reg[writeReg]=writeData;        M++;
                printf("load from memory, r[%d] = 0x%08x\n", writeReg,writeData);
        }

        //pc update
        add_pc(); // pc update
}



void add_pc(){ // regard with PC register
        pc = pc+4;
        pc0 = pc;
        pc_add1 = pc;
        pc_add2=(inst_ext<<2);
        ALUresult = pc_add1 + pc_add2;

        if((funct==jr)||(funct==jalr)){ //jr or jalr
                jumpAddress = result;
        }
        else if((jump==1)&&(funct!=jr)&&(funct!=jalr)){ //jal or j
                address = address<<2; // shift left 2
                jumpAddress=((pc&0xf0000000)|(address));
        }
        //////////////branch , prsrc2/////////////
        if(branch_mux == 1){ // branch(beq or bne)
                pc1 = ALUresult;}
        else{pc1 = pc0;} // except branch or jump
        //////////////////jump , prsrc1//////////////
        if(pc_mux==1) //  = jump
        {pc2 = jumpAddress;}
        else{pc2 = pc1;} // except jump : branch or R/I type

        pc=pc2; // pc update
        return ;
}


//set control signal
void control(){
        if(opcode == 0){regDst=1;}
        else{regDst = 0;}
        if((opcode!=0)&&(opcode!=beq)&&(opcode!=bne)) {aluSrc=1;}
        else{aluSrc=0;}
        if((opcode==lw)||(opcode==ll)){ memtoReg =1;}
        else{memtoReg = 0;}
        if((opcode!=sw)&&(opcode!=sh)&&(opcode!=sb)&&(opcode!=beq)&&(opcode!=bne)&&(opcode!=j)&&(funct!=jr)){ regWrite=1;}
        else{regWrite = 0;}
        if((opcode==lw)||(opcode==ll)||(opcode==lbu)||(opcode==lhu)){memRead = 1;}
        else{memRead = 0;}
        if((opcode==sw)||(opcode==sh)||(opcode==sb)){ memWrite=1;}
        else{memWrite=0;}
        if((opcode==j)||(opcode==jal)||(funct==jr)||(funct==jalr)){jump=1;}
        else{jump = 0;}
        if((opcode==bne)||(opcode==beq)){branch=1;}
        else{branch = 0;}
        //print control signal
        printf("regdst %d, alusrc %d, memtoreg %d, regwrite %d, memread %d, memwrite %d, jump %d, branch %d \n", regDst,aluSrc, memtoReg,regWrite, memRead, memWrite, jump, branch );

        return ;
}


void set_MUX(){
        //with control signal, set mux(5)
        if(regDst==1) {reg_mux = 1;}
        else{reg_mux = 0;}
        if(aluSrc == 1){alu_mux = 1;}
        else{alu_mux = 0;}
        if(memtoReg==1){mem_mux = 1;}
        else{mem_mux = 0;}
        if(br_taken==1){branch_mux = 1;} //execute before pcsrc1
        else{branch_mux = 0;}
        if(jump==1){pc_mux = 1;} // jr j jal jalr
        else{pc_mux = 0;}

        return ;
}

//print inst
void print_inst(){
        if(opcode==0){
                printf("opcode : 0x%x , rs : %d, rt : %d, rd : %d,shamt : %d, funct: 0x%x \n", opcode, readReg1, readReg2, writeReg, shamt, funct);
        }
        else if((opcode==0x3)||(opcode==0x2)){
                printf("opcode : 0x%x , addr : 0x%x \n", opcode, address);
        }
        else{
                printf("opcode : 0x%x , rs : %d, rt : %d, imm : 0x%x\n",opcode, readReg1, readReg2, inst_ext);
        }

}

void setting(){
        //set control signal, mux, register, aludata
        //before execute(ALU)
        control(); // set contol signal
        set_MUX(); // set mux
        set_reg(); // set register
        setALUdata(); // set aluData


}
