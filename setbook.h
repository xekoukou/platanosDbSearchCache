#ifndef PLATANOS_DB_SEARCH_SETBOOK_H_
#define PLATANOS_DB_SEARCH_SETBOOK_H_

#include"setlib/critbit.h"
#include<stdint.h>
#include<stdlib.h>

//results should be of dimension [dim][dim]
//res_size is the size of the one dim
//res_number of the other
void
setbook_contains (critbit_tree * setbook, uint64_t * uid, int dim,
                  int result[dim][dim], int *res_size, int *res_number);

int setbook_insert (critbit_tree * setbook, uint64_t * uid, int dim);

int setbook_delete (critbit_tree * setbook, uint64_t * uid, int dim);

void setbook_clear (critbit_tree * setbook);

void setbook_init (critbit_tree ** setbook);

void setbook_destroy (critbit_tree ** setbook);

#endif
