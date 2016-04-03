#include <stdio.h>
#include "single_cycle.h"

char A[4][34], V[2][34], REG[18][34]; //denote the value of register t0~t7, S0~s7, t7~t8
char im_input[1024][10], dm_input[1024][10];
char dmemory[1024][34];
int im_index=0, dm_index=0;
char PC[34], SP[34], RA[34];
int INSTR_num, MEM_num;

int cycle=0;
int write$0_error=0, number_overflow=0, memory_overflow=0, misaligned=0;

void initial_SNAP(){
    FILE *fout;
    fout=fopen("snapshot.rpt","w");

    int i, j;
    char SP_hex[10]={}, PC_hex[10]={};
    fprintf(fout,"cycle 0\n");
    for(i=0; i<32; i++){
        if(i<10) fprintf(fout,"$0%d: 0x",i);
        else fprintf(fout,"$%d: 0x",i);

        if(i==29){
            //convert SP from binary to hexadecimal
            for(j=7; j>=0; j--){
                SP_hex[j] = DECtoHEX_bit( char_BINtoDEC(SP,4,(j+1)*4-1) );
            }
            for(j=0; j<8; j++){
                fprintf(fout,"%c",SP_hex[j]);
            }
        }
        else fprintf(fout,"00000000");
        fprintf(fout,"\n");
    }

    fprintf(fout,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(fout,"%c",PC_hex[j]);
    fprintf(fout,"\n");

    fprintf(fout,"\n\n");
    fclose(fout);
}
void PC_adder(){
    int carry=0, bitsum, j;
    int add[34]={0}; add[29]=1;

    for(j=31; j>=0; j--){
        bitsum=(PC[j]-'0')+add[j]+carry;
        if(bitsum<2){
            PC[j]=bitsum+'0';
            carry=0;
        }
        else{
            PC[j]=bitsum-2+'0';
            carry=1;
        }
    }
}
void append_SNAP(){
    cycle++;
    //PC_adder();
    FILE *fout;
    fout=fopen("snapshot.rpt","a");

    int i, j;
    char SP_hex[10]={}, V_hex[10]={}, A_hex[10]={}, REG_hex[10]={};

    fprintf(fout,"cycle %d\n",cycle);
    for(i=0; i<32; i++){
        if(i<10) fprintf(fout,"$0%d: 0x",i);
        else fprintf(fout,"$%d: 0x",i);

        if(i==29){
            //convert SP from binary to hexadecimal
            for(j=7; j>=0; j--)
                SP_hex[j] = DECtoHEX_bit( char_BINtoDEC(SP,4,(j+1)*4-1) );
            for(j=0; j<8; j++)
                fprintf(fout,"%c",SP_hex[j]);
        }
        else if(i>=2 && i<=3){ //return value v0~v1
            for(j=7; j>=0; j--)
                V_hex[j] = DECtoHEX_bit( char_BINtoDEC(V[i-2],4,(j+1)*4-1) );
            for(j=0; j<8; j++)
                fprintf(fout,"%c",V_hex[j]);
        }
        else if(i>=4 && i<=7){ //arguments a0~a3
            for(j=7; j>=0; j--)
                A_hex[j] = DECtoHEX_bit( char_BINtoDEC(A[i-4],4,(j+1)*4-1) );
            for(j=0; j<8; j++)
                fprintf(fout,"%c",A_hex[j]);
        }
        else if(i>=8&&i<=25){ //register t0~t7, s0~s7, t8~t9
            for(j=7; j>=0; j--)
                REG_hex[j] = DECtoHEX_bit( char_BINtoDEC(REG[i-8],4,(j+1)*4-1) );
            for(j=0; j<8; j++)
                fprintf(fout,"%c",REG_hex[j]);
        }
        else fprintf(fout,"00000000");
        fprintf(fout,"\n");
    }

    fprintf(fout,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(fout,"%c",PC_hex[j]);
    fprintf(fout,"\n");

    fprintf(fout,"\n\n");
    fclose(fout);
}

void store_imemory(char ch){
    int i;
    unsigned char mask=0x80;
    //printf("%d instruction: ",im_index);
    for (i=0; i<8; i++){
        if(ch&mask) im_input[im_index][i]='1';
        else im_input[im_index][i]='0';
        mask=mask>>1;
        //printf("%c",imemory[im_index][i]);
    } //printf("\n");
    im_index++;
}
void store_dmemory(char ch){
    int i;
    unsigned char mask=0x80;
    //printf("%d instruction: ",dm_index);
    for (i=0; i<8; i++){
        if(ch&mask) dm_input[dm_index][i]='1';
        else dm_input[dm_index][i]='0';
        mask=mask>>1;
        //printf("%c",dmemory[dm_index][i]);
    } //printf("\n");
    dm_index++;
}
void instruction_fetch(){
    char ch;
    FILE *fin;

    fin=fopen("../testcase/iimage.bin","rt");
    while(!feof(fin)){
        fscanf(fin,"%c",&ch);
        store_imemory(ch);
    }
    fin=fopen("../testcase/dimage.bin","rt");
    while(!feof(fin)){
        fscanf(fin,"%c",&ch);
        store_dmemory(ch);
    }
    fclose(fin);
}

void initialize(){
    //store the value of PC, SP, RA
    RA[34]={};
    int i, j;
    for(i=0; i<8; i++){
        PC[i]=im_input[0][i];
        PC[i+8]=im_input[1][i];
        PC[i+16]=im_input[2][i];
        PC[i+24]=im_input[3][i];
    }
    for(i=0; i<8; i++){
        SP[i]=dm_input[0][i];
        SP[i+8]=dm_input[1][i];
        SP[i+16]=dm_input[2][i];
        SP[i+24]=dm_input[3][i];
    }

    //store the number of MEM, INSTR
    int NUM[34]={0};
    for(i=0; i<8; i++){
        NUM[i]=dm_input[4][i]-'0';
        NUM[i+8]=dm_input[5][i]-'0';
        NUM[i+16]=dm_input[6][i]-'0';
        NUM[i+24]=dm_input[7][i]-'0';
    }
    int MEM_num=int_BINtoDEC(NUM,32,31);

    for(i=0; i<8; i++){
        NUM[i]=im_input[4][i]-'0';
        NUM[i+8]=im_input[5][i]-'0';
        NUM[i+16]=im_input[6][i]-'0';
        NUM[i+24]=im_input[7][i]-'0';
    }
    int INSTR_num=int_BINtoDEC(NUM,32,31);

    //store the content of memory into dmemory
    for(i=0; i<MEM_num; i++){
        for(j=0; j<8; j++){
            dmemory[i][j]=dm_input[8+4*i][j];
            dmemory[i][j+8]=dm_input[9+4*i][j];
            dmemory[i][j+16]=dm_input[10+4*i][j];
            dmemory[i][j+24]=dm_input[11+4*i][j];
        }
    }
}

void instruction_decode(){
    FILE *fout;
    fout=fopen("error_dump.rpt","a");
    initial_SNAP();

    /**decode each instruction**/
    int i, j, INSTR[34]={0};
    for(i=0; i<INSTR_num; i++){
        for(j=0; j<32; j++) INSTR[j]=0;
        for(j=0; j<8; j++){
            INSTR[j]=imemory[8+4*i][j]-'0';
            INSTR[j+8]=imemory[9+4*i][j]-'0';
            INSTR[j+16]=imemory[10+4*i][j]-'0';
            INSTR[j+24]=imemory[11+4*i][j]-'0';
        }

        /**analyze INSTR to OPcode, RS, RT, RD, etc.**/
        int OP=int_BINtoDEC(INSTR,6,5);
        if(OP==0){
            int FUNCT=int_BINtoDEC(INSTR,6,31);
            int RS=int_BINtoDEC(INSTR,5,10);
            int RT=int_BINtoDEC(INSTR,5,15);
            int RD=int_BINtoDEC(INSTR,5,20);
            int SHAMT=int_BINtoDEC(INSTR,5,25);

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
            else if(FUNCT==8) jr(RS);
            else printf("this is error R-instruction\n");
        }
        else if(OP==2||OP==3||OP==63){
            int C=int_BINtoDEC(INSTR,26,31);
            if(OP==2) jj(C);
            else if(OP==3) jal(C);
            else if(OP==63) break;
            else printf("this is error J-instruction\n");
        }
        else{
            int RS=int_BINtoDEC(INSTR,5,10);
            int RT=int_BINtoDEC(INSTR,5,15);
            int C=int_BINtoDEC(INSTR,16,31);

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
        append_SNAP();

        /**do error detection**/
        if(write$0_error){
            fprintf(fout, "In cycle %d: Write $0 Error\n", cycle);
            write$0_error=0;
        }
        if(number_overflow){
            fprintf(fout, "In cycle %d: Number Overflow\n", cycle);
            number_overflow=0;
        }
        if(memory_overflow){
            fprintf(fout, "In cycle %d: Address Overflow\n", cycle);
            break;
        }
        if(misaligned){
            fprintf(fout, "In cycle %d: Misalignment Error\n", cycle);
            break;
        }
    }
    fclose(fout);
}

/**R-type instructions**/
void add(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=0, bitsum;

    int same=(REG[rs-8][0]==REG[rt-8][0])?1:0;
    if(same) char same_bit=REG[rs-8][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')+(REG[rt-8][j]-'0')+carry;
        if(bitsum<2){
            REG[rd-8][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd-8][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same&& (REG[rd-8][0]!=same_bit)){
        number_overflow=1;
        adderPC();
        return;
    }
    else adderPC();
}
void addu(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=0, bitsum;

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')+(REG[rt-8][j]-'0')+carry;
        if(bitsum<2){
            REG[rd-8][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd-8][j]=bitsum-2+'0';
            carry=1;
        }
    }
    adderPC();
}
void sub(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=1, bitsum;
    int complement[32]={0};
    for(j=31; j>=0; j--) complement[j]=flip( REG[rt-8][j]-'0' );
    for(j=31; j>=0; j--){
        bitsum=complement[j]+carry;
        if(bitsum<2){
            complement[j]=bitsum;
            carry=0;
        }
        else{
            complement[j]=bitsum-2;
            carry=1;
        }
    }

    int same=( (REG[rs-8][0]-'0')==complement[0] ) ? 1:0;
    if(same) char same_bit=REG[rs-8][0];
    carry=0;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')+complement[j]+carry;
        if(bitsum<2){
            REG[rd-8][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd-8][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same&& (REG[rd-8][0]!=same_bit)){
        number_overflow=1;
        adderPC();
        return;
    }
    else adderPC();
}
void and(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, bitsum;
    if(rs==0 || rt==0){
        for(j=0; j<32; j++) REG[rd-8][j]=0;
        return;
    }
    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')&(REG[rt-8][j]-'0');
        REG[rd-8][j]=bitsum+'0';
    }
    adderPC();
}
void or(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')|(REG[rt-8][j]-'0');
        REG[rd-8][j]=bitsum+'0';
    }
    adderPC();
}
void xor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')^(REG[rt-8][j]-'0');
        REG[rd-8][j]=bitsum+'0';
    }
    adderPC();
}
void nor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    for(j=31; j>=0; j--){
        bitsum=~((REG[rs-8][j]-'0')|(REG[rt-8][j]-'0'));
        REG[rd-8][j]=bitsum+'0';
    }
    adderPC();
}
void nand(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    for(j=31; j>=0; j--){
        bitsum=~((REG[rs-8][j]-'0')&(REG[rt-8][j]-'0'));
        REG[rd-8][j]=bitsum+'0';
    }
    adderPC();
}
void slt(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, result;
    if(REG[rs-8][0]!=REG[rt-8][0]){ //different sign bit
        result=(REG[rt-8][0]=='0')?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd-8][j]='1';
                else REG[rd-8][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rd-8][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs-8][j]!=REG[rt-8][j]){
                result=(REG[rt-8][j]=='1')?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd-8][j]='1';
                else REG[rd-8][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rd-8][j]='0';
    }
    adderPC();
}
void sll(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=shamt; i<32; i++)
        REG[rd-8][i-shamt]=REG[rt-8][i];
    for(i=32-shamt; i<32; i++)
        REG[rd-8][i]='0';

    adderPC();
}
void srl(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd-8][i+shamt]=REG[rt-8][i];
    for(i=0; i<shamt; i++)
        REG[rd-8][i]='0';

    adderPC();
}
void sra(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd-8][i+shamt]=REG[rt-8][i];
    for(i=0; i<shamt; i++)
        REG[rd-8][i]=REG[rt-8][0];

    adderPC();
}
void jr(int rs){
    int j;
    if(rs==0){
        for(j=31; j>=0; j--) PC[j]='0';
    }
    else{
        for(j=31; j>=0; j--) PC[j]=REG[rs-8][j];
    }
}

