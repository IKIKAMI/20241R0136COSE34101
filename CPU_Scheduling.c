#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSES 10
#define TIME_QUANTUM 4

#define ALG_FCFS 0
#define ALG_NP_SJF 1
#define ALG_P_SJF 2
#define ALG_NP_PRI 3
#define ALG_P_PRI 4
#define ALG_RR 5

typedef struct {
    int PID;
    int arrivalTime;
    int cpuBurstTime;
    int ioBurstTime;
    int priority;
    int waitingTime;
    int turnaroundTime;
    int cpuRemainingTime;
	int ioRemainingTime;
} Process;

Process processes[MAX_PROCESSES];
Process readyQueue[MAX_PROCESSES];
int processNum = 0;

void create_Process();
void init_readyQueue();

void FCFS();
void NonPreemptive_SJF();
void Preemptive_SJF();
void NonPreemptive_Priority();
void Preemptive_Priority();
void RoundRobin();

void printGantt(int processes[], int n);

typedef struct {
    float AverageWT;
    float AverageTT;
    int run;
} eval;

eval evalTable[6]; //각 알고리즘의 평균 WT과 TT를 저장 

void evaluation();
void compare();

/*************************Functions********************************/

void create_Process() {
    srand(time(NULL));
	processNum = rand() % MAX_PROCESSES + 1; // 여러 결과 보기 위해 Process 개수 랜덤 
    //processNum = MAX_PROCESSES; // 프로세스 개수 지정 
    for (int i = 0; i < processNum; i++) {
        processes[i].PID = i + 1;
        processes[i].arrivalTime = rand() % 20; // Arrival Time 0 ~ 20
        processes[i].cpuBurstTime = rand() % 20 + 1; // Cpu Burst Time 1 ~ 20
        processes[i].ioBurstTime = rand() % 10 + 1; // I/O Burst Time 1 ~ 10
        processes[i].priority = rand() % (processNum - 1) + 1; // Priority 1 ~ MAX_PROCESSES
        processes[i].waitingTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].cpuRemainingTime = processes[i].cpuBurstTime;
        processes[i].ioRemainingTime = processes[i].ioBurstTime;
	}
	
	printf("\n================================ Process ==============================\n");
    printf("PID\t Arrival Time\t CPU burst\t IO burst\t Priority\n");
	printf("=======================================================================\n");
	for(int i = 0; i < processNum; i++) {
    	printf("%3d\t %12d\t %9d\t %5d\t %8d\n", 
			processes[i].PID, processes[i].arrivalTime, processes[i].cpuBurstTime, processes[i].ioBurstTime, processes[i].priority);   
    }
}

void init_readyQueue() { // 잡큐에 이미 도착한 프로세스를 레디큐에 넣는다고 가정. 먼저 도착한 순으로 정렬
    int i;
    for (i = 0; i < processNum; i++)
        readyQueue[i] = processes[i]; // 모든 알고리즘 수행을 위해 copy
        
    Process temp;
	int j, k;
	for(j = processNum - 1; j > 0; j--){
		for(k = 0 ; k < j; k++){
			if(readyQueue[k].arrivalTime > readyQueue[k + 1].arrivalTime){ //복사해 온 프로세스들을 오름차순 정렬 
				temp = readyQueue[k + 1];
				readyQueue[k + 1] = readyQueue[k];
				readyQueue[k] = temp;
			
			}
			else if(readyQueue[k].arrivalTime == readyQueue[k + 1].arrivalTime && readyQueue[k].PID > readyQueue[k + 1].PID){ //arrival time 같으면 pid 오름차순 
				temp = readyQueue[k + 1];
				readyQueue[k + 1] = readyQueue[k];
				readyQueue[k] = temp;
			}
		}
	}

/*****정렬 확인용 출력 
	printf("\n============================ Sorted Ready Queue ==========================\n");
	printf("PID\t Arrival Time\t CPU burst\t IO burst\t Priority\n");
	printf("=========================================================================\n");
	for(int i = 0; i < processNum; i++) {
    printf("%3d\t %12d\t %9d\t %5d\t %8d\n", 
		readyQueue[i].PID, readyQueue[i].arrivalTime, readyQueue[i].cpuBurstTime, readyQueue[i].ioBurstTime, readyQueue[i].priority);   
    }
*/
}
 
void FCFS() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0; //완료된 process 개수 
    int ganttChart[MAX_PROCESSES * 2], ganttIndex = 0;

	init_readyQueue();
    printf("\n-------------------- FCFS Scheduling --------------------\n");
    
    while (completedCount != processNum) { //끝나지 않은 process 존재 
        int run = -1;
        int minArrival = 30; 
        
        for (int i = 0; i < processNum; i++) { //모든 process 탐색 
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) { //이미 도착했고, 덜 수행된 process 
                if (readyQueue[i].arrivalTime < minArrival) {
                    minArrival = readyQueue[i].arrivalTime;
                    run = i; //가장 빨리 도착한 process
                }
            }
        }

        if (run != -1) { //실행가능한 process가 존재하면 
            currentTime += readyQueue[run].cpuBurstTime; 
            readyQueue[run].cpuRemainingTime = 0;
            readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime; //끝난 시간 - 도착 시간 
            readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime; //시작 시간 - 도착 시간 
            totalWaitingTime += readyQueue[run].waitingTime;
            totalTurnaroundTime += readyQueue[run].turnaroundTime;
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            completedCount++;
        } else {
            currentTime++; //실행가능한 process 부재, process가 도착하기를 기다림 
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_FCFS, totalWaitingTime, totalTurnaroundTime, processNum);

}

