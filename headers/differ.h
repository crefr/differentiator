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
    ADD = 0,
    SUB,
    MUL,
    DIV,
    POW,
    SIN,
    COS,
    TAN,
    LN,
    LOG,
    FAC
};

typedef struct {
    enum elem_type type;
    union {
        double number;
        unsigned int var;
        enum oper op;
    } val;
} expr_elem_t;

const size_t NAME_MAX_LEN = 64;

typedef struct {
    char name[NAME_MAX_LEN];
    double value;
} var_t;

const size_t MAX_VAR_NUM = 16;

typedef struct {
    table_t oper_table;
    table_t  var_table;

    var_t vars[MAX_VAR_NUM];
    unsigned int var_num;
} diff_t;

typedef node_t * (*diff_func_t)(diff_t *, node_t *, unsigned int);

typedef struct {
    const char * name;
    enum oper num;

    bool binary;
    bool commutative;

    diff_func_t diffFunc;
} oper_t;

/// @brief evaluates the value of tree with the node as a root
double evaluate(diff_t * diff, node_t * node);

/// @brief reads equation in prefix form from the input_file
node_t * readEquationPrefix(diff_t * diff, FILE * input_file);

/// @brief make string about expression element, used in dump
void exprElemToStr(char * str, void * data);

void setVariables(diff_t * diff);

// node_t * makeDerivative(diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * taylorSeries(diff_t * diff, node_t * expr_node, unsigned int var_index, double diff_point, size_t last_member_index);

node_t * foldConstants(node_t * node, node_t * parent);

node_t * deleteNeutral(node_t * node, node_t * parent);

node_t * simplifyExpression(node_t * node);

node_t * newOprNode(enum oper op_num, node_t * left, node_t * right);

node_t * newNumNode(double num);

node_t * newVarNode(unsigned int var_index);

node_t * getVarNode(diff_t * diff, char * var_name);

size_t countVars(node_t * node, unsigned int var_index);

long unsigned int factorial(long unsigned int number);

/// @brief initializes some stuff
void diffInit(diff_t * diff);

void diffDtor(diff_t * diff);

void diffDump(diff_t * diff);

const uint32_t NUM_COLOR = 0xAAFFAAFF;
const uint32_t VAR_COLOR = 0xFFAAAAFF;
const uint32_t OPR_COLOR = 0xFFFFAAFF;

node_t * makeDerivative(diff_t * diff, node_t * expr_node, unsigned int var_index);


node_t * diffAddSub(diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffMul   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffDiv   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffPow   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffSin   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffCos   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffTan   (diff_t * diff, node_t * expr_node, unsigned int var_index);

node_t * diffLn    (diff_t * diff, node_t * expr_node, unsigned int var_index);

const oper_t opers[] = { //TODO make it independent of constant places in enum oper
    {.name = "+"  , .num = ADD, .binary = true,  .commutative = true , .diffFunc = diffAddSub},
    {.name = "-"  , .num = SUB, .binary = true,  .commutative = false, .diffFunc = diffAddSub},
    {.name = "*"  , .num = MUL, .binary = true,  .commutative = true , .diffFunc = diffMul   },
    {.name = "/"  , .num = DIV, .binary = true,  .commutative = false, .diffFunc = diffDiv   },
    {.name = "^"  , .num = POW, .binary = true,  .commutative = false, .diffFunc = diffPow   },
    {.name = "sin", .num = SIN, .binary = false, .commutative = false, .diffFunc = diffSin   },
    {.name = "cos", .num = COS, .binary = false, .commutative = false, .diffFunc = diffCos   },
    {.name = "tan", .num = TAN, .binary = false, .commutative = false, .diffFunc = diffTan   },
    {.name = "ln" , .num = LN , .binary = false, .commutative = false, .diffFunc = diffLn    },
    {.name = "log", .num = LOG, .binary = true,  .commutative = false, .diffFunc = NULL      },
    {.name = "!"  , .num = FAC, .binary = false, .commutative = false, .diffFunc = NULL      }
};
const size_t opers_size = sizeof(opers) / sizeof(*opers);

#endif
