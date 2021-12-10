#include "headers.h"
#include <math.h>

void HPF();
void HPF2();
void SRTN();
void SRTN2();
void RR(int);
void clearResources(int);
void sigIntHandler(int);
void sigChlidHandler(int);

int algo;
int remainingProcesses = -1; // to indicate when the scheduler will terminate if == 0

//Round Robin
cvector_vector_type(struct ProcessInfo) RR_Array = NULL;
int processStartTime = -1; // To help in RR
int RR_Process_index = 0; // To switch between processes
int RR_finish_flag = 0;

int stopSchFlag = 0; // Stops the scheduler at end
int runningFlag = 0; // Indicates if its currently running or not;
int pFlag = 0;

//SRTN
int preFlag = 0;

struct ProcessInfo currentProcess;
int index_currentProcess;

cvector_vector_type(struct ProcessInfo) readyQ = NULL;
table* PCB;

FILE* logsFile;
FILE* prefFile;

// Final acc vars
int total_runtime = 0;
float total_w_turnatound = 0.0f;
int total_wait = 0;
cvector_vector_type(float) WTA_vector = NULL;

int main(int argc, char * argv[])
{
    signal(SIGINT, sigIntHandler);
    signal(SIGCHLD, sigChlidHandler);

    // Handle not to call sigchild on stop
    struct sigaction act;
    act.sa_handler = sigChlidHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &act, 0) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    logsFile = fopen("./Output/Scheduler.log", "w");
    prefFile = fopen("./Output/Scheduler.pref", "w");

    initClk();
    printf("\n~ Scheduler starting ~\n");
    fputs("#At\ttime\tx\tprocess\ty\tstate\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n", logsFile);
    // Get the number of processes:
    int numOfProcesses = 0;
    sscanf(argv[2], "%d", &numOfProcesses);
    //printf("\n[SCH] num of p = %d\n", numOfProcesses);

    remainingProcesses = numOfProcesses;

    // Create PCB
    PCB = createTable(numOfProcesses);

    sscanf(argv[1], "%d", &algo); // Scan choice

    if(algo == 1)
        HPF2();
    else if(algo == 2)
        SRTN2();
    else if(algo == 3){
        int Q;
        sscanf(argv[3], "%d", &Q);
        RR(Q);
    }

    // Calculate the pref data :3
    float CPU_util = (float)total_runtime / (float)getClk() * 100.0f;
    float AVG_WTA = (float)total_w_turnatound / (float)numOfProcesses;
    float AVG_W = (float)total_wait / (float)numOfProcesses;
    // AVG WTA is considered the mean here
    float total_SD = 0.0f;
    for(int i=0 ; i < cvector_size(WTA_vector); i++){
        total_SD += pow(WTA_vector[i] - AVG_WTA, 2);
    }
    float STD_WTA = sqrt((float)total_SD / (float)numOfProcesses);

    char prefLine[128];
    sprintf(prefLine,
          "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f",
          CPU_util, AVG_WTA, AVG_W, STD_WTA);
    fputs(prefLine, prefFile);

    fclose(prefFile);
    fclose(logsFile);

    kill(getppid(), SIGINT);
}

void SRTN()
{
    printf("\n== Starting SRTN work ==\n");
    //PCB_entry* currentPCB;
    int initialTime;
    while(1){
        sleep(0.1f);
        if (cvector_empty(readyQ)){
            if(stopSchFlag == 1 && !runningFlag)
                break;
            else
                continue;
        }
        else{
            // There are processes in the system
            if(!runningFlag){
                // Ready to run
                runningFlag = 1; // Am busy right now

                // 1. Get the shortest remaining time from readyQ
                int index = 0;
                currentProcess = readyQ[0];
                for(int i = 0; i< cvector_size(readyQ); i++){
                    if (readyQ[i].remaining_time < currentProcess.remaining_time){
                        currentProcess = readyQ[i];
                        index = i;
                    }
                }
                // Pop it
                cvector_erase(readyQ, index);


                // 2. Get its status from the PCB
                //currentPCB = lookup(PCB, currentProcess.id);

                // 3. Run the process
                if(currentProcess.sys_pid == -1){
                    // Has not been touched yet
                        //currentPCB->state = RESUMED;
                        initialTime = getClk();
                    pid_t process_pid = fork();
                    if(process_pid == 0){
                        char PID[8];
                        char Runtime[8];
                        sprintf(PID, "%d", currentProcess.id);
                        sprintf(Runtime, "%d", currentProcess.runtime);
                        execl("./process.out", "./process.out", PID, Runtime, NULL);
                    }
                    currentProcess.sys_pid = process_pid;
                }
                else{
                    // Exists and we will resume it
                    //printf("\n#Exists and we will resume it#\n");
                    kill(currentProcess.sys_pid, SIGCONT);
                }
                sleep(1);
                kill(currentProcess.sys_pid, SIGSTOP);
                currentProcess.remaining_time -= (getClk() - initialTime);
                runningFlag = 0;
                cvector_push_back(readyQ, currentProcess);

                // printf("\nNum of items = %ld", cvector_size(readyQ));
                // for (int i = 0; i < cvector_size(readyQ); i++)
                // {
                //     printf("\nP%d has rt= %d", readyQ[i].id, readyQ[i].remaining_time);
                // }

            }
            else{
                // Busy running
                //printf("\n#Busy running#\n");
                // kill(currentProcess.sys_pid, SIGSTOP);
                // currentProcess.remaining_time -= (getClk() - initialTime);
                // //currentPCB->remaining_time -= (getClk() - initialTime);
                // runningFlag = 0;
                // cvector_push_back(readyQ, currentProcess);
            }
        }
    }
}

