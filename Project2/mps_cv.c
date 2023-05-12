#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include "linkedlist.h"

#define DEFAULT_N 2
#define DEFAULT_SAP "M"
#define DEFAULT_QS "RM"
#define DEFAULT_ALG "RR"
#define DEFAULT_Q 20
#define DEFAULT_INFILE "in.txt"
#define DEFAULT_OUTMODE 1
#define DEFAULT_OUTFILE "out.txt"

int n = DEFAULT_N;
char *sap = DEFAULT_SAP, *qs = DEFAULT_QS;
char *alg = DEFAULT_ALG;
int q = DEFAULT_Q;
char *infile = DEFAULT_INFILE;
int outmode = DEFAULT_OUTMODE;
char *outfile = DEFAULT_OUTFILE;
int rflag = 0, iflag = 0;
double t = 200, t1 = 10, t2 = 1000, l = 100, l1 = 10, l2 = 500, pc = 10;
struct timeval start, end;
LinkedList* analysis_list;

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s [-n N] [-a SAP QS] [-s ALG Q] [-i INFILE] [-m OUTMODE] [-o OUTFILE] [-r T T1 T2 L L1 L2 PC]\n", program_name);
}

int assign_flags(int count, char* args[]);

struct params {
    LinkedList** ready_queue;
    pthread_mutex_t *mutex, *mutex1;
    pthread_cond_t *cond;
    int cpu_id;
};

