#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <ctype.h>
#include "linkedlist.h"

#define MAX_WORD_CNT 10000 // k_max = 1000, n_max = 10
// #define SIZE 1024

int main(int argc, char *argv[])
{
    int n = atoi(argv[3]);
    int k = atoi(argv[1]);
    int SIZE = 2 * k * n * sizeof(char[MAX_WORD_LEN]);
    pid_t pids[n];
    char *ptr_array[n];
    
    const char *NAME = "/my-shared-memory-object";
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);
    char *shm_ptr = (char *) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    for (int i = 0; i < n; i++) {
        ptr_array[i] = (char *) shm_ptr + (i * (SIZE/n));
    }
    
    // fork child processes
    for (int i = 0; i < n; ++i) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            fprintf(stderr, "Fork Failed");
            return 1;
        } 
        // child process
        else if (pids[i] == 0) {
            FILE* file_ptr;
            char ch;
 
            // Open file in reading mode
            file_ptr = fopen(argv[i+4], "r");
 
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
                    // puts("3");
                    insert(&words, word, &count);
                }
            }

            sort_list(&words);

            for (int j = 0; j < k; j++) {
                ptr_array[i] += sprintf(ptr_array[i], "%s ", get_word(&words, j));
                ptr_array[i] += sprintf(ptr_array[i], "%d\n", get_count(&words,j));
            }
            // Close file
            fclose(file_ptr);
            delete_list(&words);
            exit(0);
        }
    }

    int cnt = n;
    while (cnt > 0) {
        wait(NULL);
        --cnt;
    }
    
    char *result_words_and_counts[MAX_WORD_CNT];
    int count = 0;
    for (int i = 0; i < n; i++) {
        char * token = strtok(ptr_array[i], "\n");
        // loop through the string to extract all other tokens
        while( token != NULL ) {
            result_words_and_counts[count++]=token;
            token = strtok(NULL, "\n");
        }
    }

    // printf( "-%s-", result_words_and_counts[3] ); //printing each token
    char *result_words[MAX_WORD_CNT];
    int result_counts[MAX_WORD_CNT];
    for (int i = 0; i < count; i++) {
        char * token = strtok(result_words_and_counts[i], " ");
        // loop through the string to extract all other tokens
        while( token != NULL ) {
            result_words[i] = token;
            token = strtok(NULL, " ");
            result_counts[i] = atoi(token);
            token = strtok(NULL, " ");
        }
    }

    for (int i = 0; i < count; i++) {
        if (result_words[i] == NULL)
            continue;
        
        for (int j = i + 1; j < count; j++) {
            if (result_words[j] == NULL)
                continue;
            else if (strcmp(result_words[i],result_words[j]) == 0) {
                result_counts[i] += result_counts[j];
                result_words[j] = NULL;
                result_counts[j] = -1;
            }
        }        
    }

    for (int i = 1, j; i < count; i++) {
       int key = result_counts[i];
       char *temp = result_words[i];
       j = i - 1;
       
       while (j >= 0 && result_counts[j] < key) {
           result_counts[j+1] = result_counts[j];
           result_words[j+1] = result_words[j];
           j = j - 1;
       }

       result_counts[j+1] = key;
       result_words[j+1]= temp;
    }

    for (int i = 0; i < count; i++) {
        if (result_words[i] == NULL)
            continue;
        
        for (int j = i + 1; j < count; j++)
        {
            if (result_words[j] == NULL)
                continue;
            else if (result_counts[i] == result_counts[j])
            {
                int result = strcmp(result_words[i], result_words[j]);
                if (result > 0) {
                    char * temp = result_words[i];
                    result_words[i] = result_words[j];
                    result_words[j] = temp;
                }
            }
        }
    }
    
    FILE *output = fopen(argv[2], "w");
    if (output == NULL) {
        printf("Cannot Open File\n");
        exit(1);
    }
    
    for (int i = 0; i < count; i++) {
        if (result_words[i] != NULL)
        {
            fprintf(output, "%s ", result_words[i]);
            fprintf(output, "%d\n", result_counts[i]);
        }
    }
    
    fclose(output);
}