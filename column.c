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
#define MAX_BUFFER_SIZE 5242880

#include<stdlib.h>
#include<stdint.h>
#include<assert.h>
#include<string.h>
#include"column.h"


void
column_init (struct column_t_ **column, uint64_t uid, uint8_t percentage)
{

    *column = (struct column_t_ *) malloc (sizeof (struct column_t_));
    (*column)->percentage = percentage;
    (*column)->buffer = (uint8_t *) malloc (MAX_BUFFER_SIZE);
    (*column)->size = 0;
    (*column)->uid = uid;

}

void
column_destroy (struct column_t_ *column)
{
    free (column->buffer);
    free (column);
};

//read a number which we know is 1 byte
//no checks are done

inline uint64_t
column_sread (struct column_t_ *column, uint64_t position)
{
    return (uint64_t) column->buffer[position];
}

//this is a read for big numbers
uint64_t
column_bread (struct column_t_ * column, uint64_t position, uint8_t * size)
{

    const uint8_t *ptr = column->buffer + position;
    uint32_t b;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = 0, part1 = 0, part2 = 0;

    b = *(ptr++);
    part0 = (b & 0x7F);
    if (!(b & 0x80)) {
	*size = 1;
	goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
	*size = 2;
	goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 14;
    if (!(b & 0x80)) {
	*size = 3;
	goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 21;
    if (!(b & 0x80)) {
	*size = 4;
	goto done;
    }
    b = *(ptr++);
    part1 = (b & 0x7F);
    if (!(b & 0x80)) {
	*size = 5;
	goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
	*size = 6;
	goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 14;
    if (!(b & 0x80)) {
	*size = 7;
	goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 21;
    if (!(b & 0x80)) {
	*size = 8;
	goto done;
    }
    b = *(ptr++);
    part2 = (b & 0x7F);
    if (!(b & 0x80)) {
	*size = 9;
	goto done;
    }
    b = *(ptr++);
    part2 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
	*size = 10;
	goto done;
    }

  done:

    return ((uint64_t) (part0)) |
	((uint64_t) (part1) << 28) | ((uint64_t) (part2) << 56);


}

inline uint64_t
column_read (struct column_t_ * column, uint64_t position, uint8_t * size)
{
    if (column->buffer[position] < 0x80) {
	*size = 1;
	return column->buffer[position];
    }
    else {
	return column_bread (column, position, size);
    }
}

inline int
column_write (struct column_t_ *column, uint64_t position, uint64_t value)
{

    uint8_t *target = column->buffer + position;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = (uint32_t) (value);
    uint32_t part1 = (uint32_t) (value >> 28);
    uint32_t part2 = (uint32_t) (value >> 56);

    int size;

    // Here we can't really optimize for small numbers, since the value is
    // split into three parts.  Cheking for numbers < 128, for instance,
    // would require three comparisons, since you'd have to make sure part1
    // and part2 are zero.  However, if the caller is using 64-bit integers,
    // it is likely that they expect the numbers to often be very large, so
    // we probably don't want to optimize for small numbers anyway.  Thus,
    // we end up with a hardcoded binary search tree...
    if (part2 == 0) {
	if (part1 == 0) {
	    if (part0 < (1 << 14)) {
		if (part0 < (1 << 7)) {
		    size = 1;
		    goto size1;
		}
		else {
		    size = 2;
		    goto size2;
		}
	    }
	    else {
		if (part0 < (1 << 21)) {
		    size = 3;
		    goto size3;
		}
		else {
		    size = 4;
		    goto size4;
		}
	    }
	}
	else {
	    if (part1 < (1 << 14)) {
		if (part1 < (1 << 7)) {
		    size = 5;
		    goto size5;
		}
		else {
		    size = 6;
		    goto size6;
		}
	    }
	    else {
		if (part1 < (1 << 21)) {
		    size = 7;
		    goto size7;
		}
		else {
		    size = 8;
		    goto size8;
		}
	    }
	}
    }
    else {
	if (part2 < (1 << 7)) {
	    size = 9;
	    goto size9;
	}
	else {
	    size = 10;
	    goto size10;
	}
    }

    assert (1);

  size10:target[9] = (uint8_t) ((part2 >> 7) | 0x80);
  size9:target[8] = (uint8_t) ((part2) | 0x80);
  size8:target[7] = (uint8_t) ((part1 >> 21) | 0x80);
  size7:target[6] = (uint8_t) ((part1 >> 14) | 0x80);
  size6:target[5] = (uint8_t) ((part1 >> 7) | 0x80);
  size5:target[4] = (uint8_t) ((part1) | 0x80);
  size4:target[3] = (uint8_t) ((part0 >> 21) | 0x80);
  size3:target[2] = (uint8_t) ((part0 >> 14) | 0x80);
  size2:target[1] = (uint8_t) ((part0 >> 7) | 0x80);
  size1:target[0] = (uint8_t) ((part0) | 0x80);

    target[size - 1] &= 0x7F;

    return size;

}

//up must be pointing to a key
//returns the position plus the size of that key so as to fetch the value of the key
//you must always check the size
//keys are always bigger than 1 byte
uint64_t
column_bsearch (struct column_t_ * column, uint64_t up_position, uint64_t key,
		uint8_t * size)
{

    uint64_t up = up_position;
    uint64_t down = column->size - 1;
    uint64_t mid_val;
    uint64_t mid;

    while (up + 24 <= down) {	//???correct we set 24 because we make multiple reads to go to a valid number
	mid = (up + *size + down) / 2;
	column_read (column, mid, size);
	mid += *size;
	mid_val = column_read (column, mid, size);
	if (*size == 1) {
	    mid++;
	    mid_val = column_bread (column, mid, size);
	}
	if (mid_val < key) {
	    up = mid + *size + 1;	//plus 1 for the value of the key
	}
	else if (mid_val > key) {
	    down = mid - 2;	//minus 1 more cos of the value
	}
	else {
	    return mid;
	}
    }
    mid = up;
    while (mid <= down) {
//up must always be at a key
	mid_val = column_bread (column, mid, size);
	if (mid_val < key) {
	    mid = mid + *size + 1;
	}
	else {
	    if (mid_val == key) {
		return mid;
	    }
	    else {
		*size = 0;
		return mid;	//mid here points to a key bigger than key
	    }
	}
    }
    *size = 0;
    return column->size;	//here we send the last position
}


//both percentage and column need to be of dimension dim
uint8_t *
columns_join (struct column_t_ * column[], uint8_t percentage[], int dim,
	      uint64_t * size)
{

    uint64_t position[dim];
//set positions to 0
    int i;
    uint64_t min_size = column[0]->size;
    *size = 0;
    uint8_t value_size;
    uint64_t fixed_value = column_bread (column[0], 0, &value_size);
    int fixed_dim = 0;
    for (i = 0; i < dim; i++) {
	if (column[i]->size == 0) {
	    return NULL;
	}
	uint64_t value;
	uint8_t vsize;
	if (fixed_value > (value = column_bread (column[i], 0, &vsize))) {
	    fixed_value = value;
	    fixed_dim = i;
	    value_size = vsize;
	}

	position[i] = 0;
	if (column[i]->size < min_size) {
	    min_size = column[i]->size;
	}
    }




//in order to use the column_write
    column_t result;
    result.buffer = malloc (min_size);
    uint64_t write_position = 0;



    while (1) {

	int common = 1;
	for (i = 0; i < dim; i++) {
	    position[i] =
		column_bsearch (column[i], position[i], fixed_value,
				&value_size);
	    if (value_size == 0) {
		if (position[i] == column[i]->size) {
		    return result.buffer;
		}
		else {

		    common = 0;
		    fixed_value =
			column_bread (column[i], position[i], &value_size);
		    break;
		}

	    }



	}

	if (common) {
	    memcpy (result.buffer + write_position,
		    column[0]->buffer + position[0], value_size);
	    //in a joined column, we put a zero byte after the key to distinguish the values;
	    result.buffer[write_position + value_size] = 0;
	    for (i = 0; i < dim; i++) {
		memcpy (result.buffer + write_position + value_size + i + 1,
			column[i]->buffer + position[i] + value_size, 1);
		position[i] = position[i] + value_size + 1;
	    }
	    fixed_value = column_bread (column[0], position[0], &value_size);
	    write_position = write_position + value_size + 1 + dim;
	    (*size)++;
	}
    }


}
