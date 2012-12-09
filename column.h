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
#include"varint.h"



struct column_t_
{
    uint64_t *uid;
    uint8_t *percentage;
    uint8_t *buffer;
    uint64_t size;
    uint8_t dim;
      RB_ENTRY (column_t_) field;

};

typedef struct column_t_ column_t;



typedef struct
{
    uint64_t *uid;
    uint8_t *percentage;
    uint8_t *buffer;
    uint64_t size;
    uint8_t dim;
} intersection_t;


intersection_t *intersection_enew (uint64_t * uid, uint8_t * percentage,
                                   uint8_t * buffer, uint64_t size,
                                   uint8_t dim);
//creates a buffer
intersection_t *intersection_inew (uint64_t * uid, uint8_t * percentage,
                                   uint8_t dim);

//the only difference is that we also allocate some memory for the
//red black tree
column_t *column_inew (uint64_t * uid, uint8_t * percentage, uint8_t dim);


void intersection_destroy (intersection_t ** intersection);



//used by the join function
//returns the position
//key is the key at that position
//size is the size of the key in varint

uint64_t
intersection_middle (intersection_t * intersection, uint64_t start,
                     uint64_t end, uint64_t * key, uint8_t * size);

//dim is the number of intersections used
//res_dim is the dimension of the result- it equals the size of percentage
intersection_t *intersections_join (intersection_t * intersection[],
                                    uint8_t percentage[], int dim, int res_dim);



#endif
