#ifndef ClassPredictor
#define ClassPredictor

class Predictor{
private:
    unsigned int status[16];
    unsigned int hist = 0;
    unsigned int pN;
    unsigned int psN;
    void higherTaken(int cur){
        if(status[cur] < 3) status[cur]++;
    }
    void lowerTaken(int cur){
        if(status[cur] > 0) status[cur]--;
    }
public:
    Predictor(){
        for(int i = 0; i < 16; i++) status[i] = 0;
    }
    bool ifTake(unsigned int curPc){
        pN++;
        return status[hist] / 2;
    }
    void judge(bool ifJump, bool ifCorrect, unsigned int curPc){
        if(ifJump) higherTaken(hist);
        else lowerTaken(hist);
        hist = ((hist << 1) + ifJump) & 0xF;
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