#include <stdio.h>
#include "instruction.h"

typedef struct regfile{
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
    int digit=HEXtoDEC_bit(PC[index]);
    digit=digit+4;
    if(digit<16){
        PC[index]=DECtoHEX_bit(digit);
        carry=0;
        return;
    }
    else{
        PC[index]=DECtoHEX_bit(digit-16);
        carry=1;
        index--;
        while(carry){
            if(index<2){
                printf("error: PC overflow\n");
                break;
            }

            digit=HEXtoDEC_bit(PC[index]);
            digit=digit+carry;
            if(digit<16){
                PC[index]=DECtoHEX_bit(digit);
                carry=0;
            }
            else{
                PC[index]=DECtoHEX_bit(digit-16);
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
        digit=HEXtoDEC_bit(NUM[i]);
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
            digit=HEXtoDEC_bit(input[j]);
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
            int FUNCT=BINtoDEC(INSTR,6,31);
            int RS=BINtoDEC(INSTR,5,10);
            int RT=BINtoDEC(INSTR,5,15);
            int RD=BINtoDEC(INSTR,5,20);
            int SHAMT=BINtoDEC(INSTR,5,25);

            if(FUNCT==32) add(RS,RT,RD);
            else if(FUNCT==33) addu(RS,RT,RD);
            else if(FUNCT==34) sub(RS,RT,RD);
            else if(FUNCT==36) and(RS,RT,RD);
            else if(FUNCT==37) or(RS,RT,RD);
            else if(FUNCT==38) xor(RS,RT,RD);
            else if(FUNCT==39) nor(RS,RT,RD);
            else if(FUNCT==40) nand(RS,RT,RD);
            else if(FUNCT==42) slt(RS,RT,RD);
            else if(FUNCT==0) sll(RT,RD,SHAMT);
            else if(FUNCT==2) srl(RT,RD,SHAMT);
            else if(FUNCT==3) sra(RT,RD,SHAMT);
            else if(FUNCT==8) jr(RS,RT,RD);
            else printf("this is error R-instruction\n");
        }
        else if(OP==2||OP==3||OP==63){
            int C=BINtoDEC(INSTR,26,31);
            if(OP==2) j(C);
            else if(OP==3) jal(C);
            else if(OP==63) halt();
            else printf("this is error J-instruction\n");
        }
        else{
            int RS=BINtoDEC(INSTR,5,10);
            int RT=BINtoDEC(INSTR,5,15);
            int C=BINtoDEC(INSTR,16,31);

            if(OP==8) addi(RS,RT,C);
            else if(OP==9) addiu(RS,RT,C);
            else if(OP==35) lw(RS,RT,C);
            else if(OP==33) lh(RS,RT,C);
            else if(OP==37) lhu(RS,RT,C);
            else if(OP==32) lb(RS,RT,C);
            else if(OP==36) lbu(RS,RT,C);
            else if(OP==43) sw(RS,RT,C);
            else if(OP==41) sh(RS,RT,C);
            else if(OP==40) sb(RS,RT,C);
            else if(OP==15) lui(RT,C);
            else if(OP==12) andi(RS,RT,C);
            else if(OP==13) ori(RS,RT,C);
            else if(OP==14) nori(RS,RT,C);
            else if(OP==10) slti(RS,RT,C);
            else if(OP==4) beq(RS,RT,C);
            else if(OP==5) bne(RS,RT,C);
            else if(OP==7) bgtz(RS,C);
            else printf("this is I-error instruction\n");
        }
    }
    fclose(fin);
    fclose(fout);
}

/**R-type instructions**/
void add(int rs, int rt, int rd){
    REG[rd-8]->value = REG[rs-8]->value + REG[rt-8]->value;
    //error: overflow
}
void addu(int rs, int rt, int rd){
    REG[rd-8]->value = REG[rs-8]->value + REG[rt-8]->value;
    //error: overflow
}
void sub(int rs, int rt, int rd){
    REG[rd-8]->value = REG[rs-8]->value - REG[rt-8]->value;
}
void and(int rs, int rt, int rd){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32; i++){
        rs_bit[i]=rs_bit[i]&rt_bit[i];
    }
    REG[rd-8]->value=BINtoDEC(rs_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void or(int rs, int rt, int rd){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32; i++){
        rs_bit[i]=rs_bit[i]|rt_bit[i];
    }
    REG[rd-8]->value=BINtoDEC(rs_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void xor(int rs, int rt, int rd){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32; i++){
        rs_bit[i]=rs_bit[i]^rt_bit[i];
    }
    REG[rd-8]->value=BINtoDEC(rs_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void nor(int rs, int rt, int rd){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32; i++){
        rs_bit[i]=~(rs_bit[i]|rt_bit[i]);
    }
    REG[rd-8]->value=BINtoDEC(rs_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void nand(int rs, int rt, int rd){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32; i++){
        rs_bit[i]=~(rs_bit[i]&rt_bit[i]);
    }
    REG[rd-8]->value=BINtoDEC(rs_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void slt(int rs, int rt, int rd){
    REG[rd-8]=((REG[rs-8]->value)<(REG[rt-8]->value))?1:0;
}
void sll(int rt, int rd, int shamt){
    int rd_bit[32]=DECtoBIN(REG[rd-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=shamt; i<32; i++){
        rd_bit[i-shamt]=rt_bit[i];
    }
    for(i=32-shamt; i<32; i++){
        rd_bit[i]=0;
    }
    REG[rd-8]->value=BINtoDEC(rd_bit,32,31);
    free(rd_bit);
    free(rt_bit);
}
void srl(int rt, int rd, int shamt){
    int rd_bit[32]=DECtoBIN(REG[rd-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32-shamt; i++){
        rd_bit[i+shamt]=rt_bit[i];
    }
    for(i=0; i<shamt; i++){
        rd_bit[i]=0;
    }
    REG[rd-8]->value=BINtoDEC(rd_bit,32,31);
    free(rd_bit);
    free(rt_bit);
}
void sra(int rt, int rd, int shamt){
    int rd_bit[32]=DECtoBIN(REG[rd-8]->value,32);
    int rt_bit[32]=DECtoBIN(REG[rt-8]->value,32);
    int i;
    for(i=0; i<32-shamt; i++){
        rd_bit[i+shamt]=rt_bit[i];
    }
    for(i=0; i<shamt; i++){
        rd_bit[i]=rt_bit[0];
    }
    REG[rd-8]->value=BINtoDEC(rd_bit,32,31);
    free(rd_bit);
    free(rt_bit);
}
void jr(RS,RT,RD);

/**J-type instructions**/
void j(C);
void jal(C);
void halt();

/**I-type instructions**/
void addi(RS,RT,C);
void addiu(RS,RT,C);
void lw(RS,RT,C);
void lh(RS,RT,C);
void lhu(RS,RT,C);
void lb(RS,RT,C);
void lbu(RS,RT,C);
void sw(RS,RT,C);
void sh(RS,RT,C);
void sb(RS,RT,C);
void lui(RT,C);
void andi(RS,RT,C);
void ori(RS,RT,C);
void nori(RS,RT,C);
void slti(RS,RT,C);
void beq(RS,RT,C);
void bne(RS,RT,C);
void bgtz(RS,C);



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
int HEXtoDEC_bit(char c){
    if(c=='A') return 10;
    else if(c=='B') return 11;
    else if(c=='C') return 12;
    else if(c=='D') return 13;
    else if(c=='E') return 14;
    else if(c=='F') return 15;
    else return c-'0';
}
char DECtoHEX_bit(int n){
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
(int[]) DECtoBIN(int n, int n_bits){
    int arr[n_bits]={}, dec=n, i=n_bits-1;
    int[] arr=(int[])malloc(n_bits*sizeof(int));
    while(i>0){
        arr[i]=dec%2;
        dec=dec/2;
        i--;
    }
    return arr;
}
