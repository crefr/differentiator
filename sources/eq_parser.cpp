#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "bintree.h"
#include "differ.h"

typedef enum {
    SUCCESS = 0,
    SOFT_ERROR,
    HARD_ERROR
} parser_status_t;

typedef struct {
    parser_status_t status;
    const char * orig_str;
    const char *  cur_str;
} parser_context;

static parser_context parserInit(const char * string);

static void syntaxError(const char * expected, char real);

static bool getName(char * name, parser_context * context, size_t name_max_len);

static node_t * getExpr   (diff_t * diff, parser_context * context);

static node_t * getMulDiv (diff_t * diff, parser_context * context);

static node_t * getPrimary(diff_t * diff, parser_context * context);

static node_t * getVar    (diff_t * diff, parser_context * context);

static node_t * getNumber (diff_t * diff, parser_context * context);

static parser_context parserInit(const char * string)
{
    assert(string);

    parser_context context = {};

    context.status   = SUCCESS;
    context.orig_str = string;
    context.cur_str  = context.orig_str;

    return context;
}

node_t * parseEquation(diff_t * diff, const char * string)
{
    assert(diff);
    assert(string);

    parser_context context = parserInit(string);

    node_t * node = getExpr(diff, &context);

    if (*(context.cur_str) != '\0'){
        syntaxError("end of the string ('\\0')", *(context.cur_str));
        return NULL;
    }

    return node;
}

static node_t * getExpr(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    node_t * node = getMulDiv(diff, context);

    while (*(context->cur_str) == '+' || *(context->cur_str) == '-'){
        int oper = *(context->cur_str);
        (context->cur_str)++;

        node_t * node2 = getMulDiv(diff, context);

        if (oper == '+')
            node = newOprNode(ADD, node, node2);

        else
            node = newOprNode(SUB, node, node2);
    }

    return node;
}

static node_t * getMulDiv(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    node_t * node = getPrimary(diff, context);

    while (*(context->cur_str) == '*' || *(context->cur_str) == '/'){
        int op = *(context->cur_str);
        (context->cur_str)++;

        node_t * node2 = getPrimary(diff, context);

        if (op == '*')
            node = newOprNode(MUL, node, node2);

        else
            node = newOprNode(DIV, node, node2);
    }

    return node;
}

static node_t * getPrimary(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    if (*(context->cur_str) == '('){
        (context->cur_str)++;

        node_t * node = getExpr(diff, context);

        if (*(context->cur_str) != ')'){
            syntaxError(")", *(context->cur_str));
            return NULL;
        }

        (context->cur_str)++;

        return node;
    }

    node_t * var = getVar(diff, context);
    if (var != NULL)
        return var;

    return getNumber(diff, context);
}

static node_t * getNumber(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    const char * start = context->cur_str;
    double val = 0;

    while ('0' <= *(context->cur_str) && *(context->cur_str) <= '9'){
        val = val * 10 + *(context->cur_str) - '0';
        (context->cur_str)++;
    }
    const char * end = context->cur_str;

    if (start == end){
        syntaxError("digit ('0'-'9')", *(context->cur_str));
        return NULL;
    }

    return newNumNode(val);
}

static node_t * getVar(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    char var_name[NAME_MAX_LEN] = "";

    const char * start = context->cur_str;

    if (!getName(var_name, context, NAME_MAX_LEN)){
        context->cur_str = start;
        return NULL;
    }

    return getVarNode(diff, var_name);
}

static node_t * getFunc(diff_t * diff, parser_context * context)
{
    assert(diff);
    assert(context);

    char func_name[NAME_MAX_LEN] = "";

    const char * start = context->cur_str;

    if (!getName(func_name, context, NAME_MAX_LEN)){
        context->cur_str = start;
        return NULL;
    }

    if (*(context->cur_str) != '('){
        context->cur_str = start;
        return NULL;
    }

    name_t * func = tableLookup(&(diff->oper_table), func_name);
    if (func == NULL){
        syntaxError("one of the functions", 'n');   //TODO - refactor syntaxError
        exit(1);
        // return NULL;
    }

    node_t * arg_node = getExpr(diff, context);
    if (arg_node == NULL){
        syntaxError("argument", 'h');
    }
}

static bool getName(char * name, parser_context * context, size_t name_max_len)
{
    assert(name);
    assert(context);

    if ( !(('a' <= *(context->cur_str) && *(context->cur_str) <= 'z') || (*(context->cur_str) == '_')) )
        return false;

    size_t name_index = 0;

    while ((('a' <= *(context->cur_str) && *(context->cur_str) <= 'z') || (*(context->cur_str) == '_') || ('0' <= *(context->cur_str) && *(context->cur_str) <= '9'))){
        if (name_index < name_max_len - 1){
            name[name_index] = *(context->cur_str);
            name_index++;
        }

        (context->cur_str)++;
    }

    name[name_index] = '\0';

    return true;
}

static void syntaxError(const char * expected, char real)
{
    assert(expected);
    assert(real);

    if (real == '\0')
        fprintf(stderr, "SYNTAX ERROR: expected %s, but got end of the string ('\\0')\n", expected);

    else
        fprintf(stderr, "SYNTAX ERROR: expected %s, but got '%c'\n", expected, real);
}
