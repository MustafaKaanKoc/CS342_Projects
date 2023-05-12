#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

void insert(LinkedList **head, Burst *process) {
    LinkedList* new_node = (LinkedList *) malloc(sizeof(LinkedList));
    // Set the data for the new node
    new_node->burst = malloc(sizeof(Burst));
    new_node->burst->pid = process->pid;
    new_node->burst->burst_length = process->burst_length;
    new_node->burst->arrival_time = process->arrival_time;
    new_node->burst->remaining_time = process->remaining_time;
    new_node->burst->finish_time = process->finish_time;
    new_node->burst->turnaround_time = process->turnaround_time;
    new_node->burst->processor_id = process->processor_id;
    
    new_node->next = *head;
    
    *head = new_node;
}

void delete_node(LinkedList **head, int index){
    LinkedList *cur = *head;
    LinkedList *prev = NULL;
    for (int i = 0; i < index; i++){
        prev = cur;
        cur=cur->next;
    }
    if (prev == NULL) {
        *head = cur->next;
    } else {
        prev->next = cur->next;
    }
    free(cur->burst);
    free(cur);
}

Burst* get_burst(LinkedList **head, int index){
    LinkedList *cur = *head;
    for (int i = 0; i < index; i++){
        cur=cur->next;
    }
    return cur->burst;
}

int get_length(LinkedList **head){
    LinkedList *cur = *head;
    int counter = 0;
    while (cur != NULL) {
        cur = cur->next;
        counter++;
    }
    return counter;
}

void delete_list(LinkedList **head){
    while (*head !=NULL) {
        delete_node(head, 0);
    }
}

int get_shortest_burst_index(LinkedList **head) {
    if (head == NULL || *head == NULL)
        return -1;

    LinkedList *cur = (*head);
    Burst *shortest = (*head)->burst;
    int index = 0;
    int counter = 0;
    while (cur != NULL) {
        if (cur->burst->remaining_time <= shortest->remaining_time) {
            shortest = cur->burst;
            index = counter;
        }
        cur = cur->next;
        counter++;
    }
    return index;
}

void sort_by_pid(LinkedList **head) {
    LinkedList *cur = *head;
    LinkedList *prev = NULL;
    int swapped = 1;
    while (swapped) {
        swapped = 0;
        cur = *head;
        while (cur->next != NULL) {
            if (cur->burst->pid > cur->next->burst->pid) {
                Burst *temp = cur->burst;
                cur->burst = cur->next->burst;
                cur->next->burst = temp;
                swapped = 1;
            }
            cur = cur->next;
        }
    }
}

double get_avg_turnaround(LinkedList **head){
    LinkedList *cur = *head;
    double sum = 0.0;
    int counter = 0;
    while (cur->next != NULL) {
        cur = cur->next;
        if (cur->burst->pid > 0) {
            sum += cur->burst->turnaround_time;
            counter++;
        }
    }
    return sum / counter;
}