void NonPreemptive_SJF() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0;
    int ganttChart[MAX_PROCESSES * 3], ganttIndex = 0;

	init_readyQueue();
    printf("\n------------- Non-Preemptive SJF Scheduling -------------\n");

    while (completedCount != processNum) {
        int run = -1;
        int minBurst = 30;
        
        for (int i = 0; i < processNum; i++) { 
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) { 
                if (readyQueue[i].cpuBurstTime < minBurst) {
                    minBurst = readyQueue[i].cpuBurstTime; 
                    run = i;  //burst time이 가장 짧은 process
                }
            }
        }

        if (run != -1) {
            currentTime += readyQueue[run].cpuBurstTime; 
            readyQueue[run].cpuRemainingTime = 0; 
            readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime; 
            readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime;
            totalWaitingTime += readyQueue[run].waitingTime;
            totalTurnaroundTime += readyQueue[run].turnaroundTime;
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            completedCount++;
        } else {
            currentTime++;
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_NP_SJF, totalWaitingTime, totalTurnaroundTime, processNum);
}

void Preemptive_SJF() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0;
    int ganttChart[MAX_PROCESSES * 20], ganttIndex = 0;

	init_readyQueue();
    printf("\n--------------- Preemptive SJF Scheduling ---------------\n");

    while (completedCount != processNum) {
        int run = -1;
        int minBurst = 30;
        
        for (int i = 0; i < processNum; i++) {
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) {
                if (readyQueue[i].cpuRemainingTime < minBurst) {
                    minBurst = readyQueue[i].cpuRemainingTime;
                    run = i;  //남은 burst time이 가장 짧은 process
                }
            }
        }

        if (run != -1) {
            currentTime++; //preemptive이므로 매 수행마다 확인 
            readyQueue[run].cpuRemainingTime--; 
            ganttChart[ganttIndex++] = readyQueue[run].PID; 
            if (readyQueue[run].cpuRemainingTime == 0) { //모두 수행하면 
                completedCount++; 
                readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime;
                readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime;
                totalWaitingTime += readyQueue[run].waitingTime;
                totalTurnaroundTime += readyQueue[run].turnaroundTime;
            }
        } else {
            currentTime++;
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_P_SJF, totalWaitingTime, totalTurnaroundTime, processNum);
}

void NonPreemptive_Priority() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0;
    int ganttChart[MAX_PROCESSES * 3], ganttIndex = 0;

	init_readyQueue();
    printf("\n---------- Non-Preemptive Priority Scheduling ----------\n");

    while (completedCount != processNum) {
        int run = -1;
        int highestPriority = 15;

        for (int i = 0; i < processNum; i++) {
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) {
                if (readyQueue[i].priority < highestPriority) { //priority가 작을수록 높은 우선순위 
                    highestPriority = readyQueue[i].priority;
                    run = i; //가장 낮은 priority를 가진 process 
                }
            }
        }

        if (run != -1) {
            currentTime += readyQueue[run].cpuBurstTime;
            readyQueue[run].cpuRemainingTime = 0;
            readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime;
            readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime;
            totalWaitingTime += readyQueue[run].waitingTime;
            totalTurnaroundTime += readyQueue[run].turnaroundTime;
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            completedCount++;
        } else {
            currentTime++;
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_NP_PRI, totalWaitingTime, totalTurnaroundTime, processNum);
}

void Preemptive_Priority() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0;
    int ganttChart[MAX_PROCESSES * 20], ganttIndex = 0;

	init_readyQueue();
    printf("\n------------ Preemptive Priority Scheduling ------------\n");

    while (completedCount != processNum) {
        int run = -1;
        int highestPriority = 15;

        for (int i = 0; i < processNum; i++) {
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) {
                if (readyQueue[i].priority < highestPriority) { //priority가 작을수록 높은 우선순위 
                    highestPriority = readyQueue[i].priority;
                    run = i; //가장 낮은 priority를 가진 process
                }
            }
        }

        if (run != -1) {
            currentTime++;
            readyQueue[run].cpuRemainingTime--; //preemptive이므로 매 수행마다 확인  
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            if (readyQueue[run].cpuRemainingTime == 0) { //모두 수행하면
                completedCount++;
                readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime;
                readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime;
                totalWaitingTime += readyQueue[run].waitingTime;
                totalTurnaroundTime += readyQueue[run].turnaroundTime;
            }
        } else {
            currentTime++;
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_P_PRI, totalWaitingTime, totalTurnaroundTime, processNum);
}

