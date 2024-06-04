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

eval evalTable[6]; //�� �˰����� ��� WT�� TT�� ���� 

void evaluation();
void compare();

/*************************Functions********************************/

void create_Process() {
    srand(time(NULL));
	processNum = rand() % MAX_PROCESSES + 1; // ���� ��� ���� ���� Process ���� ���� 
    //processNum = MAX_PROCESSES; // ���μ��� ���� ���� 
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

void init_readyQueue() { // ��ť�� �̹� ������ ���μ����� ����ť�� �ִ´ٰ� ����. ���� ������ ������ ����
    int i;
    for (i = 0; i < processNum; i++)
        readyQueue[i] = processes[i]; // ��� �˰��� ������ ���� copy
        
    Process temp;
	int j, k;
	for(j = processNum - 1; j > 0; j--){
		for(k = 0 ; k < j; k++){
			if(readyQueue[k].arrivalTime > readyQueue[k + 1].arrivalTime){ //������ �� ���μ������� �������� ���� 
				temp = readyQueue[k + 1];
				readyQueue[k + 1] = readyQueue[k];
				readyQueue[k] = temp;
			
			}
			else if(readyQueue[k].arrivalTime == readyQueue[k + 1].arrivalTime && readyQueue[k].PID > readyQueue[k + 1].PID){ //arrival time ������ pid �������� 
				temp = readyQueue[k + 1];
				readyQueue[k + 1] = readyQueue[k];
				readyQueue[k] = temp;
			}
		}
	}

/*****���� Ȯ�ο� ��� 
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
    int completedCount = 0; //�Ϸ�� process ���� 
    int ganttChart[MAX_PROCESSES * 2], ganttIndex = 0;

	init_readyQueue();
    printf("\n-------------------- FCFS Scheduling --------------------\n");
    
    while (completedCount != processNum) { //������ ���� process ���� 
        int run = -1;
        int minArrival = 30; 
        
        for (int i = 0; i < processNum; i++) { //��� process Ž�� 
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) { //�̹� �����߰�, �� ����� process 
                if (readyQueue[i].arrivalTime < minArrival) {
                    minArrival = readyQueue[i].arrivalTime;
                    run = i; //���� ���� ������ process
                }
            }
        }

        if (run != -1) { //���డ���� process�� �����ϸ� 
            currentTime += readyQueue[run].cpuBurstTime; 
            readyQueue[run].cpuRemainingTime = 0;
            readyQueue[run].turnaroundTime = currentTime - readyQueue[run].arrivalTime; //���� �ð� - ���� �ð� 
            readyQueue[run].waitingTime = readyQueue[run].turnaroundTime - readyQueue[run].cpuBurstTime; //���� �ð� - ���� �ð� 
            totalWaitingTime += readyQueue[run].waitingTime;
            totalTurnaroundTime += readyQueue[run].turnaroundTime;
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            completedCount++;
        } else {
            currentTime++; //���డ���� process ����, process�� �����ϱ⸦ ��ٸ� 
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
                    run = i;  //burst time�� ���� ª�� process
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
                    run = i;  //���� burst time�� ���� ª�� process
                }
            }
        }

        if (run != -1) {
            currentTime++; //preemptive�̹Ƿ� �� ���ึ�� Ȯ�� 
            readyQueue[run].cpuRemainingTime--; 
            ganttChart[ganttIndex++] = readyQueue[run].PID; 
            if (readyQueue[run].cpuRemainingTime == 0) { //��� �����ϸ� 
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
                if (readyQueue[i].priority < highestPriority) { //priority�� �������� ���� �켱���� 
                    highestPriority = readyQueue[i].priority;
                    run = i; //���� ���� priority�� ���� process 
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
                if (readyQueue[i].priority < highestPriority) { //priority�� �������� ���� �켱���� 
                    highestPriority = readyQueue[i].priority;
                    run = i; //���� ���� priority�� ���� process
                }
            }
        }

        if (run != -1) {
            currentTime++;
            readyQueue[run].cpuRemainingTime--; //preemptive�̹Ƿ� �� ���ึ�� Ȯ��  
            ganttChart[ganttIndex++] = readyQueue[run].PID;
            if (readyQueue[run].cpuRemainingTime == 0) { //��� �����ϸ�
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

    init_readyQueue(); //arrvial time ª�� ������ ���� 
    printf("\n---------------- Round Robin Scheduling ----------------\n");

    while (completedCount != processNum) {
        int run = 0;
        for (int i = 0; i < processNum; i++) { //�̸� �����صξ� timequantum�� ����ϸ� �� 
            if (readyQueue[i].arrivalTime <= currentTime && readyQueue[i].cpuRemainingTime > 0) {
                run = 1;
                
                int timeSlice = (readyQueue[i].cpuRemainingTime > TIME_QUANTUM) ? TIME_QUANTUM : readyQueue[i].cpuRemainingTime; //Ÿ�� �����̽� ���� 
                currentTime += timeSlice;
                readyQueue[i].cpuRemainingTime -= timeSlice;

                if (readyQueue[i].cpuRemainingTime == 0) { //�Ϸ�� process 
                    readyQueue[i].turnaroundTime = currentTime - readyQueue[i].arrivalTime; 
                    readyQueue[i].waitingTime = readyQueue[i].turnaroundTime - readyQueue[i].cpuBurstTime;
                    totalWaitingTime += readyQueue[i].waitingTime;
                    totalTurnaroundTime += readyQueue[i].turnaroundTime;
                    completedCount++;
                }

                for(int j = 0; j < timeSlice; j++){ //Ÿ������ ���̰� 
                	ganttChart[ganttIndex++] = readyQueue[i].PID;
         		}
            }
        }
        if (run == 0) { //���డ���� process�� �������ϸ�, ���
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

void evaluation(int alg, int totalWaitingTime, int totalTurnaroundTime, int processNum) { //�� �˰����� ��� WT�� TT, �ε��� ���� 
	evalTable[alg].AverageWT = (float)totalWaitingTime / processNum; 
	evalTable[alg].AverageTT = (float)totalTurnaroundTime / processNum;
	evalTable[alg].run = alg;
	
    printf("Average Waiting Time: %.2f\n", evalTable[alg].AverageWT);
    printf("Average Turnaround Time: %.2f\n", evalTable[alg].AverageTT);
}

void compare(){
    float minWT = evalTable[0].AverageWT; //���� ���� WT ����
    
    for (int i = 1; i < 6; i++) {  
        if (evalTable[i].AverageWT < minWT) {
            minWT = evalTable[i].AverageWT;
        }
    }

    float minTT = evalTable[0].AverageTT; //���� ���� TT ���� 
    
    for (int i = 1; i < 6; i++) {
        if (evalTable[i].AverageTT < minTT) {
            minTT = evalTable[i].AverageTT;
        }
    }
    
    printf("\n\n========================= Result =============================\n\n");
    printf("Scheduling with the lowest Average Waiting Time: ");
    for (int i = 0; i < 6; i++) {
        if (evalTable[i].AverageWT == minWT) {
        	switch (evalTable[i].run) { //�˰��� �̸��� WT ��� 
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
        	switch (evalTable[i].run) { //�˰��� �̸��� TT ���
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

