#include "headers.h"

void HPF(cvector_vector_type(struct ProcessInfo));
void clearResources(int);
void sigIntHandler(int);

int algo;
cvector_vector_type(struct ProcessInfo) readyQ = NULL;

int main(int argc, char * argv[])
{
    signal(SIGINT, sigIntHandler);
    initClk();
    printf("\n~ Scheduler starting ~\n");
    sscanf(argv[1], "%d", &algo); // Scan choice
    

    while(1){};
    

    /*
    signal(SIGINT, clearResources);

    printf("\n~ Scheduler starting ~\n");
    key_t msgQ_ID = msgget(777, 0644);
    cvector_vector_type(struct ProcessInfo) processVector = NULL;
    

    struct msgbuff_process_vector M;
    int rec_val = msgrcv(msgQ_ID, &M, sizeof(M.PI), 0, !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Errror in recive");
    else{
        printf("\n[Sch] Process Vector has recived.");
        processVector = M.PI;
    }

    // Init clock
    int pid = fork();
    if (pid == -1)
        perror("error in fork");
    else if (pid == 0){
        char *args[] = {"./clk.out", NULL};
        execve(args[0], args, NULL);
    }

    initClk();
    int oldTime = -1;
    while(1)
    {
        int currentTime = getClk();
        if (oldTime != currentTime){
            printf("[Sch]current time is %d\n", currentTime);
            oldTime = currentTime;
        }
            // struct msgbuff_process M;
            // int rec_val = msgrcv(msgQ_ID, &M, sizeof(M.PI), 0, !IPC_NOWAIT);
            // if (rec_val != -1)
            //      printf("\n[Sch] Process #%d has recived.", M.PI.id);
    }

    //TODO implement the scheduler :)
    //upon termination release the clock resources
    
    //destroyClk(true);
    // Test the live

    */
}

void sigIntHandler(int signum){
    // Recive the process from the msg Q
    key_t msgQ_ID = msgget(777, 0644);
    struct msgbuff_process M;
    int rec_val = msgrcv(msgQ_ID, &M, sizeof(M.PI), 0, !IPC_NOWAIT);

    // Push it to the ready Q
    cvector_push_back(readyQ, M.PI);
}

    void HPF(cvector_vector_type(struct ProcessInfo) readyQ)
{
    int Lowestpriority, maxProcessNum, counter;
    int endTime, cpuUtil, Avg;

    //maxProcessNum= cvector_size(msgQ);

    while (counter != maxProcessNum)
    {
        for (int i = 0; i < cvector_size(readyQ); i++)
        {
            if (readyQ[i].priority < Lowestpriority)
                Lowestpriority = readyQ[i].priority;  
          
            int pid = fork();

            if (pid == -1)
                perror("error in fork");
            else if (pid == 0)  
            {
                 char *args[] = {"./process.out", NULL};
                 execve(args[0], args, NULL);
                // exit(getClk());
                counter++;
            }
            else
            {
                int status;
                pid = wait(&status);
                
                if(WIFEXITED(status))
                {
                   endTime= WIFEXITED(status);
                }   
            }
       }
   }
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nScheduler terminating!\n");
    destroyClk(true);
    exit(0);
}