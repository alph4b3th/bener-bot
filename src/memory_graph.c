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

Node *memory_create_node(int id, const char *type, const char *property)
{
    Node *node = malloc(sizeof(Node));
    node->id = id;
    node->Next = NULL;
    node->edge_next = NULL;
    node->edges = NULL;

    strcpy(node->property, property);
    strcpy(node->type, type);

    return node;
}

void memory_add_node(Graph *graph, Node *node)
{
    node->Next = graph->nodes;
    graph->nodes = node;
    graph->node_count++;
}

void memory_add_edge(Node *from, Node *to)
{
    Node *edge = memory_create_node(to->id, to->type, to->property);
    edge->edge_next = from->edges;
    from->edges = edge;
}
Node *memory_find_node_from_id(Graph *graph, int id)
{
    Node *current = graph->nodes;

    while (current != NULL)
    {
        if (current->id == id)
        {
            return current;
        }

        current = current->Next;
    }

    return NULL;
}

void memory_show(Graph *graph)
{
    Node *current = graph->nodes;
    printf("=== Grafo de Memória ===\n");
    while (current != NULL)
    {
        printf("Nó ID: %d\n", current->id);
        printf("  Tipo     : %s\n", current->type);
        printf("  Propriedade: %s\n", current->property);

        Node *edge = current->edges;

        if (edge)
        {
            printf("  Conexões:\n");
            while (edge != NULL)
            {
                printf("    -> Nó ID: %d | Tipo: %s | Propriedade: %s\n",
                       edge->id, edge->type, edge->property);
                edge = edge->edge_next;
            }
        }
        else
        {
            printf("  Conexões: (nenhuma)\n");
        }

        printf("-------------------------\n");

        current = current->Next;
    }
}