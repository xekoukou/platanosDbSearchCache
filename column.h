/*
    Copyright contributors as noted in the AUTHORS file.
                
    This file is part of PLATANOS.

    PLATANOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU Affero General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.
            
    PLATANOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
        
    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//the size of a column cannot be bigger than 5MB TODO magical number

#ifndef PLATANOS_COLUMN_H_
#define PLATANOS_COLUMN_H_

#include<stdint.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<math.h>
#include"setlib/jlist.h"
#include"tree/tree.h"



struct column_t_
{
    uint64_t uid;
    uint8_t percentage;
    uint8_t *buffer;
    uint64_t size;
      RB_ENTRY (column_t_) field;

};

typedef struct column_t_ column_t;

void column_init (column_t ** column, uint64_t uid, uint8_t percentage);
void column_destroy (column_t ** column);



//read a number which we know is 1 byte
//no checks are done

inline uint64_t column_sread (column_t * column, int position);

//this is a read for big numbers
uint64_t column_bread (column_t * column, uint64_t position, uint8_t * size);

//this is a general read
uint64_t column_read (column_t * column, uint64_t position, uint8_t * size);

//returns the size of the value in varint
int column_write (struct column_t_ *column, uint64_t position,
                  uint64_t value);


typedef struct
{
    uint64_t position;

} llimit_t;

//TODO position why here??
struct join_t_
{
    jlist_t *jlist;
    jnode_t **barray;
    llimit_t *llimit;           //left bracket limit used for the bsearch
    int size;                   //the barray_size
    int dim;
    int *position;
};


typedef struct join_t_ join_t;

void join_new (join_t ** join, int max_dim, int max_size);

void join_destroy (join_t ** join);


//used by the join function
//returns the position
//key is the key at that position
//size is the size of the key in varint
uint64_t
column_middle (struct column_t_ *column, uint64_t start, uint64_t end,
               uint64_t * key, uint8_t * size);


//both percentage and column need to be of dimension dim
uint8_t *join_columns (join_t * join, struct column_t_ *column[],
                       uint8_t percentage[], int dim, uint64_t * size);


#endif
