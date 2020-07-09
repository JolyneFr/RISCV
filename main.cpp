#include "ClassInstruction.hpp"

unsigned int InstructionFetch(){
    unsigned int curCode = 0;
    for(int i = 0; i < 4; i++){
        unsigned char tmp = ram[registerPc.ui + i];
        curCode += ((unsigned int)tmp) << 8 * i;
    }
    return curCode;
}
unsigned int IF(){
    unsigned int res = InstructionFetch();
    ram[registerPc.ui + i] += 4;
    return res;
}

void InstructionDecode(unsigned int curCode, Inst* &Code){
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

int main(){
    //freopen("a.out", "a", stdout);
    registerPc.ui = 0;
    reg[0].ui = 0;
    fillMemory();
    while(true){
        unsigned int curCode = InstructionFetch();
        if(curCode == 0x0FF00513){
            printf("%d\n", (reg[10].ui) & 255u);
            break;
        }
        Inst* curData = nullptr;
        InstructionDecode(curCode, curData);
        curData->Execute();
        curData->MemoryAccess();
        curData->WriteBack();
        if(curData->ifJump == false) registerPc.ui += 4;
        //else{
            //printf("%d\n", (unsigned int)ram[0x1884]);
        //}
        delete curData;
    }
}