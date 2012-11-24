#ifndef PLATANOS_DB_SEARCH_CACHE_H_
#define PLATANOS_DB_SEARCH_CACHE_H_


#include"column.h"
#include"tree/tree.h"


int cmp_column_t (struct column_t_ *first, struct column_t_ *second);

RB_HEAD (columns_rb_t, column_t_);
RB_PROTOTYPE (columns_rb_t, column_t_, field, cmp_column_t);

typedef struct columns_rb_t columns_rb_t;

struct person_t_
{

    uint64_t uid;
    columns_rb_t columns_rb;
      RB_ENTRY (person_t_) field;

};
int cmp_person_t (struct person_t_ *first, struct person_t_ *second);

RB_HEAD (persons_rb_t, person_t_);
RB_PROTOTYPE (persons_rb_t, person_t_, field, cmp_person_t);


typedef struct persons_rb_t persons_rb_t;
typedef struct person_t_ person_t;


void person_init (person_t ** person, uint64_t uid);

void person_destroy (person_t * person);

struct db_scache_t_
{

    persons_rb_t persons_rb;

};


typedef struct db_scache_t_ db_scache_t;


void db_scache_init (db_scache_t ** db_scache);

void db_scache_destroy (db_scache_t * db_scache);

#endif
