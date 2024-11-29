#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bintree.h"
#include "differ.h"

node_t * getE(const char ** str);

node_t * getT(const char ** str);

node_t * getP(const char ** str);

node_t * getV(const char ** str);

node_t * getN(const char ** str);

node_t * parseEquation(const char * str)
{
    node_t * node = getE(&str);

    if (*str != '\0'){
        fprintf(stderr, "SYNTAX ERROR: expected end of the equation but scanned '%c'\n", *str);
        return NULL;
    }

    return node;
}

node_t * getE(const char ** str)
{
    node_t * node = getT(str);

    while (**str == '+' || **str == '-'){
        int oper = **str;
        (*str)++;

        node_t * node2 = getT(str);

        if (oper == '+')
            node = newOprNode(ADD, node, node2);

        else
            node = newOprNode(SUB, node, node2);
    }

    return node;
}

node_t * getT(const char ** str)
{
    node_t * node = getP(str);

    while (**str == '*' || **str == '/'){
        int op = **str;
        (*str)++;

        node_t * node2 = getP(str);

        if (op == '*')
            node = newOprNode(MUL, node, node2);

        else
            node = newOprNode(DIV, node, node2);
    }

    return node;
}

node_t * getP(const char ** str)
{
    if (**str == '('){
        (*str)++;

        node_t * node = getE(str);

        if (**str != ')'){
            printf("scanned: %c, expected ')'\n", **str);
            exit(1);
        }
        (*str)++;

        return node;
    }
    return getN(str);
}

node_t * getN(const char ** str)
{
    double val = 0;

    while ('0' <= **str && **str <= '9'){
        val = val * 10 + **str - '0';
        (*str)++;
    }
    return newNumNode(val);
}
