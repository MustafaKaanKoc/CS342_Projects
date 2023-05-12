#ifndef __LINKED_LIST_
#define __LINKED_LIST_

typedef struct burst {
    int pid, burst_length, remaining_time, turnaround_time, processor_id;
    long arrival_time, finish_time;
} Burst;

typedef struct node{
   Burst *burst;
   struct node *next;
} LinkedList;

void insert(LinkedList **head, Burst *burst);

void delete_node(LinkedList **head, int index);

Burst* get_burst(LinkedList **head, int index);

int get_length(LinkedList **head);

void delete_list(LinkedList **head);

int get_shortest_burst_index(LinkedList **head);

void sort_by_pid(LinkedList **head);

double get_avg_turnaround(LinkedList **head);

#endif