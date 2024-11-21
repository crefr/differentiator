#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"

int main()
{
    mkdir("logs", S_IFDIR);
    logStart("logs/log.html", LOG_DEBUG_PLUS, LOG_HTML);


    // (2*x)+(52)
    expr_elem_t elem1   = {.type = OPR, .val = ADD};
    elem1.val.ival = ADD;

    expr_elem_t elem11  = {.type = OPR, .val = MUL};
    elem11.val.ival = MUL;

    expr_elem_t elem111 = {.type = NUM, .val = 1.};
    elem111.val.dval = 1.;

    expr_elem_t elem112 = {.type = NUM, .val = 52.};
    elem112.val.dval = 52.;

    expr_elem_t elem12  = {.type = NUM, .val = 1.};
    elem12.val.dval = 1.;

    node_t * tree = newNode(&elem1, sizeof(expr_elem_t),
                        newNode(&elem11, sizeof(expr_elem_t),
                            newNode(&elem111, sizeof(expr_elem_t), NULL, NULL),
                            newNode(&elem112, sizeof(expr_elem_t), NULL, NULL)
                        ),
                        newNode(&elem12, sizeof(expr_elem_t), NULL, NULL)
                    );

    setGlobalX(26.0);

    double answer = evaluate(tree);
    printf("answer is %lg\n", answer);

    treeDestroy(tree);

    logExit();
    return 0;
}
