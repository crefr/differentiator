#ifndef DIFFER_INCLUDED
#define DIFFER_INCLUDED

#include <stdint.h>

#include "bintree.h"

#define  val_(node) (((expr_elem_t *)node->data)->val)
#define type_(node) (((expr_elem_t *)node->data)->type)

typedef enum {
    NUM = 0,
    OPR = 1,
    VAR = 2
} type_t;

typedef enum {
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/'
} oper_t;

typedef struct {
    type_t type;
    union {
        double number;
        int    var;
        oper_t op;
    } val;
} expr_elem_t;

void setGlobalX(double x);

/// @brief evaluates the value of tree with the node as a root
double evaluate(node_t * node);

/// @brief reads equation in prefix form from the input_file
node_t * readEquationPrefix(FILE * input_file);

/// @brief make string about expression element, used in dump
void exprElemToStr(char * str, void * data);

const uint32_t NUM_COLOR = 0xAAFFAAFF;
const uint32_t VAR_COLOR = 0xFFAAAAFF;
const uint32_t OPR_COLOR = 0xFFFFAAFF;

#endif
