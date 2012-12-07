// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include"varint.h"


//read a number which we know is 1 byte
//no checks are done

inline uint64_t
varint_sread (uint8_t * buffer, int position)
{
    return (uint64_t) buffer[position] ;
}

//this is a read for big numbers
uint64_t
varint_bread (uint8_t * buffer, int position, uint8_t * size)
{
    const uint8_t *ptr = buffer + position;
    uint32_t b;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = 0, part1 = 0, part2 = 0;

    b = *(ptr++);
    part0 = (b & 0x7F);
    if (!(b & 0x80)) {
        *size = 1;
        goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
        *size = 2;
        goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 14;
    if (!(b & 0x80)) {
        *size = 3;
        goto done;
    }
    b = *(ptr++);
    part0 |= (b & 0x7F) << 21;
    if (!(b & 0x80)) {
        *size = 4;
        goto done;
    }
    b = *(ptr++);
    part1 = (b & 0x7F);
    if (!(b & 0x80)) {
        *size = 5;
        goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
        *size = 6;
        goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 14;
    if (!(b & 0x80)) {
        *size = 7;
        goto done;
    }
    b = *(ptr++);
    part1 |= (b & 0x7F) << 21;
    if (!(b & 0x80)) {
        *size = 8;
        goto done;
    }
    b = *(ptr++);
    part2 = (b & 0x7F);
    if (!(b & 0x80)) {
        *size = 9;
        goto done;
    }
    b = *(ptr++);
    part2 |= (b & 0x7F) << 7;
    if (!(b & 0x80)) {
        *size = 10;
        goto done;
    }

  done:

    return ((uint64_t) (part0)) |
        ((uint64_t) (part1) << 28) | ((uint64_t) (part2) << 56);


}

uint64_t
varint_read (uint8_t * buffer, int position, uint8_t * size)
{
    if (buffer[position] < 0x80) {
        *size = 1;
        return buffer[position];
    }
    else {
        return varint_bread (buffer, position, size);
    }
}

int
varint_write (uint8_t * buffer, uint64_t position, uint64_t value)
{

    uint8_t *target = buffer + position;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    uint32_t part0 = (uint32_t) (value);
    uint32_t part1 = (uint32_t) (value >> 28);
    uint32_t part2 = (uint32_t) (value >> 56);

    int size;

    // Here we can't really optimize for small numbers, since the value is
    // split into three parts.  Cheking for numbers < 128, for instance,
    // would require three comparisons, since you'd have to make sure part1
    // and part2 are zero.  However, if the caller is using 64-bit integers,
    // it is likely that they expect the numbers to often be very large, so
    // we probably don't want to optimize for small numbers anyway.  Thus,
    // we end up with a hardcoded binary search tree...
    if (part2 == 0) {
        if (part1 == 0) {
            if (part0 < (1 << 14)) {
                if (part0 < (1 << 7)) {
                    size = 1;
                    goto size1;
                }
                else {
                    size = 2;
                    goto size2;
                }
            }
            else {
                if (part0 < (1 << 21)) {
                    size = 3;
                    goto size3;
                }
                else {
                    size = 4;
                    goto size4;
                }
            }
        }
        else {
            if (part1 < (1 << 14)) {
                if (part1 < (1 << 7)) {
                    size = 5;
                    goto size5;
                }
                else {
                    size = 6;
                    goto size6;
                }
            }
            else {
                if (part1 < (1 << 21)) {
                    size = 7;
                    goto size7;
                }
                else {
                    size = 8;
                    goto size8;
                }
            }
        }
    }
    else {
        if (part2 < (1 << 7)) {
            size = 9;
            goto size9;
        }
        else {
            size = 10;
            goto size10;
        }
    }

    assert (1);

  size10:target[9] = (uint8_t) ((part2 >> 7) | 0x80);
  size9:target[8] = (uint8_t) ((part2) | 0x80);
  size8:target[7] = (uint8_t) ((part1 >> 21) | 0x80);
  size7:target[6] = (uint8_t) ((part1 >> 14) | 0x80);
  size6:target[5] = (uint8_t) ((part1 >> 7) | 0x80);
  size5:target[4] = (uint8_t) ((part1) | 0x80);
  size4:target[3] = (uint8_t) ((part0 >> 21) | 0x80);
  size3:target[2] = (uint8_t) ((part0 >> 14) | 0x80);
  size2:target[1] = (uint8_t) ((part0 >> 7) | 0x80);
  size1:target[0] = (uint8_t) ((part0) | 0x80);

    target[size - 1] &= 0x7F;

    return size;

}
