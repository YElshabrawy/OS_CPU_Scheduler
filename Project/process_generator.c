#include "headers.h"
#include <string.h>

void clearResources(int);
void createClk();
void createSch();
cvector_vector_type(struct ProcessInfo) readInputFile();

struct msgbuff_process{
    long mtype;
    struct ProcessInfo PI;
};

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    printf("\n~ Process Generator starting ~\n");

    // 1. Read the input files.
    cvector_vector_type(struct ProcessInfo) processVector = NULL;
    processVector = readInputFile();

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("\nPlease choose the algorithm you want:\n");
    printf("1 => Non-preemptive Highest Priority First (HPF)\n");
    printf("2 => Shortest Remaining time Next (SRTN)\n");
    printf("3 => Round Robin (RR)\n");
    printf("Other => Quit\n");
    printf("Choice: ");
    int choice;
    scanf("%d", &choice);
    if(choice == 1)
    {
        // HPF
        printf("\nStarting HPF Algorithm\n");

    }
    else if(choice == 2)
    {
        // SRTN
        printf("\nStarting SRTN Algorithm\n");
    }
    else if(choice == 3)
    {
        // RR
        printf("Quantum: ");
        int Q;
        if(!scanf("%d", &Q))
            exit(0);

        printf("\nStarting RR Algorithm with Q = %d\n", Q);
    }
    else
        exit(0);


    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if(pid == -1)
        perror("error in fork");
    else if(pid == 0)
        createClk();

    int pid2 = fork();
    if (pid2 == -1)
        perror("error in fork");
    else if (pid2 == 0)
        createSch();

    initClk();
    int oldx = -1; // to track seconds
    while(1){
        int x = getClk();
        //if(oldx != x){
            // A second passed
            //printf("current time is %d, size = %ld\n", x, cvector_size(processVector));
            // check if a process has arrived
            if(processVector){
                for (int i = 0; i < cvector_size(processVector); i++)
                {
                    if (processVector[i].arrival_time == x){
                        // send it in a msg Q
                        key_t msgQ_ID = msgget(777, IPC_CREAT | 0644);
                        struct msgbuff_process M;
                        M.mtype = processVector[i].id;
                        M.PI = processVector[i];

                        int send_val = msgsnd(msgQ_ID, &M, sizeof(M.PI), !IPC_NOWAIT);
                        if(send_val == -1)
                            perror("Errror in send");
                        


                        printf("Process %d arrived\n", processVector[i].id);
                        cvector_erase(processVector, i);
                        printf("The size =%ld\n", cvector_size(processVector));
                    }
                }
            }
            //oldx = x;

        //}
    }

    destroyClk(true);
    
    /*
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
    */
}

void createClk() {
    char *args[] = {"./clk.out", NULL};
    execve(args[0], args, NULL);
}

void createSch()
{
    char *args[] = {"./scheduler.out", NULL};
    execve(args[0], args, NULL);
}

cvector_vector_type(struct ProcessInfo) readInputFile()
{
    FILE *fptr;
    // Handle not found
    if ((fptr = fopen("./processes.txt", "r")) == NULL)
    {
        printf("\nThe file does not exist\n");
        exit(0);
    }

    printf("\nReading the input file...\n");
    cvector_vector_type(struct ProcessInfo) processVector = NULL;

    char line[256];
    while (fgets(line, sizeof(line), fptr))
    {
        if(line[0] != '#'){
            char* token;
            char* rest = line;
            int i = 1;
            struct ProcessInfo p;

            while ((token = strtok_r(rest, "\t", &rest)))
            {
                switch (i)
                {
                    case 1:
                        {
                        p.id = atoi(token);
                        break;
                        }
                    
                    case 2:
                    {
                        p.arrival_time = atoi(token);
                        break;
                    }

                    case 3:
                    {
                        p.runtime = atoi(token);
                        break;
                    }
                    
                    case 4:
                    {
                        // Insert into Data Structure
                        p.priority = atoi(token);
                        break;
                    }
                    
                    default:
                        break;
                }
                i++;
            }
            cvector_push_back(processVector, p);
        }
    }
    printf("\nFile imported successfully!\n");
    return processVector;
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nProcess Generator terminating!\n");
    exit(0);
}
