#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"

static double GlobalX = 0;

void setGlobalX(double x)
{
    GlobalX = x;
}

double evaluate(node_t * node)
{
    assert(node);

    logPrint(LOG_DEBUG, "entered evaluate for root %p\n", node);
    logPrint(LOG_DEBUG_PLUS, "\tvalue: %lg, type: %d\n", val_(node).dval, type_(node));

    if (type_(node) == OPR){
        printf("operation %d\n", val_(node).ival);
        switch (val_(node).ival){
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
        return val_(node).dval;

    if (type_(node) == VAR)
        return GlobalX;
}
