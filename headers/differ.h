#ifndef DIFFER_INCLUDED
#define DIFFER_INCLUDED

#include "bintree.h"

#define  val_(node) (((expr_elem_t *)node->data)->val)
#define type_(node) (((expr_elem_t *)node->data)->type)

typedef enum {
    NUM = 0,
    OPR = 1,
    VAR = 2
} type_t;

typedef struct {
    type_t type;
    union {
        double dval;
        int    ival;
    } val;
} expr_elem_t;

typedef enum {
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4
} oper_t;

void setGlobalX(double x);

/// @brief evaluates the value of tree with the node as a root
double evaluate(node_t * node);

#endif
