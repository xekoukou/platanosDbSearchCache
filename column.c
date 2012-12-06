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

    *column = malloc (sizeof (struct column_t_));
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


uint64_t
column_middle (struct column_t_ *column, uint64_t start, uint64_t end,
               uint64_t * key, uint8_t * size)
{
    assert ((start + 24 <= end) | (start >= 24 + end));
    uint64_t mid = (start + end) / 2;
    varint_read (column->buffer, mid, size);
    mid += *size;
    *key = varint_read (column->buffer, mid, size);
    if (*size == 1) {
        mid++;
        *key = varint_bread (column->buffer, mid, size);
    }
    return mid;
}


intersection_t *
intersection_new (uint64_t * uid, uint8_t * percentage, uint8_t * buffer,
                  uint64_t size, uint8_t dim)
{
    intersection_t *intersection = malloc (sizeof (intersection_t));
    intersection->dim = dim;
    intersection->uid = malloc (sizeof (uint64_t) * dim);
    intersection->percentage = malloc (sizeof (uint8_t) * dim);
    intersection->size = size;
    intersection->buffer = buffer;

    memcpy (intersection->uid, uid, sizeof (uint64_t) * dim);
    memcpy (intersection->percentage, percentage, sizeof (uint8_t) * dim);

    return intersection;
}

