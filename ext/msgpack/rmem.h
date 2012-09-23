/*
 * MessagePack for Ruby
 *
 * Copyright (C) 2008-2012 FURUHASHI Sadayuki
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef MSGPACK_RUBY_RMEM_H__
#define MSGPACK_RUBY_RMEM_H__

#include "compat.h"
#include "sysdep.h"

#ifndef MSGPACK_RMEM_PAGE_SIZE
#define MSGPACK_RMEM_PAGE_SIZE (4*1024)
#endif

struct msgpack_rmem_t;
typedef struct msgpack_rmem_t msgpack_rmem_t;

struct msgpack_rmem_chunk_t;
typedef struct msgpack_rmem_chunk_t msgpack_rmem_chunk_t;

/*
 * a chunk contains 32 pages.
 * size of each buffer is MSGPACK_RMEM_PAGE_SIZE bytes.
 */
struct msgpack_rmem_chunk_t {
    unsigned int mask;
    char* pages;
    msgpack_rmem_chunk_t* next;
};

struct msgpack_rmem_t {
    msgpack_rmem_chunk_t head;
};

/* assert MSGPACK_RMEM_PAGE_SIZE % sysconf(_SC_PAGE_SIZE) == 0 */
void msgpack_rmem_init(msgpack_rmem_t* pm);

void msgpack_rmem_destroy(msgpack_rmem_t* pm);

void* _msgpack_rmem_alloc2(msgpack_rmem_t* pm);

#define _msgpack_rmem_chunk_try_alloc(c) \
    if((c)->mask != 0) { \
        _msgpack_bsp32(pos, (c)->mask); \
        (c)->mask &= ~(1 << pos); \
        return ((char*)(c)->pages) + (pos * (MSGPACK_RMEM_PAGE_SIZE)); \
    }

#define _msgpack_rmem_chunk_try_check(c, mem) \
    { \
        ptrdiff_t pdiff = ((char*)(mem)) - ((char*)(c)->pages); \
        if(0 <= pdiff \
                && pdiff % MSGPACK_RMEM_PAGE_SIZE == 0 \
                && pdiff < MSGPACK_RMEM_PAGE_SIZE * 32) { \
            return true; \
        } \
    }

#define _msgpack_rmem_chunk_try_free(c, mem) \
    { \
        ptrdiff_t pdiff = ((char*)(mem)) - ((char*)(c)->pages); \
        if(0 <= pdiff \
                && pdiff % MSGPACK_RMEM_PAGE_SIZE == 0 \
                && pdiff < MSGPACK_RMEM_PAGE_SIZE * 32) { \
            size_t pos = pdiff / MSGPACK_RMEM_PAGE_SIZE; \
            (c)->mask |= (1 << pos); \
            return true; \
        } \
    }

static inline void* msgpack_rmem_alloc(msgpack_rmem_t* pm)
{
    _msgpack_rmem_chunk_try_alloc(&pm->head);
    return _msgpack_rmem_alloc2(pm);
}

bool _msgpack_rmem_free2(msgpack_rmem_t* pm, void* mem);

static inline bool msgpack_rmem_free(msgpack_rmem_t* pm, void* mem)
{
    _msgpack_rmem_chunk_try_free(&pm->head, mem);
    return _msgpack_rmem_free2(pm, mem);
}

bool msgpack_rmem_check(msgpack_rmem_t* pm, void* mem);


#endif