void RoundRobin() {
    int currentTime = 0;
    int totalWaitingTime = 0, totalTurnaroundTime = 0;
    int completedCount = 0;
    int ganttChart[MAX_PROCESSES * 20], ganttIndex = 0;

    init_readyQueue(); //arrvial time 짧은 순으로 정렬 
    printf("\n---------------- Round Robin Scheduling ----------------\n");

    while (completedCount != processNum) {
        int run = 0;
        for (int i = 0; i < processNum; i++) { //미리 정렬해두어 timequantum만 고려하면 됨 
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) {
                run = 1;
                
                int timeSlice = (readyQueue[i].cpuRemainingTime > TIME_QUANTUM) ? TIME_QUANTUM : readyQueue[i].cpuRemainingTime; //타임 슬라이스 지정 
                currentTime += timeSlice;
                readyQueue[i].cpuRemainingTime -= timeSlice;

                if (readyQueue[i].cpuRemainingTime == 0) { //완료된 process 
                    readyQueue[i].turnaroundTime = currentTime - readyQueue[i].arrivalTime; 
                    readyQueue[i].waitingTime = readyQueue[i].turnaroundTime - readyQueue[i].cpuBurstTime;
                    totalWaitingTime += readyQueue[i].waitingTime;
                    totalTurnaroundTime += readyQueue[i].turnaroundTime;
                    completedCount++;
                }

                for(int j = 0; j < timeSlice; j++){ //타임퀀텀 보이게 
                	ganttChart[ganttIndex++] = readyQueue[i].PID;
         		}
            }
        }
        if (run == 0) { //실행가능한 process가 미존재하면, 대기
            currentTime++;
        }
    }

    printGantt(ganttChart, ganttIndex);
    evaluation(ALG_RR, totalWaitingTime, totalTurnaroundTime, processNum);
}

void printGantt(int ganttChart[], int n) {
    printf("\nGantt Chart:\n|");
    for (int i = 0; i < n; i++) {
        printf(" P%d |", ganttChart[i]);
    }
    printf("\n\n");
}

void evaluation(int alg, int totalWaitingTime, int totalTurnaroundTime, int processNum) { //각 알고리즘의 평균 WT와 TT, 인덱스 저장 
	evalTable[alg].AverageWT = (float)totalWaitingTime / processNum; 
	evalTable[alg].AverageTT = (float)totalTurnaroundTime / processNum;
	evalTable[alg].run = alg;
	
    printf("Average Waiting Time: %.2f\n", evalTable[alg].AverageWT);
    printf("Average Turnaround Time: %.2f\n", evalTable[alg].AverageTT);
}

void compare(){
    float minWT = evalTable[0].AverageWT; //가장 작은 WT 색출
    
    for (int i = 1; i < 6; i++) {  
        if (evalTable[i].AverageWT < minWT) {
            minWT = evalTable[i].AverageWT;
        }
    }

    float minTT = evalTable[0].AverageTT; //가장 작은 TT 색출 
    
    for (int i = 1; i < 6; i++) {
        if (evalTable[i].AverageTT < minTT) {
            minTT = evalTable[i].AverageTT;
        }
    }
    
    printf("\n\n========================= Result =============================\n\n");
    printf("Scheduling with the lowest Average Waiting Time: ");
    for (int i = 0; i < 6; i++) {
        if (evalTable[i].AverageWT == minWT) {
        	switch (evalTable[i].run) { //알고리즘 이름과 WT 출력 
                case 0:
                    printf("FCFS (%.2f)", minWT);
                    break;
                case 1:
                    printf("Non-Preemptive SJF (%.2f)", minWT);
                    break;
                case 2:
                    printf("Preemptive SJF (%.2f)", minWT);
                    break;
                case 3:
                    printf("Non-Preemptive Priority (%.2f)", minWT);
                    break;
                case 4:
                    printf("Preemptive Priority (%.2f)", minWT);
                    break;
                case 5:
                    printf("Round Robin (%.2f)", minWT);
                    break;
                default:
                    printf("Unknown ");
                    break;
            }
        }
    }
    printf("\n");

    printf("Scheduling with the lowest Average Turnaround Time: ");
    for (int i = 0; i < 6; i++) {
        if (evalTable[i].AverageTT == minTT) {
        	switch (evalTable[i].run) { //알고리즘 이름과 TT 출력
                case 0:
                    printf("FCFS (%.2f)", minTT);
                    break;
                case 1:
                    printf("Non-Preemptive SJF (%.2f)", minTT);
                    break;
                case 2:
                    printf("Preemptive SJF (%.2f)", minTT);
                    break;
                case 3:
                    printf("Non-Preemptive Priority (%.2f)", minTT);
                    break;
                case 4:
                    printf("Preemptive Priority (%.2f)", minTT);
                    break;
                case 5:
                    printf("Round Robin (%.2f)", minTT);
                    break;
                default:
                    printf("Unknown ");
                    break;
            }
        }
    }
    printf("\n");
}


int main() {
    create_Process();
    
	FCFS();
	NonPreemptive_SJF();
    Preemptive_SJF();
    NonPreemptive_Priority();
    Preemptive_Priority();
    RoundRobin();

	compare();

    return 0;
}

