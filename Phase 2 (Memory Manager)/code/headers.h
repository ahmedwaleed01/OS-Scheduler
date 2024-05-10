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
#include <cairo.h>
#include <math.h>
#include <sys/sem.h>
#include <limits.h>
#include <cairo.h>
typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MEM_SIZE 1024


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
    char line [1000];
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
    int memSize;
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
    float weightedTurnAroundTime;
    char state[10];
    int quantum;
    int startTime;
    int memSize;
    int startMem;
    int endMem;
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
    nodeCreated->process->quantum=newProcess->quantum;
    nodeCreated->process->memSize=newProcess->memSize;
    nodeCreated->process->startMem=newProcess->startMem;
    nodeCreated->process->endMem=newProcess->endMem;
    nodeCreated->next = NULL;
    strcpy( nodeCreated->process->state,newProcess->state);
    return nodeCreated;
};
/******************************** List ********************************************/
struct List{
    struct Node*head;
    int size;
    bool remainTime;
};
/***************************** PriorityQueue **************************************/
struct List *createPriorityQueue(){
    struct List*priorityQueue = malloc(sizeof(struct List));
    priorityQueue->head=NULL;
    priorityQueue->size=0;
    priorityQueue->remainTime=false;
    return priorityQueue;
};
struct List *createQueuePirorityRemainTime(){
    struct List*priorityQueue = malloc(sizeof(struct List));
    priorityQueue->head=NULL;
    priorityQueue->size=0;
    priorityQueue->remainTime=true;
    return priorityQueue;
};
struct List *createLinkedList(){
    struct List*LinkedList = malloc(sizeof(struct List));
    LinkedList->head=NULL;
    LinkedList->size=0;
    LinkedList->remainTime=false;
    return LinkedList;
};
void Insert (struct List *linkedList, struct processData* newProcess){
    struct Node*newNode =createNode(newProcess);
    if(linkedList->head == NULL){
        linkedList->head=newNode;
        linkedList->size++;
        return;
    }
    struct Node * ptr = linkedList->head;
    while (ptr->next !=NULL){
        ptr=ptr->next;
    }
    ptr->next = newNode;
    newNode->next = NULL;
    linkedList->size++;
}

