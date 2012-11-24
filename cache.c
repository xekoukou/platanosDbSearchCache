#include"cache.h"
#include"column.h"
#include"tree/tree.h"
#include<stdlib.h>


int
cmp_column_t (struct column_t_ *first, struct column_t_ *second)
{
    if (first->uid > second->uid) {
	return 1;
    }
    else if (first->uid < second->uid) {
	return -1;
    }
    return 0;
}

RB_GENERATE (columns_rb_t, column_t_, field, cmp_column_t);

int
cmp_person_t (struct person_t_ *first, struct person_t_ *second)
{
    if (first->uid > second->uid) {
	return 1;
    }
    else if (first->uid < second->uid) {
	return -1;
    }
    return 0;
}

RB_GENERATE (persons_rb_t, person_t_, field, cmp_person_t);


void
person_init (person_t ** person, uint64_t uid)
{
    *person = malloc (sizeof (person_t));
    RB_INIT (&((*person)->columns_rb));
    (*person)->uid = uid;
}

void
person_destroy (person_t * person)
{
    column_t *iter = NULL;

    RB_FOREACH (iter, columns_rb_t, &(person->columns_rb)) {
	column_destroy (iter);
    }

    free (person);

}

column_t *
person_csearch (person_t * person, column_t * column)
{
    return RB_FIND (columns_rb_t, &(person->columns_rb), column);
}


void
db_scache_init (db_scache_t ** db_scache)
{
    *db_scache = malloc (sizeof (db_scache_t));
    RB_INIT (&((*db_scache)->persons_rb));
}

void
db_scache_destroy (db_scache_t * db_scache)
{
    person_t *iter = NULL;

    RB_FOREACH (iter, persons_rb_t, &(db_scache->persons_rb)) {
	person_destroy (iter);
    }

    free (db_scache);

}

person_t *
db_scache_psearch (db_scache_t * db_scache, person_t * person)
{
    return RB_FIND (persons_rb_t, &(db_scache->persons_rb), person);
}
