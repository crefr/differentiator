#ifndef EQ_PARSER_INCLUDED
#define EQ_PARSER_INCLUDED

#include "bintree.h"
#include "differ.h"

node_t * parseEquation(diff_t * diff, const char * str);

#endif
