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
column_destroy (struct column_t_ **column)
{
    free ((*column)->buffer);
    free (*column);
    *column = NULL;
};

//read a number which we know is 1 byte
//no checks are done

inline uint64_t
column_sread (struct column_t_ *column, int position)
{
    return (uint64_t) column->buffer[position];
}

//this is a read for big numbers
uint64_t
column_bread (struct column_t_ * column, int position, uint8_t * size)
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
column_read (struct column_t_ * column, int position, uint8_t * size)
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

void
join_new (join_t ** join, int max_dim, int max_size)
{
    *join = calloc (1, sizeof (join_t));
    (*join)->dim = max_dim;
    int array_size = (int) log2 (max_size) + 1 + 8;     //when start and end are close
//we stop using bsearch, that is why i added 8, the maximum number of elements 24/3
//3 is is the minimum size of a key plus value
    (*join)->size = array_size;
    int max_height = (int) log2 (max_dim * array_size) + 1;
    (*join)->jlist = jlist_new (max_height);
    (*join)->barray = calloc (max_dim, sizeof (jnode_t *));
    (*join)->llimit = calloc (max_dim, sizeof (llimit_t));
    (*join)->position = calloc (max_dim, sizeof (int));
    int iter;
    for (iter = 0; iter < max_dim; iter++) {
        (*join)->barray[iter] = calloc (array_size, sizeof (jnode_t));
        int siter;
        for (siter = 0; siter < array_size; siter++) {
            (*join)->barray[iter][siter].next =
                calloc (max_height, sizeof (jnode_t *));
            (*join)->barray[iter][siter].dim = iter;
        }
    }


}

void
join_destroy (join_t ** join)
{

    int iter;
    for (iter = 0; iter < (*join)->dim; iter++) {
        int siter;
        for (siter = 0; siter < (*join)->size; siter++) {
            free ((*join)->barray[iter][siter].next);

        }
        free ((*join)->barray[iter]);
    }
    free ((*join)->position);
    free ((*join)->llimit);
    free ((*join)->barray);
    jlist_destroy (&((*join)->jlist));
    free (*join);
    *join = NULL;
}



uint64_t
column_middle (struct column_t_ *column, uint64_t start, uint64_t end,
               uint64_t * key, uint8_t * size)
{
    assert ((start + 24 <= end) | (start >= 24 + end));
    uint64_t mid = (start + end) / 2;
    column_read (column, mid, size);
    mid += *size;
    *key = column_read (column, mid, size);
    if (*size == 1) {
        mid++;
        *key = column_bread (column, mid, size);
    }
    return mid;
}

//both percentage and column need to be of dimension dim
uint8_t *
join_columns (join_t * join, struct column_t_ * column[],
              uint8_t percentage[], int dim, uint64_t * res_size)
{
    jlist_clear (join->jlist);
    uint64_t key;
    uint64_t position;
    uint8_t value;

    assert (dim < join->dim);
    int min_size = 1;
    int iter;
    for (iter = 0; iter < dim; iter++) {
        if (min_size > column[iter]->size) {
            min_size = column[iter]->size;
        }
//set the llimits and the positions to -1
        join->llimit[iter]->position = 0;
        join->position[iter] = -1;
    }
//a small memory upper limit
    uint8_t *result =
        calloc (min_size + (int) ((1 / 3) * min_size + 1) * (dim - 1), 1);

    int res_pos = 0;            //the position where we write on result



    jnode_t *min_node;          //the current minimum key on the jlist,ie jlist->head->next[0]

    key = column_bread (column[0], join->llimit[0].position, &size);
    value = column_sread (column[0], join->llimit[0].position + size);
    node_init (&(join->barray[0][join->position[0] + 1]),
               key, size, value, join->llimit[0].position);
    jlist_add (join->jlist, &(join->barray[0][join->position[0] + 1]));

    join->position[iter]++;
    min_node = join->barray[0][join->position[0] + 1];
    int cur_dim = 0;


    while (1) {
        int common = 1;
        for (iter = 0; iter < dim; iter++) {
            if (iter != cur_dim) {
                int comp = 1;
                while (comp) {

                    uint64_t rlimit;
                    int counter_ = 0;
                    uint64_t position_[9] = { 0 };
                    if (join->position[iter] >= 0) {
                        rlimit =
                            join->barray[iter][join->position[iter]].position;
                        position_[0] = join->llimit[iter].position;
                    }
                    else {
                        rlimit = column[iter]->size;

                    }
                    if (join->llimit[iter].position + 24 > rlimit) {
                        uint64_t key_[8];
                        uint8_t value_[8];
                        uint8_t size_[8];
                        while (1) {
                            key_[counter_] =
                                column_bread (column[iter],
                                              position_[counter_],
                                              &size_[counter]);
                            position_[counter_ + 1] = size_[counter] + position_[counter_];     //position at m for node m-1
                            value_[counter_] =
                                column_sread (column[iter],
                                              position_[counter_] +
                                              size_[counter]);
                            position_[counter_ + 1]++;
                            counter_++;
                            if (position_[counter_] >= rlimit) {
                                break;
                            }
                        }
                        for (siter = join->position[iter] + 1;
                             siter < counter_ + join->position[iter] + 1;
                             siter++) {
                            node_init (&(join->barray[iter][siter]),
                                       key_[counter_ - siter], size_[counter],
                                       value_[counter_ - siter],
                                       position_[counter_ + 1 - siter]);
                            jlist_add (join->jlist,
                                       &(join->barray[iter][siter]));
                        }
                        join->position[iter] += counter_;
                    }
                    else {

                        uint8_t size;
                        position =
                            column_middle (column[iter],
                                           join->llimit[iter].position,
                                           rlimit, &key, &size);
                        value =
                            column_sread (column[iter],
                                          join->
                                          barray[iter][join->position[iter] +
                                                       1].position + size);

                        node_init (join->barray[iter]
                                   [join->position[iter] + 1], key, size,
                                   position, value);
                        jlist_add (join->jlist,
                                   join->barray[iter][join->position[iter] +
                                                      1]);
                        join->position[iter]++;

                    }

                    comp = comp_jnode_t
                        (join->barray[iter][join->position[iter]], min_node);
                }
                if (comp < 0) {
                    min_node = join->barray[iter][join->position[iter]];
                    cur_dim = iter;
                    common = 0;
                    break;
                }


            }




        }

        if (common) {
//remove the only node from the skiplist

            jlist_delete (join->jlist, min_node->key);

//store the result TODO create an intersection object

            res_pos += column_write (result, res_pos, min_node->key);
            for (iter = 0; iter < dim; iter++) {
                assert (join->barray[iter][join->position[iter]].key ==
                        min_node->key);
                column_write (result, res_pos,
                              join->barray[iter][join->position[iter]].value);
                res_pos++;

                join->llimit[iter].position =
                    join->barray[iter][join->position[iter]].position +
                    join->barray[iter][join->position[iter]].size;
                join->position[iter]--;




            }


            key = column_bread (column[0], join->llimit[0].position, &size);
            value = column_sread (column[0], join->llimit[0].position + size);
            node_init (&(join->barray[0][join->position[0] + 1]),
                       key, size, value, join->llimit[0].position);
            jlist_add (join->jlist,
                       &(join->barray[0][join->position[0] + 1]));

            join->position[iter]++;
            min_node = join->barray[0][join->position[0] + 1];
            cur_dim = 0;








        }

    }
    *res_size = res_pos;
    return result;


}
