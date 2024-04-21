#include "headers.h"
int msgId_GeneratorSchedular, recVal_GeneratorSchedular, msgId_SchedularProcess,sendVal_SchedularProcess;


void HPF(){
    
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
    msgId_SchedularProcess = msgget(key_id2, 0666 | IPC_CREAT);
    if ( msgId_SchedularProcess == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    printf("message queue Id between process generator and scheduler %d\n", msgId_GeneratorSchedular );
    printf("message queue Id between scheduler and processes %d\n", msgId_SchedularProcess );

    int schedulerAlgorithm = atoi(argv[1]);
    
    if (schedulerAlgorithm == 1){
        // call HPF
        printf("HPF ALGO\n");
    }else if (schedulerAlgorithm == 2){
        //call SRTN
         printf("SRTN ALGO\n");
    }else if (schedulerAlgorithm == 3){
        //call RB
         printf("RB ALGO\n");
    }
     while (1) {
        printf("clock time : %d\n",getClk());
        sleep(2);
    }

    // msgrcv(msgID, &msg, sizeof(msg.mprocess), 0, IPC_NOWAIT) != -1

    
    
    destroyClk(true);
}
