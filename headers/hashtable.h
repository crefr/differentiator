#ifndef HASHTABLE_INCLUDED
#define HASHTABLE_INCLUDED

/// @brief structure for one element in the table
typedef struct name {
    char * name;
    struct name * next;

    void * data;
    size_t elem_size;
} name_t;

/// @brief table itself
typedef struct {
    name_t ** names;
    size_t table_size;
} table_t;

/// @brief creates table with size of table_size
table_t tableCtor(size_t table_size);

/// @brief destructs table
void tableDtor(table_t * table);

/// @brief inserts element to table BUT does NOT checks if this element already exists
void tableInsert(table_t * table, const char * name, const void * data, size_t size);

/// @brief finds element in table by its name
name_t * tableLookup(table_t * table, const char * name);

#endif
