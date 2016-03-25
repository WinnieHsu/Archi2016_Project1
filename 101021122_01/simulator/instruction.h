#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

void decode();
void initial_SNAP();
void adderPC();
void append_SNAP();

/**R-type instructions**/
void add(int rs, int rt, int rd);
void addu(int rs, int rt, int rd);
void sub(RS,RT,RD);
void and(RS,RT,RD);
void or(RS,RT,RD);
void xor(RS,RT,RD);
void nor(RS,RT,RD);
void nand(RS,RT,RD);
void slt(RS,RT,RD);
void sll(RT,RD,SHAMT);
void srl(RT,RD,SHAMT);
void sra(RT,RD,SHAMT);
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

int HEXtoDEC_bit(char c);
char DECtoHEX_bit(int n);
int BINtoDEC(int arr[], int n_bits, int start);

#endif // INSTRUCTION_H_INCLUDED
