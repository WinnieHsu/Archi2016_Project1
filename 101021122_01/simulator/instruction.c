#include <stdio.h>
#include "instruction.h"

int A[4][8], V[2][8], REG[18][8]={}; //denote the value of register t0~t7, S0~s7, t7~t8
int MEM_num, MEM_value[34]={};
char MEM[34][12], PC[12], SP[12];
int cycle;

void initialize(){
    /**read dimage and store in MEM**/
    char NUM[12];
    FILE *fin;
    fin=fopen("../testcase/dimage.bin","rt");
    if(fin==NULL) {
        printf("Fail To Open File dimage.bin!!");
        return;
    }
    fscanf(fin,"%s",SP);
    fscanf(fin,"%s",NUM);

    int MEM_num=HEXtoDEC(NUM,8,9);
    int i, j;
    for(i=0; i<MEM_num; i++){
        fscanf(fin,"%s",MEM[i][]);
    }
    fclose(fin);

    /**initialize ARG, V, REG**/
    for(i=0; i<4; i++){
        for(j=0; j<8; j++)
            A[i][j]=0;
    }
    for(i=0; i<2; i++){
        for(j=0; j<8; j++)
            V[i][j]=0;
    }
    for(i=0; i<18; i++){
        for(j=0; j<8; j++)
            REG[i][j]=0;
    }
}
void initial_SNAP(){
    cycle=0;
    FILE *fout;
    fout=fopen("../testcase/snap_test.rpt","w");
    if(fout==NULL) {
        printf("Fail To Open File snap_test.rpt!!");
        //fclose(fin);
        return;
    }
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
    fout=fopen("../testcase/snap_test.rpt","a");
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
        else{
            fprintf(fout,"0x");
            if(i>=2 && i<=3){ //return value v0~v1
                for(j=0; j<8; j++)
                    fprintf(fout,"%d",V[i-2][j]);
            }
            else if(i>=4 && i<=7){ //arguments a0~a3
                for(j=0; j<8; j++)
                    fprintf(fout,"%d",A[i-4][j]);
            }
            else if(i>=8&&i<=25){ //register t0~t7, s0~s7, t8~t9
                for(j=0; j<8; j++)
                    fprintf(fout,"%d",REG[i-8][j]);
            }
            else fprintf(fout,"00000000");
        }
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
    fin=fopen("../testcase/iimage.bin","rt");
    if(fin==NULL) {
        printf("Fail To Open File iimage.bin!!");
        return;
    }
    fout=fopen("../testcase/out1.bin","w"); //need to be deleted
    if(fout==NULL) {
        printf("Fail To Open File out1.bin!!");
        fclose(fin);
        return;
    }

    fscanf(fin,"%s",PC);
    fscanf(fin,"%s",NUM);
    int INSTR_num=HEXtoDEC(NUM,8,9);

    /**decode each instruction**/
    //while( fscanf(fin,"%s",&input)!=EOF ){ 不知道這樣寫行不行
    int i, j, k;
    for(i=0; i<INSTR_num; i++){
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
void jr(int rs, int rt, int rd){}

/**J-type instructions**/
void j(int c){}
void jal(int c){}
void halt(){
    exit(1); //???
}

/**I-type instructions**/
void addi(int rs, int rt, int c){
    REG[rt-8]->value = REG[rs-8]->value + REG[rt-8]->value;
    //error: overflow
}
void addiu(int rs, int rt, int c){
    REG[rt-8]->value = REG[rs-8]->value + REG[rt-8]->value;
    //error: overflow
}
void lw(int rs, int rt, int c){
    REG[rt-8]->value = MEM[rs+c]->value //改成address=rs+c的MEM的value才對
}
void lh(int rs, int rt, int c){}
void lhu(int rs, int rt, int c){}
void lb(int rs, int rt, int c){}
void lbu(int rs, int rt, int c){}
void sw(int rs, int rt, int c){
    MEM[rs+c]->value = REG[rt-8]->value //改成address=rs+c的MEM的value才對
}
void sh(int rs, int rt, int c){}
void sb(int rs, int rt, int c){}
void lui(int rt, int c){}
void andi(int rs, int rt, int c){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(c,32);
    int i;
    for(i=0; i<32; i++){
        rt_bit[i]=rs_bit[i]&rt_bit[i];
    }
    REG[rt-8]->value=BINtoDEC(rt_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void ori(int rs, int rt, int c){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(c,32);
    int i;
    for(i=0; i<32; i++){
        rt_bit[i]=rs_bit[i]|rt_bit[i];
    }
    REG[rt-8]->value=BINtoDEC(rt_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void nori(int rs, int rt, int c){
    int rs_bit[32]=DECtoBIN(REG[rs-8]->value,32);
    int rt_bit[32]=DECtoBIN(c,32);
    int i;
    for(i=0; i<32; i++){
        rt_bit[i]=~(rs_bit[i]|rt_bit[i]);
    }
    REG[rt-8]->value=BINtoDEC(rt_bit,32,31);
    free(rs_bit);
    free(rt_bit);
}
void slti(int rs, int rt, int c){
    REG[rt-8]=((REG[rs-8]->value)<c)?1:0;
}
void beq(int rs, int rt, int c){
    if(REG[rs-8]==REG[rt-8]){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();
}
void bne(int rs, int rt, int c){
    if(REG[rs-8]!=REG[rt-8]){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();
}
void bgtz(int rs, int c){
    if(REG[rs-8]>0){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();
}



/////////////////////////////////////////////

int main(){
    int i;
    decode();
    initial_REG_MEM()
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
int HEXtoDEC(int arr[], int n_bits, int start){
    int i, digit, term, number=0, base=1;
    for(i=start; i>(start-n_bits); i--){
        digit=HEXtoDEC_bit(arr[i]);
        if(digit==0) term=0;
        else term=digit*base;
        number=number+term;
        base=base*16;
    }
    return number;
}
int BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        sum=sum+arr[i]*base;
        base=base*2;
    }
    return sum;
}
int* DECtoBIN(int n, int n_bits){
    int arr[n_bits]={}, dec=n, i=n_bits-1;
    int[] arr=(int[])malloc(n_bits*sizeof(int));
    while(i>0){
        arr[i]=dec%2;
        dec=dec/2;
        i--;
    }
    return arr;
}
