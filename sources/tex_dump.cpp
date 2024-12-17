#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "tex_dump.h"
#include "differ.h"
#include "bintree.h"

static void dumpToTEXrecursive(tex_dump_t * tex, diff_t * diff, node_t * node, node_t * parent);

static void operatorDump(tex_dump_t * tex, diff_t * diff, node_t * node, node_t * parent);

tex_dump_t startTexDump(const char * file_name)
{
    assert(file_name);

    tex_dump_t tex = {};

    tex.file_name = file_name;
    tex.file = fopen(file_name, "w");

    fprintf(tex.file,
        "\\documentclass{article}\n"
        "\\usepackage[utf8]{inputenc}\n"
        "\\usepackage[T2A]{fontenc}\n"
        "\\usepackage[russian]{babel}\n"
        "\\usepackage{pgfplots}\n"
        "\\usepackage{geometry}\n"
        "\\geometry{\n"
        "a4paper,\n"
        "total={170mm,257mm},\n"
        "left=20mm,\n"
        "top=20mm,\n"
        "}"
        "\\title{Производничаем...}\n"
        "\\author{Производных машина}\n"
        "\\date{}\n"
        "\\begin{document}\n"
        "\t\\maketitle\n");

    return tex;
}

void endTexDump(tex_dump_t * tex)
{
    assert(tex);

    fprintf(tex->file, "\\end{document}\n");
    fclose(tex->file);

    const size_t BUFFER_LEN = 128;
    char system_str[BUFFER_LEN] = "";

    printf("file: %s\n", tex->file_name);
    sprintf(system_str, "pdflatex %s", tex->file_name);

    printf("%s\n", system_str);
    system(system_str);
}

void dumpToTEX(tex_dump_t * tex, diff_t * diff, node_t * node)
{
    assert(tex);
    assert(diff);
    assert(node);

    fprintf(tex->file, "$ ");

    dumpToTEXrecursive(tex, diff, node, NULL);

    fprintf(tex->file, " $\n\n");
    fprintf(tex->file, "\\vspace{3mm}\n");
}

static void dumpToTEXrecursive(tex_dump_t * tex, diff_t * diff, node_t * node, node_t * parent)
{
    assert(tex);
    assert(diff);
    assert(node);

    if (type_(node) == NUM){
        if (val_(node).number < 0)
            fprintf(tex->file, "(%lg)", val_(node).number);
        else
            fprintf(tex->file, "%lg", val_(node).number);

        return;
    }

    if (type_(node) == VAR){
        fprintf(tex->file, "%s", diff->vars[val_(node).op].name);
        return;
    }

    operatorDump(tex, diff, node, parent);
}

static void operatorDump(tex_dump_t * tex, diff_t * diff, node_t * node, node_t * parent)
{
    enum oper op_num = val_(node).op;

    bool need_brackets = false;

    if (parent != NULL){
        enum oper parent_op = val_(parent).op;

        if (! opers[op_num].binary && opers[parent_op].priority > opers[op_num].priority)
            need_brackets = true;
    }

    if (need_brackets)
        fprintf(tex->file, "(");

    if (opers[op_num].binary){
        switch(op_num){
            case DIV:
                fprintf(tex->file, "\\frac{");

                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, "}{");
                dumpToTEXrecursive(tex, diff, node->right, node);

                fprintf(tex->file, "}");
                break;

            case POW:
                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, "^{");
                dumpToTEXrecursive(tex, diff, node->right, node);
                fprintf(tex->file, "}");
                break;

            case MUL:
                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, " \\cdot ");
                dumpToTEXrecursive(tex, diff, node->right, node);
                break;

            default:
                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, " %s ", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->right, node);
                break;
        }
    }
    else {
        switch(op_num) {
            case COS: case SIN: case TAN: case LN:
                fprintf(tex->file, "\\%s(", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, ")");
                break;

            case FAC:
                dumpToTEXrecursive(tex, diff, node->left, node);
                fprintf(tex->file, "!");
                break;

            default:
                fprintf(tex->file, "%s", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->left, node);
                break;
        }
    }

    if (need_brackets)
        fprintf(tex->file, ")");
}

node_t * TexSimplifyExpression(tex_dump_t * tex, diff_t * diff, node_t * node)
{
    assert(tex);
    assert(node);

    bool changing = true;
    while (changing){
        changing = false;

        node = foldConstants(node, NULL, &changing);

        if (changing){
            fprintf(tex->file, "Упрощаем константы...\n\n");
            dumpToTEX(tex, diff, node);
            fprintf(tex->file, "\n\n");
        }

        changing = false;

        node = deleteNeutral(node, NULL, &changing);

        if (changing){
            fprintf(tex->file, "Удаляем лишнее...\n\n");
            dumpToTEX(tex, diff, node);
            fprintf(tex->file, "\n\n");
        }
        else
            break;
    }

    return node;
}

void TexMakePlot(tex_dump_t * tex, diff_t * diff, node_t * tree,
                  double left_border, double right_border, size_t num_of_pts, unsigned int var_index, double max_y)
{
    assert(tex);
    assert(diff);
    assert(tree);

    fprintf(tex->file,
        "\\begin{center}\n"
        "\\begin{tikzpicture}\n"
        "\\begin{axis}[\n"
        "xlabel={$x$},\n"
        "ylabel={$f(x)$},\n"
        "grid=both,\n"
        "title={График $f(x)$},\n"
        "]\n"
        "\\addplot[mark=none, color=blue] table {\n");

    double step = (right_border - left_border) / num_of_pts;

    for (double cur_x = left_border; cur_x < right_border; cur_x += step){
        diff->vars[var_index].value = cur_x;

        double cur_y = evaluate(diff, tree);

        if (fabs(cur_y) < max_y)
            fprintf(tex->file, "%lf %lf\n", cur_x, cur_y);
    }

    fprintf(tex->file,
        "};\n"
        "\\end{axis}\n"
        "\\end{tikzpicture}\n"
        "\\end{center}");
}