void enqueue(struct List *queue, struct processData* newProcess){
    struct Node*newNode =createNode(newProcess);
    if(queue->head == NULL){
        queue->head=newNode;
        queue->size++;
        return;
    }
    if ( (queue->remainTime == false && queue->head->process->priority > newProcess->priority) || 
        (queue->remainTime == true && queue->head->process->remainingTime > newProcess->remainingTime)
    ){
        struct Node*temp=queue->head;
        queue->head=newNode;
        newNode->next=temp;
        queue->size++;
        return;
    }

    if(queue->size == 1){
        if( (queue->remainTime == false && queue->head->process->priority <= newProcess->priority) ||
        (queue->remainTime == true && queue->head->process->remainingTime <= newProcess->remainingTime)){
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
        if ( (queue->remainTime == false && newProcess->priority >= nextNode->process->priority)|| 
            (queue->remainTime == true && newProcess->remainingTime >= nextNode->process->remainingTime)){
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

bool isEmpty(struct List *list){
    if (list->head == NULL) return true;
    else return false;
}

void draw_list(cairo_t *cr, struct List *list) {
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Draw list contents
    struct Node *nodeTemp = list->head;
    int y = 20; // Starting y position
    while (nodeTemp != NULL) {
        struct processData *proc = nodeTemp->process;
        char text[1000];
        sprintf(text, "Process ID: %d, Arrival Time: %d, Running Time: %d, Priority: %d, Remaining Time: %d , Finished Time: %d, state: %s",
               proc->id, proc->arrivalTime, proc->runTime, proc->priority, proc->remainingTime, proc->finishedTime, proc->state);
        cairo_set_source_rgb(cr, 0, 0, 0); // Set color to black
        cairo_move_to(cr, 10, y); // Move to position
        cairo_show_text(cr, text); // Draw text
        y += 20; // Increment y position
        nodeTemp = nodeTemp->next;
    }
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
        printf("Process ID: %d, Arrival Time: %d, Running Time: %d, Priority: %d, Remaining Time: %d , Finished Time: %d, state: %s\n",
               proc->id, proc->arrivalTime, proc->runTime, proc->priority, proc->remainingTime,proc->finishedTime,proc->state);
        nodeTemp = nodeTemp->next;
    }
}

void printProcessInfo(struct processData*process){
    printf("Process id:%d Arrival Time:%d Run Time: %d Remain Time:%d Priority: %d Finished Time: %d PID: %d Waiting Time:%d State:%s Quantum Remaining:%d Memory:%d\n",
    process->id,process->arrivalTime,process->runTime,process->remainingTime,process->priority,process->finishedTime,process->PID,
    process->waitingTime,process->state,process->quantum,process->memSize);
}

/********************
 * 
 * Message Structure
 * 
*********************/
struct msgbuff {
    int mType;
    struct processData process;
};

struct msgBuff1 {
    int mType;
    int decrement;
};

/* arg for semctl system calls. */
union Semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
};

void down(int sem)
{
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

int up(int sem)
{
    struct sembuf op;

    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
    return 1;
}

/***************************** Memory Allocation **************************************/
struct memoryNode
{
    int size;
    int pid;
    bool emptyNode;
    bool hasProcess;
    bool hasChild;
    bool isRoot;
    int startPosition;
    int endPosition;
    struct memoryNode *parent;
    struct memoryNode *left;
    struct memoryNode *right;
};

struct memoryTree
{
    struct memoryNode *root;
};

struct memoryNode *createMemoryNode(int size, int startPosition, struct memoryNode *parent)
{
    struct memoryNode *node = (struct memoryNode *)malloc(sizeof(struct memoryNode));
    if (node == NULL)
    {
        return NULL;
    }

    node->size = size;
    node->pid = -1; // no process yet is created
    node->emptyNode = true;
    node->hasProcess = false;
    node->hasChild = false;
    if (parent == NULL)
    {
        node->isRoot = true;
    }
    else
    {
        node->isRoot = false;
    }
    node->startPosition = startPosition;
    node->endPosition = startPosition + size - 1;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;

    return node;
}
struct memoryTree *createMemoryTree()
{
    struct memoryTree *tree = (struct memoryTree *)malloc(sizeof(struct memoryTree));
    if (tree == NULL)
    {
        // for memory allocation failure
        return NULL;
    }
    // used create node function to create the node
    tree->root = createMemoryNode(MEM_SIZE, 0, NULL);

    // this if for the case of failure to create node then memory previously allocated for the tree itself is no longer needed bec there is no root
    if (tree->root == NULL)
    {
        // for memory allocation failure
        free(tree);
        return NULL;
    }
    return tree;
}

struct memoryNode *findProcessNode(struct memoryNode *current, int targetPid)
{
    // check if the current node is null
    if (current == NULL)
    {
        return NULL;
    }

    // returning this node if it has the matching process ID
    if (current->pid == targetPid)
    {
        return current;
    }

    // traverse through the left subtree
    struct memoryNode *foundInLeft = findProcessNode(current->left, targetPid);
    if (foundInLeft != NULL)
    {
        return foundInLeft;
    }

    // traverse through the right subtree
    return findProcessNode(current->right, targetPid);
}

struct memoryNode *findSuitableSize(struct memoryNode *current, int requiredSize)
{
    // i am here checking if the current code is equal to null or if the required size to be allocated is smaller than the current size of the node
    if (current == NULL || current->size < requiredSize)
    {
        return NULL;
    }

    struct memoryNode *leftNode = findSuitableSize(current->left, requiredSize);
    struct memoryNode *rightNode = findSuitableSize(current->right, requiredSize);

    // i am here checking if the node exists and then i check if the size of this node is greater than the required size and if this current node is empty
    int leftSize = (leftNode != NULL && leftNode->size >= requiredSize && leftNode->emptyNode && leftNode->left == NULL && leftNode->right == NULL) ? leftNode->size : INT_MAX;
    int rightSize = (rightNode != NULL && rightNode->size >= requiredSize && rightNode->emptyNode && rightNode->left == NULL && rightNode->right == NULL) ? rightNode->size : INT_MAX;

    if (leftSize <= rightSize && leftSize < current->size)
    {
        return leftNode;
    }
    else if (rightSize < leftSize && rightSize < current->size)
    {
        return rightNode;
    }
    if (current->hasProcess == 1)
    {
        return NULL;
    }
    else
    {
        return current;
    }
}

struct memoryNode *findClosestSize(struct memoryTree *tree, int size)
{
    float log2Size = log2(size);
    int wantedSize = pow(2, ceil(log2Size));
    if (size <= 8)
    {
        wantedSize = 8;
    }
    printf("WANTED SIZE %d \n", wantedSize);
    struct memoryNode *foundNode = findSuitableSize(tree->root, wantedSize);
    if(foundNode== NULL){
        return NULL;
    }
    if (foundNode == tree->root && foundNode->hasChild == true)
    {
        return NULL;
    }
    while (foundNode->size != wantedSize)
    {
        printf("SIZE OF FOUND NODE %d \n", foundNode->size);
        struct memoryNode *leftNode = createMemoryNode(foundNode->size / 2, foundNode->startPosition, foundNode);
        struct memoryNode *rightNode = createMemoryNode(foundNode->size / 2, foundNode->startPosition + foundNode->size / 2, foundNode);
        leftNode->emptyNode = true;
        rightNode->emptyNode = true;
        foundNode->left = leftNode;
        foundNode->right = rightNode;
        foundNode->hasChild = true;

        foundNode = findSuitableSize(tree->root, wantedSize);
    }

    return foundNode;
}

void printTree(struct memoryNode *root)
{
    if (root == NULL)
        return;

    printTree(root->left); // traverse through the Left Part
    printf("Size: %d, PID: %d, Start: %d, End: %d, HAS PROCESS: %d\n", root->size, root->pid, root->startPosition, root->startPosition + root->size, root->hasProcess);
    printTree(root->right); // traverse through the right part after finishing all the left part
}

int *allocateProcess(struct memoryTree *tree, int processSize, int pid)
{
    struct memoryNode *foundNode = findClosestSize(tree, processSize);
    int *positions = (int *)malloc(sizeof(int));

    if (foundNode == NULL)
    {
        positions[0] = -1;
        positions[1] = -1;
        return positions;
    }
    foundNode->pid = pid;
    foundNode->emptyNode = false;
    foundNode->hasProcess = true;
    positions[0] = foundNode->startPosition;
    positions[1] = foundNode->endPosition;
    printTree(tree->root);
    return positions;
}

void deleteChildren(struct memoryNode *node)
{
    if (node->left)
    {
        free(node->left);
        node->left = NULL;
    }
    if (node->right)
    {
        free(node->right);
        node->right = NULL;
    }
    node->hasChild = false;
    node->emptyNode = true;
}

void mergeMemory(struct memoryNode *node)
{
    struct memoryNode *currentNode = node;

    // while (currentNode)
    // {
     
    if (currentNode->left && !currentNode->left->hasChild && !currentNode->left->hasProcess && currentNode->right && !currentNode->right->hasChild && !currentNode->right->hasProcess)
    {
        printf("ANA HENA\n");
        deleteChildren(currentNode);
    }

    if (!currentNode->isRoot)
        currentNode = currentNode->parent;
    else
        return;
    // }
}

int deallocateProcess(struct memoryTree *tree, int pid)
{
   
    struct memoryNode *foundNode = findProcessNode(tree->root, pid);
    
    if (foundNode == NULL)
        return -1;

    foundNode->pid = -1;
    foundNode->emptyNode = true;
    foundNode->hasProcess = false;

    if(foundNode->parent!=NULL){
        mergeMemory(foundNode->parent);
    }else{
        mergeMemory(foundNode);
    }
    return 1;
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
