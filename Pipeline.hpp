#ifndef _Pipeline
#define _Pipeline

#include "ClassInstruction.hpp"
#include "ClassPredictor.hpp"


buffer_IF_ID IF_ID;
buffer_ID_EXE ID_EXE;
buffer_EXE_MEM EXE_MEM;
buffer_MEM_WB MEM_WB;

bimodalPredictor pred;
int memCounter = -1;
void WB(){
    if(memCounter < 0){
        Inst* cur = MEM_WB.signalPacage;
        if(cur){
            switch(MEM_WB.T){
                case BEQ:
                case BNE:
                case BLT:
                case BGE:
                case BLTU:
                case BGEU:{
                    break;
                }
                default:{
                    cur->WriteBack();
                    break;
                }
            }
            delete cur;
            MEM_WB.signalPacage = nullptr;
            MEM_WB.T = NOP;
        }
    }
}

void MEM(){
    if(memCounter >= 0) return;
    Inst* cur = EXE_MEM.signalPacage;
    if(cur && cur->type() != END){
        cur->MemoryAccess();
        switch(EXE_MEM.T){
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:{
                MEM_WB.ifRd = false;
                MEM_WB.rd = 0;
                MEM_WB.resultRd = 0;
                break;
            }
            case SB:
            case SH:
            case SW:{
                memCounter = 0;
                MEM_WB.ifRd = false;
                MEM_WB.rd = 0;
                MEM_WB.resultRd = 0;
                break;
            }
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:{
                memCounter = 0;
            }
            default:{
                MEM_WB.ifRd = true;
                MEM_WB.resultRd = cur->getresRd();
                MEM_WB.rd = cur->getRd();
                break;
            }
        }
    }
    MEM_WB.signalPacage = cur;
    MEM_WB.T = EXE_MEM.T;
}

void EXE(){
    Inst* cur = ID_EXE.signalPacage;
    if(cur && cur->type() != END){
        if(!cur->ifJump){
            cur->Execute(ID_EXE.curPc);
        }else{
            if(ID_EXE.isBranch){
                bool isCorrect;
                cur->Execute(ID_EXE.curPc);
                if(cur->ifBranch != ID_EXE.predictedJump){
                    isCorrect = false;
                    cur->WriteBack();
                    IF_ID.code = 0;
                    IF_ID.curPc = 0;
                }else isCorrect = true;
                pred.judge(cur->ifBranch, isCorrect);
            }
        }
        switch(ID_EXE.T){
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:
            case SB:
            case SH:
            case SW:{
                EXE_MEM.ifRd = false;
                EXE_MEM.rd = 0;
                EXE_MEM.resultRd = 0;
                break;
            }
            case LB:
            case LH:
            case LW:
            case LBU:
            case LHU:{
                EXE_MEM.ifRd = true;
                EXE_MEM.resultRd = 0;
                EXE_MEM.rd = cur->getRd();
                break;
            }
            default:{
                EXE_MEM.ifRd = true;
                EXE_MEM.resultRd = cur->getresRd();
                EXE_MEM.rd = cur->getRd();
            }
        }
    }
    EXE_MEM.signalPacage = cur;
    EXE_MEM.T = ID_EXE.T;
}

bool ID(){
    bool ifstall = false;
    Inst* cur = nullptr;
    unsigned int curCode = 0;
    if(IF_ID.code){
        curCode = IF_ID.code;
        int rs1 = -1;
        int rs2 = -1;
        InstructionDecode(curCode, cur, rs1, rs2);
        if(curCode == 0x0FF00513){
            IF_ID.ifContinue = false;
            cur->type() = END;
        }
        if(MEM_WB.signalPacage && MEM_WB.ifRd && MEM_WB.rd == rs1 && rs1 != 0){
            cur->changeRs1(MEM_WB.resultRd);
        }
        if(MEM_WB.signalPacage && MEM_WB.ifRd && MEM_WB.rd == rs2 && rs2 != 0){
            cur->changeRs2(MEM_WB.resultRd);
        }
        if(EXE_MEM.signalPacage && EXE_MEM.ifRd && EXE_MEM.rd == rs1 && rs1 != 0){
            switch(EXE_MEM.signalPacage->type()){
                case LB:
                case LH:
                case LW:
                case LBU:
                case LHU:{
                    ifstall = true;
                    break;
                }
                default:{
                    cur->changeRs1(EXE_MEM.resultRd);
                    break;
                }
            }
        }
        if(EXE_MEM.signalPacage && EXE_MEM.ifRd && EXE_MEM.rd == rs2 && rs2 != 0){
            switch(EXE_MEM.signalPacage->type()){
                case LB:
                case LH:
                case LW:
                case LBU:
                case LHU:{
                    ifstall = true;
                    break;
                }
                default:{
                    cur->changeRs2(EXE_MEM.resultRd);
                    break;
                }
            }
        }
        ID_EXE.isBranch = false;
        switch(cur->type()){
            case JAL:
            case JALR:{
                cur->Execute(IF_ID.curPc);
                break;
            }
            case BEQ:
            case BNE:
            case BLT:
            case BGE:
            case BLTU:
            case BGEU:{
                ID_EXE.isBranch = true;
                bool J = pred.ifTake();
                ID_EXE.predictedJump = J;
                if(J){
                    cur->straightJump(IF_ID.curPc);
                }
                break;
            }
        }
    }
    if(!ifstall) {
        ID_EXE.signalPacage = cur;
        if(cur){
            ID_EXE.T = cur->type();
            //printf("%d %x %d\n", cur->type(), curCode, reg[18].ui);
        }else ID_EXE.T = NOP;
        ID_EXE.curPc = IF_ID.curPc;
    }else{
        ID_EXE.signalPacage = nullptr;
        ID_EXE.T = NOP;
        ID_EXE.curPc = IF_ID.curPc;
    }
    return ifstall;
}

void IF(){
    if(IF_ID.ifContinue){
        unsigned int curCode = InstructionFetch();
        IF_ID.code = curCode;
        IF_ID.curPc = registerPc.ui;
        registerPc.ui += 4;
    }else{
        IF_ID.code = 0;
        IF_ID.curPc = 0;
    }
}

#endif