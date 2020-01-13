#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

//int hex_r(char* str);
void switch_str(int i); // calculator
int str_to_int(char* str); // string -> int(hexa or decimal)

int calc_four(char* letter, int op1, int op2); // + / * - case

int r[10]; // register
char** inst_reg; // arr : store instruction from input.txt file
int count;

//////////////////////////////////////////////////////////////////////////////////
int main(void){
        FILE* stream;
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
	int num_line;
//read file

  stream = fopen("input.txt", "r");
        if(stream == NULL)
                exit(EXIT_FAILURE);
//arr malloc
count = 1;

while((read = getline(&line, &len, stream)) != -1){
count++;
}

fclose(stream);

 stream = fopen("input.txt", "r");


 inst_reg = (char**)malloc(count*sizeof(char*));

 inst_reg[0] = "\0";
int num_arr = 1;
num_line = 0;



while((read = getline(&line, &len, stream)) != -1){
	num_line++;
         inst_reg[num_arr] = (char*)malloc(sizeof(char)*len);
	strcpy(inst_reg[num_arr],line);
	num_arr++;}

	num_arr = 1;
	while(num_arr <=  num_line){
                 num_arr++;
		}	
free(line);
fclose(stream);
//exit(EXIT_SUCCESS);
switch_str(1);
printf("end of instruction\n");
free(inst_reg);
exit(EXIT_SUCCESS);

} // end of main

///////////////////////////////////////////////////

void switch_str(int i){
//token
size_t len=0;
char temp[20]; 
int x = 0;
char* temArr;
temArr = inst_reg[i];

len = strlen( inst_reg[i]);

while(x != len){
temp[x] = temArr[x];
x++;}

if(count <= i){return ;}
printf("line %d : %s", i, inst_reg[i]);

	char* letter;
	char* str1;
	char* str2;


	letter = strtok(temp," ");

///////////////////////////////////////////////////
if(*letter=='C'){
        str1 = strtok(NULL," ");
        str2 = strtok(NULL," ");

	int a, b;	
	a = str_to_int(str1);
	b = str_to_int(str2);

	if(r[a]>r[b]){r[0]=1;}
	else if(r[a]==r[b]){r[0]=0;}
	else{r[0]=-1;}

printf("\t res : R0 = %d\n",r[0] );


switch_str(++i);
}
//////////////////////////////////////////////////
else if(*letter == 'H'){
printf("\n r0 is %d \n",r[0]);
return ;
}
/////////////////////////////////////////////////////
else if(*letter =='B'){
	str1 = strtok(NULL," ");
	int a;
	int b,c;
	a = str_to_int(str1);
char* str3;

if(str2 = strtok(NULL," ")){

str3 = strtok(NULL," ");

b = str_to_int(str2);
c = str_to_int(str3);

}
;
	
if(str2 != NULL){
	if((*str1)=='R' && (*str2)=='R'){
		if(r[a] == r[b]){
printf("\t res : R[%d] and R[%d] are same (%d) ,  go to line %d \n",a,b,r[a],c);


switch_str(c);
}
		else{
printf("\t res : R[%d] and R[%d] are not same ,  go to next line \n",a,b);

switch_str(++i);
}}

else if((*str1)=='R' && *(str2)!='R'){
	if(r[a] == b){
printf("\t res : R[%d] is %d, go to line %d \n",a,b,c);
switch_str(c);
}

else{
printf("\t res : R[%d] is not %d, go to next line \n",a,b);
switch_str(++i);
}}}

else{
if(r[0] == 0){
printf("\t res : R0 is 0 ,  go to line %d \n",a);
switch_str(a);}

else{
printf("\t res : R0 is not 0 , go to next\n");

switch_str(++i);}}
}//B

//////////////////////////////////////////////////
else if(*letter == 'J'){
  str1 = strtok(NULL," ");
   
int x;
x = str_to_int(str1);
printf("\t res: go to line %d\n",x);

switch_str(x);
}

//////////////////////////////////////////////////////
else if(*letter == 'M'){
int a, b;
	str1 = strtok(NULL," ");
        str2 = strtok(NULL," ");

	a = str_to_int(str1);
	b = str_to_int(str2);

	if(*str2 == 'R'){r[a]=r[b];


        printf("\t res : R%d = %d\n",a,r[b]);
}
	else{r[a]=b;


        printf("\t res : R%d = %d\n",a,b);
}


	switch_str(++i);
}
///////////////////////////////////////////////////
else{
  	str1 = strtok(NULL," ");
        str2 = strtok(NULL," ");

	int a, b;//str1 , str2 -> int
	a = str_to_int(str1);
	b = str_to_int(str2);

	char first, second; // operand's first letter (op1,op2 each)
	first = *str1;     
	second = *str2;


if( first == 'R' && second == 'R'){

r[0] = calc_four(letter,r[a],r[b]);}

else if(first == 'R' && second == '0'){
r[0] = calc_four(letter,r[a],b);

}

else if(first == '0' && second  == 'R'){

r[0] = calc_four(letter,a,r[b]);
}

else if(first == '0' && second == '0'){
r[0] = calc_four(letter,a,b);
}

printf("\t res : R0 = %d\n",r[0]);


switch_str(++i);
}
}//end of function 

int calc_four(char* letter, int op1, int op2){
int res;
if(*letter == '*'){
res = (op1) * (op2);
return res;
}

else if(*letter == '-'){
res = (op1) - (op2);
return res;

}

else if(*letter == '+'){
res = ((op1) + (op2));
return res;
}

else if(*letter == '/'){
res = (op1) / (op2);
return res;

}


}//end of calc_four function


int str_to_int(char* str){
int ret_int;
 if(str[0]=='R') {ret_int =  strtol((str+1),NULL,10);}
else if(str[1]=='x'){ret_int = strtol(str,NULL,16);}
else{ret_int = atoi(str);}
return ret_int;
}







