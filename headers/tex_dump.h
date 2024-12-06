#ifndef TEX_DUMP_INCLUDED
#define TEX_DUMP_INCLUDED

#include "differ.h"

typedef struct {
    const char * file_name;
    FILE * file;
} tex_dump_t;

tex_dump_t startTexDump(const char * file_name);

void dumpToTEX(tex_dump_t * tex, diff_t * diff, node_t * node);

node_t * TexSimplifyExpression(tex_dump_t * tex, diff_t * diff, node_t * node);

void endTexDump(tex_dump_t * tex);

#endif
