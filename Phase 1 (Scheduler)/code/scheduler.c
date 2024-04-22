#include "headers.h"
int msgId_GeneratorSchedular, recVal_GeneratorSchedular, msgId_SchedularProcess,sendVal_SchedularProcess;
struct List * processQueue;
struct List * processFinished;
struct msgbuff msg;
struct msgBuff1 msg2;
struct processData*firstProcess =NULL;
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
     *  Case there is no process running send fork a process on the top of the queue 
     * *************************************************************************************/
    if(!isRunning){
        firstProcess= dequeue(processQueue);
        if(firstProcess== NULL) return;
        isRunning=true;
        strcpy(firstProcess->state,"started");
        char str_remainTime[10];
        sprintf(str_remainTime,"%d",firstProcess->remainingTime);

        pid= fork ();
       
        if(pid == -1){
            perror("Error in forking process;)\n");
        }
        else if (pid == 0){
                /********* Process Code *********/
            firstProcess->PID= getpid();
            printf("process %d is Created\n", firstProcess->PID);
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
             firstProcess->PID=pid;
             msg2.mType=pid;
        }
        return;
    }
    /**************************************************************************************
     * Case there is a Process running send message to process to decrement remaining time
     * wait a signal from process then it finishes
     * *************************************************************************************/
    else if(isRunning){
       
        int sendVal = msgsnd(msgId_SchedularProcess,&msg2, sizeof(msg2.decrement),!IPC_NOWAIT);
        firstProcess->remainingTime--;
        printProcessInfo(firstProcess);
        if (firstProcess->remainingTime == 0){
            waitpid(msg2.mType,NULL,0);
            isRunning= false;
            printf("finished process %d\n",getClk());
            sprintf(firstProcess->state,"finished");
            firstProcess->finishedTime=getClk();
            finishedProcesses++;
            Insert(processFinished,firstProcess);
        }

    }

}


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

    processQueue = createPriorityQueue();
    processFinished = createPriorityQueue();
    int processCounts=numberProcesses;
    while(finishedProcesses != processCounts){
            if (schedulerAlgorithm == 1){
                 // call HPF
               
                // printf("HPF ALGO\n");
                HPF();
                sleep(1);
            }else if (schedulerAlgorithm == 2){
                //call SRTN
                printf("SRTN ALGO\n");
            }else if (schedulerAlgorithm == 3){
                   //call RB
                printf("RB ALGO\n");
            }

    }
    printList(processFinished);

     while (1) {
        printf("clock time : %d\n",getClk());
        sleep(2);
    }

 
    destroyClk(true);
}
