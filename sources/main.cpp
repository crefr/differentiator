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
    logCancelBuffer();

    diff_context_t diff ={};
    diffInit(&diff);

    treeSetDumpMode(DUMP_MEDIUM);

    node_t * tree = readEquationPrefix(&diff, stdin);
    node_t * derivative = makeDerivative(&diff, tree, 0);

    treeDumpGraph(tree, exprElemToStr);
    treeDumpGraph(derivative, exprElemToStr);

    double temp = 0;
    foldConstants(derivative, &temp);

    treeDumpGraph(derivative, exprElemToStr);

    derivative = deleteNeutral(derivative, NULL);

    treeDumpGraph(derivative, exprElemToStr);

    FILE * tex_file = fopen("test.md", "w");
    dumpToTEX(tex_file, &diff, derivative);
    fclose(tex_file);

//     setVariables(&diff);
//
//     double answer = evaluate(&diff, tree);
//     printf("answer is %lg\n", answer);

    diffDump(&diff);

    treeDestroy(tree);
    treeDestroy(derivative);
    diffDtor(&diff);


    logExit();
    return 0;
}
