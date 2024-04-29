#include "headers.h"
int msgId_GeneratorSchedular, recVal_GeneratorSchedular, msgId_SchedularProcess,sendVal_SchedularProcess;
struct List * processQueue;
struct List * processFinished;
struct List * processStoppedQueue;
struct msgbuff msg;
struct msgBuff1 msg2;
struct processData*processRun =NULL;
struct processData*firstProcess=NULL;
int numberProcesses,finishedProcesses=0;
bool isRunning =false;
int pid ;


/********************************************************
 ********************************************************
 *** IMPLEMENTATION OF HIGHEST PRIORITY FIRST SCHEDULER**
 ********************************************************
 ********************************************************/

void HPF(){
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1){
        printf("time :%d\n",getClk());
        struct processData *process =(struct processData*)malloc(sizeof(msg.process));
        *process=msg.process;
        strcpy(process->state,"ready");
        enqueue(processQueue,process);
        printProcessInfo(process);
        numberProcesses --;
    }
    /**************************************************************************************
     * Case there is a Process running send message to process to decrement remaining time
     * wait a signal from process then it finishes
     * *************************************************************************************/
    if(isRunning){
        msg2.mType=processRun->PID;
        int sendVal = msgsnd(msgId_SchedularProcess,&msg2, sizeof(msg2.decrement),!IPC_NOWAIT);
        printf("***************************message recieverd   %d\n",msg2.mType);
        processRun->remainingTime--;
        printProcessInfo(processRun);
        if (processRun->remainingTime == 0){
            waitpid(msg2.mType,NULL,0);
            isRunning= false;
            printf("finished process %d\n",getClk());
            sprintf(processRun->state,"finished");
            processRun->finishedTime=getClk();
            finishedProcesses++;
            Insert(processFinished,processRun);
        }

    }
     /**************************************************************************************
     *  Case there is no process running send fork a process on the top of the queue 
     * *************************************************************************************/
    if(!isRunning){
        processRun= dequeue(processQueue);
        if(processRun== NULL) return;
        isRunning=true;
        strcpy(processRun->state,"started");
        char str_remainTime[10];
        sprintf(str_remainTime,"%d",processRun->remainingTime);

        pid= fork ();
       
        if(pid == -1){
            perror("Error in forking process;)\n");
        }
        else if (pid == 0){
                /********* Process Code *********/
            processRun->PID= getpid();
            printf("process %d is Created\n", processRun->PID);
            printProcessInfo(firstProcess);
            char *processCode;
            char processDirectory[256];
            if (getcwd(processDirectory, sizeof(processDirectory)) != NULL ) {
                 processCode = strcat(processDirectory,"/process.out");     
            } else {
                perror("Error in getting the working directory ");
            }
            execl(processCode,processCode,str_remainTime,NULL);
            exit(1);
        }else{
            processRun->PID=pid;           
        }
        return;
    }
}
/********************************************************
 ********************************************************
 *** IMPLEMENTATION OF SHORTEST REMAINING TIME FIRST ****
 ********************************************************
 ********************************************************/
