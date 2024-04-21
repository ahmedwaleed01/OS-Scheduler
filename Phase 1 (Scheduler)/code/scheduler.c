#include "headers.h"
int msgId_GeneratorSchedular, recVal_GeneratorSchedular, msgId_SchedularProcess,sendVal_SchedularProcess;
struct List * processQueue;
struct msgbuff msg;
struct processData*processRunning =NULL;
int numberProcesses,finishedProcesses=0;



void HPF(){
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1){
        printf("time :%d\n",getClk());
        struct processData *process =&msg.process;
        strcpy(process->state,"ready");
        enqueue(processQueue,process);
        // printProcessInfo(process);
        numberProcesses --;
    }
    /********************************************************************************
     *  if there is no runnig process dequeue process from queue and start running it
     * *******************************************************************************/
    if(processRunning==NULL){
        processRunning= dequeue(processQueue);
        printf("aloooooooooooooooooooooooooo %d\n",processRunning->id);

       
        strcpy(processRunning->state,"started");
        char str_remainTime[10];
        sprintf(str_remainTime,"%d",processRunning->remainingTime);
        printProcessInfo(processRunning);
        int pid = fork ();
        if(pid == -1){
            perror("Error in forking process;)\n");
        }
        else if (pid == 0){
                /********* Process Code *********/
            processRunning->PID= getpid();
            printf("process %d is Created\n", processRunning->PID);
            char *processCode;
            char processDirectory[256];
            if (getcwd(processDirectory, sizeof(processDirectory)) != NULL ) {
                 processCode = strcat(processDirectory,"/process.out");     
            } else {
                perror("Error in getting the working directory ");
            }
            execl(processCode,processCode,str_remainTime,NULL);
            exit(1);
        }
    }
    /**************************************************************************
     * Case there is a process running send message to that process to decrement
     * the remaining time.
     *************************************************************/
    else {
        

        

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
    msgId_GeneratorSchedular = msgget(key_id, 0666 | IPC_CREAT);
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

    
    while(finishedProcesses != numberProcesses){
            if (schedulerAlgorithm == 1){
                 // call HPF
                processQueue = createPriorityQueue();
                // printf("HPF ALGO\n");
                HPF();
            }else if (schedulerAlgorithm == 2){
                //call SRTN
                printf("SRTN ALGO\n");
            }else if (schedulerAlgorithm == 3){
                   //call RB
                printf("RB ALGO\n");
            }

    }

     while (1) {
        printf("clock time : %d\n",getClk());
        sleep(2);
    }

 
    destroyClk(true);
}
