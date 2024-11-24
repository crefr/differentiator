#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"
#include "hashtable.h"

static void fillOperTable();

static double GlobalX = 0;

const size_t OPER_TABLE_SIZE = 32;
static table_t G_oper_table = {};

void setGlobalX(double x)
{
    GlobalX = x;
}

static void fillOperTable()
{
    G_oper_table = tableCtor(OPER_TABLE_SIZE);

    for (size_t oper_index = 0; oper_index < oper_table_size; oper_index++){
        tableInsert(&G_oper_table, oper_table[oper_index].name, &(oper_table[oper_index].num), sizeof(enum oper));
    }
}

void diffInit()
{
    fillOperTable();
}

double evaluate(node_t * node)
{
    assert(node);

    logPrint(LOG_DEBUG, "entered evaluate for root %p\n", node);
    logPrint(LOG_DEBUG_PLUS, "\tvalue: %lg, type: %d\n", val_(node).number, type_(node));

    if (type_(node) == OPR){
        switch (val_(node).op){
            case ADD:
                return evaluate(node->left) + evaluate(node->right);

            case SUB:
                return evaluate(node->left) - evaluate(node->right);

            case MUL:
                return evaluate(node->left) * evaluate(node->right);

            case DIV:
                return evaluate(node->left) / evaluate(node->right);

            default:
                logPrint(LOG_RELEASE, "evaluate operation type error\n");
                return 0.;
        }
    }
    if (type_(node) == NUM)
        return val_(node).number;

    if (type_(node) == VAR)
        return GlobalX;

    return 0.;
}

const size_t BUFFER_LEN = 32;

node_t * readEquationPrefix(FILE * input_file)
{
    node_t * node = NULL;

    fscanf(input_file, "(");

    char buffer[BUFFER_LEN] = "";
    fscanf(input_file, " %[^()] ", buffer);

    char * opr_ptr = NULL;
    if ((opr_ptr = strpbrk(buffer, "+-*/")) != NULL){
        expr_elem_t temp = {};

        temp.type = OPR;
        temp.val.op = (enum oper)(*opr_ptr);

        node = newNode(&temp, sizeof(temp),
            readEquationPrefix(input_file),
            readEquationPrefix(input_file),
            OPR_COLOR
        );
    }
    else {
        double number = 0.;
        if (sscanf(buffer, "%lg", &number) > 0){
            expr_elem_t temp = {};

            temp.type = NUM;
            temp.val.number = number;

            node = newNode(&temp, sizeof(temp), NULL, NULL, NUM_COLOR);
        }

        else {
            expr_elem_t temp = {};

            temp.type = VAR;
            temp.val.var = 0;

            node = newNode(&temp, sizeof(temp), NULL, NULL, VAR_COLOR);
        }
    }

    fscanf(input_file, ")");

    return node;
}

void exprElemToStr(char * str, void * data)
{
    expr_elem_t * elem = (expr_elem_t *)data;

    switch (elem->type){
        case OPR:
            sprintf(str, "type = 'OPR', value.op     = %d", elem->val.op);
            break;

        case NUM:
            sprintf(str, "type = 'NUM', value.number = %lg", elem->val.number);
            break;

        case VAR:
            sprintf(str, "type = 'VAR', value.var    = %d", elem->val.var);
            break;

        default:
            logPrint(LOG_RELEASE, "incorrect elem type in elemToStr\n");
            break;
    }
}
