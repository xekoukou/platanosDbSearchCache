#include"setbook.h"
#include<stdio.h>



int
main ()
{

    critbit_tree *setbook;


    setbook_init (&setbook);

    uint64_t uid[6][15];

    uid[0][0] = 2;
    uid[0][1] = 3;
    uid[0][2] = 8;
    uid[0][3] = 6;

    setbook_insert (setbook, uid[0], 4);

    uid[1][0] = 2;
    uid[1][1] = 5;

    setbook_insert (setbook, uid[1], 2);


    uid[2][0] = 5;
    uid[2][1] = 7;


    setbook_insert (setbook, uid[2], 2);

    uid[3][0] = 5;
    uid[3][1] = 7;
    uid[3][2] = 9;

    setbook_insert (setbook, uid[3], 3);

    uid[4][0] = 9;

    setbook_insert (setbook, uid[4], 1);


    uid[5][0] = 3;
    uid[5][1] = 9;

    setbook_insert (setbook, uid[5], 1);


    uint64_t search[4];

    search[0] = 2;
    search[1] = 5;
    search[2] = 7;
    search[3] = 9;

    int result[4][4];
    int size[4];
    int res_number;

    setbook_contains (setbook, search, 4, result, size, &res_number);

    printf ("\nresults");
    int iter;
    for (iter = 0; iter < res_number; iter++) {
        printf ("\nintersection %d", iter);
        int siter;
        for (siter = 0; siter < size[iter]; siter++) {
            printf ("\n%d %d", siter, result[iter][siter]);

        }
    }

    printf ("\n");

    return 0;
}
