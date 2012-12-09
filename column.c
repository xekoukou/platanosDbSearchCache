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
//check jlist.h if you change these
//also intersection_join
#define MAX_BUF_SIZE 5242880
#define MAX_DIM_INTER 20
#define min_bracket (20*2 +2*MAX_DIM_INTER +1)

#include"column.h"



uint64_t
intersection_middle (intersection_t * intersection, uint64_t start,
                     uint64_t end, uint64_t * key, uint8_t * size)
{
    assert ((start + 20 + 2 * intersection->dim + 1 <= end) | (start >=
                                                               20 +
                                                               2 *
                                                               intersection->dim
                                                               + 1 + end));
    uint64_t mid = (start + end) / 2;
    varint_read (intersection->buffer, mid, size);
    mid += *size;
    *key = varint_read (intersection->buffer, mid, size);
    while (*size == 1) {
        mid++;
        *key = varint_read (intersection->buffer, mid, size);
    }
    return mid;
}


intersection_t *
intersection_enew (uint64_t * uid, uint8_t * percentage, uint8_t * buffer,
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

intersection_t *
intersection_inew (uint64_t * uid, uint8_t * percentage, uint8_t dim)
{
    intersection_t *intersection = malloc (sizeof (intersection_t));
    intersection->dim = dim;
    intersection->uid = malloc (sizeof (uint64_t) * dim);
    intersection->percentage = malloc (sizeof (uint8_t) * dim);
    intersection->buffer = (uint8_t *) malloc (MAX_BUF_SIZE);
    intersection->size = MAX_BUF_SIZE;


    memcpy (intersection->uid, uid, sizeof (uint64_t) * dim);
    memcpy (intersection->percentage, percentage, sizeof (uint8_t) * dim);

    return intersection;
}


void
intersection_destroy (intersection_t ** intersection)
{
    free ((*intersection)->uid);
    free ((*intersection)->percentage);
    free ((*intersection)->buffer);
    free (*intersection);
    intersection = NULL;

}

//the only difference is that we also allocate some memory for the
//red black tree
column_t *
column_inew (uint64_t * uid, uint8_t * percentage, uint8_t dim)
{
    column_t *intersection = malloc (sizeof (column_t));
    intersection->dim = dim;
    intersection->uid = malloc (sizeof (uint64_t) * dim);
    intersection->percentage = malloc (sizeof (uint8_t) * dim);
    intersection->buffer = (uint8_t *) malloc (MAX_BUF_SIZE);
    intersection->size = MAX_BUF_SIZE;


    memcpy (intersection->uid, uid, sizeof (uint64_t) * dim);
    memcpy (intersection->percentage, percentage, sizeof (uint8_t) * dim);

    return intersection;
}


typedef struct
{
    uint64_t position;
    uint8_t value[MAX_DIM_INTER];
    uint8_t size;
} stack_t;

void
stack_init (stack_t * stack, uint64_t position, uint8_t * value,
            uint8_t v_size, uint8_t size)
{
    stack->position = position;
    stack->size = size;
    memcpy (stack->value, value, v_size);



}


intersection_t *
intersections_join (intersection_t * intersection[], uint8_t percentage[],
                    int dim)
{

//the left limits on the binary search
    uint64_t llimit[dim];
    memset (llimit, 0, sizeof (llimit));
    stack_t stack[dim][65];
    jnode_t jnode[dim][65];
    memset (jnode, 0, sizeof (jnode));

//used to show the position in the stack
    int sposition[dim];
    memset (sposition, 0, sizeof (sposition));

//this needs to change in case the MAX_DIM_INTER change

//these are used to hold the results of 27 reads of an intersection
    uint64_t position[28];      //27+1 see linear search
    uint64_t key[27];
    uint8_t value[27][MAX_DIM_INTER];
    uint8_t size[27];

    jlist_t jlist;

//the position that is in the first stack element
//the right limit to the binary search
    uint64_t rlimit;

//max_height will be used on jlist_init to define the maximum heiht of the skiplist


    int result_size = intersection[0]->size;
    int result_dim = 0;
    int max_height = 0;
    int iter;
    for (iter = 0; iter < dim; iter++) {
        if (result_size > intersection[iter]->size) {
            result_size = intersection[iter]->size;
        }
        max_height += (int) log2 (intersection[iter]->size) + 1;
        result_dim += intersection[iter]->dim;

//setting the position of the first stack element to intersection->size
//so as to be picked by rlimit when we need to return or start from the
//beginning

        stack[iter][0].position = intersection[iter]->size;

    }
//the max_height is the log2 of the maximum number of elements
    max_height = (int) log2 (max_height) + 1;

//finding the minimum possible upper limit of the size by taking into acount 
//the fact that varint varries ftom 10 to 2 in our case and values are 1.
    result_size =
        ((int) result_size * 10 / 11) + 1 + result_dim * (result_size * 1 / 3);

//allocation the buffer
    uint8_t *result = malloc (result_size);
//pointer to the buffer
    int res_pos = 0;

//ordering the uids of the intersections
//this needs to change in case MAX_DIM_INTER change
    uint8_t order[2][400][2];   //MAX_DIM_INTER^2
    memset (order, 0, sizeof (order));
    int merge_size[2] = { 0 };

//initialize the order array

    for (iter = 0; iter < intersection[0]->dim; iter++) {
        order[0][iter][1] = iter;
    }
    merge_size[0] = intersection[0]->dim;

//make the merge

//used to interchange between the 2 order submatrices
    uint8_t flip = 0;
    uint8_t flipo;

    for (iter = 1; iter < dim; iter++) {
        flipo = flip;
        flip = iter & 0x1;

//used to point at the 2 to be merged "arrays"
        uint8_t point1 = 0;
        uint8_t point2 = 0;
        merge_size[flip] = 0;
        while (1) {

            if (intersection[order[flipo][point1][0]]->uid
                [order[flipo][point1][1]] > intersection[iter]->uid[point2]) {

//second byte decides the intersection
//first the uid of that intersection
                order[flip][point1 + point2][1] = point2;
                order[flip][point1 + point2][0] = iter;

                point2++;
                merge_size[flip]++;
//if we reached at the end append the other array
                if (point2 == intersection[iter]->dim) {

                    int siter;
                    for (siter = point1; siter < merge_size[flipo]; siter++) {
                        order[flip][siter + point2][0] = order[flipo][siter][0];
                        order[flip][siter + point2][1] = order[flipo][siter][1];


                    }
                    merge_size[flip] += merge_size[flipo] - point1;

                    break;

                }

            }
            else {

                order[flip][point1 + point2][0] = order[flipo][point1][0];
                order[flip][point1 + point2][1] = order[flipo][point1][1];
                point1++;
                merge_size[flip]++;


                if (point1 == merge_size[flipo]) {

                    int siter;
                    for (siter = point2; siter < intersection[iter]->dim;
                         siter++) {
                        order[flip][siter + point1][0] = iter;
                        order[flip][siter + point1][1] = siter;


                    }
                    merge_size[flip] += intersection[iter]->dim - point2;

                    break;

                }

            }
        }

    }

//the result is the order[flip] and merge_size[flip]
//here ends the MERGE subprocess


//initializing the jlist
    jlist_init (&jlist, max_height);

//this is the current minimum node in the jlist;
//this is the node new nodes compare to.

    jnode_t *min_node;

//we pick the first min_node from the 0 intersection
//to initialize the loop

//update the stack pointer
    sposition[0]++;

//read the key
    key[0] = varint_bread (intersection[0]->buffer, llimit[0], &size[0]);

//obtain the values
    for (iter = 0; iter < intersection[0]->dim; iter++) {
        value[0][iter] =
            varint_sread (intersection[0]->buffer, llimit[0] + size[0] + iter);
    }
//init the jnode
    jnode_clear (&(jnode[0][sposition[0]]));
    jnode_init (&(jnode[0][sposition[0]]), key[0], 0);
//init the stack node
    stack_init (&(stack[0][sposition[0]]), 0, value[0], intersection[0]->dim,
                size[0]);

//add the jnode in the jlist
    jlist_add (&jlist, &(jnode[0][sposition[0]]));


//pick it as the initial min_node
    min_node = &(jnode[0][sposition[0]]);


//main loop
    while (1) {


//varriable that shows if all intesections have this key
        int common = 1;

//loop on all intersections searching for the key
        for (iter = 0; iter < dim; iter++) {

//varrible that shows whether the current last element in the stack is
//greater or lower or equal than the min_node
            int comp = 1;

//if greater search for a smaller element with binary search
            while (comp == 1) {

//when the left limit of any intersection buffer reaches the size of the buffer
//we have finished our work

                if (llimit[iter] >= intersection[iter]->size) {
                    goto end;

                }

//we update the rlimit
                rlimit = stack[iter][sposition[iter]].position;


                printf ("\ndim:%d llimit:%lu rlimit:%lu", iter, llimit[iter],
                        rlimit);
//if the search interval is small we do a linear search for the next node
//otherwise we do a binary search between llimit[iter] and rlimit


                if (llimit[iter] + min_bracket > rlimit) {
//the number of keys/nodes found in the linear search inside the bracket;
                    int counter = 0;

//we start at the llimit
                    position[0] = llimit[iter];

//until we reach the rlimit
                    while (position[counter] < rlimit) {

//read the key
                        key[counter] =
                            varint_bread (intersection[iter]->buffer,
                                          position[counter], &size[counter]);

//obtain the values
                        int siter;
                        for (siter = 0; siter < intersection[iter]->dim;
                             siter++) {
                            value[counter][siter] =
                                varint_sread (intersection[iter]->buffer,
                                              position[counter] +
                                              size[counter] + siter);
                        }
                        position[counter + 1] =
                            position[counter] + size[counter] +
                            intersection[iter]->dim;
                        counter++;
                    }

//write the results to the stack and jnode
//order them correctly 
                    int siter;
                    for (siter = 1; siter <= counter; siter++) {

//init the jnode
                        jnode_clear (&(jnode[iter][sposition[iter] + siter]));
                        jnode_init (&(jnode[iter][sposition[iter] + siter]),
                                    key[counter - siter], iter);
//init the stack node
                        stack_init (&(stack[iter][sposition[iter] + siter]),
                                    position[counter - siter],
                                    value[counter - siter],
                                    intersection[iter]->dim,
                                    size[counter - siter]);

//add the jnode in the jlist
                        jlist_add (&jlist,
                                   &(jnode[iter][sposition[iter] + siter]));

                    }

//update the sposition
                    sposition[iter] += counter;

//compare with min_node
                    comp =
                        comp_jnode_t (&(jnode[iter][sposition[iter]]),
                                      min_node);

//after a linear search we have searched through all the keys of this bracket
//so we break, the min_node will have to change

                    break;

                }
                else {

//update the stack pointer
                    sposition[iter]++;

//we find the position with binary search on the bracket
                    position[0] =
                        intersection_middle (intersection[iter], llimit[iter],
                                             rlimit, &(key[0]), &(size[0]));


//obtain the values 
                    int siter;
                    for (siter = 0; siter < intersection[iter]->dim; siter++) {
                        value[0][siter] =
                            varint_sread (intersection[iter]->buffer,
                                          position[0] + size[0] + siter);
                    }
//init the jnode
                    jnode_clear (&(jnode[iter][sposition[iter]]));
                    jnode_init (&(jnode[iter][sposition[iter]]), key[0], iter);
//init the stack node
                    stack_init (&(stack[iter][sposition[iter]]), position[0],
                                value[0], intersection[iter]->dim, size[0]);

//add the jnode in the jlist
                    jlist_add (&jlist, &(jnode[iter][sposition[iter]]));


//compare with min_node
                    comp =
                        comp_jnode_t (&(jnode[iter][sposition[iter]]),
                                      min_node);

                }
            }
//now we have added one or more nodes to the jlist and we have updated the sposition[iter]

//2 cases

            if (comp < 0) {

//the new node is smaller,so we update the min_node
                min_node = &(jnode[iter][sposition[iter]]);

//the previous min_node is not a common element
                common = 0;
//we break the for loop to start from the beggining
                break;
            }

//this is when we have done a linear search and the last node is still bigger

            if (comp > 0) {

//we remove all nodes bigger than the current

                jnode_t *jtemp = jlist_first (&jlist);
                while (jtemp->key != jnode[iter][sposition[iter]].key) {


//update all the stacks that are referenced by this jnode
                    int siter;
                    for (siter = 0; siter < jtemp->dim_size; siter++) {

//update the left limit
                        llimit[jtemp->dim[siter]] =
                            stack[jtemp->dim[siter]][sposition
                                                     [jtemp->dim[siter]]].
                            position +
                            stack[jtemp->
                                  dim[siter]][sposition[jtemp->dim[siter]]].
                            size + intersection[jtemp->dim[siter]]->dim;

//update the position
                        sposition[jtemp->dim[siter]]--;
                    }

//remove the jnode from the jlist
                    jlist_delete (&jlist, jtemp->key);

                    jtemp = jlist_first (&jlist);
                }

//we set as min_node the new node
                min_node = &(jnode[iter][sposition[iter]]);

//the previous min_node is not a common element
                common = 0;

//we break the for loop since this is a new elm and we need to start from the beginning
                break;
            }
        }

//in case we found a common key

        if (common) {

//we write the result

            res_pos += varint_write (result, res_pos, min_node->key);

//we use the order[flip] and merge_size[flip]
            for (iter = 0; iter < merge_size[flip]; iter++) {

                assert (jnode[order[flip][iter][0]]
                        [sposition[order[flip][iter][0]]].key == min_node->key);
                varint_write (result, res_pos,
                              stack[order[flip][iter][0]][sposition
                                                          [order[flip][iter]
                                                           [0]]].value[order
                                                                       [flip]
                                                                       [iter]
                                                                       [1]]);
                res_pos++;

            }

//update the sposition and the llimit
            for (iter = 0; iter < dim; iter++) {
                llimit[iter] =
                    stack[iter][sposition[iter]].position +
                    stack[iter][sposition[iter]].size + intersection[iter]->dim;
                sposition[iter]--;
            }

//we delete the node from the jlist
            jlist_delete (&jlist, min_node->key);

// PICK A NEW MIN_NODE

            if (llimit[0] != intersection[0]->size) {

//update the stack pointer
                sposition[0]++;

//read the key
                key[0] =
                    varint_bread (intersection[0]->buffer, llimit[0], &size[0]);

//obtain the values
                for (iter = 0; iter < intersection[0]->dim; iter++) {
                    value[0][iter] =
                        varint_sread (intersection[0]->buffer,
                                      llimit[0] + size[0] + iter);
                }
//init the jnode
                jnode_clear (&(jnode[0][sposition[0]]));
                jnode_init (&(jnode[0][sposition[0]]), key[0], 0);
//init the stack node
                stack_init (&(stack[0][sposition[0]]), llimit[0], value[0],
                            intersection[0]->dim, size[0]);

//add the jnode in the jlist
                jlist_add (&jlist, &(jnode[0][sposition[0]]));


//pick it as the initial min_node
                min_node = &(jnode[0][sposition[0]]);

            }
            else {
//we reached at the end of intersection[0]
                goto end;

            }



        }



    }




  end:;

//create the new uid array
    uint64_t uid[MAX_DIM_INTER];
    for (iter = 0; iter < merge_size[flip]; iter++) {
        uid[iter] =
            intersection[order[flip][iter][0]]->uid[order[flip][iter][1]];
    }

//realloc the buffer to fit its real size

    result = realloc (result, res_pos);

//initialize the intersection an return it

    return intersection_enew (uid, percentage, result, res_pos,
                              merge_size[flip]);



}
