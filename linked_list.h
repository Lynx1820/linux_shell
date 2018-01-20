#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define STOUT 1
#define STIN 0
#define MAX_ARG_LEN 1040
struct Node
{
    int pid;
    int job_id; 
    char *name;
    struct Node *next;
    char *status;
} Node;


struct Node* push(struct Node **head, int ppid, char *name_s[100], char *s, int j) {
    struct Node *tmp;
    if (head == NULL) {
        tmp = (struct Node*)malloc(sizeof(struct Node));
        tmp->pid = ppid;
        tmp->name = *name_s;
        tmp->status = s;
        tmp->job_id = j;
        *head = tmp;
    } else {
        tmp = (struct Node*) malloc(sizeof(struct Node));
        tmp->pid = ppid;
        tmp->name = *name_s;
        tmp->status = s;
        tmp->job_id = j;
        tmp->next = *head;
        *head = tmp;
    }
    return *head;
}

struct Node* pop(struct Node **head) {
    struct Node *tmp;
       if (head == NULL) {
            return NULL;
             perror("attempting to access empty list");

        }
    else {
        tmp = *head;
        if(tmp) {
            if (tmp->next == NULL) {
                *head = NULL;
            }
            else {
                *head = tmp->next;
            }
        }
        return tmp;
    }
}

struct Node *remove_node(struct Node **head, int jobx) {
    struct Node *prev;
    struct Node *tmp;
    struct Node *curr;
    curr = *head;
    if (head == NULL) {
        perror("attempting to access empty list");
    }
    else if (curr->job_id == jobx) {
        tmp = *head;
        *head = NULL;
        return tmp;
    }
    else {
        while(curr->next != NULL) {
            if (curr->next->job_id == jobx) {
                prev = curr;
                tmp = curr->next;
                prev->next = curr->next->next;
                return tmp;
            }
            curr = curr->next;
        }
    }
    return NULL;
}


int get_ppid(struct Node *head, int pidx) {
    int i = 0;
    struct Node *tmp = head;

    while(tmp != NULL) {
        if (i == pidx) {
            return tmp->pid;
        }
        tmp = tmp->next;
        i++;
    }
    return -1;
}

int get_head_ppid(struct Node *head) {
    return head->pid;
}


char *get_name(struct Node *head, int pidx) {
    int i = 0;
    struct Node *tmp = head;

    while(tmp != NULL) {
        if (i == pidx) {
            return tmp->name;
        }
        tmp = tmp->next;
        i++;
    }
    return NULL;
}

char *get_status(struct Node *head, int pidx) {
    int i = 0;
    struct Node *tmp = head;

    while(tmp != NULL) {
        if (i == pidx) {
            return tmp->status;
        }
        tmp = tmp->next;
        i++;
    }
    return NULL;
}

void print_list(struct Node* head){
    struct Node* curr = head;
    write(STOUT, "----------LIST-------\n", 22);
    while(curr != NULL){
        printf("%d\n", curr->pid);
        curr=curr->next;
    }
    write(STOUT, "---------------------\n", 22);
}