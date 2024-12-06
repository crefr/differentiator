#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "derivatives.h"
#include "bintree.h"
#include "differ.h"

node_t * diffAddSub(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    enum oper op_num = val_(expr_node).op;

    return  newOprNode(op_num,
                makeDerivative(diff, expr_node->left , var_index),
                makeDerivative(diff, expr_node->right, var_index));
}

node_t * diffMul(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    node_t * dLeft  = makeDerivative(diff, expr_node->left, var_index);
    node_t * dRight = makeDerivative(diff, expr_node->right, var_index);

    node_t * cLeft  = treeCopy(expr_node->left);
    node_t * cRight = treeCopy(expr_node->right);

    return  newOprNode(ADD,
                newOprNode(MUL, dLeft, cRight),
                newOprNode(MUL, cLeft, dRight)
            );
}

node_t * diffDiv(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

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

node_t * diffPow(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    size_t num_vars_in_left  = countVars(expr_node->left,  var_index);
    printf("left:  %zu\n", num_vars_in_left);
    size_t num_vars_in_right = countVars(expr_node->right, var_index);
    printf("right: %zu\n", num_vars_in_right);

    if (num_vars_in_left == 0){
        if (num_vars_in_right == 0)
            return newNumNode(0.);

        return
            newOprNode(MUL,
                newOprNode(MUL,
                    treeCopy(expr_node),
                    newOprNode(LN, treeCopy(expr_node->left), NULL)
                ),
                makeDerivative(diff, expr_node->right, var_index)
            );
    }

    if (num_vars_in_right == 0){
        return
            newOprNode(MUL,
                newOprNode(MUL,
                    treeCopy(expr_node->right),
                    newOprNode(POW,
                        treeCopy(expr_node->left),
                        newOprNode(SUB,
                            treeCopy(expr_node->right),
                            newNumNode(1.)
                        )
                    )
                ),
                makeDerivative(diff, expr_node->left, var_index)
            );
    }

    return
        newOprNode(MUL,
            treeCopy(expr_node),
            newOprNode(ADD,
                newOprNode(DIV,
                    newOprNode(MUL,
                        makeDerivative(diff, expr_node->left, var_index),
                        treeCopy(expr_node->right)
                    ),
                    treeCopy(expr_node->left)
                ),
                newOprNode(MUL,
                    newOprNode(LN, treeCopy(expr_node->left), NULL),
                    makeDerivative(diff, expr_node->right, var_index)
                )
            )
        );

}

node_t * diffSin(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    return  newOprNode(MUL,
                newOprNode(COS, treeCopy(expr_node->left), NULL),
                makeDerivative(diff, expr_node->left, var_index)
            );
}

node_t * diffCos(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    return  newOprNode(MUL,
                newOprNode(SIN, treeCopy(expr_node->left), NULL),
                newOprNode(MUL,
                    makeDerivative(diff, expr_node->left, var_index),
                    newNumNode(-1.)
                )
            );
}

node_t * diffTan(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    return  newOprNode(DIV,
                newNumNode(1.),
                newOprNode(POW,
                    newOprNode(COS, treeCopy(expr_node->left), NULL),
                    newNumNode(2.)
                )
            );
}

node_t * diffLn(diff_t * diff, node_t * expr_node, unsigned int var_index)
{
    assert(expr_node);
    assert(type_(expr_node) == OPR);

    return  NULL;
}
