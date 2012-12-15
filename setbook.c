#include"setbook.h"


//results should be of dimension [dim][dim]
//res_size is the size of the one dim
//res_number of the other
void
setbook_contains (critbit_tree * setbook, uint64_t * uid, int dim,
                  int result[dim][dim], int res_size[dim], int *res_number)
{

    uint32_t part0;
    uint32_t part1;
    uint32_t part2;

    char value[dim][10];

    int iter;
    for (iter = 0; iter < dim; iter++) {
        part0 = uid[iter];
        part1 = uid[iter] >> 28;
        part2 = uid[iter] >> 56;

        value[iter][9] = (uint8_t) ((part2 >> 7) | 0x80);
        value[iter][8] = (uint8_t) ((part2) | 0x80);
        value[iter][7] = (uint8_t) ((part1 >> 21) | 0x80);
        value[iter][6] = (uint8_t) ((part1 >> 14) | 0x80);
        value[iter][5] = (uint8_t) ((part1 >> 7) | 0x80);
        value[iter][4] = (uint8_t) ((part1) | 0x80);
        value[iter][3] = (uint8_t) ((part0 >> 21) | 0x80);
        value[iter][2] = (uint8_t) ((part0 >> 14) | 0x80);
        value[iter][1] = (uint8_t) ((part0 >> 7) | 0x80);
        value[iter][0] = (uint8_t) ((part0) | 0x80);




    }



    critbit_node *ptr[dim];
    int lpos[dim];              //left is positive direction
    int rpos[dim];
    int where[dim];



//search results
//dim pointers to an intersection which contains the uid and has the biggest size

    int inters[dim][dim];
    int inters_size[dim];
    memset (inters_size, 0, sizeof (inters_size));


    if (!setbook->root) {

        *res_size = 0;
        return;

    }

    int level = 0;

    ptr[level] = setbook->root;

    while (1) {

        while (1 & (intptr_t) ptr[level]) {
            ptr[level] = (critbit_node *) ((uint8_t *) ptr[level] - 1);
//internal node

            int iter;
            lpos[level] = 0;
            rpos[level] = 0;

            int integral = ptr[level]->byte / 10;
            iter = integral;
            while (iter < dim) {

                if ((1 +
                     (ptr[level]->otherbits | value[iter][ptr[level]->byte -
                                                          integral *
                                                          10])) >> 8) {
                    lpos[level] = 1;
                }
                else {
                    rpos[level] = 1;
                }
                iter++;
            }
            if (lpos[level]) {
                where[level] = 1;
                level++;
                ptr[level] = ptr[level - 1]->child[where[level - 1]];

            }
            else {
                if (rpos[level]) {
                    where[level] = 0;
                    level++;
                    ptr[level] = ptr[level - 1]->child[where[level - 1]];

                }
                else {

                    level--;

                    while (1) {
                        if (where[level]) {
                            lpos[level] = 0;
                        }
                        else {
                            rpos[level] = 0;
                        }

                        if ((!lpos[level]) && (!rpos[level]) && (level >= 0)) {
                            level--;
                        }
                        else {
                            break;
                        }
                    }



                    if (level < 0) {
                        goto end;
                    }
                    else {

                        if (lpos[level]) {
                            where[level] = 1;
                        }
                        else {
                            where[level] = 0;
                        }
                        level++;

                        ptr[level] = ptr[level - 1]->child[where[level]];

                    }

                }
            }
        }

//here we do our search
        size_t plen = (int) (strlen ((const char *) ptr[level]) / 10);
        int presult[plen];

        int uiter = -1;
        int iter;
        for (iter = 0; iter < plen; iter++) {
            while (++uiter < dim) {

                if (memcmp
                    (value[uiter], (uint8_t *) ptr[level] + iter * 10,
                     10) == 0) {
                    presult[iter] = uiter;
                    break;
                }
            }
        }

        if (uiter >= dim) {
//this is not it
        }
        else {
            for (iter = 0; iter < plen; iter++) {
                if (inters_size[presult[iter]] < plen) {
                    memcpy (inters[presult[iter]], presult, sizeof (presult));
                    inters_size[presult[iter]] = plen;
                }
            }
        }



        level--;

        while (1) {
            if (where[level]) {
                lpos[level] = 0;
            }
            else {
                rpos[level] = 0;
            }

            if ((!lpos[level]) && (!rpos[level]) && (level >= 0)) {
                level--;
            }
            else {
                break;
            }
        }

        if (level < 0) {
            goto end;
        }
        else {

            if (lpos[level]) {
                where[level] = 1;
            }
            else {
                where[level] = 0;
            }
            level++;

            ptr[level] = ptr[level - 1]->child[where[level]];

        }


    }

// the end

  end:;

    iter = 0;
    int siter = 0;


    while ((iter < dim) && (inters_size[iter])) {
        int titer;
        int exists = 0;
        for (titer = 0; titer < siter; titer++) {
            if (memcmp
                (inters[iter], result[titer],
                 inters_size[iter] * sizeof (int)) == 0) {
                exists = 1;
                break;
            }
        }
        if (!exists) {
            memcpy (result[siter], inters[iter],
                    inters_size[iter] * sizeof (int));
            res_size[siter] = inters_size[iter];
            siter++;
        }
        iter++;
    }

    if (!inters_size[iter]) {
        *res_number = 0;
    }
    else {
        *res_number = siter;
    }

}


