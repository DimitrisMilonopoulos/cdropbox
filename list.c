#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "info.h"
#include "list.h"

/****************************************Basic Queue**********************************************/

struct HeadNode *createQueue()
{
    struct HeadNode *headnode = malloc(sizeof(struct HeadNode));

    if (headnode == NULL)
        return NULL;
    headnode->head = NULL;
    headnode->tail = NULL;
    return headnode;
}

int InsertNode(struct HeadNode *headnode, struct ip_port *value)
{
    struct Node *newnode = malloc(sizeof(struct Node));

    if (newnode == NULL)
    {
        return 0;
    }

    //Insert the value
    newnode->value = value;
    newnode->next = NULL;

    if (headnode->head == NULL)
    { //List is empty
        headnode->head = newnode;
        headnode->tail = newnode;
    }
    else
    {
        headnode->tail->next = newnode;
        headnode->tail = newnode;
    }
    return 1;
}

int FindNode(struct HeadNode *headnode, struct ip_port value)
{
    struct Node *curr;
    curr = headnode->head;
    while (curr != NULL)
    {
        if (curr->value->ip == value.ip && curr->value->port == value.port)
        {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

int DeleteNode(struct HeadNode *headnode, struct ip_port *value)
{
    struct Node *prev;
    struct Node *curr;

    curr = headnode->head;

    while (curr != NULL)
    {
        if (curr->value->ip == value->ip) //curr->value->port == value->port  //Check if this node
        {
            if (curr == headnode->head)
            {
                if (curr == headnode->tail) //Only item in the list
                {
                    free(curr);
                    headnode->tail = NULL;
                    headnode->head = NULL;
                    return 1;
                }
                else
                {
                    headnode->head = curr->next;
                    free(curr);
                    return 1;
                }
            }
            else if (curr->next == NULL)
            {
                prev->next = curr->next;
                headnode->head = prev;
                free(curr);
                return 1;
            }
            else
            {
                prev->next = curr->next;
                free(curr);
                return 1;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

int deleteList(struct HeadNode *headnode)
{
    struct Node *curr;
    struct Node *next;

    curr = headnode->head;
    while (curr != NULL)
    {
        next = curr->next;
        free(curr->value);
        free(curr);
        curr = next;
    }

    headnode->head = NULL;
    headnode->tail = NULL;

    return 1;
}

void printList(struct HeadNode *headnode)
{
    struct Node *curr;
    curr = headnode->head;
    while (curr != NULL)
    {
        printf("item: IP: %d PORT: %d\n", curr->value->ip, curr->value->port);
        curr = curr->next;
    }
}