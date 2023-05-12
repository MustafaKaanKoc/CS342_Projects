#ifndef __LINKED_LIST_
#define __LINKED_LIST_

#define MAX_WORD_LEN 64

typedef struct node{
   char word[MAX_WORD_LEN];
   int count;
   struct node *next;
} LinkedList;

void insert(LinkedList **head, char* word, int *count);

void delete_node(LinkedList **head, int index);

void modify_count(LinkedList **head, int index, int *value);

void modify_word(LinkedList **head, int index, char* value);

char* get_word(LinkedList **head, int index);

int get_count(LinkedList **head, int index);

int get_length(LinkedList **head);

void sort_list(LinkedList **head);

void delete_list(LinkedList **head);

#endif