#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linkedlist.h"

struct param {
    char *k;
    char *file;
};

LinkedList* results = NULL;
struct param params[10];

void *process_file(void *args) {
    struct param * parameters = (struct param*) args;
    FILE* file_ptr;

    // Open file in reading mode
    file_ptr = fopen(parameters->file , "r");

    if (file_ptr == NULL) {
        printf("Cannot Open Input File\n");
        exit(1);
    }

    char word[MAX_WORD_LEN];
    LinkedList* words;

    // Read words from file
    while (fscanf(file_ptr, "%s", word) == 1) {
        // convert word to uppercase
        for (int i = 0; word[i]; i++) {
            word[i] = toupper(word[i]);
        }
        // check if word is already in words array
        int found = 0;
        for (int i = 0; i < get_length(&words); i++) {
            if (strcmp(get_word(&words, i), word) == 0) {
                int count = get_count(&words, i) + 1;
                modify_count(&words, i, &count);
                found = 1;
                break;
            }
        }

        // if word is not in words array, add it
        if (!found) {
            int count = 1;
            insert(&words, word, &count);
        }
    }

    sort_list(&words);

    for (int i = 0; i < atoi(parameters->k); i++) {
        int count = get_count(&words, i);
        insert(&results, get_word(&words, i), &count);
    }

    // Close file
    fclose(file_ptr);
}

int main(int argc, char *argv[])
{
    int n = atoi(argv[3]);
    int k = atoi(argv[1]);
    pthread_t tids[n];
    
    // fork child processes
    for (int i = 0; i < n; i++) {
        params[i].file = argv[i+4];
        params[i].k = argv[1];
        pthread_create(&tids[i], NULL, process_file, &params[i]);
    }  

    for (int i = 0; i < n; i++){
        pthread_join(tids[i], NULL);
    }
    
    for (int i = 0; i < get_length(&results); i++) {

        for (int j = i + 1; j < get_length(&results); j++) {

            if (strcmp(get_word(&results, i), get_word(&results, j)) == 0) {

                int sum = get_count(&results, i) + get_count(&results, j);
                modify_count(&results, i, &sum);
                delete_node(&results, j);
                
            }
        }
    }

    sort_list(&results);

    FILE *output = fopen(argv[2], "w");
    if (output == NULL) {
        printf("Cannot Open Output File\n");
        exit(1);
    }
    
    for (int i = 0; i < get_length(&results); i++) {
        fprintf(output, "%s ", get_word(&results, i));
        fprintf(output, "%d\n", get_count(&results, i));
    }
    
    fclose(output);
}