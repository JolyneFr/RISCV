#ifndef ClassPredictor
#define ClassPredictor

class bimodalPredictor{
private:
    unsigned int status[32];
    unsigned int pN;
    unsigned int psN;
    void higherTaken(int cur){
        if(status[cur] < 3) status[cur]++;
    }
    void lowerTaken(int cur){
        if(status[cur] > 0) status[cur]--;
    }
public:
    bimodalPredictor(){
        for(int i = 0; i < 32; i++) status[i] = 0;
    }
    bool ifTake(unsigned int curPc){
        pN++;
        return status[curPc % 32] / 2;
    }
    void judge(bool ifJump, bool ifCorrect, unsigned int curPc){
        if(ifJump) higherTaken(curPc % 32);
        else lowerTaken(curPc % 32);
        if(ifCorrect) psN++;
    }
    void revise(){
        pN--;
    }
    double Accuracy(){
        if(pN == 0) return 0;
        return ((double)psN) / pN;
    }
};

#endif