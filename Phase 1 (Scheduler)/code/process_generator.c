#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);

    // Read the input files.
    FILE *fptr;
    fptr = fopen("processes.txt", "r");
    if (fptr == NULL){
        perror("Error Openning File");
        return 1;
    }
    // Count the Number of Processes
    int numberProcesses = CountProcess("processes.txt");

    // Create Process Table
    struct processInputData processTable[numberProcesses];
    for(int i=0; i<numberProcesses;i++){
        fscanf(fptr,"%d\t%d\t%d\t%d",&processTable[i].id,&processTable[i].arrivalTime,&processTable[i].runTime,&processTable[i].priority);
        // printf("%d: %d %d %d %d\n",i,processTable[i].id,processTable[i].arrivalTime,processTable[i].runTime,processTable[i].priority);
    }

    // Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algorithmChosen;
    printf("Choose The Algorithm: \n");
    printf("1. Non-preemptive Highest Priority First (HPF) \n");
    printf("2. Shortest Remaining time Next (SRTN)\n");
    printf("3. Round Robin (RR) \n");
    scanf("%d",&algorithmChosen);
    // convert it to string to send to schedular
    char str_algorithmChosen[10];
    sprintf(str_algorithmChosen,"%d",algorithmChosen);

    /*  Initiate and create the scheduler and clock processes.
     *  Getting Routes for Scheduler and Clock code path    */
    char schedularDirectory[256];
    char clockDicrectory[256];
    char *schedularCode,*clockCode;

    // Get the current working directory and concatenate the code.out path
    if (getcwd(schedularDirectory, sizeof(schedularDirectory)) != NULL && getcwd(clockDicrectory, sizeof(clockDicrectory)) != NULL) {
       schedularCode = strcat(schedularDirectory,"/scheduler.out");
       clockCode  =  strcat(clockDicrectory,"/clk.out");
    } else {
        perror("Error in getting the working directory ");
        return 1;
    }

    pid_t pidScheduler = fork ();
    if (pidScheduler==-1){
        printf("Error in forking scheduler\n");
        exit(-1);
    }

    if(pidScheduler == 0){ 
                    /**** Scheduler Code ****/
        execl(schedularCode,schedularCode,str_algorithmChosen, NULL);
        perror("Error executing scheduler!");
        exit(-1);
    }
    else{ 
             /**process generator code**/
        pid_t pidClock = fork();
        if (pidClock== -1){
            printf("Error in forking Clock\n");
            exit(-1);
        }

        if( pidClock == 0){ 
              /***** Clock Code *****/
            execl(clockCode,clockCode,NULL);
            perror("Error executing scheduler!");
            exit(-1);
        }
    }
    // Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // Create a data structure for processes and provide it with its parameters.
    struct processData processDataTable[numberProcesses];
    for (int i=0 ; i<numberProcesses; i++){
        processDataTable[i].id=processTable[i].id;
        processDataTable[i].arrivalTime=processTable[i].arrivalTime;
        processDataTable[i].runTime=processTable[i].runTime;
        processDataTable[i].priority=processTable[i].priority;
        processDataTable[i].remainingTime=processTable[i].runTime;
        processDataTable[i].finishedTime = 0;
        processDataTable[i].turnAroundTime=0;
        processDataTable[i].waitingTime=0;
        processDataTable[i].weightedTurnAroundTime=0;
        strcpy(processDataTable[i].state,"ready");
    }
    // Create message queue between process generator and scheduler
    key_t key_id;
    key_id = ftok("keyfile", 60);
    int msgId_GeneratorSchedular = msgget(key_id, 0666 | IPC_CREAT);
    if (msgId_GeneratorSchedular == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    printf("message queue Id between process generator and scheduler %d\n", msgId_GeneratorSchedular );
    // Send the information to the scheduler at the appropriate time.
    struct msg msg;
    msg.mType = 1;
    int sendVal;
    int countProcessSent=0;
    while (countProcessSent < numberProcesses){
         /************ if process Arrived send it to the scheduler  ****************/
        if (processDataTable[countProcessSent].arrivalTime <= getClk()){
            msg.process=processDataTable[countProcessSent];
            sendVal = msgsnd(msgId_GeneratorSchedular,&msg, sizeof(struct msg),!IPC_NOWAIT);
            if(sendVal == -1){
                perror("Error in sending process to the Scheduler");
                exit(-1);
            }
            countProcessSent++;
        }
    }
    printf("Process Generator Finsihed in sending Processes to the Scheduler\n");

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    exit(EXIT_SUCCESS);
}
