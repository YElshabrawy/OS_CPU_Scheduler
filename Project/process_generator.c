#include "headers.h"

void clearResources(int);
void createClk();

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    printf("\n~ Process Generator starting ~\n");
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.

    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if(pid == -1)
        perror("error in fork");
    else if(pid == 0)
        createClk();
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    while(1){
        sleep(1);
        // To get time use this
        int x = getClk();
        printf("current time is %d\n", x);
    }
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

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("Process Generator terminating!\n");
    exit(0);
}
