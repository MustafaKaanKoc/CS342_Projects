#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

void insert(LinkedList **head, char *word, int *count) {
    LinkedList* new_node = (LinkedList *) malloc(sizeof(LinkedList));
    // Set the data for the new node
    strcpy(new_node->word, word);
    new_node->count = *count;
    
    new_node->next = *head;
    
    *head = new_node;
}

void delete_node(LinkedList** head, int index) {
    // If the list is empty, return
    if (*head == NULL) {
        return;
    }
    
    // Find the node with the key value
    LinkedList* current = *head;
    for (int i = 0; i < index - 1; i++)
        current = current->next;
    
    LinkedList *to_delete = current->next;
    current->next = (current->next)->next;

    free(to_delete);
}

void modify_count(LinkedList **head, int index, int *value){
    LinkedList *cur = *head;

    for (int i = 0; i < index; i++){
        cur = cur->next;
    }

    cur->count = *value;
}

void modify_word(LinkedList** head,int index,char* value){
    LinkedList *cur = *head;

    for (int i = 0; i < index; i++){
        cur = cur->next;
    }

    strcpy(cur->word, value);
}

int get_count(LinkedList **head, int index){
    LinkedList *cur = *head;
    for (int i = 0; i < index; i++){
        cur=cur->next;
    }
    return cur->count;
}

char* get_word(LinkedList **head, int index){
    LinkedList * cur = *head;
    for (int i = 0; i < index; i++) {
        cur = cur->next;
    }
    return cur->word;
}

int get_length(LinkedList** head){
    if ( *head == NULL)
        return 0;
    
    LinkedList *cur = *head;
    int counter=1;
    while( cur->next != NULL) {
        counter++;
        cur = cur->next;
    }
    
    return counter;
}

void sort_list(LinkedList **head){
    for (int i = 1, j; i < get_length(head); i++) {
        int key = get_count(head, i);
        char temp[MAX_WORD_LEN];
        strcpy(temp, get_word(head, i));
        j = i - 1;
       
        while (j >= 0 && get_count(head, j) < key) {
            int new_value = get_count(head, j);
            modify_count(head, j+1, &new_value);
            modify_word(head, j+1, get_word(head, j));
            j = j - 1;
        }

        modify_count(head, j+1, &key);
        modify_word(head, j+1, temp);
    }

    for (int i = 0; i < get_length(head); i++) {
        
        for (int j = i + 1; j < get_length(head); j++) {

            if (get_count(head, i) == get_count(head, j)) {

                int result = strcmp(get_word(head, i), get_word(head, j));
                if (result > 0) {
                    char temp[MAX_WORD_LEN];
                    strcpy(temp, get_word(head, i));
                    modify_word(head, i, get_word(head, j));
                    modify_word(head, j, temp);
                }
            }
        }
    }

    
}

void delete_list(LinkedList **head){
    while (*head !=NULL) {
        delete_node(head, 0);
    }
}