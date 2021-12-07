#include "headers.h"
#include <string.h>

void clearResources(int);
void createClk();
cvector_vector_type(struct ProcessInfo) readInputFile();

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    printf("\n~ Process Generator starting ~\n");
    // 1. Read the input files.
    cvector_vector_type(struct ProcessInfo) processVector = NULL;
    processVector = readInputFile();

    if (processVector)
    {
        for (int i = 0; i < cvector_size(processVector); i++)
        {
            printf("\nFor process #%d\n", i + 1);
            printf("\nProcess ID = %d\n", processVector[i].id);
            printf("Process AT = %d\n", processVector[i].arrival_time);
            printf("Process RT = %d\n", processVector[i].runtime);
            printf("Process P = %d\n", processVector[i].priority);
        }
    }
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
    return processVector;
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nProcess Generator terminating!\n");
    exit(0);
}
