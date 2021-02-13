#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <queue>
#include <stack>
#include <list>

using namespace std;

class Operation{
    public:
        int JobID, OpID;
        int reqSlice; // the number of required slices
        int reqDuration; // the duration required
        int reqOpNum; // the number of depending operations
        int startTime, endTime;
        int reqEndTime;
        int totalDuration;
        double jobWeight;
        int totalDep;
        vector<int> reqOp;
        vector<int> usedSlice;
        Operation(int jobID, int opID, double jobW){
            JobID = jobID;
            OpID = opID;
            jobWeight = jobW;
            totalDuration = 0;
        }
};

Operation fake(0, 0, 0.0);

class Job{
    public:
        int opNum; // the number of operations
        double weight;
        vector<Operation> ops;
        vector<int> orders;
        bool *done;
        Job(){
            ops.push_back(fake); // to let operation counting starts from 1
        }
        void opDone(){
            done = (bool *)malloc((opNum + 1) * sizeof(bool));
            memset(done, false, sizeof(done));
        }
};

class Ratio{
    public:
        int ID;
        double ratio;
        Ratio(){
            ID=0;
            ratio=0.0;
        }
};

typedef struct {
    int start, end, slice;
    //int id;
} emptySlot;

void DFS(Job *curJob, int curOp, bool *visited){
    visited[curOp] = true;
    int prevOpNum = curJob->ops[curOp].reqOpNum, prevOp;
    for(int i = 0; i < prevOpNum; i ++){
        prevOp = curJob->ops[curOp].reqOp[i];
        if(!visited[prevOp]){
            DFS(curJob, prevOp, visited);
        }
    }
    curJob->orders.push_back(curOp);
}

/*void countTotalDuration(Job *curJob, int curOp){
	int prevOpNum = curJob->ops[curOp].reqOpNum, prevOp;
	curJob->ops[curOp].totalDuration += curJob->ops[curOp].reqDuration;
	for(int i = 0; i < prevOpNum; i ++){
		prevOp = curJob->ops[curOp].reqOp[i];
		curJob->ops[prevOp].totalDuration += curJob->ops[curOp].totalDuration;
	}
}*/

struct durationComp
{
    bool operator()(Operation const& a, Operation const& b)
    {
        if (a.reqSlice == b.reqSlice)
            return a.reqDuration < b.reqDuration;
        return a.reqSlice < b.reqSlice;
    }
};

struct endTimeComp
{
    bool operator()(Operation const& a, Operation const& b)
    {
        return a.endTime > b.endTime;
    }
};

bool compRatio(Ratio a,Ratio b){
    if(a.ratio<b.ratio)
        return true;
    else
        return false;
}

bool compSlot(emptySlot a,emptySlot b){
    if(a.start<b.start)
        return true;
    else
        return false;
}

