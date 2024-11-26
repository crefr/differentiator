#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"
#include "hashtable.h"

static void fillOperTable(diff_context_t * diff);

static node_t * newOprNode(enum oper op_num, node_t * left, node_t * right);

static node_t * newNumNode(double num);

static node_t * newVarNode(unsigned int var_index);

const size_t OPR_TABLE_SIZE = 32;

const size_t VAR_TABLE_SIZE = 32;

void diffInit(diff_context_t * diff)
{
    assert(diff);

    diff->oper_table = tableCtor(OPR_TABLE_SIZE);
    diff-> var_table = tableCtor(VAR_TABLE_SIZE);

    fillOperTable(diff);
}

void diffDtor(diff_context_t * diff)
{
    tableDtor(&(diff->oper_table));
    tableDtor(&(diff-> var_table));
}

static void fillOperTable(diff_context_t * diff)
{
    assert(diff);

    for (size_t oper_index = 0; oper_index < opers_size; oper_index++){
        tableInsert(&(diff->oper_table), opers[oper_index].name, opers + oper_index, sizeof(oper_t));
    }
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

bool foldConstants(node_t * node, double * ans)
{
    if (type_(node) == VAR)
        return false;

    if (type_(node) == NUM){
        *ans = val_(node).number;
        return true;
    }

    if (opers[val_(node).op].binary){ //TODO make the same for unary
        double  left_val = 0;
        double right_val = 0;

        bool  left_is_const = foldConstants(node->left , &left_val);
        bool right_is_const = foldConstants(node->right, &right_val);

        if (left_is_const){
            treeDestroy(node->left);
            node->left = newNumNode(left_val);
            node->left->parent = node;
        }
        if (right_is_const){
            treeDestroy(node->right);
            node->right = newNumNode(right_val);
            node->right->parent = node;
        }

        if (left_is_const && right_is_const){
            switch (val_(node).op){
                case ADD:
                    *ans = left_val + right_val;
                    break;

                case SUB:
                    *ans = left_val - right_val;
                    break;

                case MUL:
                    *ans = left_val * right_val;
                    break;

                case DIV:
                    *ans = left_val / right_val;
                    break;

                default:
                    return false;
            }
            return true;
        }
    }
    return false;
}

node_t * deleteNeutral(node_t * node, node_t * parent)
{
    if (node == NULL)
        return NULL;

    if (type_(node) != OPR)
        return node;

    node->left  = deleteNeutral(node->left,  node);
    node->right = deleteNeutral(node->right, node);

    node_t * cur_node = node->left;
    node_t * another_node = node->right;
    while (cur_node != NULL) {
        switch (val_(node).op){
            case MUL:
                if (type_(cur_node) == NUM){
                    /* x*1 = x */
                    if (val_(cur_node).number == 1.){
                        delNode(cur_node);
                        delNode(node);

                        another_node->parent = parent;
                        return another_node;
                    }
                    /* x*0 = 0 */
                    else if (val_(cur_node).number == 0.){
                        treeDestroy(another_node);
                        delNode(node);

                        cur_node->parent = parent;
                        return cur_node;
                    }
                }
            case ADD:
                if (type_(cur_node) == NUM){
                    if (val_(cur_node).number == 0.){
                        delNode(cur_node);
                        delNode(node);

                        another_node->parent = parent;
                        return another_node;
                    }
                }
            default:
                break;
        }

        if (cur_node == node->left){
            cur_node     = node->right;
            another_node = node->left;
        }
        else
            cur_node = NULL;
    }
    return node;
}

const size_t BUFFER_LEN = 32;

node_t * readEquationPrefix(diff_context_t * diff, FILE * input_file)
{
    node_t * node = NULL;

    fscanf(input_file, " ( ");

    char buffer[BUFFER_LEN] = "";
    fscanf(input_file, " %[^() ] ", buffer);

    name_t * oper_in_table = tableLookup(&(diff->oper_table), buffer);
    if (oper_in_table != NULL){
        oper_t * operation = (oper_t *)(oper_in_table->data);

        if (operation->binary){
            node_t * left_operand  = readEquationPrefix(diff, input_file);
            node_t * right_operand = readEquationPrefix(diff, input_file);

            node = newOprNode(operation->num, left_operand, right_operand);
        }
        else {
            node_t * operand  = readEquationPrefix(diff, input_file);

            node = newOprNode(operation->num, operand, NULL);
        }
    }
    else {
        double number = 0.;
        if (sscanf(buffer, "%lg", &number) > 0){
            node = newNumNode(number);
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
            node = newVarNode(var_index);
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
            sprintf(str, "type = 'OPR', operation '%s' ", opers[elem->val.op]);
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

node_t * makeDerivative(diff_context_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(diff);
    assert(expr_node);

    switch (type_(expr_node)){
        case NUM: {
            return newNumNode(0.);
        }
        case VAR: {
            if (val_(expr_node).var == var_index){
                return newNumNode(1.);
            }
            return treeCopy(expr_node);
        }
        case OPR: {
            switch(val_(expr_node).op){
                case ADD: {
                    return newOprNode(ADD,
                        makeDerivative(diff, expr_node->left , var_index),
                        makeDerivative(diff, expr_node->right, var_index));
                }
                case SUB: {
                    return newOprNode(SUB,
                        makeDerivative(diff, expr_node->left , var_index),
                        makeDerivative(diff, expr_node->right, var_index));
                }
                case MUL: {
                    node_t * dLeft  = makeDerivative(diff, expr_node->left, var_index);
                    node_t * dRight = makeDerivative(diff, expr_node->right, var_index);

                    node_t * cLeft  = treeCopy(expr_node->left);
                    node_t * cRight = treeCopy(expr_node->right);

                    return  newOprNode(ADD,
                                newOprNode(MUL, dLeft, cRight),
                                newOprNode(MUL, cLeft, dRight)
                            );
                }
                case DIV: {
                    node_t * dLeft  = makeDerivative(diff, expr_node->left, var_index);
                    node_t * dRight = makeDerivative(diff, expr_node->right, var_index);

                    node_t * cLeft  = treeCopy(expr_node->left);
                    node_t * cRightUp = treeCopy(expr_node->right);

                    node_t * cRightDown = treeCopy(expr_node->right);

                    return  newOprNode(DIV,
                                newOprNode(SUB,
                                    newOprNode(MUL, dLeft, cRightUp),
                                    newOprNode(MUL, cLeft, dRight)
                                ),
                                newOprNode(POW, cRightDown, newNumNode(2.))
                            );
                }
                case POW: { //TODO add exponent part's derivative
                    return  newOprNode(MUL,
                                treeCopy(expr_node->right),
                                newOprNode(POW,
                                    treeCopy(expr_node->left),
                                    newOprNode(SUB,
                                        treeCopy(expr_node->right),
                                        newNumNode(1.)
                                    )
                                )
                            );
                }
                case SIN: {
                    return  newOprNode(MUL,
                                newOprNode(COS, treeCopy(expr_node->left), NULL),
                                makeDerivative(diff, expr_node->left, var_index)
                            );
                }
                case COS: {
                    return  newOprNode(MUL,
                                newOprNode(COS, treeCopy(expr_node->left), NULL),
                                newOprNode(MUL,
                                    makeDerivative(diff, expr_node->left, var_index),
                                    newNumNode(-1.)
                                )
                            );
                }
                case TAN: {
                    return  newOprNode(DIV,
                                newNumNode(1.),
                                newOprNode(POW,
                                    newOprNode(COS, treeCopy(expr_node->left), NULL),
                                    newNumNode(2.)
                                )
                            );
                }

                default:
                    printf("havent implemented this operator yet (%d)...\n", val_(expr_node).op);
                    return NULL;
            }
        }
    }
}

static node_t * newOprNode(enum oper op_num, node_t * left, node_t * right)
{
    expr_elem_t operation = {};
    operation.type = OPR;
    operation.val.var = op_num;

    return newNode(&operation, sizeof(operation), left, right, OPR_COLOR);
}

static node_t * newNumNode(double num)
{
    expr_elem_t number = {};
    number.type = NUM;
    number.val.number = num;

    return newNode(&number, sizeof(number), NULL, NULL, NUM_COLOR);
}

static node_t * newVarNode(unsigned int var_index)
{
    expr_elem_t variable = {};
    variable.type = VAR;
    variable.val.var = var_index;

    return newNode(&variable, sizeof(variable), NULL, NULL, VAR_COLOR);
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

    logPrint(LOG_DEBUG, "<h2>---DIFFERENTIATOR DUMP END---</h2>\n");
}

static void dumpToTEXrecursive(FILE * out_file, diff_context_t * diff, node_t * node);

void dumpToTEX(FILE * out_file, diff_context_t * diff, node_t * node)
{
    fprintf(out_file, "$ ");

    dumpToTEXrecursive(out_file, diff, node);

    fprintf(out_file, " $\n");

}

static void dumpToTEXrecursive(FILE * out_file, diff_context_t * diff, node_t * node)
{
    if (type_(node) == NUM){
        fprintf(out_file, "%lg", val_(node).number);
        return;
    }

    if (type_(node) == VAR){
        fprintf(out_file, "%s", diff->vars[val_(node).op].name);
        return;
    }

    enum oper op_num = val_(node).op;
    if (opers[op_num].binary){
        switch(op_num){
            case DIV:
                fprintf(out_file, "\\frac{");

                dumpToTEXrecursive(out_file, diff, node->left);
                fprintf(out_file, "}{");
                dumpToTEXrecursive(out_file, diff, node->right);

                fprintf(out_file, "}");
                break;

            case POW:
                dumpToTEXrecursive(out_file, diff, node->left);
                fprintf(out_file, "^{");
                dumpToTEXrecursive(out_file, diff, node->right);
                fprintf(out_file, "}");
                break;

            case MUL:
                fprintf(out_file, "(");

                dumpToTEXrecursive(out_file, diff, node->left);
                fprintf(out_file, " \\cdot ", opers[op_num].name);
                dumpToTEXrecursive(out_file, diff, node->right);

                fprintf(out_file, ")");
                break;

            default:
                fprintf(out_file, "(");

                dumpToTEXrecursive(out_file, diff, node->left);
                fprintf(out_file, " %s ", opers[op_num].name);
                dumpToTEXrecursive(out_file, diff, node->right);

                fprintf(out_file, ")");
                break;
        }
    }
    else {
        switch(op_num) {
            case COS: case SIN: case TAN:
            fprintf(out_file, "\\%s ", opers[op_num].name);
                dumpToTEXrecursive(out_file, diff, node->left);
                break;

            default:
                fprintf(out_file, "%s", opers[op_num].name);
                dumpToTEXrecursive(out_file, diff, node->left);
                break;
        }
    }
}
