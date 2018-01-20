#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define STOUT 1
#define STIN 0
#define MAX_ARG_LEN 1040
struct Node2
{
    int job_id; 
    struct Node2 *next;
} Node2;


struct Node2* push_job(struct Node2 **head, int job) {
    struct Node2 *tmp;
    tmp = (struct Node2*)malloc(sizeof(struct Node2));
    if (*head == NULL) {
        tmp->job_id= job;
        *head = tmp;
    } else {
        tmp->job_id = job;
        tmp->next = *head;
        *head = tmp;
    }
    return *head;
}

int pop_job(struct Node2 **head) {
    struct Node2 *tmp;
       if (head == NULL) {
            perror("attempting to access empty list");
            return -1; 
        }
    else {
        tmp = *head;
        if(tmp) {
            *head = tmp->next;
        }
        return tmp->job_id;
    }
}