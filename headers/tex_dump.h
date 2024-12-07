#ifndef TEX_DUMP_INCLUDED
#define TEX_DUMP_INCLUDED

#include "differ.h"

/// @brief context structure for tex dump
typedef struct {
    const char * file_name;
    FILE * file;
} tex_dump_t;

/// @brief initialising struncture tex_dump_t to dump in file with name "file_name"
tex_dump_t startTexDump(const char * file_name);

/// @brief dupms expression to tex file
void dumpToTEX(tex_dump_t * tex, diff_t * diff, node_t * node);

/// @brief simplifies expression writing step by step to tex file
node_t * TexSimplifyExpression(tex_dump_t * tex, diff_t * diff, node_t * node);

/// @brief ends tex dump, closes tex file
void endTexDump(tex_dump_t * tex);

#endif
