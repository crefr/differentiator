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

    diffInit();

    node_t * tree = readEquationPrefix(stdin);
    setGlobalX(26.0);

    treeDumpGraph(tree, exprElemToStr);

    double answer = evaluate(tree);
    printf("answer is %lg\n", answer);

    treeDestroy(tree);

    logExit();
    return 0;
}
