#ifndef EQ_PARSER_INCLUDED
#define EQ_PARSER_INCLUDED

#include "bintree.h"
#include "differ.h"

/// @brief parses expression using recursive descent
node_t * parseEquation(diff_t * diff, const char * string);

#endif
