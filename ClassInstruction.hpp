#ifndef _Instruction
#define _Instruction

#include "BaseSettings.hpp"

unsigned int InstructionFetch(){
    unsigned int curCode = 0;
    for(int i = 0; i < 4; i++){
        unsigned char tmp = ram[registerPc.ui + i];
        curCode += ((unsigned int)tmp) << 8 * i;
    }
    return curCode;
}



class Inst{
public:
    bool ifJump = false;
    bool ifBranch = false;
    virtual inst_type& type() = 0;
    virtual void Execute(unsigned int pc) = 0;
    virtual void MemoryAccess() = 0;
    virtual void WriteBack() = 0;
    virtual void straightJump(unsigned int pc) = 0;
    virtual unsigned int getRd() = 0;
    //virtual unsigned int getImm() = 0;
    //virtual unsigned int getresPc() = 0;
    virtual unsigned int getresRd() = 0;
    virtual void changeRs1(unsigned int res) = 0;
    virtual void changeRs2(unsigned int res) = 0;
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){}
    void Execute(unsigned int pc){
        switch(instType){
            case LUI:{
                resultRd.ui = imm.i;
                break;
            }
            case AUIPC:{
                resultRd.i = (int)pc + imm.i;
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
    unsigned int getRd() {return rd.ui;}
    unsigned int getresRd() {return resultRd.ui;}
    void changeRs1(unsigned int res){}
    void changeRs2(unsigned int res){}
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){}
    void Execute(unsigned int pc){
        switch(instType){
            case JAL:{
                resultRd.ui = pc + 4;
                resultPc.i = (int)pc + imm.i;
                registerPc.i = resultPc.i;
                break;
            }
        }
    }
    void MemoryAccess(){}
    void WriteBack(){
        switch(instType){
            case JAL:{
                if(rd.ui == 0) break;
                reg[rd.ui].ui = resultRd.ui;
                break;
            }
        }
    } 
    unsigned int getRd() {return rd.ui;}
    unsigned int getresRd() {return resultRd.ui;}
    void changeRs1(unsigned int res){}
    void changeRs2(unsigned int res){}
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){}
    void Execute(unsigned int pc){
        switch(instType){
            case JALR:{
                resultRd.ui = pc + 4;
                resultPc.ui = (vrs.ui + imm.ui) &~ 1;
                registerPc.i = resultPc.i;
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
    unsigned int getRd() {return rd.ui;}
    unsigned int getresRd() {return resultRd.ui;}
    void changeRs1(unsigned int res){vrs.ui = res;}
    void changeRs2(unsigned int res){}
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){
        resultPc.i = (int)pc + imm.i;
        registerPc.i = resultPc.i;
    }
    void Execute(unsigned int pc){
        ifBranch = false;
        resultPc.i = (int)pc + 4;
        switch(instType){
            case BEQ:{
                if(vrs1.ui == vrs2.ui){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
            case BNE:{
                if(vrs1.ui != vrs2.ui){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
            case BLT:{
                if(vrs1.i < vrs2.i){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
            case BGE:{
                if(vrs1.i >= vrs2.i){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
            case BLTU:{
                if(vrs1.ui < vrs2.ui){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
            case BGEU:{
                if(vrs1.ui >= vrs2.ui){
                    ifBranch = true;
                    resultPc.i = (int)pc + imm.i;
                }
                break;
            }
        }      
    }
    void MemoryAccess(){} 
    void WriteBack(){registerPc.i = resultPc.i;} 
    unsigned int getRd() {return 0;}
    unsigned int getresRd() {return 0;}
    void changeRs1(unsigned int res){vrs1.ui = res;}
    void changeRs2(unsigned int res){vrs2.ui = res;}
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){}
    void Execute(unsigned int pc){
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
    unsigned int getRd() {return 0;}
    unsigned int getresRd() {return 0;}
    void changeRs1(unsigned int res){vrs1.ui = res;}
    void changeRs2(unsigned int res){vrs2.ui = res;}
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
    inst_type& type(){return instType;}
    void straightJump(unsigned int pc){}
    void Execute(unsigned int pc){
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
    unsigned int getRd() {return rd.ui;}
    unsigned int getresRd() {return resultRd.ui;}
    void changeRs1(unsigned int res){vrs1.ui = res;}
    void changeRs2(unsigned int res){vrs2.ui = res;}
};

void InstructionDecode(unsigned int curCode, Inst* &Code, int &Rs1, int &Rs2){
    unsigned int opcode = curCode & 0x7F;
    unsigned int funtc3 = (curCode >> 12) & 0x7;
    unsigned int funtc7 = (curCode >> 25) & 0x7F;
    inst_type curType;
    switch(opcode){
        case 0b0110111:{
            curType = LUI;
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int imm = curCode & (0xFFFFF << 12);
            InstU* tmp = new InstU(curType, rd, imm);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        case 0b0010111:{
            curType = AUIPC;
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int imm = curCode & (0xFFFFF << 12);
            InstU* tmp = new InstU(curType, rd, imm);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        case 0b1101111:{
            curType = JAL;
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int imm = ((curCode >> 21) & 0x3FF) << 1;
            imm |= ((curCode >> 20) & 0x1) << 11;
            imm |= ((curCode >> 12) & 0xFF) << 12;
            if(curCode >> 31) imm |= (0xFFF << 20);
            InstJ* tmp = new InstJ(curType, rd, imm);
            Code = tmp;
            Code->ifJump = true;
            break;
        }
        case 0b1100111:{
            curType = JALR;
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int rs = (curCode >> 15) & 0x1F;
            Rs1 = rs;
            unsigned int vrs = reg[rs].ui;
            unsigned int imm = (curCode >> 20) & 0xFFF;
            if(curCode >> 31) imm |= (0x1FFFFF << 11);
            InstI* tmp = new InstI(curType, rd, vrs, imm);
            Code = tmp;
            Code->ifJump = true;
            break;
        }
        case 0b1100011:{
            switch(funtc3){
                case 0b000:{
                    curType = BEQ;
                    break;
                }
                case 0b001:{
                    curType = BNE;
                    break;
                }
                case 0b100:{
                    curType = BLT;
                    break;
                }
                case 0b101:{
                    curType = BGE;
                    break;
                }
                case 0b110:{
                    curType = BLTU;
                    break;
                }
                case 0b111:{
                    curType = BGEU;
                    break;
                }
            }
            unsigned int rs1 = (curCode >> 15) & 0x1F;
            unsigned int rs2 = (curCode >> 20) & 0X1F;
            Rs1 = rs1;
            Rs2 = rs2;
            unsigned int vrs1 = reg[rs1].ui;
            unsigned int vrs2 = reg[rs2].ui;
            unsigned int imm = ((curCode >> 8) & 0xF) << 1;
            imm |= ((curCode >> 25) & 0x3F) << 5;
            imm |= ((curCode >> 7) & 0x1) << 11;
            if(curCode >> 31) imm |= (0xFFFFF << 12);
            InstB* tmp = new InstB(curType, vrs1, vrs2, imm);
            Code = tmp;
            Code->ifJump = true;
            break;
        }
        case 0b0000011:{
            switch(funtc3){
                case 0b000:{
                    curType = LB;
                    break;
                }
                case 0b001:{
                    curType = LH;
                    break;
                }
                case 0b010:{
                    curType = LW;
                    break;
                }
                case 0b100:{
                    curType = LBU;
                    break;
                }
                case 0b101:{
                    curType = LHU;
                    break;
                }
            }
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int rs = (curCode >> 15) & 0x1F;
            Rs1 = rs;
            unsigned int vrs = reg[rs].ui;
            unsigned int imm = (curCode >> 20) & 0xFFF;
            if(curCode >> 31) imm |= (0x1FFFFF << 11);
            InstI* tmp = new InstI(curType, rd, vrs, imm);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        case 0b0100011:{
            switch(funtc3){
                case 0b000:{
                    curType = SB;
                    break;
                }
                case 0b001:{
                    curType = SH;
                    break;
                }
                case 0b010:{
                    curType = SW;
                    break;
                }
            }
            unsigned int rs1 = (curCode >> 15) & 0x1F;
            unsigned int rs2 = (curCode >> 20) & 0x1F;
            Rs1 = rs1;
            Rs2 = rs2;
            unsigned int vrs1 = reg[rs1].ui;
            unsigned int vrs2 = reg[rs2].ui;
            unsigned int imm = (curCode >> 7) & 0x1F;
            imm |= ((curCode >> 25) & 0x7F) << 5;
            if(curCode >> 31) imm |= (0x1FFFFF << 11);
            InstS* tmp = new InstS(curType, vrs1, vrs2, imm);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        case 0b0010011:{
            switch(funtc3){
                case 0b000:{
                    curType = ADDI;
                    break;
                }
                case 0b010:{
                    curType = SLTI;
                    break;
                }
                case 0b011:{
                    curType = SLTIU;
                    break;
                }
                case 0b100:{
                    curType = XORI;
                    break;
                }
                case 0b110:{
                    curType = ORI;
                    break;
                }
                case 0b111:{
                    curType = ANDI;
                    break;
                }
                case 0b001:{
                    curType = SLLI;
                    break;
                }
                case 0b101:{
                    switch(funtc7){
                        case 0b0000000:{
                            curType = SRLI;
                            break;
                        }
                        case 0b0100000:{
                            curType = SRAI;
                            break;
                        }
                    }
                    break;
                }
            }
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int rs = (curCode >> 15) & 0x1F;
            Rs1 = rs;
            unsigned int vrs = reg[rs].ui;
            unsigned int imm = (curCode >> 20) & 0xFFF;
            if(curCode >> 31) imm |= (0x1FFFFF << 11);
            InstI* tmp = new InstI(curType, rd, vrs, imm);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        case 0b0110011:{
            switch(funtc3){
                case 0b000:{
                    switch(funtc7){
                        case 0b0000000:{
                            curType = ADD;
                            break;
                        }
                        case 0b0100000:{
                            curType = SUB;
                            break;
                        }
                    }
                    break;
                }
                case 0b001:{
                    curType = SLL;
                    break;
                }
                case 0b010:{
                    curType = SLT;
                    break;
                }
                case 0b011:{
                    curType = SLTU;
                    break;
                }
                case 0b100:{
                    curType = XOR;
                    break;
                }
                case 0b101:{
                    switch(funtc7){
                        case 0b0000000:{
                            curType = SRL;
                            break;
                        }
                        case 0b0100000:{
                            curType = SRA;
                            break;
                        }
                    }
                    break;
                }
                case 0b110:{
                    curType = OR;
                    break;
                }
                case 0b111:{
                    curType = AND;
                    break;
                }
            }
            unsigned int rd = (curCode >> 7) & 0x1F;
            unsigned int rs1 = (curCode >> 15) & 0x1F;
            unsigned int rs2 = (curCode >> 20) & 0x1F;
            Rs1 = rs1;
            Rs2 = rs2;
            unsigned int vrs1 = reg[rs1].ui;
            unsigned int vrs2 = reg[rs2].ui;
            InstR* tmp = new InstR(curType, rd, vrs1, vrs2);
            Code = tmp;
            Code->ifJump = false;
            break;
        }
        default:{
            printf("decoding error\n");
        }
    }
}

struct buffer_IF_ID{
    unsigned int code = 0;
    bool ifContinue = 1;
    unsigned int curPc;
};

struct buffer_ID_EXE{
    Inst* signalPacage = nullptr;
    inst_type T;
    unsigned int curPc;
    bool isBranch = 0;
    bool predictedJump;
};

struct buffer_EXE_MEM{
    Inst* signalPacage = nullptr;
    inst_type T;
    bool ifRd = 0;
    unsigned int rd;
    unsigned int resultRd;
};

struct buffer_MEM_WB{
    Inst* signalPacage = nullptr;
    inst_type T;
    bool ifRd = 0;
    unsigned int rd;
    unsigned int resultRd;
};

#endif