#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tex_dump.h"
#include "differ.h"
#include "bintree.h"

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

static void dumpToTEXrecursive(tex_dump_t * tex, diff_t * diff, node_t * node);

void dumpToTEX(tex_dump_t * tex, diff_t * diff, node_t * node)
{
    assert(tex);
    assert(diff);
    assert(node);

    fprintf(tex->file, "$ ");

    dumpToTEXrecursive(tex, diff, node);

    fprintf(tex->file, " $\n\n");
}

static void dumpToTEXrecursive(tex_dump_t * tex, diff_t * diff, node_t * node)
{
    assert(tex);
    assert(diff);
    assert(node);

    if (type_(node) == NUM){
        fprintf(tex->file, "%lg", val_(node).number);
        return;
    }

    if (type_(node) == VAR){
        fprintf(tex->file, "%s", diff->vars[val_(node).op].name);
        return;
    }

    enum oper op_num = val_(node).op;
    if (opers[op_num].binary){
        switch(op_num){
            case DIV:
                fprintf(tex->file, "\\frac{");

                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, "}{");
                dumpToTEXrecursive(tex, diff, node->right);

                fprintf(tex->file, "}");
                break;

            case POW:
                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, "^{");
                dumpToTEXrecursive(tex, diff, node->right);
                fprintf(tex->file, "}");
                break;

            case MUL:
                fprintf(tex->file, "(");

                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, " \\cdot ");
                dumpToTEXrecursive(tex, diff, node->right);

                fprintf(tex->file, ")");
                break;

            default:
                fprintf(tex->file, "(");

                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, " %s ", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->right);

                fprintf(tex->file, ")");
                break;
        }
    }
    else {
        switch(op_num) {
            case COS: case SIN: case TAN:
                fprintf(tex->file, "(\\%s ", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, ")");
                break;

            case FAC:
                dumpToTEXrecursive(tex, diff, node->left);
                fprintf(tex->file, "!");
                break;

            default:
                fprintf(tex->file, "%s", opers[op_num].name);
                dumpToTEXrecursive(tex, diff, node->left);
                break;
        }
    }
}
