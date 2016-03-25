#ifndef INSTRUCTION_H_INCLUDED
#define INSTRUCTION_H_INCLUDED

void decode();
void initial_SNAP();
void adderPC();
void append_SNAP();



int HEXtoDEC(char c);
char DECtoHEX(int n);
int BINtoDEC(int arr[], int n_bits, int start);

#endif // INSTRUCTION_H_INCLUDED