void SRTN(){
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1){
        printf("time :%d\n",getClk());
        struct processData *process =(struct processData*)malloc(sizeof(msg.process));
        *process=msg.process;
        strcpy(process->state,"ready");
        enqueue(processQueue,process);
        printProcessInfo(process);
        numberProcesses --;
    }
    if(isRunning) {
        msg2.mType=processRun->PID;
        printf("///////////////////PROCEESSS REMAIN TIMEEEEEE: %d   %d\n",processRun->remainingTime,msg2.mType);
        int sendVal = msgsnd(msgId_SchedularProcess,&msg2, sizeof(msg2.decrement),!IPC_NOWAIT);
        processRun->remainingTime--;
        printProcessInfo(processRun);
        if (processRun->remainingTime == 0){
            waitpid(msg2.mType,NULL,0);
            isRunning= false;
            printf("finished process %d\n",getClk());
            sprintf(processRun->state,"finished");
            processRun->finishedTime=getClk();
            finishedProcesses++;
            printf("NUMBER FINISHED PROCESSSSSSSSS: %d\n",finishedProcesses);
            Insert(processFinished,processRun);
        }

    }
    if(!isRunning){
        if(!isEmpty(processStoppedQueue) && !isEmpty(processQueue) 
        && processStoppedQueue->head->process->remainingTime <= processStoppedQueue->head->process->remainingTime){
            processRun = dequeue(processStoppedQueue);
            strcpy(processRun->state,"resumed");
            msg2.mType=processRun->PID;
            printf("PROCESSSSSSSSSSSSSSSSSSS  %d   STATEEEEEEEEEEE :: %s\n",msg2.mType,processRun->state);
            isRunning=true;
        }
        else {
             processRun= dequeue(processQueue);
             if(processRun== NULL) return;
            isRunning=true;
            strcpy(processRun->state,"started");
            char str_remainTime[10];
            sprintf(str_remainTime,"%d",processRun->remainingTime);

             pid= fork ();
       
            if(pid == -1){
              perror("Error in forking process;)\n");
            }
            else if (pid == 0){
                        /********* Process Code *********/
                processRun->PID= getpid();
                printf("process %d is Created\n", processRun->PID);
                printProcessInfo(processRun);
                char *processCode;
                char processDirectory[256];
                if (getcwd(processDirectory, sizeof(processDirectory)) != NULL ) {
                    processCode = strcat(processDirectory,"/process.out");     
                }else {
                    perror("Error in getting the working directory ");
                }
                execl(processCode,processCode,str_remainTime,NULL);
                exit(1);
            }else{
                processRun->PID=pid;
                msg2.mType=pid;
            }
            return;
        }
    }
    else if (isRunning && !isEmpty(processQueue) && processRun->remainingTime > processQueue->head->process->remainingTime){
            strcpy(processRun->state,"stopped");
            enqueue(processStoppedQueue,processRun);
            processRun= dequeue(processQueue);
            if(processRun== NULL) return;
            strcpy(processRun->state,"started");
            char str_remainTime[10];
            sprintf(str_remainTime,"%d",processRun->remainingTime);

            pid= fork ();
       
            if(pid == -1){
                perror("Error in forking process;)\n");
            }
            else if (pid == 0){
                /********* Process Code *********/
                processRun->PID= getpid();
                printf("process %d is Created\n", processRun->PID);
                printProcessInfo(processRun);
                char *processCode;
                char processDirectory[256];
            if (getcwd(processDirectory, sizeof(processDirectory)) != NULL ) {
                 processCode = strcat(processDirectory,"/process.out");     
            } else {
                perror("Error in getting the working directory ");
            }
                execl(processCode,processCode,str_remainTime,NULL);
                exit(1);
            }else{
                processRun->PID=pid;
                msg2.mType=pid;
                printf("testtttttttttttttttttttttttttttttttttttttttttttttttttt %d \n", msg2.mType);
            }  
            processRun->PID=pid;
            msg2.mType=pid;
            printf("testtttttttttttttttttttttttttttttttttttttttttttttttttt2222 %d \n", msg2.mType);
            return;
    }
}
// // Function to handle output files (scheduler.log and scheduler.perf)
// void handleOutputFiles(struct List *processFinished) {
//     // Open log file for writing
//     FILE *logFile = fopen("scheduler.log", "w");
//     if (logFile == NULL) {
//         perror("Error opening log file");
//         exit(EXIT_FAILURE);
//     }

//     // Open performance file for writing
//     FILE *perfFile = fopen("scheduler.perf", "w");
//     if (perfFile == NULL) {
//         perror("Error opening performance file");
//         exit(EXIT_FAILURE);
//     }

//     // Variables for performance metrics calculation
//     int totalProcesses = 0;
//     int totalExecutionTime = 0;
//     int totalWaitingTime = 0;
//     int totalTurnaroundTime = 0;
//     int squaredWTA = 0;

