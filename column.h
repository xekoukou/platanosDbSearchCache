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



#ifdef __cplusplus
extern "C"
{
#endif


    typedef struct column_t_ column_t;
    typedef struct column_reader_t_ column_reader_t;
    typedef struct column_writer_t_ column_writer_t;

    void column_init (column_t ** column);
    void column_destroy (column_t ** column);

    void column_reader_init (column_reader_t column_reader,
			     column_t * column);
    void column_writer_init (column_writer_t column_writer, column_t column);


#ifdef __cplusplus
}
#endif

#endif
