#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "rm.h"


// global variables a

int DA;  // indicates if deadlocks will be avoided or not
int N;   // number of processes
int M;   // number of resource types
int ExistingRes[MAXR]; // Existing resources vector
//..... other definitions/variables .....
//.....
//.....

int AvailableRes[MAXR];
int RequestRes[MAXP][MAXR];
long tids[MAXP];
int MaxDemands[MAXP][MAXR];
int Allocation[MAXP][MAXR];
int Need[MAXP][MAXR];
int state[MAXP]; // 0 = inactive, 1 = active, 2 = blocked
pthread_mutex_t lock;
pthread_cond_t conds[MAXP];


// end of global variables
int rm_thread_started(int tid)
{
    tids[tid] = pthread_self();
    state[tid] = 1;
    printf("Thread with tid %d became alive\n", tid);

    return 0;
}


int rm_thread_ended()
{
    int this_tid = -1;
    for (int i = 0; i < N; i++)
    {
        if(tids[i] == pthread_self())
        {
            this_tid = i;
            break;
        }
    }

    if (this_tid == -1)
    {
        printf("Error: thread not found\n");
        return -1;
    }

    state[this_tid] = 0;
    for (int j = 0; j < M; j++)
    {
        Allocation[this_tid][j] = 0;
        Need[this_tid][j] = 0;
        MaxDemands[this_tid][j] = 0;
    }
    printf("Thread with tid %d ended\n", this_tid);
    return 0;
}


int rm_claim (int claim[])
{   
    int this_tid = -1;
    for (int i = 0; i < N ; i++)
    {
        if (tids[i] == pthread_self())
        {
            this_tid = i;
            break;
        }
    }

    if (this_tid == -1)
    {
        printf("Error: thread not found\n");
        return -1;
    }

    for (int j = 0; j < M;j++)
    {
        if (claim[j] > ExistingRes[j])
        {
            printf("Error: cannot claim instances more than existing\n");
            return -1;
        }
        
        MaxDemands[this_tid][j] = claim[j];
        Need[this_tid][j] = claim[j];
        printf("Max demand for thread %d indicated %d resources\n", this_tid, claim[j]);
    }

    return 0;
}


int rm_init(int p_count, int r_count, int r_exist[],  int avoid)
{
    if (p_count < 0 || r_count < 0 || p_count > MAXP || r_count > MAXR)
    {
        return -1;
    }
    
    pthread_mutex_init(&lock, NULL);
    for (int i = 0 ; i < p_count ; i++)
    {
        pthread_cond_init(&conds[i], NULL);
    }
    
    DA = avoid;
    N = p_count;
    M = r_count;

    for (int i = 0; i < N; i++)
    {
        state[i] = 0;
    }

    pthread_mutex_lock(&lock);

    for (int i = 0; i < M; ++i)
    {
        ExistingRes[i] = r_exist[i];
        AvailableRes[i] = r_exist[i];
        Allocation[i][i] = 0;
    }

    pthread_mutex_unlock(&lock);
   
    return 0;
}