/**J-type instructions**/
void jj(int c){}
void jal(int c){}

/**I-type instructions**/
void addi(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j=31, carry=0, bitsum;
    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    int same=(REG[rs-8][0]==cbin[0]+'0')?1:0;
    if(same) char same_bit=REG[rs-8][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt-8][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt-8][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same&& (REG[rt-8][0]!=same_bit)){
        number_overflow=1;
        adderPC();
        return;
    }
    else adderPC();
}
void addiu(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j=31, carry=0, bitsum;
    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt-8][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt-8][j]=bitsum-2+'0';
            carry=1;
        }
    }
    adderPC();
}
void lw(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }
    //REG[rt-8] = MEM[rs+c] //改成address=rs+c的MEM的value才對
    adderPC();
}
void lh(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }
    adderPC();
}
void lhu(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }
    adderPC();
}
void lb(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }
    adderPC();
}
void lbu(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }
    adderPC();
}
void sw(int rs, int rt, int c){
    //MEM[rs+c]->value = REG[rt-8]->value //改成address=rs+c的MEM的value才對
    adderPC();
}
void sh(int rs, int rt, int c){
    adderPC();
}
void sb(int rs, int rt, int c){
    adderPC();
}
void lui(int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(16 bit)
    int j=15, digit=c, cbin[18]={0};
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=15; j>=0; j--) REG[rt-8][j]=cbin[j]+'0';
    adderPC();
}
void andi(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j;
    if(rs==0 || c==0){
        for(j=0; j<32; j++) REG[rt-8][j]='0';
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum; j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')&cbin[j];
        REG[rt-8][j]=bitsum+'0';
    }
    adderPC();
}
void ori(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=(REG[rs-8][j]-'0')|cbin[j];
        REG[rt-8][j]=bitsum+'0';
    }
    adderPC();
}
void nori(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, carry=0, bitsum, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    for(j=31; j>=0; j--){
        bitsum=~((REG[rs-8][j]-'0')|cbin[j]);
        REG[rt-8][j]=bitsum+'0';
    }
    adderPC();
}
void slti(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    //convert c from decimal to binary(sign extension)
    int digit=c, cbin[34]={0}, j=31;
    while(digit>0){
        cbin[j]=digit%2;
        digit=digit/2;
        j--;
    }

    int result;
    if(REG[rs-8][0]!=(cbin[0]+'0')){ //different sign bit
        result=(cbin[0]==0)?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt-8][j]='1';
                else REG[rt-8][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt-8][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs-8][j]!=(cbin[j]+'0')){
                result=(cbin[j]==1)?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt-8][j]='1';
                else REG[rt-8][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt-8][j]='0';
    }
    adderPC();
}
void beq(int rs, int rt, int c){
    int j, result=1;
    for(j=31; j>=0; j--){
        if(REG[rs-8][j]!=REG[rt-8][j]){
            result=0;
            break;
        }
    }
    /*if(result){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();*/
}
void bne(int rs, int rt, int c){
    int j, result=0;
    for(j=31; j>=0; j--){
        if(REG[rs-8][j]!=REG[rt-8][j]){
            result=1;
            break;
        }
    }
    /*if(result){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();*/
}
void bgtz(int rs, int c){
    int j, result=0;
    for(j=0; j<32; j--){
        if(REG[rs-8][0]=='1'){ //is negative
            result=0;
            break;
        }
        if(REG[rs-8][j]=='1'){ //is positive and >0
            result=1;
            break;
        }
    }
    /*if(result){
        PC=PC+4+4*c; //need to be modified
    }
    else
        adderPC();*/
}


