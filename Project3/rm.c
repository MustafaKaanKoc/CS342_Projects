#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "rm.h"


// global variables

int DA;  // indicates if deadlocks will be avoided or not
int N;   // number of processes
int M;   // number of resource typesasdasd
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
pthread_mutex_t lock;
pthread_cond_t conds[MAXP];


// end of global variables
int rm_thread_started(int tid)
{
    tids[tid]=pthread_self();
    printf("Thread with tid %d became alive\n", tid) ;
   
    return 0;
}


int rm_thread_ended()
{
    int ret = 0;
    return (ret);
}


int rm_claim (int claim[])
{   
    int this_tid;
    for(int i = 0 ;i < N ; i++){
        if(tids[i] == pthread_self()){
            this_tid = tids[i];
            break;
        }
    }

    for(int j = 0; j < M;j++){
        if(claim[j] > ExistingRes[j]){
            printf("Error: cannot claim instances more than existing\n");
            return -1;
        }
        else{
            MaxDemands[this_tid][j] = claim[j];  
            printf("Max demand for thread %d indicated %d resources\n",this_tid,claim[j]);
        }
    }
              
    

    return 0;
}


int rm_init(int p_count, int r_count, int r_exist[],  int avoid)
{
    if(p_count < 0 || r_count < 0 || p_count > MAXP || r_count > MAXR)
        return -1;   
    
    pthread_mutex_init(&lock,NULL);
    for(int i = 0 ; i < N; i ++){
        pthread_cond_init(&conds[i], NULL);
    }
    DA = avoid;
    N = p_count;
    M = r_count;

    for (int i = 0; i < M; ++i){
        ExistingRes[i] = r_exist[i];
        AvailableRes[i] = r_exist[i];
    }
   
    return  0;
}


int rm_request (int request[])
{   
    for (int i = 0; i < MAXR; i++)
    {
        if(request[i] < 0) {
            printf("Error: requested resources cannot be negative");
            return -1;
        }
    }
    
    int this_tid;
    for(int i = 0 ;i < N ; i++){
        if(tids[i] == pthread_self()){
            this_tid = tids[i];
            break;
        }
    }

    pthread_mutex_lock(&lock);

    if (!DA) {       
        for(int i =0; i < M; i++){
            if(ExistingRes[i] < request[i]){
                printf("Error: requested resources cannot be larger than existing resources");
                pthread_mutex_unlock(&lock);
                return -1;
            }
        }

        for(int i = 0; i < M; i++)
            RequestRes[this_tid][i]=request[i];

        pthread_cond_wait(&conds[this_tid], &lock);

        for(int i = 0; i < M; i++) {
            AvailableRes[i] -= request[i]; 
            Allocation[this_tid][i] = request[i];
            printf("%d resources requested\n" , request[i]);
        }
                   
        pthread_mutex_unlock(&lock);
    }
    else {
        for (int i = 0; i < M; i++) {
            if (request[i] > MaxDemands[i][pthread_self()]) {
                printf("Error: requested resources cannot be larger than max demands");
                return -1;
            }
        }
        //deadlock avoidance varsa
    }
    pthread_mutex_unlock(&lock);

    return 0;
}


int rm_release (int release[])
{
    int this_tid;
    for(int i = 0 ;i < N ; i++){
        if(tids[i] == pthread_self()){
            this_tid = tids[i];
            break;
        }
    }
    pthread_mutex_lock(&lock);


    for(int i = 0; i < M; i++){
        if(release[i] > Allocation[this_tid][i]){
            printf("Error: cannot release resources more than allocated");
            pthread_mutex_unlock(&lock);
            return -1;
        }
        else{
            //threadlere sinyal at
            AvailableRes[i] += release[i];
            printf("%d resources released\n" , release[i]);

        }

    }
    pthread_mutex_unlock(&lock); 
    return 0;
}


int rm_detection()
{
    int ret = 0;
    
    return (ret);
}


void rm_print_state (char hmsg[])
{
    printf("***********************\n***%s***\n***********************\n",hmsg);
    printf("\nExist:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d \n",i);
    
    for (int i = 0; i < M; i++)
        printf("    %d  \n",ExistingRes[i]);

    printf("\nAvailable:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d \n",i);
    for (int i = 0; i < M; i++)
        printf("    %d  \n",AvailableRes[i]);

 

    printf("\nMax Demand:\n");
    for (int i = 0; i < M; i++)    
        printf("    R%d \n",i);
    for (int i = 0; i < M; i++){
        for (int j = 0; j < N; j++)
            printf("T%d: %d  \n",j,MaxDemands[i][j]);        
        
    }
        

    return;
}