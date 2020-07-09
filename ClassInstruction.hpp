#include <iostream>
#include <cstring>
#include <cstdio>
typedef enum{LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, 
            LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, 
            XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT,
            SLTU, XOR, SRL, SRA, OR, AND} inst_type;

union _data{
    int i;
    unsigned int ui;
};
_data registerPc;
_data reg[32];
unsigned char ram[0xFFFFF];

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

class Inst{
public:
    bool ifJump;
    virtual void Execute() = 0;
    virtual void MemoryAccess() = 0;
    virtual void WriteBack() = 0;
};

class InstU: public Inst{
private:
    inst_type instType;
    _data rd;
    _data imm;
    _data resultRd;
public:
    InstU(inst_type iT, unsigned int RD, unsigned int IMM): instType(iT){
        rd.ui = RD;
        imm.ui = IMM;
    }
    void Execute(){
        switch(instType){
            case LUI:{
                break;
            }
            case AUIPC:{
                resultRd.i = registerPc.i + imm.i;
                break;
            }
        }
    }
    void MemoryAccess(){}
    void WriteBack(){
        switch(instType){
            case LUI:{
                if(rd.ui == 0) break;
                reg[rd.ui].ui = imm.ui;
                break;
            }
            case AUIPC:{
                if(rd.ui == 0) break;
                reg[rd.ui].ui = resultRd.ui;
                break;
            }
        }
    } 
};

class InstJ: public Inst{
private:
    inst_type instType;
    _data rd;
    _data imm;
    _data resultRd, resultPc;
public:
    InstJ(inst_type iT, unsigned int RD, unsigned int IMM): instType(iT){
        rd.ui = RD;
        imm.ui = IMM;
    }
    void Execute(){
        switch(instType){
            case JAL:{
                resultRd.ui = registerPc.ui + 4;
                resultPc.i = registerPc.i + imm.i;
                break;
            }
        }
    }
    void MemoryAccess(){}
    void WriteBack(){
        switch(instType){
            case JAL:{
                registerPc.i = resultPc.i;
                if(rd.ui == 0) break;
                reg[rd.ui].ui = resultRd.ui;
                break;
            }
        }
    } 
};

class InstI: public Inst{
private:
    inst_type instType;
    _data rd;
    _data vrs, imm;
    _data resultRd, resultPc;
public:
    InstI(inst_type iT, unsigned int RD, unsigned int VRS, unsigned int IMM): instType(iT){
        rd.ui = RD;
        vrs.ui = VRS;
        imm.ui = IMM;
    }
    
    void Execute(){
        switch(instType){
            case JALR:{
                resultRd.ui = registerPc.ui + 4;
                resultPc.ui = (vrs.ui + imm.ui) &~ 1;
                break;
            }
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:
            case ADDI:{
                resultRd.ui = vrs.ui + imm.ui;
                break;
            }
            case SLTI:{
                resultRd.ui = vrs.i < imm.i;
                break;
            }
            case SLTIU:{
                resultRd.ui = vrs.ui < imm.ui;
                break;
            }
            case XORI:{
                resultRd.i = vrs.i ^ imm.i;
                break;
            }
            case ORI:{
                resultRd.i = vrs.i | imm.i;
                break;
            }
            case ANDI:{
                resultRd.i = vrs.i & imm.i;
                break;
            }
            case SLLI:{
                resultRd.ui = vrs.ui << imm.ui;
                break;
            }
            case SRLI:{
                resultRd.ui = vrs.ui >> imm.ui;
                break;
            }
            case SRAI:{
                resultRd.i = vrs.i >> imm.ui;
                break;
            }
        }
    }
    void MemoryAccess(){
        switch(instType){
            case JALR:{
                break;
            }
            case LB:{
                resultRd.ui = read1(resultRd.ui);
                if((resultRd.ui >> 7) & 1) resultRd.ui |= (0xFFFFFF << 8);
                break;
            }
            case LH:{
                resultRd.ui = read2(resultRd.ui);
                if((resultRd.ui >> 15) & 1) resultRd.ui |= (0xFFFF << 16);
                break;
            }
            case LW:{
                resultRd.ui = read4(resultRd.ui);
                break;
            }
            case LBU:{
                resultRd.ui = read1(resultRd.ui);
                resultRd.ui &= 0xFF;
                break;
            }
            case LHU:{
                resultRd.ui = read2(resultRd.ui);
                resultRd.ui &= 0xFFFF;
                break;
            }
            default: break;
        }
    } 
    void WriteBack(){
        switch(instType){
            case JALR:{
                registerPc.i = resultPc.i;
                if(rd.ui == 0) break;
                reg[rd.ui].ui = resultRd.ui;
                break;
            }
            default:{
                if(rd.ui == 0) break;
                reg[rd.ui].ui = resultRd.ui;
                break;
            }
        }
    } 
};

