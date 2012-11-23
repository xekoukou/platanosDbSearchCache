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

#include<google/protobuf/io/zero_copy_stream_impl.h>
#include<google/protobuf/io/coded_stream.h>
#include<stdlib.h>
#include<stdint.h>
#include<new>

using
    google::protobuf::io::CodedInputStream;
using
    google::protobuf::io::CodedOutputStream;
using
    google::protobuf::io::ArrayOutputStream;


struct column_t_
{
    uint8_t
	percentage;
    uint8_t *
	buffer;
    CodedOutputStream *
	writer;
    ArrayOutputStream *
	array;
};

struct column_reader_t_
{
    char
    reader[sizeof (CodedInputStream)];
};

struct column_writer_t_
{
    char
    writer[sizeof (CodedOutputStream)];
    char
    array[sizeof (ArrayOutputStream)];
};

void
column_init (struct column_t_ **column)
{

    *column = (column_t_ *) malloc (sizeof (column_t_));
    (*column)->percentage = 0;
    (*column)->buffer = (uint8_t *) malloc (MAX_BUFFER_SIZE);
}

void
column_reader_init (struct column_reader_t_ column_reader,
		    struct column_t_ *column)
{
    new (column_reader.reader) CodedInputStream (column->buffer,
						 MAX_BUFFER_SIZE);
}

void
column_writer_init (struct column_writer_t_ column_writer,
		    struct column_t_ *column)
{
    new (column_writer.array) ArrayOutputStream (column->buffer,
						 MAX_BUFFER_SIZE, -1);
    new (column_writer.
	 writer) CodedOutputStream ((ArrayOutputStream *) column_writer.
				    array);
}


void
column_destroy (struct column_t_ *column)
{
    free (column->buffer);
    free (column);
};
