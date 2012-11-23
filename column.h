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

typedef struct column_t_ column_t;

void column_init (column_t ** column);
void column_destroy (column_t ** column);



//read a number which we know is 1 byte
//no checks are done

inline uint64_t column_sread (column_t * column, uint64_t position);

//this is a read for big numbers
uint64_t column_bread (column_t * column, uint64_t position);

//this is a general read
inline uint64_t column_read (column_t * column, uint64_t position);

inline int column_write (struct column_t_ *column, uint64_t position,
			 uint64_t value);

#endif
