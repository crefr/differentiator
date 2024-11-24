#ifndef BINTREE_INCLUDED
#define BINTREE_INCLUDED

#include <stdint.h>

/// @brief structure for node
typedef struct node {
    void * data;
    size_t elem_size;

    node * parent;

    node * left;
    node * right;

    uint32_t color_for_dump;
} node_t;

typedef void(*printfunc_t)(void *);
typedef int (*compare_func_t)(void *, void *);
typedef void(*elemtostr_func_t)(char *, void *);

/// @brief dump mode
typedef enum {
    DUMP_HARD,
    DUMP_MEDIUM,
    DUMP_SOFT
} dump_mode_t;

/// @brief creates new node
node_t * newNode(void * data, size_t elem_size, node_t * left_child, node_t * right_child, uint32_t color);

/// @brief deletes node
void delNode(node_t * node);

/// @brief recursively destroys tree starting with the node
void treeDestroy(node_t * node);

/// @brief copies tree with the node as the root
node_t * treeCopy(node_t * node);

/// @brief prints tree starting with the node
void printTree(node_t * node, printfunc_t printElem);

/// @brief adds new node as in sorting bin tree
void treeSortAddNode(node_t * node, void * data, size_t elem_size, compare_func_t cmp, uint32_t color);

/// @brief finds node in tree starting with the root_node
node_t * treeFindNode(node_t * root_node, void * data, compare_func_t cmp);

/// @brief sets dump mode
void treeSetDumpMode(dump_mode_t mode);

/// @brief dumps graph to log file, operates with char strings
void treeDumpGraph(node_t * root_node, elemtostr_func_t elemToStr);

/// @brief makes dot file for current tree, starting with the root_node
void treeMakeDot(node_t * root_node, elemtostr_func_t elemToStr, FILE * dot_file);

/*---------------------------------------------------------------------------------*/
int cmpInt(void * first, void * second);

void intToStr(char * str, void * data);

void printInt(void * data);
/*---------------------------------------------------------------------------------*/

const char * const  ROOT_COLOR = "#FFFFAA";
const char * const  LEFT_COLOR = "#AAFFAA";
const char * const RIGHT_COLOR = "#FFAAAA";

#endif