void
intersection_destroy (intersection_t ** intersection)
{
    free ((*intersection)->uid);
    free ((*intersection)->percentage);
    free (*intersection);
    intersection = NULL;

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


//both percentage and column need to be of dimension dim
//TODO take the percentages into consideration
intersection_t *
join_columns (join_t * join, struct column_t_ *column[],
              uint8_t percentage[], int dim)
{
    assert (dim <= join->dim);

    jlist_clear (join->jlist);
    uint64_t key;
    uint64_t position;
    uint8_t value;
    uint8_t size;

    int min_size = column[0]->size;
    int iter;
    for (iter = 0; iter < dim; iter++) {
        if (min_size > column[iter]->size) {
            min_size = column[iter]->size;
        }
//set the llimits and the positions to -1
        join->llimit[iter].position = 0;
        join->position[iter] = -1;
    }
//a small memory upper limit
    uint8_t *result =
        calloc (min_size + (int) ((1 / 3) * min_size + 1) * (dim - 1), 1);

    int res_pos = 0;            //the position where we write on result



    jnode_t *min_node;          //the current minimum key on the jlist,ie jlist->head->next[0]

    key = varint_bread (column[0]->buffer, join->llimit[0].position, &size);
    value = varint_sread (column[0]->buffer, join->llimit[0].position + size);
    jnode_init (&(join->barray[0][join->position[0] + 1]),
                key, size, join->llimit[0].position, value);
    jlist_add (join->jlist, &(join->barray[0][join->position[0] + 1]));

    join->position[0]++;
    min_node = join->barray[0] + join->position[0];
    int cur_dim = 0;


    while (1) {
        int common = 1;
        for (iter = 0; iter < dim; iter++) {
            if (iter != cur_dim) {
                int comp = 1;
                while (comp == 1) {

                    uint64_t rlimit;
                    if (join->position[iter] >= 0) {
                        rlimit =
                            join->barray[iter][join->position[iter]].position;
                    }
                    else {
                        if (join->llimit[iter].position == column[iter]->size) {
                            goto end;
                        }
                        rlimit = column[iter]->size;

                    }

                    if (join->llimit[iter].position + 24 > rlimit) {



                        int counter_ = 0;
                        uint64_t position_[9];
                        uint64_t key_[8];
                        uint8_t value_[8];
                        uint8_t size_[8];
                        position_[0] = join->llimit[iter].position;


                        while (position_[counter_] < rlimit) {
                            key_[counter_] =
                                varint_bread (column[iter]->buffer,
                                              position_[counter_],
                                              &size_[counter_]);
                            position_[counter_ + 1] = size_[counter_] + position_[counter_];    //position at m for node m-1
                            value_[counter_] =
                                varint_sread (column[iter]->buffer,
                                              position_[counter_] +
                                              size_[counter_]);
                            position_[counter_ + 1]++;
                            counter_++;
                        }
                        int siter;
                        for (siter = 1; siter < counter_ + 1; siter++) {
                            jnode_init (&
                                        (join->barray[iter][siter +
                                                            join->position
                                                            [iter]]),
                                        key_[counter_ - siter],
                                        size_[counter_ - siter],
                                        position_[counter_ - siter],
                                        value_[counter_ - siter]);
                            jlist_add (join->jlist, &(join->barray[iter]
                                                      [siter +
                                                       join->position
                                                       [iter]]));
                        }
                        join->position[iter] += counter_;

                        comp = comp_jnode_t
                            (&(join->barray[iter][join->position[iter]]),
                             min_node);

                        break;
                    }
                    else {

                        uint8_t size;
                        position =
                            column_middle (column[iter],
                                           join->llimit[iter].position,
                                           rlimit, &key, &size);
                        value =
                            varint_sread (column[iter]->buffer,
                                          position + size);

                        jnode_init (&(join->barray[iter]
                                      [join->position[iter] + 1]), key, size,
                                    position, value);
                        jlist_add (join->jlist,
                                   &(join->barray[iter][join->position[iter] +
                                                        1]));
                        join->position[iter]++;

                        comp = comp_jnode_t
                            (&(join->barray[iter][join->position[iter]]),
                             min_node);


                    }

                }
                if (comp < 0) {
                    min_node = join->barray[iter] + join->position[iter];
                    cur_dim = iter;
                    common = 0;
                    break;
                }
                if (comp > 0) {
                    //check jlist_add to learn why you should not change this
                    while (jlist_first (join->jlist)->key !=
                           join->barray[iter][join->position[iter]].key) {

                        join->llimit[(jlist_first (join->jlist))->dim].
                            position =
                            (jlist_first (join->jlist))->position +
                            (jlist_first (join->jlist))->size + 1;
                        join->position[(jlist_first (join->jlist))->dim]--;
                        jlist_delete (join->jlist,
                                      (jlist_first (join->jlist))->key);




                    }





                    min_node = join->barray[iter] + join->position[iter];
                    cur_dim = iter;

                    common = 0;
                    break;
                }


            }




        }
//this is the last comp
        if (common == 1) {
//remove the only node from the skiplist

            jlist_delete (join->jlist, min_node->key);

//store the result TODO create an intersection object

            res_pos += varint_write (result, res_pos, min_node->key);
            for (iter = 0; iter < dim; iter++) {
                assert (join->barray[iter][join->position[iter]].key ==
                        min_node->key);
                varint_write (result, res_pos,
                              join->barray[iter][join->position[iter]].value);
                res_pos++;

                join->llimit[iter].position =
                    join->barray[iter][join->position[iter]].position +
                    join->barray[iter][join->position[iter]].size + 1;
                join->position[iter]--;




            }


            key =
                varint_bread (column[0]->buffer, join->llimit[0].position,
                              &size);
            value =
                varint_sread (column[0]->buffer,
                              join->llimit[0].position + size);
            jnode_init (&(join->barray[0][join->position[0] + 1]), key, size,
                        join->llimit[0].position, value);
            jlist_add (join->jlist,
                       &(join->barray[0][join->position[0] + 1]));

            join->position[iter]++;
            min_node = join->barray[0] + join->position[0];
            cur_dim = 0;








        }

    }
  end:;

    uint64_t uid[dim];
    for (iter = 0; iter < dim; iter++) {
        uid[iter] = column[iter]->uid;
    }
    result = realloc (result, res_pos);
    return intersection_new (uid, percentage, result, res_pos, dim);
}
