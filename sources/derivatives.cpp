#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bintree.h"
#include "differ.h"

#define OPR_ newOprNode
#define NUM(number) newNumNode(number)
#define DL_ makeDerivative(diff, node->left , var_index)
#define DR_ makeDerivative(diff, node->right, var_index)
#define CL_ treeCopy(node->left )
#define CR_ treeCopy(node->right)

#define D_(node) makeDerivative(diff, node, var_index)
#define C_(node) treeCopy(node)

node_t * diffAddSub(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    enum oper op_num = val_(node).op;

    return  OPR_(op_num,
                D_(node->left),
                D_(node->right));
}

node_t * diffMul(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return  OPR_(ADD,
                OPR_(MUL, DL_, CR_),
                OPR_(MUL, CL_, DR_)
            );
}

node_t * diffDiv(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return  OPR_(DIV,
                OPR_(SUB,
                    OPR_(MUL, DL_, CR_),
                    OPR_(MUL, CL_, DR_)
                ),
                OPR_(POW, CR_, NUM(2.))
            );
}

node_t * diffPow(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    size_t num_vars_in_left  = countVars(node->left,  var_index);
    size_t num_vars_in_right = countVars(node->right, var_index);

    if (num_vars_in_left == 0){
        if (num_vars_in_right == 0)
            return NUM(0.);

        return
            OPR_(MUL,
                OPR_(MUL,
                    C_(node),
                    OPR_(LN, CL_, NULL)
                ),
                DR_
            );
    }

    if (num_vars_in_right == 0){
        return
            OPR_(MUL,
                OPR_(MUL,
                    CR_,
                    OPR_(POW,
                        CL_,
                        OPR_(SUB,
                            CR_,
                            NUM(1.)
                        )
                    )
                ),
                DL_
            );
    }

    return
        OPR_(MUL,
            C_(node),
            OPR_(ADD,
                OPR_(DIV,
                    OPR_(MUL,
                        DL_,
                        CR_
                    ),
                    CL_
                ),
                OPR_(MUL,
                    OPR_(LN, CL_, NULL),
                    DR_
                )
            )
        );

}

node_t * diffSin(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return  OPR_(MUL,
                OPR_(COS, CL_, NULL),
                DL_
            );
}

node_t * diffCos(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return  OPR_(MUL,
                OPR_(SIN, CL_, NULL),
                OPR_(MUL,
                    DL_,
                    NUM(-1.)
                )
            );
}

node_t * diffTan(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return  OPR_(DIV,
                NUM(1.),
                OPR_(POW,
                    OPR_(COS, CL_, NULL),
                    NUM(2.)
                )
            );
}

node_t * diffLn(diff_t * diff, node_t * node, unsigned int var_index)
{
    assert(node);
    assert(type_(node) == OPR);

    return
        OPR_(DIV,
            DL_,
            CL_
        );
}
