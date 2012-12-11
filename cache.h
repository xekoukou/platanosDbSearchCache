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


#ifndef PLATANOS_DB_SEARCH_CACHE_H_
#define PLATANOS_DB_SEARCH_CACHE_H_


#include"column.h"
#include"hash/khash.h"
#include<stdlib.h>
KHASH_MAP_INIT_INT64 (columns, intersection_t);

//the uid of the person is used to insert the structure into the hash
KHASH_MAP_INIT_INT64 (persons, khash_t(columns));


#endif
