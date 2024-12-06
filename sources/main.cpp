#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "bintree.h"
#include "differ.h"
#include "logger.h"
#include "eq_parser.h"
#include "tex_dump.h"

int main()
{
    mkdir("logs", S_IFDIR);
    logStart("logs/log.html", LOG_DEBUG, LOG_HTML);
    // logCancelBuffer();

    diff_t diff ={};
    diffInit(&diff);

    treeSetDumpMode(DUMP_MEDIUM);

    tex_dump_t tex = startTexDump("test.tex");

    char buffer[128] = {};
    scanf("%[^\n]", buffer);

    node_t * tree       = parseEquation(&diff, buffer);

    treeDumpGraph(tree, exprElemToStr);

    node_t * derivative = makeDerivative(&diff, tree, 0);
    treeDumpGraph(derivative, exprElemToStr);

    node_t * taylor = taylorSeries(&diff, tree, 0, 0, 8);
    treeDumpGraph(taylor, exprElemToStr);

    fprintf(tex.file, "Исходное выражение: \n\n");
    dumpToTEX(&tex, &diff, tree);
    tree       = TexSimplifyExpression(&tex, &diff, tree);

    fprintf(tex.file, "Производная: \n\n");
    dumpToTEX(&tex, &diff, derivative);
    derivative = TexSimplifyExpression(&tex, &diff, derivative);

    fprintf(tex.file, "Разложение Тейлора в окрестности 0: \n\n");
    dumpToTEX(&tex, &diff, taylor);
    taylor     = TexSimplifyExpression(&tex, &diff, taylor);

    treeDumpGraph(derivative, exprElemToStr);

    endTexDump(&tex);

    diffDump(&diff);

    treeDestroy(tree);
    treeDestroy(derivative);
    treeDestroy(taylor);
    diffDtor(&diff);

    logExit();
    return 0;
}
