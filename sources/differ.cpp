#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"
#include "hashtable.h"
#include "derivatives.h"

static void fillOperTable(diff_t * diff);

const size_t OPR_TABLE_SIZE = 32;

const size_t VAR_TABLE_SIZE = 32;

void diffInit(diff_t * diff)
{
    assert(diff);

    diff->oper_table = tableCtor(OPR_TABLE_SIZE);
    diff-> var_table = tableCtor(VAR_TABLE_SIZE);

    fillOperTable(diff);
}

void diffDtor(diff_t * diff)
{
    assert(diff);

    tableDtor(&(diff->oper_table));
    tableDtor(&(diff-> var_table));
}

static void fillOperTable(diff_t * diff)
{
    assert(diff);

    for (size_t oper_index = 0; oper_index < opers_size; oper_index++){
        tableInsert(&(diff->oper_table), opers[oper_index].name, opers + oper_index, sizeof(oper_t));
    }
}

node_t * makeDerivative(diff_t * diff, node_t * expr_node, unsigned int var_index)
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
            enum oper op_num = val_(expr_node).op;
            diff_func_t diffFunc = opers[op_num].diffFunc;

            return diffFunc(diff, expr_node, var_index);
        }
    }
}

double calcOper(enum oper op_num, double left_val, double right_val)
{
    double new_val = 0.;

    switch (op_num){
        case ADD:
            new_val = left_val + right_val;
            break;

        case SUB:
            new_val = left_val - right_val;
            break;

        case MUL:
            new_val = left_val * right_val;
            break;

        case DIV:
            new_val = left_val / right_val;
            break;

        case POW:
            new_val = pow(left_val, right_val);
            break;

        case FAC:
            new_val = (double)factorial((long unsigned int)left_val);
            break;

        case SIN:
            new_val = sin(left_val);
            break;

        case COS:
            new_val = cos(left_val);
            break;

        case TAN:
            new_val = tan(left_val);
            break;

        case LN:
            new_val = log(left_val);
            break;

        case LOG:
            new_val = log(right_val) / log(left_val);
            break;

        default:
            fprintf(stderr, "CANNOT CALCULATE THIS OPERATION: %d\n", op_num);
            exit(1);
    }
    return new_val;
}

double evaluate(diff_t * diff, node_t * node)
{
    assert(node);
    assert(diff);

    logPrint(LOG_DEBUG_PLUS, "entered evaluate for root %p\n", node);
    logPrint(LOG_DEBUG_PLUS, "\tvalue: %lg, type: %d\n", val_(node).number, type_(node));

    if (type_(node) == NUM)
        return val_(node).number;

    if (type_(node) == VAR)
        return diff->vars[val_(node).var].value;

    if (type_(node) == OPR){
        enum oper op_num = val_(node).op;

        if (opers[op_num].binary)
            return calcOper(op_num, evaluate(diff, node->left), evaluate(diff, node->right));

        return calcOper(op_num, evaluate(diff, node->left), 0.);
    }

    return 0.;
}

node_t * simplifyExpression(node_t * node)
{
    assert(node);

    bool changing = true;
    while (changing){
        changing = false;

        node = foldConstants(node, NULL, &changing);
        node = deleteNeutral(node, NULL, &changing);
    }

    return node;
}

node_t * foldConstants(node_t * node, node_t * parent, bool * changed_tree)
{
    if (node == NULL)
        return NULL;

    if (type_(node) == VAR)
        return node;

    if (type_(node) == NUM)
        return node;

    node->left  = foldConstants(node->left , node, changed_tree);
    node->right = foldConstants(node->right, node, changed_tree);

    enum oper op_num = val_(node).op;

    double new_val = 0.;

    if (opers[op_num].binary){
        if (type_(node->left) == NUM && type_(node->right) == NUM){
            double  left_val = val_(node->left ).number;
            double right_val = val_(node->right).number;

            new_val = calcOper(op_num, left_val, right_val);
        }
        else
            return node;
    }
    else {
        if (type_(node->left) == NUM){
            double val = val_(node->left).number;
            new_val = calcOper(op_num, val, 0);
        }
        else
            return node;
    }

    treeDestroy(node);

    node_t * new_node = newNumNode(new_val);
    new_node->parent = parent;

    *changed_tree = true;

    return new_node;
}

static node_t * delNeutralInCommutatives(node_t * node, node_t * parent, bool * changed_tree);

static node_t * delNeutralInNonCommutatives(node_t * node, node_t * parent, bool * changed_tree);

node_t * deleteNeutral(node_t * node, node_t * parent, bool * changed_tree)
{
    if (node == NULL)
        return NULL;

    if (type_(node) != OPR)
        return node;

    node->left  = deleteNeutral(node->left,  node, changed_tree);
    node->right = deleteNeutral(node->right, node, changed_tree);

    if (opers[val_(node).op].commutative)
        return delNeutralInCommutatives(node, parent, changed_tree);

    return delNeutralInNonCommutatives(node, parent, changed_tree);
}

