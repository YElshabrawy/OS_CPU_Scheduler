#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();

    int ID, runtime;
    // runtime is the time the procces will run regaurless the process runtime
    // e.g in SRTN this nummber is always 1 so each process will run only 1 s
    sscanf(argv[1], "%d", &ID);
    sscanf(argv[2], "%d", &runtime);

    printf("\n[P] Process #%d running\n", ID);

    remainingtime = runtime;
    while (remainingtime > 0)
    {
        printf("\n[P] Process #%d running, RT=%d\n", ID, remainingtime);
        remainingtime--;
        sleep(1);
    }
    destroyClk(0);
    exit(ID);
}
