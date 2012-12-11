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


#include"column.h"
#include<stdlib.h>
#include<stdio.h>
#include"assert.h"


int
main ()
{

    intersection_t *column[5];

    uint64_t common_value = 11110;

    uint64_t j;

    for (j = 0; j < 5; j++) {
        uint8_t perce = 80;
        column[j] = intersection_inew (&j, &perce, 1);

        uint64_t position = 0;
        int i;
        for (i = 0; i < 200; i++) {
            uint64_t size;
            if ((common_value < 256 + 256 * (i + 1))
                && (common_value > 256 * (i + 1))) {
                size = varint_write (column[j]->buffer, position, common_value);

                position += size;

                varint_write (column[j]->buffer, position, 92);

                position++;


            }
            else {
                uint64_t value = rand () % 256 + 256 * (i + 1);
                size = varint_write (column[j]->buffer, position, value);



                position += size;

                varint_write (column[j]->buffer, position, rand () % 100);

                position++;
            }
        }

        column[j]->size = position;
    }

    for (j = 0; j < 5; j++) {
        printf ("\ncolumn %lu size:%lu", j, column[j]->size);

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

    intersection =
        intersections_join ((intersection_t **) column, percentage, 5, 5);
    assert (intersection->buffer != NULL);


    printf ("\nsize:%lu", intersection->size);

    uint64_t position = 0;
    while (position < intersection->size) {
        uint8_t size;
        uint64_t key = varint_read (intersection->buffer, position, &size);
        position += size;
        uint8_t siz;
        uint64_t value1 = varint_read (intersection->buffer, position, &siz);
        uint64_t value2 =
            varint_read (intersection->buffer, position + 1, &siz);
        uint64_t value3 =
            varint_read (intersection->buffer, position + 2, &siz);
        uint64_t value4 =
            varint_read (intersection->buffer, position + 3, &siz);
        uint64_t value5 =
            varint_read (intersection->buffer, position + 4, &siz);
        assert (siz == 1);
        position += 5;
        printf
            ("\nkey:%lu value1:%lu value2:%lu value3:%lu value4:%lu value5:%lu ",
             key, value1, value2, value3, value4, value5);

    }


    return 0;
}
