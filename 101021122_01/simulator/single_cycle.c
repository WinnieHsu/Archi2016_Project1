#include <stdio.h>
#include "single_cycle.h"

char REG[32][34]={}; //denote the value of register $zero, at, v0~v1, a0~a3, t0~t7, s0~s7, t7~t8, k0~k1, gp, sp, fp, ra
char im_input[1026][10]={}, dm_input[1026][10]={};
char PC_init[34]={}, PC_now[34]={};
char imemory[256][34]={}, dmemory[256][34]={};
int PC_initptr=0, PC_nowptr=0;
int im_index=0, dm_index=0;

int INSTR_num=0, MEM_num=0;
int cycle=0, halt=0; //int index=0=PC_nowptr/4?
int write$0_error=0, number_overflow=0, memory_overflow=0, misaligned=0;

FILE *snap, *error;

int test=0; //whether to printf

void initial_SNAP(){
    int i, j;
    char SP_hex[10]={}, PC_hex[10]={};
    fprintf(snap,"cycle 0\n");
    for(i=0; i<32; i++){
        if(i<10) fprintf(snap,"$0%d: 0x",i);
        else fprintf(snap,"$%d: 0x",i);

        if(i==29){
            //convert SP from binary to hexadecimal
            for(j=7; j>=0; j--){
                SP_hex[j] = DECtoHEX_bit( char_BINtoDEC(REG[29],4,(j+1)*4-1) );
            }
            for(j=0; j<8; j++){
                fprintf(snap,"%c",SP_hex[j]);
            }
        }
        else fprintf(snap,"00000000");
        fprintf(snap,"\n");
    }

    fprintf(snap,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC_init,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(snap,"%c",PC_hex[j]);

    fprintf(snap,"\n\n\n");
}
void PC_adder(){
    int carry=0, bitsum, j=29;
    int add[34]={0}; add[29]=1;

    bitsum=(PC_now[j]-'0')+add[j]+carry;
    if(bitsum<2){
        PC_now[j]=bitsum+'0';
        carry=0;
    }
    else{
        PC_now[j]=bitsum-2+'0';
        carry=1;
        j--;
        while(carry && j>=0){
            bitsum=(PC_now[j]-'0')+add[j]+carry;
            if(bitsum<2){
                PC_now[j]=bitsum+'0';
                carry=0;
                break;
            }
            else{
                PC_now[j]=bitsum-2+'0';
                carry=1;
                j--;
            }
        }
    }
    PC_nowptr=char_BINtoDEC(PC_now,32,31);
}
void append_SNAP(){
    cycle++;
    snap=fopen("snapshot.rpt","a");

    int i, j;
    char REG_hex[10]={}, PC_hex[10]={};

    fprintf(snap,"cycle %d\n",cycle);
    for(i=0; i<32; i++){
        if(i<10) fprintf(snap,"$0%d: 0x",i);
        else fprintf(snap,"$%d: 0x",i);

        for(j=7; j>=0; j--)
            REG_hex[j] = DECtoHEX_bit( char_BINtoDEC(REG[i],4,(j+1)*4-1) );
        for(j=0; j<8; j++)
            fprintf(snap,"%c",REG_hex[j]);

        fprintf(snap,"\n");
    }

    fprintf(snap,"PC: 0x");
    //convert PC from binary to hexadecimal
    for(j=7; j>=0; j--)
        PC_hex[j] = DECtoHEX_bit( char_BINtoDEC(PC_now,4,(j+1)*4-1) );
    for(j=0; j<8; j++)
        fprintf(snap,"%c",PC_hex[j]);

    fprintf(snap,"\n\n\n");
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
    int i,j;
    //initialize REG, imemory, dmemory
    for(i=0; i<32; i++){
        for(j=0; j<34; j++){
            REG[i][j]='0';
        }
    }
    for(i=0; i<256; i++){
        for(j=0; j<34; j++){
            dmemory[i][j]='0';
            imemory[i][j]='0';
        }
    }

    //store the value of PC, SP
    for(i=0; i<8; i++){
        PC_init[i]=im_input[0][i];
        PC_init[i+8]=im_input[1][i];
        PC_init[i+16]=im_input[2][i];
        PC_init[i+24]=im_input[3][i];

        PC_now[i]=im_input[0][i];
        PC_now[i+8]=im_input[1][i];
        PC_now[i+16]=im_input[2][i];
        PC_now[i+24]=im_input[3][i];
    }
    PC_initptr=char_BINtoDEC(PC_init, 32, 31);
    PC_nowptr=PC_initptr;

    if(test==1) printf("enter initialize with PC_initptr=%d\n",PC_initptr);
    for(i=0; i<8; i++){ //SP
        REG[29][i]=dm_input[0][i];
        REG[29][i+8]=dm_input[1][i];
        REG[29][i+16]=dm_input[2][i];
        REG[29][i+24]=dm_input[3][i];
    }

    //store the number of MEM, INSTR
    int NUM[34]={0};
    for(i=0; i<8; i++){
        NUM[i]=dm_input[4][i]-'0';
        NUM[i+8]=dm_input[5][i]-'0';
        NUM[i+16]=dm_input[6][i]-'0';
        NUM[i+24]=dm_input[7][i]-'0';
    }
    MEM_num=int_BINtoDEC(NUM,32,31);

    for(i=0; i<8; i++){
        NUM[i]=im_input[4][i]-'0';
        NUM[i+8]=im_input[5][i]-'0';
        NUM[i+16]=im_input[6][i]-'0';
        NUM[i+24]=im_input[7][i]-'0';
    }
    INSTR_num=int_BINtoDEC(NUM,32,31);
    if(test==1) printf("enter initialize with MEM_num=%d, INSTR_num=%d\n",MEM_num,INSTR_num);

    //store the content of memory into dmemory
    for(i=0; i<MEM_num; i++){
        for(j=0; j<8; j++){
            dmemory[i][j]=dm_input[8+4*i][j];
            dmemory[i][j+8]=dm_input[9+4*i][j];
            dmemory[i][j+16]=dm_input[10+4*i][j];
            dmemory[i][j+24]=dm_input[11+4*i][j];
        }
    }

    //store the content of instructions into imemory
    for(i=0; i<INSTR_num; i++){
        for(j=0; j<8; j++){
            imemory[PC_initptr/4+i][j]=im_input[8+4*i][j];
            imemory[PC_initptr/4+i][j+8]=im_input[9+4*i][j];
            imemory[PC_initptr/4+i][j+16]=im_input[10+4*i][j];
            imemory[PC_initptr/4+i][j+24]=im_input[11+4*i][j];
        }
    }
    if(test==1) printf("finish initialize register and memory\n");
}

void instruction_decode(){
    /**decode each instruction**/
    int i, j;
    int OP, FUNCT, RS, RT, RD, SHAMT, C, signedC;
    if(test==1) printf("enter ID with PC_nowptr=%d\n",PC_nowptr);
    while(1){
    /*int INSTR[34]={0};
    while(1){
        for(j=0; j<32; j++) INSTR[j]=0;
        for(j=0; j<8; j++){
            INSTR[j]=im_input[8+4*index][j]-'0';
            INSTR[j+8]=im_input[9+4*index][j]-'0';
            INSTR[j+16]=im_input[10+4*index][j]-'0';
            INSTR[j+24]=im_input[11+4*index][j]-'0';
        }*/
        index++;
        PC_adder();
        if(test==1) printf("enter ID after PC_adder() PC_nowptr=%d\n",PC_nowptr);

        /**analyze INSTR=imemory[PC_nowptr] to OPcode, RS, RT, RD, etc.**/
        OP = char_BINtoDEC(imemory[PC_nowptr/4-1],6,5);
        if(OP==0){
            FUNCT = char_BINtoDEC(imemory[PC_nowptr/4-1],6,31);
            RS = char_BINtoDEC(imemory[PC_nowptr/4-1],5,10);
            RT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,15);
            RD = char_BINtoDEC(imemory[PC_nowptr/4-1],5,20);
            SHAMT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,25);

            if(test==1) printf("R-type: (FUNCT,RS,RT,RD,SHAMT)=(%d,%d,%d,%d,%d)\n",FUNCT,RS,RT,RD,SHAMT);
            if(FUNCT==32){
                add(RS,RT,RD);
                if(test==1) printf("this is R-type add(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==33){
                addu(RS,RT,RD);
                if(test==1) printf("this is R-type addu(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==34){
                sub(RS,RT,RD);
                if(test==1) printf("this is R-type sub(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==36){
                and(RS,RT,RD);
                if(test==1) printf("this is R-type and(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==37){
                or(RS,RT,RD);
                if(test==1) printf("this is R-type or(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==38){
                xor(RS,RT,RD);
                if(test==1) printf("this is R-type xor(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==39){
                nor(RS,RT,RD);
                if(test==1) printf("this is R-type nor(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==40){
                nand(RS,RT,RD);
                if(test==1) printf("this is R-type nand(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==42){
                slt(RS,RT,RD);
                if(test==1) printf("this is R-type slt(%d,%d,%d)\n",RS,RT,RD);
            }
            else if(FUNCT==0){
                sll(RT,RD,SHAMT);
                if(test==1) printf("this is R-type sll(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==2){
                srl(RT,RD,SHAMT);
                if(test==1) printf("this is R-type srl(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==3){
                sra(RT,RD,SHAMT);
                if(test==1) printf("this is R-type sra(%d,%d,%d)\n",RT,RD,SHAMT);
            }
            else if(FUNCT==8){
                jr(RS);
                if(test==1) printf("this is R-type jr(%d)\n",RS);
            }
            else{
                if(test==1) printf("this is error R-instruction\n");
            }
        }
        else if(OP==2||OP==3||OP==63){
            C = char_BINtoDEC(imemory[PC_nowptr/4-1],26,31);

            if(test==1) printf("J-type: (c)=(%d)\n",C);
            if(OP==2){
                jj(C);
                if(test==1) printf("this J-type jj(%d)\n",C);
            }
            else if(OP==3){
                jal(C);
                if(test==1) printf("this J-type jal(%d)\n",C);
            }
            else if(OP==63) halt=1;
            else{
                if(test==1) printf("this is error J-instruction\n");
            }
        }
        else{
            RS = char_BINtoDEC(imemory[PC_nowptr/4-1],5,10);
            RT = char_BINtoDEC(imemory[PC_nowptr/4-1],5,15);
            C = char_BINtoDEC(imemory[PC_nowptr/4-1],16,31);
            signedC = signed_char_BINtoDEC(imemory[PC_nowptr/4-1],16,31);

            if(test==1) printf("I-type: (RS,RT,C,signedC)=(%d,%d,%d,%d)\n",RS,RT,C,signedC);
            if(OP==8){
                addi(RS,RT,C);
                if(test==1) printf("this I-type addi(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==9){
                addiu(RS,RT,C);
                if(test==1) printf("this I-type addiu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==35){
                lw(RS,RT,signedC);
                if(test==1) printf("this I-type lw(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==33){
                lh(RS,RT,signedC);
                if(test==1) printf("this I-type lh(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==37){
                lhu(RS,RT,C);
                if(test==1) printf("this I-type lhu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==32){
                lb(RS,RT,signedC);
                if(test==1) printf("this I-type lb(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==36){
                lbu(RS,RT,C);
                if(test==1) printf("this I-type lbu(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==43){
                sw(RS,RT,signedC);
                if(test==1) printf("this I-type sw(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==41){
                sh(RS,RT,signedC);
                if(test==1) printf("this I-type sh(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==40){
                sb(RS,RT,signedC);
                if(test==1) printf("this I-type sb(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==15){
                lui(RT,C);
                if(test==1) printf("this I-type lui(%d,%d)\n",RT,C);
            }
            else if(OP==12){
                andi(RS,RT,C);
                if(test==1) printf("this I-type andi(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==13){
                ori(RS,RT,C);
                if(test==1) printf("this I-type ori(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==14){
                nori(RS,RT,C);
                if(test==1) printf("this I-type nori(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==10){
                slti(RS,RT,C);
                if(test==1) printf("this I-type slti(%d,%d,%d)\n",RS,RT,C);
            }
            else if(OP==4){
                beq(RS,RT,signedC);
                if(test==1) printf("this I-type beq(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==5){
                bne(RS,RT,signedC);
                if(test==1) printf("this I-type bne(%d,%d,%d)\n",RS,RT,signedC);
            }
            else if(OP==7){
                bgtz(RS,signedC);
                if(test==1) printf("this I-type bgtz(%d,%d)\n",RS,signedC);
            }
            else{
                if(test==1) printf("this is I-error instruction\n");
            }
        }
        append_SNAP();

        /**do error detection**/
        if(write$0_error){
            fprintf(error, "In cycle %d: Write $0 Error\n", cycle);
            write$0_error=0;
        }
        if(number_overflow){
            fprintf(error, "In cycle %d: Number Overflow\n", cycle);
            number_overflow=0;
        }
        if(memory_overflow){
            fprintf(error, "In cycle %d: Address Overflow\n", cycle);
            break;
        }
        if(misaligned){
            fprintf(error, "In cycle %d: Misalignment Error\n", cycle);
            break;
        }

        if(PC_nowptr>PC_initptr+INSTR_num) break;
        if(halt) break;
    }
}

/**R-type instructions**/
void add(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, carry=0, bitsum;
    int same=(REG[rs][0]==REG[rt][0])?1:0;
    char same_bit;

    if(same) same_bit=REG[rs-8][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+(REG[rt][j]-'0')+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rd][0]!=same_bit){
            number_overflow=1;
            return;
        }
    }
}
void addu(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=0, bitsum;

    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+(REG[rt][j]-'0')+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
}
void sub(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, carry=1, bitsum;
    int complement[32]={0};
    for(j=31; j>=0; j--) complement[j]=flip( REG[rt][j]-'0' );
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

    int same=( (REG[rs][0]-'0')==complement[0] ) ? 1:0;
    char same_bit;
    if(same) same_bit=REG[rs][0];
    carry=0;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+complement[j]+carry;
        if(bitsum<2){
            REG[rd][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rd][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rd][0]!=same_bit){
            number_overflow=1;
            PC_adder();
            return;
        }
    }
}
void and(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, bitsum;
    if(rs==0 || rt==0){
        for(j=0; j<32; j++) REG[rd][j]=0;
        return;
    }
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')&(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void or(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')|(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void xor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')^(REG[rt][j]-'0');
        REG[rd][j]=bitsum+'0';
    }
}
void nor(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=~((REG[rs][j]-'0')|(REG[rt][j]-'0'));
        REG[rd][j]=bitsum+'0';
    }
}
void nand(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }
    int j, bitsum;
    for(j=31; j>=0; j--){
        bitsum=~((REG[rs][j]-'0')&(REG[rt][j]-'0'));
        REG[rd][j]=bitsum+'0';
    }
}
void slt(int rs, int rt, int rd){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int j, result;
    if(REG[rs][0]!=REG[rt][0]){ //different sign bit
        result=(REG[rt][0]=='0')?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd][j]='1';
                else REG[rd][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rd][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs][j]!=REG[rt][j]){
                result=(REG[rt][j]=='1')?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rd][j]='1';
                else REG[rd][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rd][j]='0';
    }
}
void sll(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=shamt; i<32; i++)
        REG[rd][i-shamt]=REG[rt][i];
    for(i=32-shamt; i<32; i++)
        REG[rd][i]='0';

}
void srl(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd][i+shamt]=REG[rt][i];
    for(i=0; i<shamt; i++)
        REG[rd][i]='0';

}
void sra(int rt, int rd, int shamt){
    if(rd==0){
        write$0_error=1;
        return;
    }

    int i;
    for(i=0; i<32-shamt; i++)
        REG[rd][i+shamt]=REG[rt][i];
    for(i=0; i<shamt; i++)
        REG[rd][i]=REG[rt][0];

}
void jr(int rs){
    int j;
    for(j=31; j>=0; j--) PC_now[j]=REG[rs][j];
}

/**J-type instructions**/
void jj(int c){
    PC_nowptr=c;
}
void jal(int c){
    PC_nowptr=c;
}

/**I-type instructions**/
void addi(int rs, int rt, int c){
    //printf("enter addi with rs=%d, rt=%d, c=%d\n",rs,rt,c);
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

    int same=(REG[rs][0]==cbin[0]+'0')?1:0;
    char same_bit;
    if(same) same_bit=REG[rs][0];
    for(j=31; j>=0; j--){
        bitsum=(REG[rs][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt][j]=bitsum-2+'0';
            carry=1;
        }
    }
    if(same){
        if(REG[rt][0]!=same_bit){
            number_overflow=1;
            PC_adder();
            return;
        }
    }
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
        bitsum=(REG[rs][j]-'0')+cbin[j]+carry;
        if(bitsum<2){
            REG[rt][j]=bitsum+'0';
            carry=0;
        }
        else{
            REG[rt][j]=bitsum-2+'0';
            carry=1;
        }
    }
}
void lw(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<32; i++){
        REG[rt][i]=dmemory[result][i];
    }
}
void lh(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        REG[rt][i+16]=dmemory[result][i+16];
    }

    if(dmemory[result][0]=='0'){
        for(i=0; i<16; i++) REG[rt][i]='0';
    }
    else{
        for(i=0; i<16; i++) REG[rt][i]='1';
    }
}
void lhu(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        REG[rt][i+16]=dmemory[result][i+16];
    }

    for(i=0; i<16; i++) REG[rt][i]='0';

}
void lb(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        REG[rt][i+24]=dmemory[result][i+24];
    }

    if(dmemory[result][0]=='0'){
        for(i=0; i<24; i++) REG[rt][i]='0';
    }
    else{
        for(i=0; i<24; i++) REG[rt][i]='1';
    }
}
void lbu(int rs, int rt, int c){
    if(rt==0) write$0_error=1;

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+c;
    if(check%2!=0)
        misaligned=1;

    if(write$0_error==1 || memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        REG[rt][i+24]=dmemory[result][i+24];
    }

    for(i=0; i<24; i++) REG[rt][i]='0';

}
void sw(int rs, int rt, int c){
    //MEM[rs+c]->value = REG[rt-8]->value //改成address=rs+c的MEM的value才對

    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<32; i++){
        dmemory[result][i]=REG[rt][i];
    }
}
void sh(int rs, int rt, int c){
    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<16; i++){
        dmemory[result][i+16]=REG[rt][i+16];
    }

}
void sb(int rs, int rt, int c){
    int result = char_BINtoDEC(REG[rs],32,31) + c;
    if(char_BINtoDEC(REG[rs],32,31)>0 && c>0 && result<0)
        number_overflow=1;
    if(result>1023 || result<0)
        memory_overflow=1;

    int check = (REG[rs][31]-'0')+(REG[rs][30]-'0')*2+c;
    if(check%4!=0)
        misaligned=1;

    if(memory_overflow==1 || misaligned==1) return;

    int i;
    for(i=0; i<8; i++){
        dmemory[result][i+24]=REG[rt][i+24];
    }
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

    for(j=15; j>=0; j--) REG[rt][j]=cbin[j]+'0';
}
void andi(int rs, int rt, int c){
    if(rt==0){
        write$0_error=1;
        return;
    }

    int j;
    if(rs==0 || c==0){
        for(j=0; j<32; j++) REG[rt][j]='0';
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
        bitsum=(REG[rs][j]-'0')&cbin[j];
        REG[rt][j]=bitsum+'0';
    }
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
        bitsum=(REG[rs][j]-'0')|cbin[j];
        REG[rt][j]=bitsum+'0';
    }
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
        bitsum=~((REG[rs][j]-'0')|cbin[j]);
        REG[rt][j]=bitsum+'0';
    }
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
    if(REG[rs][0]!=(cbin[0]+'0')){ //different sign bit
        result=(cbin[0]==0)?1:0;
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt][j]='1';
                else REG[rt][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt][j]='0';
    }
    else{ //same sign bit
        for(j=0; j<32; j++){
            if(REG[rs][j]!=(cbin[j]+'0')){
                result=(cbin[j]==1)?1:0;
                break;
            }
        }
        if(result){
            for(j=31; j>=0; j--){
                if(j==31) REG[rt][j]='1';
                else REG[rt][j]='0';
            }
        }else
            for(j=31; j>=0; j--) REG[rt][j]='0';
    }
}
void beq(int rs, int rt, int c){
    if(c>0 && (PC_nowptr*4 + c*4)<0) number_overflow=1;

    int j, result=1;
    for(j=31; j>=0; j--){
        if(REG[rs][j]!=REG[rt][j]){
            result=0;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c; //check?
    }
}
void bne(int rs, int rt, int c){
    if(c>0 && (PC_nowptr*4 + c*4)<0) number_overflow=1;

    int j, result=0;
    for(j=31; j>=0; j--){
        if(REG[rs][j]!=REG[rt][j]){
            result=1;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c;
    }
}
void bgtz(int rs, int c){
    int j, result=0;
    for(j=0; j<32; j--){
        if(REG[rs][0]=='1'){ //is negative
            result=0;
            break;
        }
        if(REG[rs][j]=='1'){ //is positive and >0
            result=1;
            break;
        }
    }
    if(result){
        PC_nowptr=PC_nowptr+c;
    }
}


/////////////////////////////////////////////

int main(){
    instruction_fetch();
    initialize();

    snap=fopen("snapshot.rpt","w");
    error=fopen("error_dump.rpt","a");
    initial_SNAP();

    instruction_decode();

    fclose(snap);
    fclose(error);
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
    //if(test==1) printf("enter BINtoDEC(arr[0]=%c, %d bits star from %d)\n",arr[0],n_bits,start);
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(arr[i]=='1'){
            sum=sum+base;
        }
        base=base*2;
    }
    return sum;
}
int signed_char_BINtoDEC(char arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(i==start-n_bits+1){
            if(arr[i]=='1') sum=sum-base;
        }
        else{
            if(arr[i]=='1') sum=sum+base;
            base=base*2;
        }
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
int signed_int_BINtoDEC(int arr[], int n_bits, int start){
    int sum=0, base=1, i;
    for(i=start; i>(start-n_bits); i--){
        if(i==start-n_bits+1)
            sum=sum-arr[i]*base;
        else{
            sum=sum+arr[i]*base;
            base=base*2;
        }
    }
    return sum;
}
int flip(int n){
    return (n==0)?1:0;
}