void SRTN2(){
    printf("\n== Starting SRTN work ==\n");
    while(1){
        sleep(0.1f);
        if(cvector_empty(readyQ))
        {
            // Handle
            stopSchFlag = (remainingProcesses == 0);
            if (stopSchFlag && !runningFlag)
                break;
            else
                continue;
        }
        else
        {
            if(processStartTime == -1){
                // ready to run
                currentProcess = readyQ[0];
                index_currentProcess = 0;
                // Get the srt process
                for(int i = 0; i < cvector_size(readyQ); i++){
                    if(readyQ[i].remaining_time <= currentProcess.remaining_time){
                        currentProcess = readyQ[i];
                        index_currentProcess = i;
                    }
                } 
                
                // pop it
                cvector_erase(readyQ, index_currentProcess);

                if(currentProcess.sys_pid == -1){
                    // Has not forked yet
                    processStartTime = getClk();

                    if(currentProcess.waiting_time == -1){
                         // hasnt run b4
                        currentProcess.waiting_time = getClk() - currentProcess.arrival_time;

                        // Log it
                        char logLine[128];
                        sprintf(
                            logLine,
                            "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                            getClk(), currentProcess.id, "started", currentProcess.arrival_time, currentProcess.runtime,
                            currentProcess.remaining_time, currentProcess.waiting_time
                        );
                        fputs(logLine, logsFile);

                        // Fork it
                        pid_t process_pid = fork();
                        if(process_pid == 0){
                            char PID[8];
                            char Runtime[8];
                            sprintf(PID, "%d", currentProcess.id);
                            sprintf(Runtime, "%d", currentProcess.runtime);
                            execl("./process.out", "./process.out", PID, Runtime, NULL);
                        }

                        currentProcess.sys_pid = process_pid;
                    }
                }
                else
                {
                    // Forked but resumed
                    processStartTime = getClk();

                    char logLine[128];
                    sprintf(
                        logLine,
                        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                        getClk(), currentProcess.id, "resumed", currentProcess.arrival_time, currentProcess.runtime,
                        currentProcess.remaining_time, currentProcess.waiting_time
                    );
                    fputs(logLine, logsFile);

                    kill(currentProcess.sys_pid, SIGCONT);
                }
            
            }
            else
            {
                // Stopping
                if(getClk() >= processStartTime + 1){
                    kill(currentProcess.sys_pid, SIGSTOP);
                    processStartTime = -1;

                    //currentProcess.remaining_time -= Q;
                    currentProcess.remaining_time -= 1;

                    char logLine[128];
                    sprintf(
                        logLine,
                        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                        getClk(), currentProcess.id, "stopped", currentProcess.arrival_time, currentProcess.runtime,
                            currentProcess.remaining_time, currentProcess.waiting_time
                        );
                        fputs(logLine, logsFile);

                        cvector_push_back(readyQ, currentProcess);
                }
            }
        }
    }
}

void HPF()
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

