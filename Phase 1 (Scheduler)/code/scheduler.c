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


int main(int argc, char * argv[])
{
    initClk();  
     int width = 600; // Width of the surface
    int height = 600; // Height of the surface
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);
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
    draw_list(cr, processFinished);
    cairo_surface_write_to_png(surface, "output.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
  

     while (1) {
        printf("clock time : %d\n",getClk());
        sleep(2);
    }

 
    destroyClk(true);
}
