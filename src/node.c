//
// Created by bryanzierk on 2/7/22.
//

#include <node.h>

// Add a new node to the end of the linked list which contains the designated bg pid
void add_node(struct node **head, int bg_pid) {
    struct node *new_node = malloc(sizeof(struct node));

    struct node *last = *head;

    new_node->bg_pid = bg_pid;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    }

    if (last != NULL) {
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_node;
    }
}

void remove_node(struct node **head, int pid) {
    struct node *curr_node = *head;
    struct node *next;
    struct node *prev;

    if (curr_node == NULL) {
        return;
    }

    while (curr_node->bg_pid != pid) {
        // pid not found in linked list
        if (curr_node->next == NULL) {
            return;
        } else {
            prev = curr_node;
            curr_node = curr_node->next;
        }
    }

    if (curr_node == *head) {
        *head = curr_node->next;
    } else {
        prev->next = curr_node->next;
        free(curr_node);
    }
}

// Iterates through the linked list freeing any allocated memory
void free_list(struct node **head) {
    struct node *curr_node = *head;
    struct node *next;

    while (curr_node != NULL) {
        next = curr_node->next;
        free(curr_node);
        curr_node = next;
    }
    *head = NULL;
}

void display_list(struct node **head) {
    struct node **curr_node = head;

    while (*curr_node) {
        printf("bg_pid: %d\n", (*curr_node)->bg_pid);
        fflush(stdout);
        curr_node = &(*curr_node)->next;
    }
}