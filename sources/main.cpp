#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "differ.h"
#include "bintree.h"
#include "logger.h"
#include "eq_parser.h"

int main()
{
    mkdir("logs", S_IFDIR);
    logStart("logs/log.html", LOG_DEBUG, LOG_HTML);
    // logCancelBuffer();

    diff_t diff ={};
    diffInit(&diff);

    treeSetDumpMode(DUMP_MEDIUM);

    // node_t * tree       = readEquationPrefix(&diff, stdin);

    char buffer[128] = {};
    scanf("%[^\n]", buffer);

    node_t * tree       = parseEquation(&diff, buffer);
    treeDumpGraph(tree, exprElemToStr);

    node_t * derivative = makeDerivative(&diff, tree, 0);
    treeDumpGraph(derivative, exprElemToStr);

    node_t * taylor = taylorSeries(&diff, tree, 0, 0, 8);
    treeDumpGraph(taylor, exprElemToStr);

    tree       = simplifyExpression(tree);
    derivative = simplifyExpression(derivative);
    taylor     = simplifyExpression(taylor);

    treeDumpGraph(derivative, exprElemToStr);

    FILE * tex_file = fopen("test.md", "w");
    dumpToTEX(tex_file, &diff, tree);
    dumpToTEX(tex_file, &diff, derivative);
    dumpToTEX(tex_file, &diff, taylor);
    fclose(tex_file);

    diffDump(&diff);

    treeDestroy(tree);
    treeDestroy(derivative);
    treeDestroy(taylor);
    diffDtor(&diff);


    logExit();
    return 0;
}
