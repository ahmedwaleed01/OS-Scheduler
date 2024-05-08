#include "headers.h"
int msgId_GeneratorSchedular, recVal_GeneratorSchedular, msgId_SchedularProcess, sendVal_SchedularProcess;
struct List *processQueue;
struct List *processFinished;
struct List *processStoppedQueue;
struct List *calcQueue;
struct msgbuff msg;
struct msgBuff1 msg2;
struct processData *processRun = NULL;
struct processData *firstProcess = NULL;
int numberProcesses, finishedProcesses = 0;
bool isRunning = false;
int pid;
FILE *LogFile; // this is the log file
FILE *perfFile;
int countActive=1;
bool isFirstProcess = true;
int startTime = 0;
int endTime = 0;
union Semun semun;
int sem_sync = -1;


void Log(struct processData *process)
{
    if (process)
    {
        if (!strcmp(process->state, "finished"))
        {
            fprintf(LogFile, "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n", getClk(), process->id, process->state, process->arrivalTime, process->runTime, process->remainingTime, process->waitingTime, process->turnAroundTime, process->weightedTurnAroundTime);
        }
        else
        {
            fprintf(LogFile, "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), process->id, process->state, process->arrivalTime, process->runTime, process->remainingTime, process->waitingTime);
        }
    }
    else
    {
        return;
    }
}

/********************************************************
 ********************************************************
 *** IMPLEMENTATION OF HIGHEST PRIORITY FIRST SCHEDULER**
 ********************************************************
 ********************************************************/

void HPF()
{
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1)
    {
       
        printf("time :%d\n", getClk());
        struct processData *process = (struct processData *)malloc(sizeof(msg.process));
        *process = msg.process;
        strcpy(process->state, "ready");
        //Log(process);
        enqueue(processQueue, process);
        printProcessInfo(process);
        numberProcesses--;
    }
    /**************************************************************************************
     * Case there is a Process running send message to process to decrement remaining time
     * wait a signal from process then it finishes
     * *************************************************************************************/
    if (isRunning)
    {
        countActive++;
        msg2.mType = processRun->PID;
        int sendVal = msgsnd(msgId_SchedularProcess, &msg2, sizeof(msg2.decrement), !IPC_NOWAIT);
        printf("***************************message recieverd   %d\n", msg2.mType);
        processRun->remainingTime--;
        printProcessInfo(processRun);
        Log(processRun);
        if (processRun->remainingTime == 0)
        {
            waitpid(msg2.mType, NULL, 0);
            isRunning = false;
            printf("finished process %d\n", getClk());
            sprintf(processRun->state, "finished");
            processRun->finishedTime = getClk();
            endTime = getClk();
            finishedProcesses++;
            processRun->turnAroundTime=processRun->finishedTime -processRun->arrivalTime;
            processRun->weightedTurnAroundTime=(float)processRun->turnAroundTime/processRun->runTime;
            Log(processRun);
            Insert(processFinished, processRun);
        }
    }
    /**************************************************************************************
     *  Case there is no process running send fork a process on the top of the queue
     * *************************************************************************************/
    if (!isRunning)
    {
        
        processRun = dequeue(processQueue);
        if (processRun == NULL)
            return;
        isRunning = true;
        strcpy(processRun->state, "started");
        processRun->waitingTime=getClk()-processRun->arrivalTime;
        char str_remainTime[10];
        sprintf(str_remainTime, "%d", processRun->remainingTime);

        pid = fork();

        if (pid == -1)
        {
            perror("Error in forking process;)\n");
        }
        else if (pid == 0)
        {
            /********* Process Code *********/
            processRun->PID = getpid();
            // processRun->startTime=getClk();
            processRun->waitingTime=getClk()-processRun->arrivalTime;
            printf("process %d is Created\n", processRun->PID);
            printProcessInfo(processRun);
            char *processCode;
            char processDirectory[256];
            if (getcwd(processDirectory, sizeof(processDirectory)) != NULL)
            {
                processCode = strcat(processDirectory, "/process.out");
            }
            else
            {
                perror("Error in getting the working directory ");
            }
            execl(processCode, processCode, str_remainTime, NULL);
            exit(1);
        }
        else
        {
            processRun->PID = pid;
            msg2.mType = pid;
        }
        Log(processRun);
        return;
    }
}
/********************************************************
 ********************************************************
 *** IMPLEMENTATION OF SHORTEST REMAINING TIME FIRST ****
 ********************************************************
 ********************************************************/
