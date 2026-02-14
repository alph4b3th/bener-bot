#ifndef MEMORY_GRAPH_H
#define MEMORY_GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    int id;
    char type[50];
    char property[100];

    struct Node *Next;
    struct Node *edges;
    struct Node *edge_next;
} Node;

typedef struct Graph
{
    Node *nodes;
    int node_count;
} Graph;


Node *memory_create_node(int id, const char *type, const char *property);
void memory_add_node(Graph *graph, Node *node);
void memory_add_edge(Node *from, Node *to);
Node* memory_find_node_from_id(Graph *graph, int id);
void memory_show(Graph *graph);

#endif // MEMORY_GRAPH_H
