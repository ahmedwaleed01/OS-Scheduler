#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    // signal(SIGINT, clearResources);

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
        int result=fscanf(fptr,"%d\t%d\t%d\t%d",&processTable[i].id,&processTable[i].arrivalTime,&processTable[i].runTime,&processTable[i].priority);
         if (result != 4) {
            printf("Error reading data for process %d\n", i);
            continue; // Skip to the next iteration if there was an error
        }
        // printf("%d: %d %d %d %d\n",i,processTable[i].id,processTable[i].arrivalTime,processTable[i].runTime,processTable[i].priority);
    }

    // Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algorithmChosen;
    printf("Choose The Algorithm: \n");
    printf("1. Non-preemptive Highest Priority First (HPF) \n");
    printf("2. Shortest Remaining time Next (SRTN)\n");
    printf("3. Round Robin (RR) \n");
    scanf("%d",algorithmChosen);


    // 3. Initiate and create the scheduler and clock processes.


    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