void SRTN()
{
    /************ Every clock cycle get process arrived from process generator ****************/
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1)
    {
        printf("time :%d\n", getClk());
        struct processData *process = (struct processData *)malloc(sizeof(msg.process));
        *process = msg.process;
        strcpy(process->state, "ready");
        enqueue(processQueue, process);
        printProcessInfo(process);
        numberProcesses--;
    }
    /***********************************************************************
     * send message to running process to decrement its remaining time by one
     *  ********************************************************************/
    if (isRunning)
    {
        countActive++;
        msg2.mType = processRun->PID;
        int sendVal = msgsnd(msgId_SchedularProcess, &msg2, sizeof(msg2.decrement), !IPC_NOWAIT);
        processRun->remainingTime--;
        Log(processRun);
        printProcessInfo(processRun);
        if (processRun->remainingTime == 0)
        {
            waitpid(msg2.mType, NULL, 0);
            isRunning = false;
            printf("finished process %d\n", getClk());
            sprintf(processRun->state, "finished");
            processRun->finishedTime = getClk();
            endTime = getClk();
            finishedProcesses++;
            processRun->turnAroundTime=processRun->finishedTime -processRun->arrivalTime;
            processRun->weightedTurnAroundTime=(float)processRun->turnAroundTime/processRun->runTime;
            Log(processRun);
            printf("NUMBER FINISHED PROCESSSSSSSSS: %d\n", finishedProcesses);
            Insert(processFinished, processRun);
        }
    }
    /**************************************************************************************
     *  Case there is no process running:
     *  -I Have to check which process to run the stopped process or process in the ready queue
     * *************************************************************************************/
    if (!isRunning)
    {
        if ((!isEmpty(processStoppedQueue) && !isEmpty(processQueue) && processStoppedQueue->head->process->remainingTime <= processQueue->head->process->remainingTime )||
            !isEmpty(processStoppedQueue) && isEmpty(processQueue)
        )
        {
            processRun = dequeue(processStoppedQueue);
            strcpy(processRun->state, "resumed");
            msg2.mType = processRun->PID;
            isRunning = true;
        }
        else
        {
            processRun = dequeue(processQueue);
            if (processRun == NULL)
                return;
            isRunning = true;
            strcpy(processRun->state, "started");
            processRun->waitingTime=getClk()-processRun->arrivalTime;
            Log(processRun);
            char str_remainTime[10];
            sprintf(str_remainTime, "%d", processRun->remainingTime);

            pid = fork();

            if (pid == -1)
            {
                perror("Error in forking process;)\n");
            }
            else if (pid == 0)
            {
                /********* Process Code *********/
                processRun->PID = getpid();
                processRun->startTime=getClk();
                printf("process %d is Created\n", processRun->PID);
                printProcessInfo(processRun);
                char *processCode;
                char processDirectory[256];
                if (getcwd(processDirectory, sizeof(processDirectory)) != NULL)
                {
                    processCode = strcat(processDirectory, "/process.out");
                }
                else
                {
                    perror("Error in getting the working directory ");
                }
                execl(processCode, processCode, str_remainTime, NULL);
                exit(1);
            }
            else
            {
                processRun->PID = pid;
                msg2.mType = pid;
            }
            return;
        }
    }
     /**************************************************************************************
     *  Case there is a running process
     *  -I Have to check if there is an already process in my ready queue have less remainingTime
     *  if condition is true stop the running process and fork the new process.
     * *************************************************************************************/
    else if (isRunning && !isEmpty(processQueue) && processRun->remainingTime > processQueue->head->process->remainingTime)
    {
        strcpy(processRun->state, "stopped");
        Log(processRun);
        enqueue(processStoppedQueue, processRun);
        processRun = dequeue(processQueue);
        if (processRun == NULL)
            return;
        strcpy(processRun->state, "started");
        processRun->waitingTime=getClk()-processRun->arrivalTime;
        Log(processRun);
        char str_remainTime[10];
        sprintf(str_remainTime, "%d", processRun->remainingTime);

        pid = fork();

        if (pid == -1)
        {
            perror("Error in forking process;)\n");
        }
        else if (pid == 0)
        {
            /********* Process Code *********/
            processRun->PID = getpid();
            processRun->startTime=getClk();
            printf("process %d is Created\n", processRun->PID);
            printProcessInfo(processRun);
            char *processCode;
            char processDirectory[256];
            if (getcwd(processDirectory, sizeof(processDirectory)) != NULL)
            {
                processCode = strcat(processDirectory, "/process.out");
            }
            else
            {
                perror("Error in getting the working directory ");
            }
            execl(processCode, processCode, str_remainTime, NULL);
            exit(1);
        }
        else
        {
            processRun->PID = pid;
            msg2.mType = pid;
        }
        processRun->PID = pid;
        msg2.mType = pid;
        return;
    }
}

