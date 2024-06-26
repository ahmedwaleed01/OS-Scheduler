#include "headers.h"


/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();
    
    //TODO it needs to get the remaining time from somewhere
     // Create message queue between scheduler and processes
    key_t key_id2;
    key_id2 = ftok("keyfile", 75);
    int msgId_SchedularProcess = msgget(11111, 0666 | IPC_CREAT);
    if ( msgId_SchedularProcess == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    printf("message queue Id between scheduler and processes %d\n", msgId_SchedularProcess );
    int remainingtime = atoi(argv[1]);
    struct msgbuff msg;
    struct msgBuff1 msg2;
    while (remainingtime > 0)
    {
    
        printf("PROCESS ID: %d time is %d\n",getpid(),getClk());
        msgrcv(msgId_SchedularProcess, &msg2, sizeof(msg2.decrement), getpid(), !IPC_NOWAIT);
        
        remainingtime --;
        printf("message from scheduler type: %d\n",msg2.mType);

        printf("The Remaining Time %d clock time: %d\n",remainingtime,getClk());
        if (remainingtime == 0){
            printf("process %d finished\n",getpid());
            break;
        }

    }
    
    destroyClk(false);
    
    return 0;
}