void* processor_sim(void* args){
    struct params parameters = *(struct params*) args;
    LinkedList** ready_queue = parameters.ready_queue;
    pthread_mutex_t* mutex = parameters.mutex;
    pthread_mutex_t* mutex1 = parameters.mutex1;
    int cpu_id = parameters.cpu_id;

    while (1) {
        while (1) {
            pthread_mutex_lock(mutex);
            if (get_length(ready_queue) > 0) {
                break;
            }
            pthread_mutex_unlock(mutex);
            usleep(1000);
        }
        Burst *b = malloc(sizeof(Burst));
        if (strcmp(alg, "FCFS") == 0) {
            b->pid = get_burst(ready_queue, get_length(ready_queue) - 1)->pid;
            if (b->pid == -1) {
                pthread_mutex_unlock(mutex);
                free(b);
                break;
            }
            b->burst_length = get_burst(ready_queue, get_length(ready_queue) - 1)->burst_length;
            b->arrival_time = get_burst(ready_queue, get_length(ready_queue) - 1)->arrival_time;
            b->remaining_time = get_burst(ready_queue, get_length(ready_queue) - 1)->remaining_time;
            b->finish_time = get_burst(ready_queue, get_length(ready_queue) - 1)->finish_time;
            b->turnaround_time = get_burst(ready_queue, get_length(ready_queue) - 1)->turnaround_time;
            b->processor_id = cpu_id;
            delete_node(ready_queue, get_length(ready_queue) - 1);
            pthread_mutex_unlock(mutex);
            gettimeofday(&end, NULL);
            if (outmode == 2 || outmode == 3)
                printf("time=%ld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                    (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
            
            usleep(b->burst_length * 1000);

            gettimeofday(&end, NULL);
            b->finish_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
            b->turnaround_time = b->finish_time - b->arrival_time;
            b->remaining_time = 0;
            if (outmode == 3)
                printf("time=%ld. Burst finished. cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                    (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
            
            pthread_mutex_lock(mutex1);
            insert(&analysis_list, b);
            pthread_mutex_unlock(mutex1);
            free(b);
        }
        else if (strcmp(alg, "SJF") == 0) {
            int index = get_shortest_burst_index(ready_queue);
            b->pid = get_burst(ready_queue, index)->pid;
            if (b->pid == -1) {
                pthread_mutex_unlock(mutex);
                free(b);
                break;
            }
            b->burst_length = get_burst(ready_queue, index)->burst_length;
            b->arrival_time = get_burst(ready_queue, index)->arrival_time;
            b->remaining_time = get_burst(ready_queue, index)->remaining_time;
            b->finish_time = get_burst(ready_queue, index)->finish_time;
            b->turnaround_time = get_burst(ready_queue, index)->turnaround_time;
            b->processor_id = cpu_id;
            delete_node(ready_queue, index);
            pthread_mutex_unlock(mutex);
            gettimeofday(&end, NULL);
            if (outmode == 2 || outmode == 3)
                printf("time=%ld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                    (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
            
            usleep(b->burst_length * 1000);

            gettimeofday(&end, NULL);
            b->finish_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
            b->turnaround_time = b->finish_time - b->arrival_time;
            b->remaining_time = 0;
            if (outmode == 3)
                printf("time=%ld. Burst finished. cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                    (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
            pthread_mutex_lock(mutex1);
            insert(&analysis_list, b);
            pthread_mutex_unlock(mutex1);
            free(b);
        }
        else if(strcmp(alg, "RR") == 0) {
            
            if (get_length(ready_queue) == 1 && get_burst(ready_queue, 0)->pid == -1) {
                pthread_mutex_unlock(mutex);
                free(b);
                break;
            }
            b->pid = get_burst(ready_queue, get_length(ready_queue) - 1)->pid;
            b->burst_length = get_burst(ready_queue, get_length(ready_queue) - 1)->burst_length;
            b->arrival_time = get_burst(ready_queue, get_length(ready_queue) - 1)->arrival_time;
            b->remaining_time = get_burst(ready_queue, get_length(ready_queue) - 1)->remaining_time;
            b->finish_time = get_burst(ready_queue, get_length(ready_queue) - 1)->finish_time;
            b->turnaround_time = get_burst(ready_queue, get_length(ready_queue) - 1)->turnaround_time;
            b->processor_id = cpu_id;
            if (outmode == 2 || outmode == 3)
                    printf("time=%ld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                        (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
            delete_node(ready_queue, get_length(ready_queue) - 1);
            pthread_mutex_unlock(mutex);

            if (b->remaining_time <= q) {
                usleep(b->remaining_time * 1000);
                b->remaining_time = 0;
                gettimeofday(&end, NULL);
                b->finish_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
                b->turnaround_time = b->finish_time - b->arrival_time;
                if (outmode == 3)
                    printf("time=%ld. Burst finished. cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                        (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->processor_id, b->pid, b->burst_length, b->remaining_time);
                pthread_mutex_lock(mutex1);
                insert(&analysis_list, b);
                pthread_mutex_unlock(mutex1);
                free(b);
            }
            else {
                usleep(q * 1000);
                gettimeofday(&end, NULL);
                if (outmode == 3)
                    printf("time=%ld. Time slice (q=%d) expired. cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                        (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, q, b->processor_id, b->pid, b->burst_length, b->remaining_time - q);
                
                b->remaining_time -= q;
                pthread_mutex_lock(mutex);
                if (get_length(ready_queue) > 0 && get_burst(ready_queue, 0)->pid == -1) {
                    Burst* dummy = malloc(sizeof(Burst));
                    dummy->pid = -1;
                    dummy->burst_length = __INT_MAX__;
                    dummy->arrival_time = -1;
                    dummy->remaining_time = __INT_MAX__;
                    dummy->finish_time = -1;
                    dummy->turnaround_time = -1;
                    dummy->processor_id = -1;
                    delete_node(ready_queue, 0);
                    insert(ready_queue, b);
                    insert(ready_queue, dummy);
                    pthread_mutex_unlock(mutex);
                    free(dummy);
                    free(b);
                }
                else {
                    insert(ready_queue, b);
                    pthread_mutex_unlock(mutex);
                    free(b);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Read flags and arguments from the command line
    assign_flags(argc, argv);
   
    LinkedList **queues;
    pthread_mutex_t *queue_locks;
    pthread_mutex_t analysis_lock;
    pthread_cond_t *queue_conds;
    struct params *params_array;
    FILE* fp;

    if (n == 1 || strcmp(sap, "S") == 0) {
        queues = malloc(sizeof(LinkedList *));
        queue_locks = malloc(sizeof(pthread_mutex_t));
        queue_conds = malloc(sizeof(pthread_cond_t));
        pthread_mutex_init(queue_locks, NULL);
        pthread_cond_init(queue_conds, NULL);
    }
    else {
        queues = malloc(n * sizeof(LinkedList *));
        queue_locks = malloc(n * sizeof(pthread_mutex_t));
        queue_conds = malloc(n * sizeof(pthread_cond_t));
        for (int i = 0; i < n; i++) {
            pthread_mutex_init(&queue_locks[i], NULL);
            pthread_cond_init(&queue_conds[i], NULL);
        }
    }
    
    pthread_mutex_init(&analysis_lock, NULL);

    pthread_t threads[n];
    
    params_array = malloc(n * sizeof(struct params));

    if (strcmp(sap, "M") == 0) {
        for (int i = 0; i < n; i++) {
            params_array[i].cpu_id = i+1;
            params_array[i].ready_queue = &queues[i];
            params_array[i].mutex = &queue_locks[i];
            params_array[i].mutex1 = &analysis_lock;
            params_array[i].cond = &queue_conds[i];
            pthread_create(&threads[i], NULL, processor_sim, &params_array[i]);
        }
    }
    else if (strcmp(sap, "S") == 0)
    {
        for (int i = 0; i < n; i++) {
            params_array[i].cpu_id = i+1;
            params_array[i].ready_queue = queues;
            params_array[i].mutex = queue_locks;
            params_array[i].mutex1 = &analysis_lock;
            params_array[i].cond = queue_conds;
            pthread_create(&threads[i], NULL, processor_sim, &params_array[i]);
        }
    }  
    
    if (rflag || (!iflag && !rflag))
    {
        infile = "random-generated-bursts.txt";
        fp = fopen(infile, "w");
        if (fp == NULL) {
            fprintf(stderr, "Error: Could not open file %s\n", infile);
            return 1;
        }
        srand(time(NULL));

        for (int i = 0; i < pc; i++)
        {
            double t_u = (double) rand() / RAND_MAX; // generate a random number between 0 and 1
            double t_lambda = 1.0 / t;
            double t_x = -log(1 - t_u) / t_lambda;
            while (t_x < t1 || t_x > t2) {
                t_u = (double) rand() / RAND_MAX; // generate a random number between 0 and 1
                t_lambda = 1.0 / t;
                t_x = -log(1 - t_u) / t_lambda;
            }
            int rounded_t_x = (int) round(t_x);

            double l_u = (double) rand() / RAND_MAX; // generate a random number between 0 and 1
            double l_lambda = 1.0 / l;
            double l_x = -log(1 - t_u) / l_lambda;
            while (l_x < t1 || l_x > l2) {
                l_u = (double) rand() / RAND_MAX; // generate a random number between 0 and 1
                l_lambda = 1.0 / t;
                l_x = -log(1 - l_u) / l_lambda;
            }
            int rounded_l_x = (int) round(l_x);
            
            fprintf(fp, "PL %d\n", rounded_l_x);
            fprintf(fp, "IAT %d\n", rounded_t_x);
        }
        fclose(fp);
    }
    
    fp = fopen(infile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", infile);
        return 1;
    }

    int last_processor = 0;
    if (strcmp(sap, "M") == 0 && strcmp(qs, "RM") == 0)
        last_processor = n;
    char buffer[100];
    int pids = 1;
    gettimeofday(&start, NULL);
    while (fgets(buffer, sizeof(buffer), fp)) {
        int pl, iat;
        if (sscanf(buffer, "PL %d\n", &pl) == 1) {
            Burst* b = (Burst*)malloc(sizeof(Burst));
            gettimeofday(&end, NULL);
            b->pid = pids;
            b->burst_length = pl;
            b->remaining_time = pl;
            b->arrival_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
            b->finish_time = 0;
            b->turnaround_time = 0;
            b->processor_id = 1;
            
            if (strcmp(sap, "M") == 0) {
                if (strcmp(qs, "RM") == 0) {
                    if (last_processor == n)
                        last_processor = 1;
                    else
                        ++last_processor;

                    b->processor_id = last_processor;
                    if (outmode == 3)
                        printf("time=%ld. Add process %d to queue %d. Burst Length: %d\n",
                            (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->pid, b->processor_id, b->burst_length);
                    
                    pthread_mutex_lock(&queue_locks[last_processor - 1]);                    
                    insert(&queues[last_processor - 1], b);
                    if (get_length(&queues[last_processor - 1]) == 1)
                        pthread_cond_signal(&queue_conds[last_processor - 1]);
                    pthread_mutex_unlock(&queue_locks[last_processor - 1]);
                    free(b);
                }
                else {
                    int lengths[n];

                    for (int i = 0; i < n; i++) {
                        lengths[i] = 0;
                        pthread_mutex_lock(&queue_locks[i]);
                        for (int j = 0; j < get_length(&queues[i]); j++)
                            lengths[i] += get_burst(&queues[i], j)->burst_length;
                        pthread_mutex_unlock(&queue_locks[i]);
                    }
                    
                    int min_length = lengths[0];
                    int processor_to_use = 0;
                    for (int i = 1; i < n; i++) {
                        if (lengths[i] < min_length) {
                            min_length = lengths[i];
                            processor_to_use = i;
                        }
                    }
                     b->processor_id = processor_to_use + 1;

                    if (outmode == 3)
                        printf("time=%ld. Add process %d to queue %d. Burst Length: %d\n",
                            (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->pid, b->processor_id, b->burst_length);
                            
                   
                    pthread_mutex_lock(&queue_locks[processor_to_use]);
                    insert(&queues[processor_to_use], b);
                    if (get_length(&queues[processor_to_use]) == 1)
                        pthread_cond_signal(&queue_conds[processor_to_use]);
                    pthread_mutex_unlock(&queue_locks[processor_to_use]);
                    free(b);
                }
            }
            else if (strcmp(sap, "S") == 0) {
                if (outmode == 3)
                        printf("time=%ld. Add process %d to queue  %d. Burst Length: %d\n",
                            (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000, b->pid, b->processor_id, b->burst_length);
                pthread_mutex_lock(queue_locks);
                insert(queues, b);
                if (get_length(queues) == 1)
                    pthread_cond_signal(queue_conds);
                pthread_mutex_unlock(queue_locks);
                free(b);
            }
            
            pids++;
        }
        else if (sscanf(buffer, "IAT %d\n", &iat) == 1) {
            usleep(iat * 1000);
        }    
    }
    fclose(fp);
    if (rflag || (!iflag && !rflag))
      remove(infile);
    
    Burst* dummy = malloc(sizeof(Burst));
    dummy->pid = -1;
    dummy->burst_length = __INT_MAX__;
    dummy->remaining_time = __INT_MAX__;
    dummy->arrival_time = -1;
    dummy->finish_time = -1;
    dummy->turnaround_time = -1;
    dummy->processor_id = -1;

    if (strcmp(sap, "M") == 0) {
        for (int i = 0; i < n; i++) {
            pthread_mutex_lock(&queue_locks[i]);
            insert(&queues[i], dummy);
            if (get_length(&queues[i]) == 1)
                pthread_cond_signal(&queue_conds[i]);
            pthread_mutex_unlock(&queue_locks[i]);
        }
    }
    else if (strcmp(sap, "S") == 0) {
        pthread_mutex_lock(queue_locks);
        insert(queues, dummy);
        if (get_length(queues) == 1)
            pthread_cond_signal(queue_conds);
        pthread_mutex_unlock(queue_locks);
    }

    for(int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    if (strcmp(sap, "M") == 0) {
        for (int i = 0; i < n; i++) {
            pthread_mutex_destroy(&queue_locks[i]);
            pthread_cond_destroy(&queue_conds[i]);
        }
    }
    else if (strcmp(sap, "S") == 0) {
        pthread_mutex_destroy(queue_locks);
        pthread_cond_destroy(queue_conds);
    }
    pthread_mutex_destroy(&analysis_lock);

    sort_by_pid(&analysis_list);
    printf("pid\tcpu\tburstlen\tarv\tfinish\twaitingtime\tturnaround\n");
    for (int i = 0; i < get_length(&analysis_list); i++) {
        printf("%d\t%d\t%d\t\t%ld\t%ld\t%d\t\t%d\n", 
            get_burst(&analysis_list, i)->pid, 
            get_burst(&analysis_list, i)->processor_id, 
            get_burst(&analysis_list, i)->burst_length, 
            get_burst(&analysis_list, i)->arrival_time, 
            get_burst(&analysis_list, i)->finish_time, 
            get_burst(&analysis_list, i)->turnaround_time - get_burst(&analysis_list, i)->burst_length,
            get_burst(&analysis_list, i)->turnaround_time);
    }
    printf("Average Turnaround Time: %f\n", get_avg_turnaround(&analysis_list));

    if (strcmp(sap, "M") == 0) {
        for (int i = 0; i < n; i++) {
            delete_list(&queues[i]);
        }
    }
    else if (strcmp(sap, "S") == 0) {
        delete_list(queues);
    }
    
    free(dummy);
    free(queues);
    free(queue_locks);
    delete_list(&analysis_list);
    free(params_array);
    free(analysis_list);
    
    return 0;
}

int assign_flags(int count, char* args[]) {
    for (int j = 1; j < count; j++) {
        if (strcmp(args[j], "-n") == 0) {
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -n option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            n = atoi(args[j]);
        } 
        else if (strcmp(args[j], "-a") == 0) {
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -a option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            sap = args[j];
            if (strcmp(sap, "M") == 0 && n <= 1) {
                fprintf(stderr, "Error: -n value must be greater than 1 for multi-queue approach\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing second argument for -a option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            qs = args[j];
        }
        else if (strcmp(args[j], "-s") == 0) {
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -s option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            alg = args[j];
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing second argument for -s option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            q = atoi(args[j]);
        } 
        else if (strcmp(args[j], "-i") == 0) {
            iflag = 1;
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -i option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            infile = args[j];
        } 
        else if (strcmp(args[j], "-m") == 0) {
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -m option\n");
                print_usage(args[0]);
                exit(EXIT_FAILURE);
            }
            outmode = atoi(args[j]);
        } 
        else if(strcmp(args[j], "-o") == 0) {
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -o option\n");
                return 1;
            }
            outfile = args[j];
        }
        else if (strcmp(args[j], "-r") == 0) {
            rflag = 1;
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            t = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            t1 = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            t2 = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            l = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            l1 = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            l2 = atoi(args[j]);
            j++;
            if (j >= count) {
                fprintf(stderr, "Error: Missing argument for -r option\n");
                return 1;
            }
            pc = atoi(args[j]);
        } 
        else {
            fprintf(stderr, "Error: Unrecognized option %s\n", args[j]);
            return 1;
        }
    }
    return 0;
}