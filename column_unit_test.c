#include"column.h"
#include<stdlib.h>
#include<stdio.h>
#include"assert.h"


int
main ()
{

    column_t *column[5];

    uint64_t common_value = 11110;

    int j;

    for (j = 0; j < 5; j++) {

        column_init (&column[j], 1, 80);

        uint64_t position = 0;
        int i;
        for (i = 0; i < 200; i++) {
            uint64_t size;
            if ((common_value < 256 + 256 * (i + 1))
                && (common_value > 256 * (i + 1))) {
                size = varint_write (column[j]->buffer, position, common_value);
            }
            else {
                uint64_t value = rand () % 256 + 256 * (i + 1);
                size = varint_write (column[j]->buffer, position, value);


            }
            position += size;

            varint_write (column[j]->buffer, position, rand () % 100);

            position++;
        }

        column[j]->size = position;
    }

    for (j = 0; j < 5; j++) {
        printf ("\ncolumn %d size:%lu", j, column[j]->size);

        uint64_t position = 0;
        while (position < column[j]->size) {
            uint8_t size;
            uint64_t key = varint_read (column[j]->buffer, position, &size);
            position += size;
            uint64_t value = varint_read (column[j]->buffer, position, &size);
            assert (size == 1);
            position++;
            printf ("\nkey:%lu value:%lu", key, value);

        }
    }



    intersection_t *intersection;
    uint8_t percentage[] = { 90, 90, 90, 90, 90 };

    join_t *join;
    join_new(&join,5,5000000);
    intersection = join_columns (join,column, percentage, 5);
    assert (intersection->buffer != NULL);


    printf ("\nsize:%lu", intersection->size);

    uint64_t position = 0;
    while (position < intersection->size) {
        uint8_t size;
        uint64_t key = varint_read (intersection->buffer, position, &size);
        position += size;
        uint64_t value1 = varint_read (intersection->buffer, position + 1, &size);
        uint64_t value2 = varint_read (intersection->buffer, position + 2, &size);
        uint64_t value3 = varint_read (intersection->buffer, position + 3, &size);
        uint64_t value4 = varint_read (intersection->buffer, position + 4, &size);
        uint64_t value5 = varint_read (intersection->buffer, position + 5, &size);
        assert (size == 1);
        position+=5;
        printf
            ("\nkey:%lu value1:%lu value2:%lu value3:%lu value4:%lu value5:%lu ",
             key, value1, value2, value3, value4, value5);

    }


    return 0;
}
