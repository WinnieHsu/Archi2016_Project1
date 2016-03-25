#include <stdio.h>
#include "instruction.h"

typedef struct reg32{
    int index;
    int value;
} REGISTERS;

REGISTERS* REG[18];
//extern char PC[12], SP[12];
char PC[12], SP[12];
int cycle;

void initial_REG(){
    REG[18]=(REGISTERS*)malloc(18*sizeof(REGISTERS));
    int i;
    for(i=0; i<18; i++){
        REG[i]->index=8+i;
        REG[i]->value=0;
    }
}
void initial_SNAP(){
    cycle=0;
    FILE *fin, *fout;
    char SP[12];
    fin=fopen("../test/dimage.bin","rt");
    if(fin==NULL) {
        printf("Fail To Open File dimage.bin!!");
        return;
    }
    fout=fopen("../test/snap_test.rpt","w");
    if(fout==NULL) {
        printf("Fail To Open File snap_test.rpt!!");
        fclose(fin);
        return;
    }
    fscanf(fin,"%s",SP);

    int i, j;
    fprintf(fout,"cycle 0\n");
    for(i=0; i<32; i++){
        if(i<10) fprintf(fout,"$0%d: ",i);
        else fprintf(fout,"$%d: ",i);

        if(i==29){
            for(j=0; j<10; j++){
                fprintf(fout,"%c",SP[j]);
            }
        }
        else fprintf(fout,"0x00000000");
        fprintf(fout,"\n");
    }

    fprintf(fout,"PC: ");
    for(j=0; j<10; j++){
        fprintf(fout,"%c",PC[j]);
    } fprintf(fout,"\n");

    fprintf(fout,"\n\n");
    fclose(fin);
    fclose(fout);
}
void adderPC(){
    int carry=0, index=9;
    int digit=HEXtoDEC(PC[index]);
    digit=digit+4;
    if(digit<16){
        PC[index]=DECtoHEX(digit);
        carry=0;
        return;
    }
    else{
        PC[index]=DECtoHEX(digit-16);
        carry=1;
        index--;
        while(carry){
            if(index<2){
                printf("error: PC overflow\n");
                break;
            }

            digit=HEXtoDEC(PC[index]);
            digit=digit+carry;
            if(digit<16){
                PC[index]=DECtoHEX(digit);
                carry=0;
            }
            else{
                PC[index]=DECtoHEX(digit-16);
                carry=1;
                index--;
            }
        }
    }
}
void append_SNAP(){
    cycle++;
    FILE *fout;
    fout=fopen("../test/snap_test.rpt","a");
    if(fout==NULL) {
        printf("Fail To Open File snap_test.rpt!!");
        //fclose(fin);
        return;
    }

    int i, j;
    fprintf(fout,"cycle %d\n",cycle);
    for(i=0; i<32; i++){
        if(i<10) fprintf(fout,"$0%d: ",i);
        else fprintf(fout,"$%d: ",i);

        if(i==29){ //stack pointer SP
            for(j=0; j<10; j++){
                fprintf(fout,"%c",SP[j]);
            }
        }
        /*
        else if(i>=2 && i<=3){ //return value v0~v1
        }
        else if(i>=4 && i<=7){ //arguments a0~a3
        }
        else if((i>=8&&i<=15) || (i>=24&&i<=25)){ //t registers t0~t7, t8~t9
        }
        else if(i>=16 && i<=23){ //s registers s0~s7
        }*/
        else fprintf(fout,"0x00000000");
        fprintf(fout,"\n");
    }

    fprintf(fout,"PC: ");
    adderPC();
    for(j=0; j<10; j++){
        fprintf(fout,"%c",PC[j]);
    } fprintf(fout,"\n");

    fprintf(fout,"\n\n");
    fclose(fout);
}