void HPF2(){
    printf("\n== Starting HPF work ==\n");
    while(1)
    {
        sleep(0.1f);
        if(cvector_empty(readyQ)){
            // Handle
            stopSchFlag = (remainingProcesses == 0);
            if (stopSchFlag && !runningFlag)
                break;
            else
                continue;
        }
        else
        {
            if(!runningFlag){
                runningFlag = 1;
                // get the lowest priority in the q
                currentProcess = readyQ[0];
                int index = 0;
                for(int i = 0 ; i < cvector_size(readyQ); i++){
                    if(readyQ[i].priority < currentProcess.priority){
                        currentProcess = readyQ[i];
                        index = i;
                    }
                }
                // Pop it
                cvector_erase(readyQ, index);

                if(currentProcess.waiting_time == -1){
                    // hasnt run b4
                    currentProcess.waiting_time = getClk() - currentProcess.arrival_time;
                }
                char logLine[128];
                sprintf(logLine,
                "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                getClk(), currentProcess.id, "started", currentProcess.arrival_time, currentProcess.runtime, currentProcess.remaining_time, currentProcess.waiting_time
                );
                fputs(logLine, logsFile);

                // Run it
                pid_t process_pid = fork();
                    if(process_pid == 0){
                        char PID[8];
                        char Runtime[8];
                        sprintf(PID, "%d", currentProcess.id);
                        sprintf(Runtime, "%d", currentProcess.runtime);
                        execl("./process.out", "./process.out", PID, Runtime, NULL);
                    }
            }
            else
                continue;
        }

    }

    printf("\nAna 5latht :3\n");
}

void RR(int Quantum){
    int Q = Quantum;
    printf("\n== Starting RR Q=%d work ==\n", Q);

    while(1){
        sleep(0.1f);
        // Handle finish
        if(cvector_empty(readyQ)){
            // Handle
            stopSchFlag = (remainingProcesses == 0);
            if (stopSchFlag)
                break;
            else
                continue;
        }
        else
        {
            if(processStartTime == -1){
                // Ready to run
                currentProcess = readyQ[RR_Process_index];
                RR_Process_index = (RR_Process_index + 1) % cvector_size(readyQ); // Lw fyh new process fl Q hana5odha next

                if(readyQ[RR_Process_index].sys_pid == -2)
                    continue; // finished already
                if(readyQ[RR_Process_index].sys_pid == -1)
                {
                    // Has not forked yet
                    processStartTime = getClk();

                    if(readyQ[RR_Process_index].waiting_time == -1){
                    // hasnt run b4
                    readyQ[RR_Process_index].waiting_time = getClk() - readyQ[RR_Process_index].arrival_time;
                    }

                    // Log it
                    char logLine[128];
                    sprintf(
                        logLine,
                        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                        getClk(), readyQ[RR_Process_index].id, "started", readyQ[RR_Process_index].arrival_time, readyQ[RR_Process_index].runtime,
                        readyQ[RR_Process_index].remaining_time, readyQ[RR_Process_index].waiting_time
                    );
                    fputs(logLine, logsFile);

                    // Run it
                    pid_t process_pid = fork();
                    if(process_pid == 0){
                        char PID[8];
                        char Runtime[8];
                        sprintf(PID, "%d", readyQ[RR_Process_index].id);
                        sprintf(Runtime, "%d", readyQ[RR_Process_index].runtime);
                        execl("./process.out", "./process.out", PID, Runtime, NULL);
                    }

                    readyQ[RR_Process_index].sys_pid = process_pid;
                    //readyQ[RR_Process_index].sys_pid = process_pid;
                }
                else
                {
                    // Forked and resumed
                    processStartTime = getClk();

                    char logLine[128];
                    sprintf(
                        logLine,
                        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                        getClk(), readyQ[RR_Process_index].id, "resumed", readyQ[RR_Process_index].arrival_time, readyQ[RR_Process_index].runtime,
                        readyQ[RR_Process_index].remaining_time, readyQ[RR_Process_index].waiting_time
                    );
                    fputs(logLine, logsFile);

                    kill(readyQ[RR_Process_index].sys_pid, SIGCONT);
                }
            }
            else
            {
                // Stoping
                if(getClk() >= processStartTime + Q){
                    kill(readyQ[RR_Process_index].sys_pid, SIGSTOP);
                    processStartTime = -1;
                    //currentProcess.remaining_time -= Q;
                    readyQ[RR_Process_index].remaining_time -= Q;

                    char logLine[128];
                    sprintf(
                        logLine,
                        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
                        getClk(), readyQ[RR_Process_index].id, "stopped", readyQ[RR_Process_index].arrival_time, readyQ[RR_Process_index].runtime,
                        readyQ[RR_Process_index].remaining_time, readyQ[RR_Process_index].waiting_time
                    );
                    fputs(logLine, logsFile);
                }
            }
        }
    }
}
/*void RR(cvector_vector_type(struct ProcessInfo) readyQ, int Quantum)
{
    int Beginning_Time=-1;
    int Stop=0;
    while (1)  //To loop until all processes finish
        {   


            if (cvector_empty == 0)
                {
                    fprintf("No process in the Queue\n");
                    break;
                }
            else if (cvector_empty == 1)
                {  
                    for(int i=0; i<cvector_size(readyQ), i++)
                    { 
                        Stop=0;    
                        while(Stop==0)
                        {        
                            if(Beginning_Time==-1)
                            {
                                if(readyQ[i]->sys_pid==-1)
                                {
                                    //write in the output file the starting time
                            
                            
                                    pid_t Process=fork();
                                    readyQ[i]->sys_pid=Process;
                                }
                                else
                                {
                                    //write in the output file when the process resumed
                                    kill(readyQ[i]->sys_pid,SIGCONT);
                                }

                            }
                            else
                            {
                                if(getclk()>=(Beginning_Time+Quantum))
                                {   
                                    //Write in the output file when the process stopped
                                    kill(readyQ[i]->sys_pid,SIGSTOP);
                                    readyQ[i]->remaining_time=remaining_time - Quantum;
                                    Stop=1;
                                }

                                
                            }
                        }                    
                    }   
                }
        }


}
*/
// ana ha comment sawany 3shan a run bas okeeh?

