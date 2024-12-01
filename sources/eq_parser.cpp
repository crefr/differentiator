#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "bintree.h"
#include "differ.h"

static node_t * getExpr   (diff_t * diff, const char ** str);

static node_t * getMulDiv (diff_t * diff, const char ** str);

static node_t * getPrimary(diff_t * diff, const char ** str);

static node_t * getVar    (diff_t * diff, const char ** str);

static node_t * getNumber (diff_t * diff, const char ** str);

node_t * parseEquation(diff_t * diff, const char * str)
{
    assert(str);

    node_t * node = getExpr(diff, &str);

    if (*str != '\0'){
        fprintf(stderr, "SYNTAX ERROR: expected end of the equation but scanned '%c'\n", *str);
        return NULL;
    }

    return node;
}

static node_t * getExpr(diff_t * diff, const char ** str)
{
    assert(str);

    node_t * node = getMulDiv(diff, str);

    while (**str == '+' || **str == '-'){
        int oper = **str;
        (*str)++;

        node_t * node2 = getMulDiv(diff, str);

        if (oper == '+')
            node = newOprNode(ADD, node, node2);

        else
            node = newOprNode(SUB, node, node2);
    }

    return node;
}

static node_t * getMulDiv(diff_t * diff, const char ** str)
{
    assert(str);

    node_t * node = getPrimary(diff, str);

    while (**str == '*' || **str == '/'){
        int op = **str;
        (*str)++;

        node_t * node2 = getPrimary(diff, str);

        if (op == '*')
            node = newOprNode(MUL, node, node2);

        else
            node = newOprNode(DIV, node, node2);
    }

    return node;
}

static node_t * getPrimary(diff_t * diff, const char ** str)
{
    assert(str);

    if (**str == '('){
        (*str)++;

        node_t * node = getExpr(diff, str);

        if (**str != ')'){
            printf("scanned: %c, expected ')'\n", **str);
            exit(1);
        }
        (*str)++;

        return node;
    }

    if ('a' <= **str && **str <= 'z' || **str == '_')
        return getVar(diff, str);

    return getNumber(diff, str);
}

static node_t * getNumber(diff_t * diff, const char ** str)
{
    assert(str);

    double val = 0;

    while ('0' <= **str && **str <= '9'){
        val = val * 10 + **str - '0';
        (*str)++;
    }
    return newNumNode(val);
}

static node_t * getVar(diff_t * diff, const char ** str)
{
    assert(str);

    char   var_name[VAR_NAME_MAX_LEN] = "";
    size_t name_index = 0;

    while ( (name_index < VAR_NAME_MAX_LEN - 1) &&
            (('a' <= **str && **str <= 'z') || (**str == '_') || ('0' <= **str && **str <= '9'))){
        var_name[name_index] = **str;
        name_index++;
        (*str)++;
    }

    return getVarNode(diff, var_name);
}