void decode(){
    FILE *fin, *fout;
    char NUM[12], input[12];
    int INSTR[34]={};
    int number=0;
    fin=fopen("../test/iimage.bin","rt");
    if(fin==NULL) {
        printf("Fail To Open File iimage.bin!!");
        return;
    }
    fout=fopen("../test/out1.bin","w");
    if(fout==NULL) {
        printf("Fail To Open File out1.bin!!");
        fclose(fin);
        return;
    }

    int i, j, k, digit, base=1, term=0;
    fscanf(fin,"%s",PC);
    fscanf(fin,"%s",NUM);
    for(i=9; i>=2; i--){
        digit=HEXtoDEC(NUM[i]);
        if(digit==0) term=0;
        else term=digit*base;
        number=number+term;
        base=base*16;
    }
    //fprintf(fout,"there are %d instructions\n",number);
    /**decode each instruction**/
    //while( fscanf(fin,"%s",&input)!=EOF ){
    for(i=0; i<number; i++){
        fscanf(fin,"%s",input);
        /**from hexadecimal input to binary INSTR**/
        for(j=0; j<32; j++) INSTR[j]=0;
        for(j=9; j>=2; j--){
            digit=HEXtoDEC(input[j]);
            k=(j-1)*4-1;
            //printf("digit %d = INSTR[%d~%d] = ",digit,k-3,k);
            while(digit>0){
                INSTR[k]=digit%2;
                digit=digit/2;
                k--;
            }
        }
        for(j=0; j<32; j++) fprintf(fout,"%d",INSTR[j]);
        fprintf(fout,"\n");

        /**analyze INSTR to OPcode, RS, RT, RD, etc.**/
        int OP=BINtoDEC(INSTR,6,5);
        if(OP==0){
            //R-type instructions => go to ALU.c?
            int FUNCT=BINtoDEC(INSTR,6,31);
            int RS=BINtoDEC(INSTR,5,10);
            int RT=BINtoDEC(INSTR,5,15);
            int RD=BINtoDEC(INSTR,5,20);
            int SHAMT=BINtoDEC(INSTR,5,25);
            if(FUNCT==32) add(RS,RT,RD);
            else if(FUNCT==33) printf("this is R-type instruction addu\n");
            else if(FUNCT==34) printf("this is R-type instruction sub\n");
            else if(FUNCT==36) printf("this is R-type instruction and\n");
            else if(FUNCT==37) printf("this is R-type instruction or\n");
            else if(FUNCT==38) printf("this is R-type instruction xor\n");
            else if(FUNCT==39) printf("this is R-type instruction nor\n");
            else if(FUNCT==40) printf("this is R-type instruction nand\n");
            else if(FUNCT==42) printf("this is R-type instruction slt\n");
            else if(FUNCT==0) printf("this is R-type instruction sll\n");
            else if(FUNCT==2) printf("this is R-type instruction srl\n");
            else if(FUNCT==3) printf("this is R-type instruction sra\n");
            else if(FUNCT==8) printf("this is R-type instruction jr\n");
            else printf("this is error R-instruction\n");
        }
        else{
            if(OP==8) printf("this is I-type instruction addi\n");
            else if(OP==9) printf("this is I-type instruction addiu\n");
            else if(OP==35) printf("this is I-type instruction lw\n");
            else if(OP==33) printf("this is I-type instruction lh\n");
            else if(OP==37) printf("this is I-type instruction lhu\n");
            else if(OP==32) printf("this is I-type instruction lb\n");
            else if(OP==36) printf("this is I-type instruction lbu\n");
            else if(OP==43) printf("this is I-type instruction sw\n");
            else if(OP==41) printf("this is I-type instruction sh\n");
            else if(OP==40) printf("this is I-type instruction sb\n");
            else if(OP==15) printf("this is I-type instruction lui\n");
            else if(OP==12) printf("this is I-type instruction andi\n");
            else if(OP==13) printf("this is I-type instruction ori\n");
            else if(OP==14) printf("this is I-type instruction nori\n");
            else if(OP==10) printf("this is I-type instruction slti\n");
            else if(OP==4) printf("this is I-type instruction beq\n");
            else if(OP==5) printf("this is I-type instruction bne\n");
            else if(OP==7) printf("this is I-type instruction bgtz\n");

            else if(OP==2) printf("this is J-type instruction j\n");
            else if(OP==3) printf("this is J-type instruction jal\n");
            else if(OP==63) printf("this is Specialized instruction halt\n");
            else printf("this is error instruction\n");
        }

    }
    fclose(fin);
    fclose(fout);
}

void add(int rs, int rt, int rd){
    int i;
    for(i=0; i<=18; i++){
    }
}
/////////////////////////////////////////////

int main(){
    int i;
    decode();
    initial_SNAP();
    append_SNAP();
    append_SNAP();
    append_SNAP();
    return 0;
}

/////////////////////////////////////////////
int HEXtoDEC(char c){
    if(c=='A') return 10;
    else if(c=='B') return 11;
    else if(c=='C') return 12;
    else if(c=='D') return 13;
    else if(c=='E') return 14;
    else if(c=='F') return 15;
    else return c-'0';
}
char DECtoHEX(int n){
    if(n==10) return 'A';
    else if(n==11) return 'B';
    else if(n==12) return 'C';
    else if(n==13) return 'D';
    else if(n==14) return 'E';
    else if(n==15) return 'F';
    else return n+'0';
}
int BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        sum=sum+arr[i]*base;
        base=base*2;
    }
    return sum;
}