void sigIntHandler(int signum)
{
    // Recive the process from the msg Q
    key_t msgQ_ID = msgget(777, 0644);
    struct msgbuff_process M;
    int rec_val = msgrcv(msgQ_ID, &M, sizeof(M.PI), 0, !IPC_NOWAIT);

    // Push it to the ready Q
    if(M.PI.priority >= 0 && M.PI.priority <= 10){
        // This check is done as the ^C could run a new process
        cvector_push_back(readyQ, M.PI);
        //printf("\n[H] p%d added, at t = %d", M.PI.id, getClk());
    }

    if(algo == 2){
        if(currentProcess.priority >= 0 && currentProcess.priority <= 10 && M.PI.runtime < currentProcess.runtime)
            preFlag = 1;
    }
    // Insert a PCB entry
    // PCB_entry *pcb_p = malloc(sizeof(PCB_entry));
    // pcb_p->id = M.PI.id;
    // pcb_p->state = STARTED;
    // pcb_p->remaining_time = M.PI.runtime;
    // pcb_p->execution_time = 0;
    // pcb_p->waiting_time = INT_MAX; // inf until it starts

    // hash_map_insert(PCB, pcb_p->id, pcb_p);
    //printf("\n[PCB] p #%d added, PCB size = %d", pcb_p->id, PCB->size);
}

void sigChlidHandler(int signum){
    //printf("\n[X] SIGCHILD TURN\n");

    int status = -1;
    wait(&status);
    if(WIFEXITED(status)){
        int pid = WEXITSTATUS(status);
        //printf("\n[x] Recived with exit code = %d", pid);
        if(algo == 1){
            // HPF
            runningFlag = 0;
            remainingProcesses--;
        }
        if(algo == 2){
            printf("\n[X] Vector size = %ld\n", cvector_size(readyQ));
            runningFlag = 0; // test might be deleted
            remainingProcesses--;
            processStartTime = -1;
            currentProcess.sys_pid = -99;
            cvector_erase(readyQ, index_currentProcess);
        }
        if(algo == 3){
            //RR
            //printf("\n[X] am here!!\n");
            runningFlag = 0; // test might be deleted
            remainingProcesses--;
            currentProcess = readyQ[RR_Process_index];
            readyQ[RR_Process_index].sys_pid = -2;
            processStartTime = -1;
            //cvector_erase(readyQ, indx);
            // Check if all processes are done 
            int done = 1;
            for(int i = 0 ; i < cvector_size(readyQ); i++){
                if(readyQ[i].sys_pid != -2)
                    done = 0;
            }

            if(done == 1){
                //printf("\nRR 5latht :3 rem p = %d\n", remainingProcesses);

                cvector_free(readyQ);

                printf("\nReady Q size= %ld\n", cvector_size(readyQ));
                
            }
        }

        total_runtime += currentProcess.runtime;
        total_wait += currentProcess.waiting_time;

        char logLine[128];
        int turnaround = getClk() - currentProcess.arrival_time;
        float w_turnaround = (float)turnaround / (float)currentProcess.runtime;
        total_w_turnatound += w_turnaround;
        cvector_push_back(WTA_vector, w_turnaround);

        sprintf(logLine,
        "At\ttime\t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d TA %d WTA %.2f\n",
        getClk(), currentProcess.id, "finished", currentProcess.arrival_time, currentProcess.runtime, 0, currentProcess.waiting_time, turnaround, w_turnaround
        );
        fputs(logLine, logsFile);
    }
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    printf("\nScheduler terminating!\n");
    destroyClk(true);
    exit(0);
}
