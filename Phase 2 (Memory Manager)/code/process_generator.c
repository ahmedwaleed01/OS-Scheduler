#include "headers.h"

void clearResources(int);
int algorithmChosen;
char quatum[100]="2";
// void callBackFunc(GtkWidget* widget,gpointer data){
//      const char *buttonLabel = (const char*)data;
//     algorithmChosen=atoi(buttonLabel);
//     printf("Algorithm Chosen: %d\n", algorithmChosen);
//     GtkWidget *window = GTK_WIDGET(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW));
//     gtk_widget_destroy(window);
// }
// void entry_changed(GtkEditable *editable, gpointer user_data)
// {
//     const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable)); 
//     g_strlcpy(quatum, text, sizeof(quatum)); 
// }
// void destroy(GtkWidget* widget, gpointer data) 
// { 
//     gtk_main_quit(); 
// } 
  
int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    /************************************
     *******Initilize GUI Window ********
     ***********************************/
    // GtkWidget *window;
    // GtkWidget *vbox_main;
    // GtkWidget *vbox_buttons;
    // GtkWidget *label;
    // GtkWidget *buttonHPF;
    // GtkWidget *buttonSRTN;
    // GtkWidget *buttonRB;
    // GtkWidget *entry; 
    // GtkWidget *labelQ;
    // gtk_init(&argc, &argv);
    // window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // gtk_window_set_title(GTK_WINDOW(window), "Scheduler App");
    // g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);
    // gtk_container_set_border_width(GTK_CONTAINER(window), 50);

    /************************************
    *******Add GUI Components **********
    ***********************************/
    // vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    // gtk_container_add(GTK_CONTAINER(window), vbox_main);
    // label = gtk_label_new("Choose the scheduler algorithm:");
    // gtk_box_pack_start(GTK_BOX(vbox_main), label, FALSE, FALSE, 0);
    // vbox_buttons = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    // gtk_box_pack_start(GTK_BOX(vbox_main), vbox_buttons, FALSE, FALSE, 0);
    // buttonHPF = gtk_button_new_with_label("HPF");
    // buttonSRTN = gtk_button_new_with_label("SRTN");
    // buttonRB = gtk_button_new_with_label("RB");
    // entry = gtk_entry_new();
    // labelQ = gtk_label_new("Enter the Quantum value in Case RB :)");
    // gtk_box_pack_start(GTK_BOX(vbox_main), labelQ, FALSE, FALSE, 0); 
    // gtk_box_pack_start(GTK_BOX(vbox_main), entry, FALSE, FALSE, 0); 
    // g_signal_connect(G_OBJECT(buttonHPF), "clicked", G_CALLBACK(callBackFunc), "1");
    // g_signal_connect(G_OBJECT(buttonSRTN), "clicked", G_CALLBACK(callBackFunc), "2");
    // g_signal_connect(G_OBJECT(buttonRB), "clicked", G_CALLBACK(callBackFunc), "3");
    // g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(entry_changed), NULL);
    // gtk_box_pack_start(GTK_BOX(vbox_buttons), buttonHPF, TRUE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(vbox_buttons), buttonSRTN, TRUE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(vbox_buttons), buttonRB, TRUE, TRUE, 0);

    // gtk_widget_show_all(window);

    // gtk_main();
    printf("Choose Algothrim: \n");
    scanf("%d",&algorithmChosen);

    // Read the input files.
    FILE *fptr;
    fptr = fopen("processes.txt", "r");
    if (fptr == NULL){
        perror("Error Openning File");
        return 1;
    }
    // Count the Number of Processes
    int numberProcesses = CountProcess("processes.txt");

    printf("PROCESSSSSSSSSSSSSSSSSSSSSSSSSS  SSSS %d\n",numberProcesses);

    // Create Process Table
    struct processInputData processTable[numberProcesses];
   for(int i=0; i<numberProcesses; i++){
    fscanf(fptr, "%d\t%d\t%d\t%d\t%d", &processTable[i].id, &processTable[i].arrivalTime, &processTable[i].runTime, &processTable[i].priority, &processTable[i].memSize);
        printf("mem test %d\n", processTable[i].memSize);
        printf("pri test %d\n", processTable[i].priority);
    }


    printf("ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss mem:%d pri:%d  ar:%d\n",processTable[0].memSize,processTable[0].priority,processTable[0].arrivalTime);

    char str_algorithmChosen[10];
    char str_numberProcesses[10];
    sprintf(str_algorithmChosen,"%d",algorithmChosen);
    sprintf(str_numberProcesses,"%d",numberProcesses);

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
        execl(schedularCode,schedularCode,str_algorithmChosen,str_numberProcesses,quatum, NULL);
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
        processDataTable[i].memSize = processTable[i].memSize;
        processDataTable[i].remainingTime=processTable[i].runTime;
        processDataTable[i].finishedTime = 0;
        processDataTable[i].turnAroundTime=0;
        processDataTable[i].waitingTime=0;
        processDataTable[i].weightedTurnAroundTime=0;
        processDataTable[i].PID=0;
        strcpy(processDataTable[i].state,"ready");
        printf("testtttttttttttttttttttttttttttttaaaaaaaaaaaaaaaaaaaa %d %d\n",processDataTable[i].memSize,processDataTable[i].priority);
    }
    // Create message queue between process generator and scheduler
    key_t key_id;
    key_id = ftok("keyfile", 60);
    int msgId_GeneratorSchedular = msgget(55555, 0666 | IPC_CREAT);
    if (msgId_GeneratorSchedular == -1)
    {
        perror("Error in create message queue:)");
        exit(-1);
    }
    printf("message queue Id between process generator and scheduler %d\n", msgId_GeneratorSchedular );
    // Send the information to the scheduler at the appropriate time.
    struct msgbuff msg;
    msg.mType = 2;
    int sendVal;
    int countProcessSent=0;
    int time=processDataTable[0].arrivalTime;
    while (countProcessSent < numberProcesses){             
         /************ if process Arrived send it to the scheduler  ****************/
        if(time <= getClk()) {
            while (processDataTable[countProcessSent].arrivalTime == time){
                msg.process=processDataTable[countProcessSent];
                printf("henaaaaaaaaaaaaaaaaaaaa %d %d %d %d\n",processDataTable[countProcessSent].memSize, processDataTable[countProcessSent].priority,countProcessSent,numberProcesses);
                printf("process :%d is sent %d\n",msg.process.id,msg.process.memSize);
                sendVal = msgsnd(msgId_GeneratorSchedular,&msg, sizeof(msg.process),!IPC_NOWAIT);
                if(sendVal == -1){
                    perror("Error in sending process to the Scheduler");
                    exit(-1);
                }
                countProcessSent++;
                if(countProcessSent>=numberProcesses) break;
            }
            time++;
        }
    }
    printf("Process Generator Finsihed in sending Processes to the Scheduler\n");
    while(1);
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    exit(EXIT_SUCCESS);
}