/////////////////////////////////////////////

int main(){
    instruction_fetch();
    initialize();
    instruction_decode();
    return 0;
}

/////////////////////////////////////////////

/*int HEXtoDEC_bit(char c){
    if(c=='A') return 10;
    else if(c=='B') return 11;
    else if(c=='C') return 12;
    else if(c=='D') return 13;
    else if(c=='E') return 14;
    else if(c=='F') return 15;
    else return c-'0';
}*/
char DECtoHEX_bit(int n){
    if(n==10) return 'A';
    else if(n==11) return 'B';
    else if(n==12) return 'C';
    else if(n==13) return 'D';
    else if(n==14) return 'E';
    else if(n==15) return 'F';
    else return n+'0';
}
/*int char_HEXtoDEC(char arr[], int n_bits, int start){
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
int int_HEXtoDEC(int arr[], int n_bits, int start){
    int i, digit, term, number=0, base=1;
    for(i=start; i>(start-n_bits); i--){
        digit=arr[i];
        if(digit==0) term=0;
        else term=digit*base;
        number=number+term;
        base=base*16;
    }
    return number;
}*/
int char_BINtoDEC(char arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        sum=sum+(arr[i]-'0')*base;
        base=base*2;
    }
    return sum;
}
int int_BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        sum=sum+arr[i]*base;
        base=base*2;
    }
    return sum;
}
int flip(int n){
    return (n==0)?1:0;
}
