#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"
#include "hashtable.h"

static void fillOperTable(diff_context_t * diff);

const size_t OPR_TABLE_SIZE = 32;

const size_t VAR_TABLE_SIZE = 32;


static void fillOperTable(diff_context_t * diff)
{
    assert(diff);

    for (size_t oper_index = 0; oper_index < opers_size; oper_index++){
        tableInsert(&(diff->oper_table), opers[oper_index].name, &(opers[oper_index].num), sizeof(enum oper));
    }
}

void diffInit(diff_context_t * diff)
{
    assert(diff);

    diff->oper_table = tableCtor(OPR_TABLE_SIZE);
    diff->var_table  = tableCtor(VAR_TABLE_SIZE);

    fillOperTable(diff);
}

void diffDtor(diff_context_t * diff)
{
    tableDtor(&(diff->oper_table));
    tableDtor(&(diff-> var_table));
}

double evaluate(diff_context_t * diff, node_t * node)
{
    assert(node);

    logPrint(LOG_DEBUG, "entered evaluate for root %p\n", node);
    logPrint(LOG_DEBUG_PLUS, "\tvalue: %lg, type: %d\n", val_(node).number, type_(node));

    if (type_(node) == OPR){
        switch (val_(node).op){
            case ADD:
                return evaluate(diff, node->left) + evaluate(diff, node->right);

            case SUB:
                return evaluate(diff, node->left) - evaluate(diff, node->right);

            case MUL:
                return evaluate(diff, node->left) * evaluate(diff, node->right);

            case DIV:
                return evaluate(diff, node->left) / evaluate(diff, node->right);

            default:
                logPrint(LOG_RELEASE, "evaluate operation type error\n");
                return 0.;
        }
    }
    if (type_(node) == NUM)
        return val_(node).number;

    if (type_(node) == VAR){
        return diff->vars[val_(node).var].value;
    }

    return 0.;
}

const size_t BUFFER_LEN = 32;

node_t * readEquationPrefix(diff_context_t * diff, FILE * input_file)
{
    node_t * node = NULL;

    fscanf(input_file, " ( ");

    char buffer[BUFFER_LEN] = "";
    fscanf(input_file, " %[^() ] ", buffer);

    name_t * operation = tableLookup(&(diff->oper_table), buffer);
    if (operation != NULL){
        expr_elem_t temp = {};

        temp.type = OPR;
        temp.val.op = (enum oper) *((int *)(operation->data));

        node = newNode(&temp, sizeof(temp),
            readEquationPrefix(diff, input_file),
            readEquationPrefix(diff, input_file),
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
            unsigned int var_index = 0;

            name_t * variable = tableLookup(&(diff->var_table), buffer);
            if (variable == NULL){
                tableInsert(&(diff->var_table), buffer, &(diff->var_num), sizeof(diff->var_num));
                var_index = diff->var_num;

                strncpy(diff->vars[var_index].name, buffer, VAR_NAME_MAX_LEN - 1);

                diff->var_num++;
            }
            else {
                var_index = *(unsigned int *)(variable->data);
            }

            expr_elem_t temp = {};

            temp.type = VAR;
            temp.val.var = var_index;

            node = newNode(&temp, sizeof(temp), NULL, NULL, VAR_COLOR);
        }
    }

    fscanf(input_file, " )");

    return node;
}

void setVariables(diff_context_t * diff)
{
    for (size_t var_index = 0; var_index < diff->var_num; var_index++){
        printf("enter value of variable '%s':\n", diff->vars[var_index].name);
        scanf(" %lg", &(diff->vars[var_index].value));
        printf("scanned\n");
    }
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
            sprintf(str, "type = 'VAR', value.var    = %u", elem->val.var);
            break;

        default:
            logPrint(LOG_RELEASE, "incorrect elem type in elemToStr\n");
            break;
    }
}

void diffDump(diff_context_t * diff)
{
    logPrint(LOG_DEBUG, "<h2>-----DIFFERENTIATOR DUMP-----</h2>\n");

    logPrint(LOG_DEBUG, "variables:\n");
    for (unsigned int var_index = 0; var_index < diff->var_num; var_index++){
        logPrint(LOG_DEBUG, "\tvariable #%u:\n"
                            "\t\tname:  %s\n"
                            "\t\tvalue: %lg\n",
                             var_index, diff->vars[var_index].name, diff->vars[var_index].value);
    }
}