int rm_request(int request[]) {
    int can_request = 1;
    int this_tid = -1;

    for (int i = 0; i < N; i++) 
    {
        if (tids[i] == pthread_self()) 
        {
            this_tid = i;
        }
    }

    if (this_tid == -1) 
    {
        printf("Error: thread not found\n");
        return -1;
    }

    // check if request exceeds existing resources
    for (int i = 0; i < M; i++) 
    {
        if (request[i] > ExistingRes[i]) 
        {
            printf("Error: requested instances for resource %d exceeds existing instances\n", i);
            return -1;
        }
    }

    if (DA)
    {
        pthread_mutex_lock(&lock);

        for (int i = 0; i < M; i++)
        {
            Allocation[this_tid][i] += request[i];
            Need[this_tid][i] -= request[i];
            AvailableRes[i] -= request[i];
            printf("Trying to allocate %d instances of resource %d to thread %d\n", request[i], i, this_tid);
        }

        int work[M];
        int finish[N];

        // calculate the new work and finish arrays
        for (int i = 0; i < M; i++) 
        {
            work[i] = AvailableRes[i];
        }
        for (int i = 0; i < N; i++) 
        {
            finish[i] = 0;
        }

        // check if the new state is safe
        int found = 0;
        for (int i = 0; i < N; i++) 
        {
            if (finish[i] == 0) 
            {
                int j;
                for (j = 0; j < M; j++) 
                {
                    if (Need[i][j] > work[j]) 
                    {
                        break;
                    }
                }
                if (j == M) 
                {
                    found++;
                    finish[i] = 1;
                    for (j = 0; j < M; j++) 
                    {
                        work[j] += Allocation[i][j];
                    }
                }
            }
        }

        // if the new state is safe, grant the resources
        if (found == N) 
        {
            printf("The new state is safe, resources allocated\n");
            pthread_mutex_unlock(&lock);
            return 0;
        } 
        else 
        {
            // if the new state is not safe, revert the changes
            for (int i = 0; i < M; i++) 
            {
                Allocation[this_tid][i] -= request[i];
                Need[this_tid][i] += request[i];
                AvailableRes[i] += request[i];
            }
            printf("The new state is not safe, resources not allocated\n");
            // if the new state is not safe, block the thread
            state[this_tid] = 2;
            for (int i = 0; i < M; i++) 
            {
                RequestRes[this_tid][i] = request[i];
            }
            while (state[this_tid] == 2) 
            {
                pthread_cond_wait(&conds[this_tid], &lock);
            }
            if (state[this_tid] == 1) 
            {
                for (int i = 0; i < M; i++) 
                {
                    AvailableRes[i] -= request[i];
                    Allocation[this_tid][i] += request[i];
                    Need[this_tid][i] -= request[i];
                    RequestRes[this_tid][i] = 0;
                }
                pthread_mutex_unlock(&lock);
                return 0;
            } 
            else 
            {
                // this should never happen
                pthread_mutex_unlock(&lock);
                printf("Error: invalid state %d for thread %d\n", state[this_tid], this_tid);
                return -1;
            }
        }
        
    }

    // check if request can be granted immediately
    pthread_mutex_lock(&lock);

    for (int i = 0; i < M; i++) 
    {
        if (request[i] > AvailableRes[i]) 
        {
            can_request = 0;
            break;
        }
    }

    // if request can be granted immediately, allocate resources and return
    if (can_request) 
    {
        for (int i = 0; i < M; i++) 
        {
            AvailableRes[i] -= request[i];
            Allocation[this_tid][i] += request[i];
            Need[this_tid][i] -= request[i];
        }
        printf("Resources allocated, to thread %d\n", this_tid);
        pthread_mutex_unlock(&lock);
        return 0;
    }

    // if request cannot be granted immediately, block until resources are available
    state[this_tid] = 2; // blocked
    for (int i = 0; i < M; i++) 
    {
        RequestRes[this_tid][i] = request[i];
    }
    printf("Thread %d blocked\n", this_tid);
    while (!can_request) 
    {
        pthread_cond_wait(&conds[this_tid], &lock);

        can_request = 1;
        for (int i = 0; i < M; i++) 
        {
            if (RequestRes[this_tid][i] > AvailableRes[i]) 
            {
                can_request = 0;
                break;
            }
        }
    }

    // allocate resources and unblock
    for (int i = 0; i < M; i++) 
    {
        AvailableRes[i] -= RequestRes[this_tid][i];
        Allocation[this_tid][i] += RequestRes[this_tid][i];
        Need[this_tid][i] -= RequestRes[this_tid][i];
        RequestRes[this_tid][i] = 0;
    }
    printf("Resources allocated, thread %d unblocked\n", this_tid);

    return 0;
}


