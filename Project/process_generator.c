#include "headers.h"
#include <string.h>

void clearResources(int);
void createClk();
void readInputFile();

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    printf("\n~ Process Generator starting ~\n");
    // 1. Read the input files.
    readInputFile();
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.

    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if(pid == -1)
        perror("error in fork");
    else if(pid == 0)
        createClk();
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

void createClk() {
    char *args[] = {"./clk.out", NULL};
    execve(args[0], args, NULL);
}

void readInputFile(){
    FILE *fptr;
    // Handle not found
    if ((fptr = fopen("./processes.txt", "r")) == NULL)
    {
        printf("\nThe file does not exist\n");
        exit(0);
    }

    printf("\nReading the input file...\n");
    struct Queue *Process_q = createQueue();
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

            enQueue(Process_q, &p);
            printf("\nQueue Front : %d \n", (Process_q->front->key)->id);
            printf("\nQueue Rear : %d\n", (Process_q->rear->key)->id);
        }
    }
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nProcess Generator terminating!\n");
    exit(0);
}
