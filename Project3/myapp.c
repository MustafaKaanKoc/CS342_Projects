#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include "rm.h"

#define NUMR 5    // number of resource types
#define NUMP 3        // number of threads

int AVOID = 1;
int exist[5] =  {10, 5, 7, 12, 15};  // resources existing in the system

void pr (int tid, char astr[], int m, int r[])
{
    int i;
    printf ("thread %d, %s, [", tid, astr);
    for (i=0; i<m; ++i) {
        if (i==(m-1))
            printf ("%d", r[i]);
        else
            printf ("%d,", r[i]);
    }
    printf ("]\n");
}


void setarray (int r[MAXR], int m, ...)
{
    va_list valist;
    int i;
    
    va_start (valist, m);
    for (i = 0; i < m; i++) {
        r[i] = va_arg(valist, int);
    }
    va_end(valist);
    return;
}


void *threadfunc1 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int request3[MAXR];
    int claim[MAXR];
    
    tid = *((int*)a);
    rm_thread_started (tid);

    claim[0] = 7;
    claim[1] = 5;
    claim[2] = 7;
    claim[3] = 10;
    claim[4] = 9;
    rm_claim (claim);
    
    request1[0] = 6;
    request1[1] = 1;
    request1[2] = 4;
    request1[3] = 5;
    request1[4] = 6;
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(4);

    request2[0] = 0;
    request2[1] = 3;
    request2[2] = 2;
    request2[3] = 5;
    request2[4] = 3;
    pr (tid, "REQ", NUMR, request2);
    rm_request (request2);

    sleep(2);
    
    request3[0] = 1;
    request3[1] = 1;
    request3[2] = 1;
    request3[3] = 0;
    request3[4] = 0;
    pr (tid, "REQ", NUMR, request3);
    rm_request (request3);

    sleep(2);
    rm_release (request1);
    rm_release (request2);
    rm_release (request3);
    sleep(2);
    rm_thread_ended();
    pthread_exit(NULL);
}


void *threadfunc2 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    claim[0] = 8;
    claim[1] = 4;
    claim[2] = 7;
    claim[3] = 8;
    claim[4] = 10;
    rm_claim (claim);

    request1[0] = 4;
    request1[1] = 4;
    request1[2] = 3;
    request1[3] = 7;
    request1[4] = 9;
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(1);
    
    request2[0] = 2;
    request2[1] = 0;
    request2[2] = 2;
    request2[3] = 1;
    request2[4] = 1;
    pr (tid, "REQ", NUMR, request2);
    rm_request (request2);

    sleep(2);

    sleep(2);
    rm_release (request1);
    rm_release (request2);
    sleep(2);
    rm_thread_ended ();
    pthread_exit(NULL);
}

void *threadfunc3 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int request3[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    claim[0] = 3;
    claim[1] = 2;
    claim[2] = 1;
    claim[3] = 5;
    claim[4] = 4;
    rm_claim (claim);

    sleep(1);
    request1[0] = 2;
    request1[1] = 0;
    request1[2] = 0;
    request1[3] = 0;
    request1[4] = 0;
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(2);
    
    request2[0] = 0;
    request2[1] = 0;
    request2[2] = 1;
    request2[3] = 3;
    request2[4] = 3;
    pr (tid, "REQ", NUMR, request2);
    rm_request (request2);

    sleep(1);

    request3[0] = 1;
    request3[1] = 2;
    request3[2] = 0;
    request3[3] = 0;
    request3[4] = 0;
    pr (tid, "REQ", NUMR, request3);
    rm_request (request3);

    sleep(2);
    rm_release (request1);
    rm_release (request2);
    rm_release (request3);
    sleep(2);
    rm_thread_ended ();
    pthread_exit(NULL);
}


int main(int argc, char **argv)
{
    int i;
    int tids[NUMP];
    pthread_t threadArray[NUMP];
    int count;
    int ret;

    if (argc != 2) {
        printf ("usage: ./app avoidflag\n");
        exit (1);
    }

    AVOID = atoi (argv[1]);
    

    if (AVOID == 1)
        rm_init (NUMP, NUMR, exist, 1);
    else
        rm_init (NUMP, NUMR, exist, 0);

    i = 0;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc1, (void *)
                    (void*)&tids[i]);
    
    i = 1;  // we select a tid for the thread

    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc2, (void *)
                    (void*)&tids[i]);

    i = 2;  // we select a tid for the thread

    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc3, (void *)
                    (void*)&tids[i]);

    count = 0;
    while ( count < 15) {
        sleep(1);
        rm_print_state("The current state");
        ret = rm_detection();
        if (ret > 0) {
            printf ("deadlock detected, count=%d\n", ret);
            rm_print_state("state after deadlock");
        }
        count++;
    }
    

    for (i = 0; i < NUMP; ++i) {
        pthread_join (threadArray[i], NULL);
        printf ("joined\n");
    }
    rm_print_state("state after all threads terminated");
}