/********************************************************
 ********************************************************
 *** IMPLEMENTATION OF ROUND ROBIN SCHEDULER ************
 ********************************************************
 ********************************************************/

void RB(int quantumValue)
{
    printf("ROUND ROBIN\n");
    while (msgrcv(msgId_GeneratorSchedular, &msg, sizeof(msg.process), 0, IPC_NOWAIT) != -1)
    {
        struct processData *process = (struct processData *)malloc(sizeof(msg.process));
        *process = msg.process;
        process->quantum = quantumValue;
        strcpy(process->state, "ready");
        Insert(processQueue, process);
        printProcessInfo(process);
        numberProcesses--;
    }

    if (isRunning)
    {
        countActive++;
        msg2.mType = processRun->PID;
        int sendVal = msgsnd(msgId_SchedularProcess, &msg2, sizeof(msg2.decrement), !IPC_NOWAIT);
        processRun->remainingTime--;
        processRun->quantum--;
        printProcessInfo(processRun);
        Log(processRun);
        if (processRun->remainingTime == 0)
        {
            waitpid(msg2.mType, NULL, 0);
            isRunning = false;
            printf("finished process %d\n", getClk());
            sprintf(processRun->state, "finished");
            processRun->finishedTime = getClk();
            endTime = getClk();
            finishedProcesses++;
            processRun->turnAroundTime=processRun->finishedTime -processRun->arrivalTime;
            processRun->weightedTurnAroundTime=(float)processRun->turnAroundTime/processRun->runTime;
            Log(processRun);
            Insert(processFinished, processRun);
        }
        else if (processRun->quantum == 0)
        {
            processRun->quantum = quantumValue;
            strcpy(processRun->state, "stopped");
            Log(processRun);
            Insert(processQueue, processRun); 
            isRunning = false; 
        }
        // started, resumed, stopped, finished.
    }
    /**************************************************************************************
     *  Case there is no process running send fork a process on the top of the queue
     * *************************************************************************************/
    if (!isRunning)
    {
        if (!isEmpty(processQueue))
        {
            processRun = dequeue(processQueue);
            if (processRun == NULL)
                return;
            isRunning = true;
            if(strcmp("stopped",processRun->state)==0){
                strcpy(processRun->state, "resumed");
                Log(processRun); 
            }else{
                printf("processssssssssssssss state %s",processRun->state);
                strcpy(processRun->state, "started");
                processRun->waitingTime=getClk()-processRun->arrivalTime;  
                Log(processRun); 
                char str_remainTime[10];
                sprintf(str_remainTime, "%d", processRun->remainingTime);

                pid = fork();

                if (pid == -1)
                {
                    perror("Error in forking process;)\n");
                }
                else if (pid == 0)
                {
                        /********* Process Code *********/
                    processRun->PID = getpid();
                    processRun->startTime=getClk();
                    printProcessInfo(processRun);
                    char *processCode;
                    char processDirectory[256];
                    processRun->quantum = quantumValue;
                    if (getcwd(processDirectory, sizeof(processDirectory)) != NULL)
                    {
                        processCode = strcat(processDirectory, "/process.out");
                    }
                    else
                    {
                        perror("Error in getting the working directory ");
                    }
                    execl(processCode, processCode, str_remainTime, NULL);
                    exit(1);
                }
            }
            processRun->PID = pid;
            msg2.mType = pid;
            return;
        }
    }
}


