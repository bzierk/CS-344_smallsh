//
// Created by bryanzierk on 2/7/22.
//

#ifndef CS_344_SMALLSH_NODE_H
#define CS_344_SMALLSH_NODE_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

struct node {
    pid_t bg_pid;
    struct node *next;
};

void add_node(struct node **head, int bg_pid);
void free_list(struct node **head);
void display_list(struct node **head);
void remove_node(struct node **head, int pid);

#endif //CS_344_SMALLSH_NODE_H