//     // Process each finished process
//     struct Node *current = processFinished->head;
//     while (current != NULL) {
        
//         struct processData *process = current->process;

//         // Calculate process metrics
//         int turnaroundTime = process->finishedTime - process->arrivalTime;
//         int weightedTurnaroundTime = turnaroundTime / process->runTime; //int weightedTurnaroundTime = turnaroundTime * process->priority;
//         int waitingTime = turnaroundTime - process->executionTime;

//         // Write process events to log file
//         fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", process->startTime, process->pid, process->arrivalTime, process->executionTime, process->remainingTime, waitingTime);
//         fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", process->finishedTime, process->pid, process->arrivalTime, process->executionTime, 0, waitingTime, turnaroundTime, (double)weightedTurnaroundTime / process->executionTime);

//         // Update total metrics
//         totalProcesses++;
//         totalExecutionTime += process->executionTime;
//         totalWaitingTime += waitingTime;
//         totalTurnaroundTime += turnaroundTime;
//         squaredWTA += weightedTurnaroundTime * weightedTurnaroundTime;

//         // Move to the next process
//         current = current->next;
//     }

//     // Calculate performance metrics
//     double cpuUtilization = (double)totalExecutionTime / totalTurnaroundTime * 100;
//     double avgWTA = (double)totalTurnaroundTime / totalProcesses;
//     double avgWaiting = (double)totalWaitingTime / totalProcesses;
//     double stdWTA = sqrt((double)squaredWTA / totalProcesses - avgWTA * avgWTA);

//     // Write performance metrics to perf file

    
//     fprintf(perfFile, "CPU utilization = %.2f%%\n", cpuUtilization);
//     fprintf(perfFile, "Avg WTA = %.2f\n", avgWTA);
//     fprintf(perfFile, "Avg Waiting = %.2f\n", avgWaiting);
//     fprintf(perfFile, "Std WTA = %.2f\n", stdWTA);

//     // Close log and perf files
//     fclose(logFile);
//     fclose(perfFile);
// }

int main(int argc, char * argv[])
{
    initClk();  
    //TODO implement the scheduler :)
    //upon termination release the clock resources.

    // Create message queue between process generator and scheduler
    key_t key_id;
    key_id = ftok("keyfile", 60);
    msgId_GeneratorSchedular = msgget(55555, 0666 | IPC_CREAT);
    if (msgId_GeneratorSchedular == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    // Create message queue between scheduler and processes
    key_t key_id2;
    key_id2 = ftok("keyfile", 75);
    msgId_SchedularProcess = msgget(11111, 0666 | IPC_CREAT);
    if ( msgId_SchedularProcess == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    printf("message queue Id between process generator and scheduler %d\n", msgId_GeneratorSchedular );
    printf("message queue Id between scheduler and processes %d\n", msgId_SchedularProcess );

    int schedulerAlgorithm = atoi(argv[1]);
    numberProcesses = atoi(argv[2]);

    if (schedulerAlgorithm == 1)  processQueue = createPriorityQueue();
    else if (schedulerAlgorithm == 2){  
        processQueue = createQueuePirorityRemainTime();
        processStoppedQueue =createQueuePirorityRemainTime();
    }

    processFinished = createPriorityQueue();
    int processCounts=numberProcesses;
    while(finishedProcesses != processCounts){
            if (schedulerAlgorithm == 1){
                 // call HPF
                HPF();
                sleep(1);
            }else if (schedulerAlgorithm == 2){
                // //call SRTN
                SRTN();
                sleep(1);
            }else if (schedulerAlgorithm == 3){
                   //call RB
                printf("RB ALGO\n");
            }

    }
    printList(processFinished);

    //     // Handle output files
    // handleOutputFiles(processFinished);

     while (1) {
        printf("clock time : %d\n",getClk());
        sleep(2);
    }

 
    destroyClk(true);
}
