#include "headers.h"
#include <string.h>

void clearResources(int);
void handleAlarm(int);
void createClk();
void createSch();
cvector_vector_type(struct ProcessInfo) readInputFile();

pid_t clk_pid, sch_pid;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    printf("\n~ Process Generator starting ~\n");

    // 1. Read the input files.
    cvector_vector_type(struct ProcessInfo) processVector = NULL;
    processVector = readInputFile();
    int numberOfProcesses = cvector_size(processVector);
    // send it through execl
    char numberOfProcesses_str[8];
    sprintf(numberOfProcesses_str, "%d", numberOfProcesses);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("\nPlease choose the algorithm you want:\n");
    printf("1 => Non-preemptive Highest Priority First (HPF)\n");
    printf("2 => Shortest Remaining time Next (SRTN)\n");
    printf("3 => Round Robin (RR)\n");
    printf("Other => Quit\n");
    printf("Choice: ");
    // Not int but will be coonverted "to send in execl"
    char choice[8];
    char Q[8]; // RR Quantum

    fgets(choice, 8, stdin);
    //scanf("%d", &choice);
    if(atoi(choice) == 1)
    {
        // HPF
        printf("\nStarting HPF Algorithm\n");

    }
    else if(atoi(choice) == 2)
    {
        // SRTN
        printf("\nStarting SRTN Algorithm\n");
    }
    else if(atoi(choice) == 3)
    {
        // RR
        printf("Quantum: ");
        fgets(Q, 8, stdin);

        printf("\nStarting RR Algorithm with Q = %d\n", atoi(Q));
    }
    else
        exit(0);

    // 3. Creating the clock and scheduler
    clk_pid = fork();
    if(clk_pid == 0){
        printf("\n[PG] Starting the clock\n");
        execl("./clk.out", "./clk.out", NULL);
    } 
    else{
        sch_pid = fork();
        if(sch_pid == 0){
            // not RR
            printf("\n[PG] Starting the scheduler\n");
            if(atoi(choice) != 3)
                execl("./scheduler.out", "./scheduler.out", choice, numberOfProcesses_str, NULL);
            else
                execl("./scheduler.out", "./scheduler.out", choice, numberOfProcesses_str, Q, NULL);
        }
        else{
            // Main Parent
            key_t msgQ_ID = msgget(777, IPC_CREAT | 0644);
            initClk();
            bool end_flag = 0;
            while(1){
                if(!end_flag){
                    int currentStep = getClk();
                    for (int i = 0; i < cvector_size(processVector); i++){
                        if (processVector[i].arrival_time <= currentStep)
                        {
                            // Arrived ;)
                            struct msgbuff_process M;
                            M.mtype = processVector[i].id;
                            M.PI = processVector[i];
                            int send_val = msgsnd(msgQ_ID, &M, sizeof(M.PI), !IPC_NOWAIT);
                            if (send_val == -1)
                                perror("Errror in send");
                            else{
                                //printf("\nProcess #%d sent to msgQ at t=%d\n", processVector[i].id, currentStep);
                                kill(sch_pid, SIGINT);
                                cvector_erase(processVector, i);
                            }
                        }
                    }
                    if (cvector_size(processVector) == 0){
                        end_flag = 1;
                        //printf("\nAll process sent to the scheduler\n");
                    }
                    sleep(0.1f);
                }
                sleep(1);
            }
        }
    }
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
                        p.remaining_time = atoi(token);
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
            p.sys_pid = -1;
            p.waiting_time = -1;
            cvector_push_back(processVector, p);
        }
    }
    printf("\nFile imported successfully!\n");
    return processVector;

    fclose(fptr);
}

void handleAlarm(int signum){
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nProcess Generator terminating!\n");
    msgctl(msgget(777, 0644), IPC_RMID, NULL);
    msgctl(msgget(666, 0644), IPC_RMID, NULL);
    kill(clk_pid, SIGINT);
    kill(sch_pid, SIGKILL);
    exit(0);
}
