#ifndef _BaseSettings
#define _BaseSettings

#include <iostream>
#include <cstring>
#include <cstdio>
typedef enum{LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, 
            LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, 
            XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT,
            SLTU, XOR, SRL, SRA, OR, AND, NOP, END} inst_type;
typedef enum{_IF, _ID, _EXE, _MEM, _WB} stage;

union _data{
    int i;
    unsigned int ui;
};

_data registerPc;
_data reg[32];
unsigned char ram[0xFFFFF];

//read & write certain length

unsigned int read1(unsigned int p){
    return (unsigned int)ram[p];
}
unsigned int read2(unsigned int p){
    return (unsigned int)ram[p] | (((unsigned int)ram[p + 1]) << 8);
}
unsigned int read4(unsigned int p){
    return (unsigned int)ram[p] | (((unsigned int)ram[p + 1]) << 8) | (((unsigned int)ram[p + 2]) << 16) | (((unsigned int)ram[p + 3]) << 24);
}
void write1(unsigned int p, unsigned int v){
    ram[p] = (unsigned char)v;
}
void write2(unsigned int p, unsigned int v){
    ram[p] = (unsigned char)v;
    ram[p + 1] = (unsigned char)(v >> 8);
}
void write4(unsigned int p, unsigned int v){
    ram[p] = (unsigned char)v;
    ram[p + 1] = (unsigned char)(v >> 8);
    ram[p + 2] = (unsigned char)(v >> 16);
    ram[p + 3] = (unsigned char)(v >> 24);
}

//input

void fillMemory(){
    unsigned int curAddress = 0;
    char input[13];
    while(scanf("%s", input) != EOF){
        if(input[0] == '@'){
            sscanf(input + 1, "%x", &curAddress);
        }else{
            sscanf(input, "%x", &ram[curAddress++]);
            scanf("%x", &ram[curAddress++]);
            scanf("%x", &ram[curAddress++]);
            scanf("%x", &ram[curAddress++]);
        }
    }
}

//buffer



#endif