#include "Pipeline.hpp"

int main(){
    registerPc.ui = 0;
    reg[0].ui = 0;
    bool check = true;
    fillMemory();
    while(IF_ID.ifContinue || EXE_MEM.T != END){
        WB();
        if(memCounter < 0){
            MEM();
        }else{
            memCounter ++;
            if(memCounter == 2) memCounter = -1;
        }
        if(memCounter >= 0) continue;
        EXE();
        if(ID()) continue;
        IF();
    }
    printf("%d\n", (reg[10].ui) & 255u);
    //printf("%f\n",pred.Accuracy());
    return 0;
}
