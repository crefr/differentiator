#ifndef DIFFER_INCLUDED
#define DIFFER_INCLUDED

#include <stdint.h>

#include "bintree.h"

#define  val_(node) (((expr_elem_t *)node->data)->val)
#define type_(node) (((expr_elem_t *)node->data)->type)

enum elem_type{
    NUM = 0,
    OPR = 1,
    VAR = 2
};

enum oper{
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/',
    POW = '^',
    SIN,
    COS,
    TAN
};

typedef struct {
    const char * name;
    enum oper num;
} oper_t;

typedef struct {
    enum elem_type type;
    union {
        double number;
        int    var;
        enum oper op;
    } val;
} expr_elem_t;

const oper_t oper_table[] = {
    {.name = "+"  , .num = ADD},
    {.name = "add", .num = ADD},

    {.name = "-"  , .num = SUB},
    {.name = "*"  , .num = MUL},
    {.name = "/"  , .num = DIV},
    {.name = "^"  , .num = POW},
    {.name = "sin", .num = SIN},
    {.name = "cos", .num = COS},
    {.name = "tan", .num = TAN}
};
const size_t oper_table_size = sizeof(oper_table) / sizeof(*oper_table);

void setGlobalX(double x);

/// @brief evaluates the value of tree with the node as a root
double evaluate(node_t * node);

/// @brief reads equation in prefix form from the input_file
node_t * readEquationPrefix(FILE * input_file);

/// @brief make string about expression element, used in dump
void exprElemToStr(char * str, void * data);

/// @brief initializes some stuff
void diffInit();

const uint32_t NUM_COLOR = 0xAAFFAAFF;
const uint32_t VAR_COLOR = 0xFFAAAAFF;
const uint32_t OPR_COLOR = 0xFFFFAAFF;

#endif