bool insertBubble(list<emptySlot> &emptySlots, int *lastEmptyEnd, Job *Jobs, Operation curOperation, int *opOrderPointers, priority_queue<Operation, vector<Operation>, durationComp> &waitingPQ){
    fprintf(stderr, "Jod %d op %d bubble\n", curOperation.JobID, curOperation.OpID);
    if(emptySlots.size() <= 0) return false;

    list<emptySlot>::iterator it1 = emptySlots.begin(), it2, it3;
    int lateStart, earlyEnd;
    vector<list<emptySlot>::iterator> slots;

    for(; it1 != emptySlots.end(); it1 ++){ //push proper slots into "slots"
        if(it1->end - it1->start < curOperation.reqDuration || it1->start >= curOperation.startTime || it1->end < curOperation.reqEndTime + curOperation.reqDuration){
            continue;
        }
        slots = vector<list<emptySlot>::iterator>(1, it1);
        if (it1->start < curOperation.reqEndTime){
            lateStart = curOperation.reqEndTime;
        }
        else
            lateStart = it1->start; 
        earlyEnd = it1->end;
        it2 = it1; it2 ++;
        for(; it2 != emptySlots.end() && slots.size() < curOperation.reqSlice; it2 ++){
            if(it2->end - it2->start < curOperation.reqDuration) continue;
            else if(it2->end - lateStart < curOperation.reqDuration) continue;
            else if(earlyEnd - it2->start < curOperation.reqDuration) continue;
            else if(it2->end < curOperation.reqEndTime + curOperation.reqDuration) continue;

            if(it2->start > lateStart) lateStart = it2->start;
            if(it2->end < earlyEnd) earlyEnd = it2->end;
            slots.push_back(it2);
            fprintf(stderr, "pushed %d %d %d\n", it2->start, it2->end, it2->slice);
        }
        if(slots.size() == curOperation.reqSlice && lateStart >= curOperation.reqEndTime) break;
    }

    if(slots.size() < curOperation.reqSlice) return false;
    //if(lateStart < curOperation.reqEndTime) return false;
    
    for(int i = 0; i < slots.size(); i++){
        fprintf(stderr, "slot %d %d %d\n", slots[i]->start, slots[i]->end, slots[i]->slice);
    }

    //Because we move the operation, we can add the original position of the operation to "emptySlots"
    // !!!!! error? overlap
    int curJobID = curOperation.JobID, curOpID = curOperation.OpID;
    emptySlot newSlot;
    if (lateStart + curOperation.reqDuration <= Jobs[curJobID].ops[curOpID].startTime){
        for (int i = 0;i < Jobs[curJobID].ops[curOpID].reqSlice;i ++){
            newSlot.slice = Jobs[curJobID].ops[curOpID].usedSlice[i];
            newSlot.start = Jobs[curJobID].ops[curOpID].startTime;
            newSlot.end = Jobs[curJobID].ops[curOpID].endTime;
            it3 = emptySlots.begin();       
            while (it3->start < newSlot.start && it3 != emptySlots.end()){
                it3 ++;
            }
            emptySlots.insert(it3, newSlot);
            fprintf(stderr, "empty slots insert %d %d %d after %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice, it3->start, it3->end, it3->slice);
        }
    }
    else{
        for (int i = 0;i < Jobs[curJobID].ops[curOpID].reqSlice;i ++){
            newSlot.slice = Jobs[curJobID].ops[curOpID].usedSlice[i];
            newSlot.start = lateStart + curOperation.reqDuration;
            newSlot.end = Jobs[curJobID].ops[curOpID].endTime;
            it3 = emptySlots.begin();       
            while (it3->start < newSlot.start && it3 != emptySlots.end()){
                it3 ++;
            }
            emptySlots.insert(it3, newSlot);
            fprintf(stderr, "empty slots insert %d %d %d after %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice, it3->start, it3->end, it3->slice);
        }
    }
    
    //update the used slot: cut/divide
    Jobs[curJobID].ops[curOpID].usedSlice = vector<int>();
    while(!slots.empty()){
        it1 = slots.back();
        slots.pop_back();
        Jobs[curJobID].ops[curOpID].usedSlice.push_back(it1->slice);
        //printf("%d %d insert at %d %d slice %d\n", curJobID, curOpID, it1->start, it1->end, it1->slice);
        if(lateStart == it1->start && lateStart + curOperation.reqDuration == it1->end){
            fprintf(stderr, "=1=\n");
            //emptySlots.erase(it1);
            //it1->start += curOperation.reqDuration;
            it1->end = lateStart;
            it3 = it1;
            it3 ++;
            if (it3->start < it1->start){
                newSlot.start = lateStart + curOperation.reqDuration; 
                newSlot.end = it1->end;
                newSlot.slice = it1->slice;
                it1->end = it1->start;
                //emptySlots.erase(it1);           
                while (it3->start < it1->start && it3 != emptySlots.end()){
                    it3 ++;
                }
                emptySlots.insert(it3, newSlot);
                fprintf(stderr, "empty slots insert %d %d %d after %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice, it3->start, it3->end, it3->slice);
            }
        }
        else if(lateStart == it1->start){
            fprintf(stderr, "=2=\n");
            it1->start += curOperation.reqDuration;
            it3 = it1;
            it3 ++;
            if (it3->start < it1->start){
                newSlot.start = lateStart + curOperation.reqDuration; 
                newSlot.end = it1->end;
                newSlot.slice = it1->slice;
                it1->end = it1->start;
                //emptySlots.erase(it1);           
                while (it3->start < it1->start && it3 != emptySlots.end()){
                    it3 ++;
                }
                emptySlots.insert(it3, newSlot);
                fprintf(stderr, "empty slots insert %d %d %d after %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice, it3->start, it3->end, it3->slice);
            }
        }
        else if(lateStart + curOperation.reqDuration == it1->end){
            fprintf(stderr, "=3=\n");
            it1->end = lateStart;
        }
        else{
            fprintf(stderr, "=4=\n");
            newSlot.start = lateStart + curOperation.reqDuration; 
            newSlot.end = it1->end;
            newSlot.slice = it1->slice;
            it1->end = lateStart;
            it3 = it1;
            it3 ++;
            if (it3->start < it1->start){
                newSlot.start = lateStart + curOperation.reqDuration; 
                newSlot.end = it1->end;
                newSlot.slice = it1->slice;
                it1->end = it1->start;
                //emptySlots.erase(it1);           
                while (it3->start < it1->start && it3 != emptySlots.end()){
                    it3 ++;
                }
                emptySlots.insert(it3, newSlot);
                fprintf(stderr, "empty slots insert %d %d %d after %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice, it3->start, it3->end, it3->slice);
            }
        }
    }
    //update the property of current operation
    Jobs[curJobID].ops[curOpID].startTime = lateStart;
    Jobs[curJobID].ops[curOpID].endTime = lateStart + curOperation.reqDuration;
    Jobs[curJobID].done[curOpID] = true;

    //update reqEndTime for all the operations in current Job
    for (int j = 1;j <= Jobs[curJobID].opNum;j ++){
        if (j == curOpID){
            continue;
        }
        int maxReqEndTime = 0;
        for (int k = 0;k < Jobs[curJobID].ops[j].reqOpNum;k ++){
            int reqOpID = Jobs[curJobID].ops[j].reqOp[k];
            if (Jobs[curJobID].ops[reqOpID].endTime > maxReqEndTime){
                maxReqEndTime = Jobs[curJobID].ops[reqOpID].endTime;
            }
        }
        Jobs[curJobID].ops[j].reqEndTime = maxReqEndTime;
        fprintf(stderr, "Job %d op %d reqEndTime = %d\n", curJobID, j, Jobs[curJobID].ops[j].reqEndTime);
    }
    /*int j = opOrderPointers[curJobID];
    while(j < Jobs[curJobID].opNum){
        int nextOp = Jobs[curJobID].orders[j];
        bool available = true;
        for(int k = 0; k < Jobs[curJobID].ops[nextOp].reqOpNum; k ++){
            if(!Jobs[curJobID].done[ Jobs[curJobID].ops[nextOp].reqOp[k] ]){
                available = false;
                break;
            }
        }
        if(!available) break;
        curOperation = Jobs[curJobID].ops[nextOp];
        curOperation.reqEndTime = Jobs[curJobID].ops[curOpID].endTime;
        //waitingPQ.push(curOperation);
        j ++;
    }
    opOrderPointers[curJobID] = j;*/
    /*int emtySlotsSize = emptySlots.size();
    sort(&emptySlots[0],&emptySlots[emtySlotsSize - 1],compSlot);*/
    return true;
}

double calcResult(Job *Jobs, int jobNum){
    int makespan = -1, jobEndTime;
    double result = 0;
    for(int i = 1; i <= jobNum; i ++){
        jobEndTime = -1;
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            if(Jobs[i].ops[j].endTime > jobEndTime){
                jobEndTime = Jobs[i].ops[j].endTime;
            }
        }
        if(jobEndTime > makespan){
            makespan = jobEndTime;
        }
        result += jobEndTime * Jobs[i].weight;
    }
    result += makespan;
    return result;
}

int main(){
    int sliceNum, jobNum, reqOpID;
    scanf("%d%d", &sliceNum, &jobNum);
    int doneOpNum[jobNum + 1];
    list<emptySlot> emptySlots; int lastEmptyEnd[sliceNum + 1]; emptySlot newSlot; // for insertBubble
    for (int i = 0;i <= jobNum;i ++){
        doneOpNum[i] = 0;
    }
    int opOrderPointers[jobNum + 1], curSlice, T = 0, curJobID, curOpID;
    Job Jobs[jobNum + 1];
    for(int i = 1; i <= jobNum; i ++){
        scanf("%d%lf", &Jobs[i].opNum, &Jobs[i].weight);
        //totalOpNum += Jobs[i].opNum;
        bool visited[Jobs[i].opNum + 1];
        Jobs[i].opDone();
        memset(visited, false, sizeof(visited));
        visited[0] = true;
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            Operation curOp(i,j, Jobs[i].weight);
            scanf("%d%d%d", &curOp.reqSlice, &curOp.reqDuration, &curOp.reqOpNum);
            if(curOp.reqOpNum == 0){
                Jobs[i].orders.push_back(j);
                visited[j] = true;
            }
            for(int k = 1; k <= curOp.reqOpNum; k ++){
                scanf("%d", &reqOpID);
                curOp.reqOp.push_back(reqOpID);
            }
            Jobs[i].ops.push_back(curOp);
        }
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            if(!visited[j]){
                DFS(&Jobs[i], j, visited);
            }
        }

        /*for(int j = Jobs[i].opNum - 1; j >= 0; j --){
            curOpID = Jobs[i].orders[j];
            countTotalDuration(&Jobs[i], curOpID);
        }*/
    }
    
    priority_queue<Operation, vector<Operation>, durationComp> waitingPQ[jobNum + 1];
    priority_queue<Operation, vector<Operation>, endTimeComp> workingPQ[jobNum + 1];
    queue<int> availaSlice;
    Operation curOperation(0, 0, 0.0);
    for(int i = 1; i <= jobNum; i ++){ // step 1
        int j = 0;
        while(j < Jobs[i].opNum && Jobs[i].ops[ Jobs[i].orders[j] ].reqOpNum == 0){
            waitingPQ[i].push(Jobs[i].ops[ Jobs[i].orders[j] ]);
            j ++;
        }
        opOrderPointers[i] = j;
    }

    
    int JobfinTime[jobNum+1];
    for (int l = 1;l <= jobNum;l ++){
        JobfinTime[l]=0;
        T = 0;
        availaSlice = queue<int>();
        for(int i = 1; i <= sliceNum; i ++){
            availaSlice.push(i);
        }
        while(doneOpNum[l] < Jobs[l].opNum){
            while(!availaSlice.empty() && !waitingPQ[l].empty()){ // step 2
                if(waitingPQ[l].top().reqSlice > availaSlice.size()) break;
                curOperation = waitingPQ[l].top();
                waitingPQ[l].pop();
                for(int i = 1; i <= curOperation.reqSlice; i ++){
                    curSlice = availaSlice.front();
                    availaSlice.pop();
                    curOperation.usedSlice.push_back(curSlice);
                    Jobs[curOperation.JobID].ops[curOperation.OpID].usedSlice.push_back(curSlice);
                    fprintf(stderr, "adding job %d's operation %d with slice %d\n", curOperation.JobID, curOperation.OpID, curSlice);
                }
                curOperation.startTime = T;
                Jobs[curOperation.JobID].ops[curOperation.OpID].startTime = T;
                curOperation.endTime = T + curOperation.reqDuration;
                Jobs[curOperation.JobID].ops[curOperation.OpID].endTime = curOperation.endTime;
                workingPQ[l].push(curOperation);
            }
            curOperation = workingPQ[l].top(); // step 3: get the first job in workingPQ done and get available slices
            workingPQ[l].pop();
            doneOpNum[l] ++;
            curJobID = curOperation.JobID;
            curOpID = curOperation.OpID;
            Jobs[curJobID].done[curOpID] = true;
            T = curOperation.endTime;
            for(int i = 0; i < curOperation.reqSlice; i++){
                availaSlice.push(curOperation.usedSlice[i]);
                fprintf(stderr, "slice %d is available\n", curOperation.usedSlice[i]);
            }
            int j = opOrderPointers[curJobID];
            while(j < Jobs[curJobID].opNum){
                int nextOp = Jobs[curJobID].orders[j];
                bool available = true;
                for(int k = 0; k < Jobs[curJobID].ops[nextOp].reqOpNum; k ++){
                    if(!Jobs[curJobID].done[ Jobs[curJobID].ops[nextOp].reqOp[k] ]){
                        available = false;
                        break;
                    }
                }
                if(!available) break;
                curOperation = Jobs[curJobID].ops[nextOp];
                waitingPQ[l].push(curOperation);
                j ++;
            }
            opOrderPointers[curJobID] = j;
        }
        JobfinTime[l]=T;
        //cout<<"jobID "<<l<<" time "<<JobfinTime[l]<<endl;
        //assign job finish time?
    }

    Ratio orderJob[jobNum+1];
    for(int i=1;i<=jobNum;i++){
        orderJob[i].ID=i;
        orderJob[i].ratio=(double)JobfinTime[i]/Jobs[i].weight;
    }
    sort(&orderJob[1],&orderJob[1]+jobNum,compRatio);
    /*for(int i=1;i<=jobNum;i++){
        fprintf(stderr, "ID %d ratio %f\n", orderJob[i].ID, orderJob[i].ratio);
    }*/

    //update startTime & endTime
    for (int i = 1;i <= sliceNum;i ++){
        lastEmptyEnd[i] = 0;
    }
    T = 0;
    for (int i = 1;i <= jobNum;i ++){
        //fprintf(stderr, "jobID = %d, opNum = %d\n", orderJob[i].ID, Jobs[orderJob[i].ID].opNum);
        int curJobID = orderJob[i].ID;
        for (int j = 1;j <= Jobs[curJobID].opNum;j ++){
            Jobs[curJobID].ops[j].startTime += T;
            Jobs[curJobID].ops[j].endTime += T;
        }
        T += JobfinTime[curJobID];
    }

    //set reqEndtime
    for (int i = 1;i <= jobNum;i ++){
        //fprintf(stderr, "jobID = %d, opNum = %d\n", orderJob[i].ID, Jobs[orderJob[i].ID].opNum);
        int curJobID = orderJob[i].ID;
        for (int j = 1;j <= Jobs[curJobID].opNum;j ++){
            int maxReqEndTime = 0;
            for (int k = 0;k < Jobs[curJobID].ops[j].reqOpNum;k ++){
                reqOpID = Jobs[curJobID].ops[j].reqOp[k];
                if (Jobs[curJobID].ops[reqOpID].endTime > maxReqEndTime){
                    maxReqEndTime = Jobs[curJobID].ops[reqOpID].endTime;
                }
            }
            Jobs[curJobID].ops[j].reqEndTime = maxReqEndTime;
            //fprintf(stderr, "Job %d op %d reqEndTime = %d\n", curJobID, j, Jobs[curJobID].ops[j].reqEndTime);
        }
    }
    
    /*for(int i = 1; i <= jobNum; i ++){
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            printf("%d", Jobs[i].ops[j].startTime);
            for(int k = 0; k < Jobs[i].ops[j].reqSlice; k ++)
                printf(" %d", Jobs[i].ops[j].usedSlice[k]);
            printf("\n");
        }
    }
    printf("==before==\n");*/

    //push empty slots
    for (int i = 1;i <= jobNum;i ++){
        int curJobID = orderJob[i].ID;
        for (int j = 1;j <= Jobs[curJobID].opNum;j ++){
            for (int k = 0;k < Jobs[curJobID].ops[j].reqSlice;k ++){
                curSlice = Jobs[curJobID].ops[j].usedSlice[k];
                if(Jobs[curJobID].ops[j].startTime > lastEmptyEnd[curSlice]){
                    newSlot.start = lastEmptyEnd[curSlice]; newSlot.end = Jobs[curJobID].ops[j].startTime; newSlot.slice = curSlice; //btmp.id = ++bubbleID;
                    emptySlots.push_back(newSlot);
                    fprintf(stderr, "empty slots pushed %d %d %d\n", newSlot.start, newSlot.end, newSlot.slice);
                }
                lastEmptyEnd[curSlice] = Jobs[curJobID].ops[j].endTime;
            }
        }
        T += JobfinTime[curJobID];
    }
    
    //swap
    priority_queue<Operation, vector<Operation>, durationComp> newWaitingPQ;
    for (int i = 1;i <= jobNum;i ++){
        int curJobID = orderJob[i].ID;          
        for (int j = 1;j <= Jobs[curJobID].opNum;j ++){
            /*list<emptySlot>::iterator it1 = emptySlots.begin(); 
            fprintf(stderr, "emtySlots: \n");
            for (;it1 != emptySlots.end();it1 ++){
                fprintf(stderr, "%d %d %d\n", it1->start, it1->end, it1->slice);
            }*/
            curOperation = Jobs[curJobID].ops[j];
            insertBubble(emptySlots, lastEmptyEnd, Jobs, curOperation, opOrderPointers, newWaitingPQ);
        }
    }

    //print answer
    for(int i = 1; i <= jobNum; i ++){
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            printf("%d", Jobs[i].ops[j].startTime);
            for(int k = 0; k < Jobs[i].ops[j].reqSlice; k ++)
                printf(" %d", Jobs[i].ops[j].usedSlice[k]);
            printf("\n");
        }
    }

    fprintf(stderr, "%.8f\n", calcResult(Jobs, jobNum));
    return 0;
}