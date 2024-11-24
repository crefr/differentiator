#ifndef DIFFER_INCLUDED
#define DIFFER_INCLUDED

#include <stdint.h>

#include "bintree.h"
#include "hashtable.h"

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
        unsigned int var;
        enum oper op;
    } val;
} expr_elem_t;

const size_t VAR_NAME_MAX_LEN = 8;

typedef struct {
    char name[VAR_NAME_MAX_LEN];
    double value;
} var_t;

const size_t MAX_VAR_NUM = 16;

typedef struct {
    table_t oper_table;
    table_t  var_table;

    var_t vars[MAX_VAR_NUM];
    unsigned int var_num;
} diff_context_t;

const oper_t opers[] = {
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
const size_t opers_size = sizeof(opers) / sizeof(*opers);

/// @brief evaluates the value of tree with the node as a root
double evaluate(diff_context_t * diff, node_t * node);

/// @brief reads equation in prefix form from the input_file
node_t * readEquationPrefix(diff_context_t * diff, FILE * input_file);

/// @brief make string about expression element, used in dump
void exprElemToStr(char * str, void * data);

void setVariables(diff_context_t * diff);

/// @brief initializes some stuff
void diffInit(diff_context_t * diff);

void diffDtor(diff_context_t * diff);

void diffDump(diff_context_t * diff);

const uint32_t NUM_COLOR = 0xAAFFAAFF;
const uint32_t VAR_COLOR = 0xFFAAAAFF;
const uint32_t OPR_COLOR = 0xFFFFAAFF;

#endif
