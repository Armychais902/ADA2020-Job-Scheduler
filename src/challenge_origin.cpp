#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <queue>

using namespace std;

class Operation{
    public:
        int JobID, OpID;
        int reqSlice; // the number of required slices
        int reqDuration; // the duration required
        int reqOpNum; // the number of depending operations
        int startTime, endTime;
        int totalDuration;
        double jobWeight;
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

void countTotalDuration(Job *curJob, int curOp, bool *visited, int durationSum){
    int prevOpNum = curJob->ops[curOp].reqOpNum, prevOp;
    durationSum = durationSum + curJob->ops[curOp].reqDuration;
    //curJob->ops[curOp].totalDuration = max(durationSum, curJob->ops[curOp].totalDuration);
    curJob->ops[curOp].totalDuration += durationSum;
    for(int i = 0; i < prevOpNum; i ++){
        prevOp = curJob->ops[curOp].reqOp[i];
        if(!visited[prevOp]){
            countTotalDuration(curJob, prevOp, visited, durationSum);
        }
    }
}

struct durationComp
{
    bool operator()(Operation const& a, Operation const& b)
    {
        if(a.jobWeight == b.jobWeight) return a.totalDuration < b.totalDuration;
        else return a.jobWeight < b.jobWeight;
    }
};

struct endTimeComp
{
    bool operator()(Operation const& a, Operation const& b)
    {
        return a.endTime > b.endTime;
    }
};

int main(){
    int sliceNum, jobNum, reqOpID, totalOpNum = 0, doneOpNum = 0;
    scanf("%d%d", &sliceNum, &jobNum);
    int opOrderPointers[jobNum + 1], curSlice, T = 0, curJobID, curOpID;
    Job Jobs[jobNum + 1];
    for(int i = 1; i <= jobNum; i ++){
        scanf("%d%lf", &Jobs[i].opNum, &Jobs[i].weight);
        totalOpNum += Jobs[i].opNum;
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

        for(int j = Jobs[i].opNum - 1; j >= 0; j --){
            curOpID = Jobs[i].orders[j];
            countTotalDuration(&Jobs[i], curOpID, visited, 0);
        }
    }
    
    priority_queue<Operation, vector<Operation>, durationComp> waitingPQ;
    priority_queue<Operation, vector<Operation>, endTimeComp> workingPQ;
    queue<int> availaSlice;
    Operation curOperation(0, 0, 0.0);
    for(int i = 1; i <= jobNum; i ++){ // step 1
        int j = 0;
        while(j < Jobs[i].opNum && Jobs[i].ops[ Jobs[i].orders[j] ].reqOpNum == 0){
            waitingPQ.push(Jobs[i].ops[ Jobs[i].orders[j] ]);
            j ++;
        }
        opOrderPointers[i] = j;
    }

    for(int i = 1; i <= sliceNum; i ++){
        availaSlice.push(i);
    }

    while(doneOpNum < totalOpNum){
        while(!availaSlice.empty() && !waitingPQ.empty()){ // step 2
            if(waitingPQ.top().reqSlice > availaSlice.size()) break;
            curOperation = waitingPQ.top();
            waitingPQ.pop();
            for(int i = 1; i <= curOperation.reqSlice; i ++){
                curSlice = availaSlice.front();
                availaSlice.pop();
                curOperation.usedSlice.push_back(curSlice);
                Jobs[curOperation.JobID].ops[curOperation.OpID].usedSlice.push_back(curSlice);
                //printf("adding job %d's operation %d with slice %d\n", curOperation.JobID, curOperation.OpID, curSlice);
            }
            curOperation.startTime = T;
            Jobs[curOperation.JobID].ops[curOperation.OpID].startTime = T;
            curOperation.endTime = T + curOperation.reqDuration;
            Jobs[curOperation.JobID].ops[curOperation.OpID].endTime = curOperation.endTime;
            workingPQ.push(curOperation);
        }
        curOperation = workingPQ.top(); // step 3: get the first job in workingPQ done and get available slices
        workingPQ.pop();
        doneOpNum ++;
        curJobID = curOperation.JobID;
        curOpID = curOperation.OpID;
        Jobs[curJobID].done[curOpID] = true;
        T = curOperation.endTime;
        for(int i = 0; i < curOperation.reqSlice; i++){
            availaSlice.push(curOperation.usedSlice[i]);
            //printf("slice %d is available\n", curOperation.usedSlice[i]);
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
            waitingPQ.push(curOperation);
            j ++;
        }
        opOrderPointers[curJobID] = j;
    }

    for(int i = 1; i <= jobNum; i ++){
        for(int j = 1; j <= Jobs[i].opNum; j ++){
            printf("%d", Jobs[i].ops[j].startTime);
            for(int k = 0; k < Jobs[i].ops[j].reqSlice; k ++)
                printf(" %d", Jobs[i].ops[j].usedSlice[k]);
            printf("\n");
        }
    }

    return 0;
}