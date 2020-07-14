#ifndef ClassPredictor
#define ClassPredictor

class bimodalPredictor{
private:
    unsigned int status;
    unsigned int pN;
    unsigned int psN;
    void higherTaken(){
        if(status < 3) status++;
    }
    void lowerTaken(){
        if(status > 0) status--;
    }
public:
    bimodalPredictor():status(1), pN(0), psN(0){}
    bool ifTake(){
        pN++;
        return status / 2;
    }
    void judge(bool ifJump, bool ifCorrect){
        if(ifJump) higherTaken();
        else lowerTaken();
        if(ifCorrect) psN++;
    }
    double Accuracy(){
        if(pN == 0) return 0;
        return ((double)psN) / pN;
    }
};

#endif