static node_t * delNeutralInCommutatives(node_t * node, node_t * parent, bool * changed_tree)
{
    assert(node);
    assert(type_(node) == OPR);

    bool last_changed = *changed_tree;
    *changed_tree = true;

    node_t * cur_node = node->left;
    node_t * another_node = node->right;
    while (cur_node != NULL) {
        if (type_(cur_node) == NUM){
            switch (val_(node).op){
                case MUL:
                    logPrint(LOG_DEBUG_PLUS, "in mul case...\n");
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
                    break;
                case ADD:
                    logPrint(LOG_DEBUG_PLUS, "in add case...\n");
                    if (val_(cur_node).number == 0.){
                        delNode(cur_node);
                        delNode(node);

                        another_node->parent = parent;
                        return another_node;
                    }
                    break;
                default:
                    break;
            }
        }

        if (cur_node == node->left){
            cur_node     = node->right;
            another_node = node->left;
        }
        else
            cur_node = NULL;
    }

    *changed_tree = last_changed;

    return node;
}

static node_t * delNeutralInNonCommutatives(node_t * node, node_t * parent, bool * changed_tree)
{
    assert(node);
    assert(type_(node) == OPR);

    node_t * left  = node->left;
    node_t * right = node->right;

    bool last_changed = *changed_tree;
    *changed_tree = true;

    switch (val_(node).op){
        case SUB:
            if (type_(right) == NUM){
                if (val_(right).number == 0.){
                    delNode(right);
                    delNode(node);

                    left->parent = parent;

                    return left;
                }
            }
            break;
        case POW:
            if (type_(right) == NUM){
                if (val_(right).number == 1.){
                    delNode(right);
                    delNode(node);

                    left->parent = parent;

                    return left;
                }
                else if (val_(right).number == 0.){
                    treeDestroy(node);

                    node_t * new_node = newNumNode(1.);
                    new_node->parent = parent;

                    return new_node;
                }
            }
            if (type_(left) == NUM){
                if (val_(left).number == 1. || val_(left).number == 0.){
                    treeDestroy(right);
                    delNode(node);

                    left->parent = parent;

                    return left;
                }
            }
            break;
    }

    *changed_tree = last_changed;

    return node;
}

const size_t BUFFER_LEN = 32;

node_t * readEquationPrefix(diff_t * diff, FILE * input_file)
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
            node = getVarNode(diff, buffer);
        }
    }

    fscanf(input_file, " )");

    return node;
}

void setVariables(diff_t * diff)
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
            sprintf(str, "type = 'OPR', operation '%s' ", opers[elem->val.op].name);
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

node_t * taylorSeries(diff_t * diff, node_t * expr_node, unsigned int var_index, double diff_point, size_t last_member_index)
{
    assert(diff);
    assert(expr_node);

    diff->vars[var_index].value = diff_point;

    node_t * taylor = newNumNode(0.);
    node_t * cur_derivative = treeCopy(expr_node);

    for (size_t taylor_index = 0; taylor_index < last_member_index; taylor_index++){
        double cur_derivative_num = evaluate(diff, cur_derivative);

        taylor = newOprNode(ADD,
                    taylor,
                    newOprNode(MUL,
                        newOprNode(DIV,
                            newNumNode(cur_derivative_num),
                            newOprNode(FAC, newNumNode(taylor_index), NULL)),
                        newOprNode(POW,
                            newVarNode(var_index),
                            newNumNode((double)taylor_index)
                        )
                    )
                );

        node_t * old_derivative = cur_derivative;

        cur_derivative = makeDerivative(diff, cur_derivative, 0);
        cur_derivative = simplifyExpression(cur_derivative);

        treeDestroy(old_derivative);
    }

    treeDestroy(cur_derivative);

    return taylor;
}

node_t * newOprNode(enum oper op_num, node_t * left, node_t * right)
{
    expr_elem_t operation = {};
    operation.type = OPR;
    operation.val.var = op_num;

    return newNode(&operation, sizeof(operation), left, right, OPR_COLOR);
}

node_t * newNumNode(double num)
{
    expr_elem_t number = {};
    number.type = NUM;
    number.val.number = num;

    return newNode(&number, sizeof(number), NULL, NULL, NUM_COLOR);
}

node_t * newVarNode(unsigned int var_index)
{
    expr_elem_t variable = {};
    variable.type = VAR;
    variable.val.var = var_index;

    return newNode(&variable, sizeof(variable), NULL, NULL, VAR_COLOR);
}

void diffDump(diff_t * diff)
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

node_t * getVarNode(diff_t * diff, char * var_name)
{
    unsigned int var_index = 0;

    name_t * variable = tableLookup(&(diff->var_table), var_name);

    if (variable == NULL){
        tableInsert(&(diff->var_table), var_name, &(diff->var_num), sizeof(diff->var_num));
        var_index = diff->var_num;

        strncpy(diff->vars[var_index].name, var_name, NAME_MAX_LEN - 1);

        diff->var_num++;
    }
    else {
        var_index = *(unsigned int *)(variable->data);
    }

    return newVarNode(var_index);
}

size_t countVars(node_t * node, unsigned int var_index)
{
    assert(node);

    if (type_(node) == NUM)
        return 0;

    if (type_(node) == VAR){
        if (val_(node).var == var_index)
            return 1;

        return 0;
    }

    if (opers[val_(node).op].binary)
        return countVars(node->left, var_index) + countVars(node->right, var_index);

    return countVars(node->left, var_index);
}

long unsigned int factorial(long unsigned int number)
{
    long unsigned int ans = 1;

    while (number != 0)
        ans *= (number--);

    return ans;
}
