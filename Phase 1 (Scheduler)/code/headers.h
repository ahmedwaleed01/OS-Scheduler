#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300


//don't mess with this variable//
int * shmaddr;                 //
//===============================

int getClk()
{
    return *shmaddr;
}

/*****************************************
************ Common Functions*************
*
******************************************/
/**** Count Number of Processes *****/
int CountProcess (char* fileName){
    FILE* fptr1;
    fptr1 = fopen("processes.txt", "r");
    if (fptr1 == NULL){
        perror("Error Openning File");
        return 1;
    };
    char line [10];
    int numberProcesses=0;
    while (fgets(line, sizeof(line),fptr1) != NULL){;
        numberProcesses++;
    };
    fclose(fptr1);
    return numberProcesses;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}

/*
  Create Data Structure for processes
*/
/*************************** Process Definition ********************************/
struct processInputData{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
};
struct processData{
    int id;
    int PID;
    int runTime;
    int arrivalTime;
    int priority;
    int remainingTime;
    int turnAroundTime;
    int finishedTime;
    int waitingTime;
    int weightedTurnAroundTime;
    char state[10];
};
/************************ Node Definition ***************************************/
struct Node{
    struct processData*process;
    struct Node*next;
};
/***********************Generate Node of process ********************************/
struct Node* createNode(struct processData* newProcess){
    struct Node*nodeCreated=malloc(sizeof(struct Node));
    nodeCreated->process= malloc(sizeof(struct processData));
    nodeCreated->process->id=newProcess->id;
    nodeCreated->process->runTime=newProcess->runTime;
    nodeCreated->process->arrivalTime=newProcess->arrivalTime;
    nodeCreated->process->priority=newProcess->priority;
    nodeCreated->process->remainingTime=newProcess->remainingTime;
    nodeCreated->process->turnAroundTime=newProcess->turnAroundTime;
    nodeCreated->process->finishedTime=newProcess->finishedTime;
    nodeCreated->process->waitingTime=newProcess->waitingTime;
    nodeCreated->process->weightedTurnAroundTime=newProcess->weightedTurnAroundTime;
    nodeCreated->process->PID=newProcess->PID;
    nodeCreated->next = NULL;
    strcpy( nodeCreated->process->state,newProcess->state);
    return nodeCreated;
};
/***************************** List ***********************************************/
struct List{
    struct Node*head;
    int size;
};
/***************************** PriorityQueue **************************************/
struct List *createPriorityQueue(){
    struct List*priorityQueue = malloc(sizeof(struct List));
    priorityQueue->head=NULL;
    priorityQueue->size=0;
    return priorityQueue;
};
void enqueue(struct List *queue, struct processData* newProcess){
    struct Node*newNode =createNode(newProcess);
    if(queue->head == NULL){
        queue->head=newNode;
        queue->size++;
        return;
    }
    if(queue->head->process->priority > newProcess->priority){
        struct Node*temp=queue->head;
        queue->head=newNode;
        newNode->next=temp;
        queue->size++;
        return;
    }
    if(queue->size == 1){
        if(queue->head->process->priority <= newProcess->priority){
           queue->head->next = newNode; 
        }else{
            struct Node*temp=queue->head;
            queue->head=newNode;
            newNode->next=temp;
        }
        queue->size++;
        return;
    }
    struct Node*prevNode=queue->head,*nextNode=queue->head->next;
    for(int i=0;i < queue->size -1 ; i++){
        if ( newProcess->priority >= nextNode->process->priority ){
           prevNode = prevNode->next;
           nextNode = nextNode->next;
        }else{
            break;
        }
    }
    prevNode->next = newNode;
    newNode->next = nextNode;
    queue->size++;
};

struct processData *dequeue(struct List *queue){
    if (queue == NULL || queue->head == NULL){
        return NULL;
    }
    struct Node *firstNode = queue->head;
    queue->head = firstNode->next;

    struct processData *processFirstNode = firstNode->process;
    free(firstNode);
    queue->size--;
    return processFirstNode;
}

void printList(struct List *list)
{
    if (list == NULL){
        printf("List is not initialized.\n");
        return;
    }
    if (list->size==0){
        printf("List is empty.\n");
        return;
    }
    printf("List Contents:\n");
    struct Node *nodeTemp = list->head;
    while (nodeTemp != NULL)
    {
        struct processData *proc = nodeTemp->process;
        printf("Process ID: %d, Arrival Time: %d, Running Time: %d, Priority: %d, Remaining Time: %d\n",
               proc->id, proc->arrivalTime, proc->runTime, proc->priority, proc->remainingTime);
        nodeTemp = nodeTemp->next;
    }
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}