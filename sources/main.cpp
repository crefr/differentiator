#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "bintree.h"
#include "differ.h"
#include "logger.h"
#include "eq_parser.h"
#include "tex_dump.h"

const size_t BUFFER_LEN = 128;

int main()
{
    mkdir("logs", 0777);
    logStart("logs/log.html", LOG_DEBUG, LOG_HTML);
    // logCancelBuffer();

    diff_t diff ={};
    diffInit(&diff);

    treeSetDumpMode(DUMP_MEDIUM);

    tex_dump_t tex = startTexDump("test.tex");

    char buffer[BUFFER_LEN] = {};
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

    fprintf(tex.file, "\\vspace{5mm}\n");

    fprintf(tex.file, "Производная: \n\n");
    dumpToTEX(&tex, &diff, derivative);
    derivative = TexSimplifyExpression(&tex, &diff, derivative);

    fprintf(tex.file, "\\vspace{5mm}\n");

    fprintf(tex.file, "Разложение Тейлора в окрестности 0: \n\n");
    dumpToTEX(&tex, &diff, taylor);
    taylor     = TexSimplifyExpression(&tex, &diff, taylor);

    TexMakePlot(&tex, &diff, tree, -10., 10., 1000, 0);

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
