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
                size = column_write (column[j], position, common_value);
            }
            else {
                uint64_t value = rand () % 256 + 256 * (i + 1);
                size = column_write (column[j], position, value);


            }
            position += size;

            column_write (column[j], position, rand () % 100);

            position++;
        }

        column[j]->size = position;
    }

    for (j = 0; j < 5; j++) {
        printf ("\ncolumn %d size:%lu", j, column[j]->size);

        uint64_t position = 0;
        while (position < column[j]->size) {
            uint8_t size;
            uint64_t key = column_read (column[j], position, &size);
            position += size;
            uint64_t value = column_read (column[j], position, &size);
            assert (size == 1);
            position++;
            printf ("\nkey:%lu value:%lu", key, value);

        }
    }



    column_t jcolumn;
    uint8_t percentage[] = { 90, 90, 90, 90, 90 };
    jcolumn.buffer = columns_join (column, percentage, 5, &(jcolumn.size));
    assert (jcolumn.buffer != NULL);


    printf ("\nsize:%lu", jcolumn.size);

    uint64_t position = 0;
    while (position < jcolumn.size) {
        uint8_t size;
        uint64_t key = column_read (&jcolumn, position, &size);
        position += size;
        uint64_t value1 = column_read (&jcolumn, position + 1, &size);
        uint64_t value2 = column_read (&jcolumn, position + 2, &size);
        uint64_t value3 = column_read (&jcolumn, position + 3, &size);
        uint64_t value4 = column_read (&jcolumn, position + 4, &size);
        uint64_t value5 = column_read (&jcolumn, position + 5, &size);
        assert (size == 1);
        position++;
        printf
            ("\nkey:%lu value1:%lu value2:%lu value3:%lu value4:%lu value5:%lu ",
             key, value1, value2, value3, value4, value5);

    }


    return 0;
}
