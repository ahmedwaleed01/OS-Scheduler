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

    while (remainingtime > 0)
    {
        sleep(1);
        remainingtime --;
        if (remainingtime == 0){
            printf("process %d finished\n",getpid());
            msg.mType=getpid();
            // int sendVal = msgsnd(msgId_SchedularProcess,&msg, sizeof(msg.process),!IPC_NOWAIT);
            // if(sendVal == -1){
            //     perror("Error in sending process to the Scheduler");
            //     exit(-1);
            // }
            break;
        }

    }
    
    destroyClk(false);
    
    return 0;
}
