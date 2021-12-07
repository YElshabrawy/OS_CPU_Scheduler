#include "headers.h"

void HPF(cvector_vector_type(struct ProcessInfo));

int main(int argc, char * argv[])
{
    //initClk();
    key_t upq_id = msgget(777, 0644);
    printf("\n~ Scheduler starting ~\n");
    cvector_vector_type(struct ProcessInfo) readyQ = NULL;

    int rec_val = msgrcv(downq_id, &message, sizeof(message.mtext), message.mtype, !IPC_NOWAIT);


    //TODO implement the scheduler :)
    //upon termination release the clock resources
    
    //destroyClk(true);
    // Test the live
}

void HPF(cvector_vector_type(struct ProcessInfo) readyQ)
{
   int lowestpriority, maxProcessNum, counter;
   int endTime, cpuUtil, Avg;

   
   //maxProcessNum= cvector_size(msgQ);

   while (counter!=maxProcessNum)
   {
       for(int i =0; i < cvector_size(readyQ); i++)
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