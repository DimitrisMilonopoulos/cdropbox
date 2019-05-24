#ifndef _LIST_
#define _LIST_

struct Node
{
    struct ip_port *value;
    struct Node *next;
};
struct HeadNode
{
    struct Node *head;
    struct Node *tail;
};

/**Basic Queue**/
struct HeadNode *createQueue();
int InsertNode(struct HeadNode *, struct ip_port *);
int DeleteNode(struct HeadNode *, struct ip_port *);
int deleteList(struct HeadNode *);
void printList(struct HeadNode *headnode);
int FindNode(struct HeadNode *headnode, struct ip_port value);
#endif