class InstB: public Inst{
private:
    inst_type instType;
    _data vrs1, vrs2;
    _data imm;
    _data resultPc;
public:
    InstB(inst_type iT, unsigned int VRS1, unsigned int VRS2, unsigned int IMM): instType(iT){
        vrs1.ui = VRS1;
        vrs2.ui = VRS2;
        imm.ui = IMM;
    }
    
    void Execute(){
        switch(instType){
            case BEQ:{
                if(vrs1.ui == vrs2.ui)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
            case BNE:{
                if(vrs1.ui != vrs2.ui)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
            case BLT:{
                if(vrs1.i < vrs2.i)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
            case BGE:{
                if(vrs1.i >= vrs2.i)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
            case BLTU:{
                if(vrs1.ui < vrs2.ui)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
            case BGEU:{
                if(vrs1.ui >= vrs2.ui)
                    resultPc.i = registerPc.i + imm.i;
                else resultPc.i = registerPc.i + 4;
                break;
            }
        }      
    }
    void MemoryAccess(){} 
    void WriteBack(){
        registerPc.i = resultPc.i;
    } 
};

class InstS: public Inst{
private:
    inst_type instType;
    _data vrs1, vrs2;
    _data imm;
    _data pRam, resultRs;
public:
    InstS(inst_type iT, unsigned int VRS1, unsigned int VRS2, unsigned int IMM): instType(iT){
        vrs1.ui = VRS1;
        vrs2.ui = VRS2;
        imm.ui = IMM;
    }
    
    void Execute(){
        switch(instType){
            case SB:
            case SH:
            case SW:{
                pRam.ui = vrs1.i + imm.i;
                break;
            }
        }
    }
    void MemoryAccess(){
        switch(instType){
            case SB:{
                write1(pRam.ui, vrs2.ui);
                break;
            }
            case SH:{
                write2(pRam.ui, vrs2.ui);
                break;
            }
            case SW:{
                write4(pRam.ui, vrs2.ui);
                break;
            }
        }
    }
    void WriteBack(){} 
};

class InstR: public Inst{
private:
    inst_type instType;
    _data rd, vrs1, vrs2;
    _data resultRd;
public:
    InstR(inst_type iT, unsigned int RD, unsigned int VRS1, unsigned int VRS2): instType(iT){
        rd.ui = RD;
        vrs1.ui = VRS1;
        vrs2.ui = VRS2;
    }
    
    void Execute(){
        switch(instType){
            case ADD:{
                resultRd.i = vrs1.i + vrs2.i;
                break;
            }
            case SUB:{
                resultRd.i = vrs1.i - vrs2.i;
                break;
            }
            case SLL:{
                resultRd.ui = vrs1.ui << vrs2.ui;
                break;
            }
            case SLT:{
                resultRd.ui = vrs1.i < vrs2.i;
                break;
            }
            case SLTU:{
                resultRd.ui = vrs1.ui < vrs2.ui;
                break;
            }
            case XOR:{
                resultRd.i = vrs1.i ^ vrs2.i;
                break;
            }
            case SRL:{
                resultRd.ui = vrs1.ui >> vrs2.ui;
                break;
            }
            case SRA:{
                resultRd.i = vrs1.i >> vrs2.i;
                break;
            }
            case OR:{
                resultRd.i = vrs1.i | vrs2.i;
                break;
            }
            case AND:{
                resultRd.i = vrs1.i & vrs2.i;
                break;
            }
        }
    }
    void MemoryAccess(){}
    void WriteBack(){
        if(rd.ui != 0) 
            reg[rd.ui].ui = resultRd.ui;
    }
};