int rm_release(int release[]) 
{
    int this_tid = -1;

    for (int i = 0; i < N; i++) 
    {
        if (tids[i] == pthread_self()) 
        {
            this_tid = i;
        }
    }
    
    pthread_mutex_lock(&lock); // acquire the lock to ensure mutual exclusion
    
    // check if the thread is active or blocked
    if (state[this_tid] != 1) 
    {
        pthread_mutex_unlock(&lock);
        return -1; // return -1 if the thread is not active
    }
    
    // check if the thread has allocated the resources it wants to release
    for (int i = 0; i < M; i++) 
    {
        if (release[i] > Allocation[this_tid][i]) 
        {
            pthread_mutex_unlock(&lock);
            return -1; // return -1 if the thread is trying to release more than allocated
        }
    }
    
    // release the resources
    for (int i = 0; i < M; i++)
    {
        Allocation[this_tid][i] -= release[i];
        AvailableRes[i] += release[i];
        Need[this_tid][i] += release[i];
    }
    printf("Resources released, thread %d\n", this_tid);
    // check if any blocked threads can be unblocked
    for (int i = 0; i < N; i++) 
    {
        if (state[i] == 2) 
        {
            int can_allocate = 1;
            for (int j = 0; j < M; j++) 
            {
                if (RequestRes[i][j] > AvailableRes[j]) 
                {
                    can_allocate = 0;
                    break;
                }
            }
            if (can_allocate) {
                printf("Thread %d unblocked\n", i);
                state[i] = 1; // unblock the thread
                pthread_cond_signal(&conds[i]); // signal the thread
            }
        }
    }
    
    pthread_mutex_unlock(&lock); // release the lock
    return 0; // return 0 upon success
}


int rm_detection()
{
    pthread_mutex_lock(&lock); // acquire the lock to ensure mutual exclusion
    
    // Initialize the needed matrices for Banker's algorithm
    int Finish[N];
    for (int i = 0; i < N; i++) 
    {
        Finish[i] = 0;
    }
    int Work[M];
    for (int i = 0; i < M; i++) 
    {
        Work[i] = AvailableRes[i];
    }
    int Need_copy[N][M];
    for (int i = 0; i < N; i++) 
    {
        for (int j = 0; j < M; j++) 
        {
            Need_copy[i][j] = Need[i][j];
        }
    }
    
    // Apply Banker's algorithm to detect deadlocks
    int deadlock_count = 0;
    int safe_sequence[N];
    int safe_index = 0;
    for (int count = 0; count < N; count++) 
    {
        int found = 0;
        for (int i = 0; i < N; i++) 
        {
            if (Finish[i] == 0) 
            {
                int j;
                for (j = 0; j < M; j++) 
                {
                    if (Need_copy[i][j] > Work[j]) 
                    {
                        break;
                    }
                }
                if (j == M) 
                {
                    for (int k = 0; k < M; k++) 
                    {
                        Work[k] += Allocation[i][k];
                    }
                    Finish[i] = 1;
                    found = 1;
                    safe_sequence[safe_index++] = i;
                }
            }
        }
        if (!found) 
        {
            break;
        }
    }
    
    // check if a deadlock has been detected
    if (safe_index != N) 
    {
        deadlock_count = N - safe_index;
    }
    
    pthread_mutex_unlock(&lock); // release the lock
    
    // return the number of deadlocked processes
    return deadlock_count;
        
}


void rm_print_state (char hmsg[])
{
    printf("***********************\n***%s***\n***********************\n",hmsg);
    printf("\nExist:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d ",i);

    printf("\n");

    for (int i = 0; i < M; i++)
        printf("    %d  ",ExistingRes[i]);
    printf("\n");

    //-------------------------------------------------------------------
    printf("\nAvailable:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d ",i);

    printf("\n");

    for (int i = 0; i < M; i++)
        printf("    %d  ",AvailableRes[i]);
    printf("\n");

    //-------------------------------------------------------------------
    printf("\nAllocation:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d",i);

    printf("\n");

    for (int i = 0; i < N; i++){
        printf("T%d: ",i);
        for (int j = 0; j < M; j++)
            printf("%d      ",Allocation[i][j]);       
        printf("\n");

    }


    //-------------------------------------------------------------------
    printf("\nRequest:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d",i);

    printf("\n");

    for (int i = 0; i < N; i++){
        printf("T%d: ",i);
        for (int j = 0; j < M; j++)
            printf("%d      ",RequestRes[i][j]);       
        printf("\n");

    }
    //-------------------------------------------------------------------
    printf("\nMax Demand:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d",i);

    printf("\n");

    for (int i = 0; i < N; i++){
        printf("T%d: ",i);
        for (int j = 0; j < M; j++)
            printf("%d      ",MaxDemands[i][j]);       
        printf("\n");

    }
    //-------------------------------------------------------------------
    printf("\nNeed:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d",i);

    printf("\n");

    for (int i = 0; i < N; i++){
        printf("T%d: ",i);
        for (int j = 0; j < M; j++)
            printf("%d      ",Need[i][j]);       
        printf("\n");

    }
    printf("\n***********************\n");
}