int main(int argc, char *argv[])
    {
        initClk();
        // TODO implement the scheduler :)
        // upon termination release the clock resources.

        /************Configure Output Image************/
        int width = 600;                    
        int height = 600;                   
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cairo_t *cr = cairo_create(surface);

        // Create message queue between process generator and scheduler
        key_t key_id;
        key_id = ftok("keyfile", 60);
        msgId_GeneratorSchedular = msgget(55555, 0666 | IPC_CREAT);
        if (msgId_GeneratorSchedular == -1)
        {
            perror("Error in create message queue:)");
            exit(-1);
        }
        // create a log file
        LogFile = fopen("scheduler.log", "w");
        if (LogFile == NULL)
        {
            printf("Error opening the file.\n");
            return 1;
        }
        fprintf(LogFile, "#At\ttime\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n");
        // Create message queue between scheduler and processes
        key_t key_id2;
        key_id2 = ftok("keyfile", 75);
        msgId_SchedularProcess = msgget(11111, 0666 | IPC_CREAT);
        if (msgId_SchedularProcess == -1)
        {
            perror("Error in create message queue:)");
            exit(-1);
        }
        printf("message queue Id between process generator and scheduler %d\n", msgId_GeneratorSchedular);
        printf("message queue Id between scheduler and processes %d\n", msgId_SchedularProcess);

        int sem_sync = -1;
        key_t key_sem_sync = ftok("keyfile", 80);
        sem_sync = semget(key_sem_sync,1,0666|IPC_CREAT);

        int schedulerAlgorithm = atoi(argv[1]);
        numberProcesses = atoi(argv[2]);
        int quantumValue = atoi(argv[3]);
        
        if (schedulerAlgorithm == 1)
            processQueue = createPriorityQueue();
        else if (schedulerAlgorithm == 2)
        {
            processQueue = createQueuePirorityRemainTime();
            processStoppedQueue = createQueuePirorityRemainTime();
        }else if (schedulerAlgorithm == 3){
            processQueue= createLinkedList();
            processStoppedQueue=createLinkedList();
        }

        processFinished = createPriorityQueue();
        int processCounts = numberProcesses;
        printf("nooooooooooooooooooooooooooooooooo processs %d\n", processCounts);
        while (finishedProcesses != processCounts)
        {
            if (schedulerAlgorithm == 1)
            {
                // call HPF
                down(sem_sync);
                HPF();
                sleep(1);
            }
            else if (schedulerAlgorithm == 2)
            {
                //call SRTN
                down(sem_sync);
                SRTN();
                sleep(1);
            }
            else if (schedulerAlgorithm == 3)
            {
                // call RB
                down(sem_sync);
                RB(quantumValue);
                sleep(1);
            }
        }
        fclose(LogFile);
        /****Print List of Finished Process in the terminal*****/
        printList(processFinished);

        /**********Draw The Output Image********/
        draw_list(cr, processFinished);
        cairo_surface_write_to_png(surface, "output.png");
        cairo_destroy(cr);
        cairo_surface_destroy(surface);

        calcQueue = createPriorityQueue();
        perfFile = fopen("scheduler.perf", "w");
        float cpuUtil = ((float)countActive / (endTime - startTime)) * 100;
        float avgWaitTime = 0;
        float avgWTA = 0;
        float stdWTA = 0;
        int waitingTimeSum;
        float sumWTASquared = 0;
        float weightedTASum = 0;
        int processesCount = processFinished->size;
        for (int i = 0; i < processesCount; i++)
        {
            struct processData *calcProcess = dequeue(processFinished);
            calcProcess->turnAroundTime = calcProcess->finishedTime - calcProcess->arrivalTime;
            calcProcess->waitingTime = calcProcess->turnAroundTime - calcProcess->runTime;
            calcProcess->weightedTurnAroundTime = (float)calcProcess->turnAroundTime / calcProcess->runTime;
            waitingTimeSum += calcProcess->waitingTime;
            weightedTASum += calcProcess->weightedTurnAroundTime;
            Insert(processFinished,calcProcess);
        }
        avgWaitTime = (float)waitingTimeSum / processesCount;
        avgWTA = (float)weightedTASum / processesCount;

        for (int i = 0; i < processesCount; i++)
        {
            struct processData *calcProcess = dequeue(processFinished);
            float difference = calcProcess->weightedTurnAroundTime - avgWTA;
            sumWTASquared += (difference * difference);
        }

        float meanWTASquared = sumWTASquared / processesCount;
        stdWTA = sqrt(meanWTASquared);
        fprintf(perfFile, "CPU\tutlilization\t=\t%.2f%%\n", cpuUtil);
        fprintf(perfFile, "Avg\tWTA\t=\t%.2f\n", avgWTA);
        fprintf(perfFile, "Avg\tWaiting\t=\t%.2f\n", avgWaitTime);
        fprintf(perfFile, "Std\tWTA\t=\t%.2f\n", stdWTA);
        fclose(perfFile);

        destroyClk(true);
}