int
setbook_insert (critbit_tree * setbook, uint64_t * uid, int dim)
{

    uint32_t part0;
    uint32_t part1;
    uint32_t part2;

    char value[dim * 10 + 1];
    value[dim * sizeof (uint64_t)] = '\0';

    int iter;
    for (iter = 0; iter < dim; iter++) {
        part0 = uid[iter];
        part1 = uid[iter] >> 28;
        part2 = uid[iter] >> 56;

        value[10 * iter + 9] = (uint8_t) ((part2 >> 7) | 0x80);
        value[10 * iter + 8] = (uint8_t) ((part2) | 0x80);
        value[10 * iter + 7] = (uint8_t) ((part1 >> 21) | 0x80);
        value[10 * iter + 6] = (uint8_t) ((part1 >> 14) | 0x80);
        value[10 * iter + 5] = (uint8_t) ((part1 >> 7) | 0x80);
        value[10 * iter + 4] = (uint8_t) ((part1) | 0x80);
        value[10 * iter + 3] = (uint8_t) ((part0 >> 21) | 0x80);
        value[10 * iter + 2] = (uint8_t) ((part0 >> 14) | 0x80);
        value[10 * iter + 1] = (uint8_t) ((part0 >> 7) | 0x80);
        value[10 * iter + 0] = (uint8_t) ((part0) | 0x80);




    }


    return critbit_insert (setbook, value);

}

int
setbook_delete (critbit_tree * setbook, uint64_t * uid, int dim)
{

    uint32_t part0;
    uint32_t part1;
    uint32_t part2;

    char value[dim * 10 + 1];
    value[dim * sizeof (uint64_t)] = '\0';

    int iter;
    for (iter = 0; iter < dim; iter++) {
        part0 = uid[iter];
        part1 = uid[iter] >> 28;
        part2 = uid[iter] >> 56;

        value[10 * iter + 9] = (uint8_t) ((part2 >> 7) | 0x80);
        value[10 * iter + 8] = (uint8_t) ((part2) | 0x80);
        value[10 * iter + 7] = (uint8_t) ((part1 >> 21) | 0x80);
        value[10 * iter + 6] = (uint8_t) ((part1 >> 14) | 0x80);
        value[10 * iter + 5] = (uint8_t) ((part1 >> 7) | 0x80);
        value[10 * iter + 4] = (uint8_t) ((part1) | 0x80);
        value[10 * iter + 3] = (uint8_t) ((part0 >> 21) | 0x80);
        value[10 * iter + 2] = (uint8_t) ((part0 >> 14) | 0x80);
        value[10 * iter + 1] = (uint8_t) ((part0 >> 7) | 0x80);
        value[10 * iter + 0] = (uint8_t) ((part0) | 0x80);




    }



    return critbit_delete (setbook, value);

}

void
setbook_init (critbit_tree ** setbook)
{
    *setbook = calloc (1, sizeof (critbit_tree));
}

void
setbook_clear (critbit_tree * setbook)
{
    critbit_clear (setbook);

}

void
setbook_destroy (critbit_tree ** setbook)
{
    critbit_clear (*setbook);
    free (*setbook);
    *setbook = NULL;
}
