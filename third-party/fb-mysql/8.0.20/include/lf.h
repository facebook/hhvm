/* Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef _lf_h
#define _lf_h

/**
  @file include/lf.h
*/

#include "my_config.h"

#include <stddef.h>
#include <sys/types.h>

#include <atomic>

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/service_mysql_alloc.h"
#include "sql_string.h"

/*
  wait-free dynamic array, see lf_dynarray.c

  4 levels of 256 elements each mean 4311810304 elements in an array - it
  should be enough for a while
*/
#define LF_DYNARRAY_LEVEL_LENGTH 256
#define LF_DYNARRAY_LEVELS 4

struct LF_DYNARRAY {
  std::atomic<void *> level[LF_DYNARRAY_LEVELS];
  uint size_of_element;
};

typedef int (*lf_dynarray_func)(void *, void *);

void lf_dynarray_init(LF_DYNARRAY *array, uint element_size);
void lf_dynarray_destroy(LF_DYNARRAY *array);
void *lf_dynarray_value(LF_DYNARRAY *array, uint idx);
void *lf_dynarray_lvalue(LF_DYNARRAY *array, uint idx);
int lf_dynarray_iterate(LF_DYNARRAY *array, lf_dynarray_func func, void *arg);

/*
  pin manager for memory allocator, lf_alloc-pin.c
*/

#define LF_PINBOX_PINS 4
#define LF_PURGATORY_SIZE 10

typedef void lf_pinbox_free_func(void *, void *, void *);

struct LF_PINBOX {
  LF_DYNARRAY pinarray;
  lf_pinbox_free_func *free_func;
  void *free_func_arg;
  uint free_ptr_offset;
  std::atomic<uint32> pinstack_top_ver; /* this is a versioned pointer */
  std::atomic<uint32> pins_in_array;    /* number of elements in array */
};

struct LF_PINS {
  std::atomic<void *> pin[LF_PINBOX_PINS];
  LF_PINBOX *pinbox;
  void *purgatory;
  uint32 purgatory_count;
  std::atomic<uint32> link;
  /* we want sizeof(LF_PINS) to be 64 to avoid false sharing */
#if SIZEOF_INT * 2 + SIZEOF_CHARP * (LF_PINBOX_PINS + 2) != 64
  char pad[64 - sizeof(uint32) * 2 - sizeof(void *) * (LF_PINBOX_PINS + 2)];
#endif
};

/*
  compile-time assert, to require "no less than N" pins
  it's enough if it'll fail on at least one compiler, so
  we'll enable it on GCC only, which supports zero-length arrays.
*/
#if defined(__GNUC__) && defined(MY_LF_EXTRA_DEBUG)
#define LF_REQUIRE_PINS(N)                                                   \
  static const char require_pins[LF_PINBOX_PINS - N] MY_ATTRIBUTE((unused)); \
  static const int LF_NUM_PINS_IN_THIS_FILE = N;
#else
#define LF_REQUIRE_PINS(N)
#endif

static inline void lf_pin(LF_PINS *pins, int pin, void *addr) {
#if defined(__GNUC__) && defined(MY_LF_EXTRA_DEBUG)
  assert(pin < LF_NUM_PINS_IN_THIS_FILE);
#endif
  pins->pin[pin].store(addr);
}

static inline void lf_unpin(LF_PINS *pins, int pin) {
#if defined(__GNUC__) && defined(MY_LF_EXTRA_DEBUG)
  assert(pin < LF_NUM_PINS_IN_THIS_FILE);
#endif
  pins->pin[pin].store(nullptr);
}

void lf_pinbox_init(LF_PINBOX *pinbox, uint free_ptr_offset,
                    lf_pinbox_free_func *free_func, void *free_func_arg);
void lf_pinbox_destroy(LF_PINBOX *pinbox);
LF_PINS *lf_pinbox_get_pins(LF_PINBOX *pinbox);
void lf_pinbox_put_pins(LF_PINS *pins);
void lf_pinbox_free(LF_PINS *pins, void *addr);

/*
  memory allocator, lf_alloc-pin.c
*/
typedef void lf_allocator_func(uchar *);

struct LF_ALLOCATOR {
  LF_PINBOX pinbox;
  std::atomic<uchar *> top;
  uint element_size;
  std::atomic<uint32> mallocs;
  lf_allocator_func *constructor; /* called, when an object is malloc()'ed */
  lf_allocator_func *destructor;  /* called, when an object is free()'d    */
};

#define lf_alloc_init(A, B, C) lf_alloc_init2(A, B, C, NULL, NULL)
void lf_alloc_init2(LF_ALLOCATOR *allocator, uint size, uint free_ptr_offset,
                    lf_allocator_func *ctor, lf_allocator_func *dtor);
void lf_alloc_destroy(LF_ALLOCATOR *allocator);
uint lf_alloc_pool_count(LF_ALLOCATOR *allocator);

static inline void lf_alloc_direct_free(LF_ALLOCATOR *allocator, void *addr) {
  if (allocator->destructor) {
    allocator->destructor((uchar *)addr);
  }
  my_free(addr);
}

void *lf_alloc_new(LF_PINS *pins);

struct LF_HASH;

typedef uint lf_hash_func(const LF_HASH *, const uchar *, size_t);
typedef void lf_hash_init_func(uchar *dst, const uchar *src);

#define LF_HASH_UNIQUE 1
#define MY_LF_ERRPTR ((void *)(intptr)1)

/* lf_hash overhead per element (that is, sizeof(LF_SLIST) */
extern MYSQL_PLUGIN_IMPORT const int LF_HASH_OVERHEAD;

/**
  Callback for extracting key and key length from user data in a LF_HASH.
  @param      arg    Pointer to user data.
  @param[out] length Store key length here.
  @return            Pointer to key to be hashed.

  @note Was my_hash_get_key, with lots of C-style casting when calling
        my_hash_init. Renamed to force build error (since signature changed)
        in case someone keeps following that coding style.
 */
typedef const uchar *(*hash_get_key_function)(const uchar *arg, size_t *length);

struct LF_HASH {
  LF_DYNARRAY array;             /* hash itself */
  LF_ALLOCATOR alloc;            /* allocator for elements */
  hash_get_key_function get_key; /* see HASH */
  CHARSET_INFO *charset;         /* see HASH */
  lf_hash_func *hash_function;   /* see HASH */
  uint key_offset, key_length;   /* see HASH */
  uint element_size;             /* size of memcpy'ed area on insert */
  uint flags;                    /* LF_HASH_UNIQUE, etc */
  std::atomic<int32> size;       /* size of array */
  std::atomic<int32> count;      /* number of elements in the hash */
  /**
    "Initialize" hook - called to finish initialization of object provided by
     LF_ALLOCATOR (which is pointed by "dst" parameter) and set element key
     from object passed as parameter to lf_hash_insert (pointed by "src"
     parameter). Allows to use LF_HASH with objects which are not "trivially
     copyable".
     NULL value means that element initialization is carried out by copying
     first element_size bytes from object which provided as parameter to
     lf_hash_insert.
  */
  lf_hash_init_func *initialize;
};

#define lf_hash_init(A, B, C, D, E, F, G) \
  lf_hash_init2(A, B, C, D, E, F, G, NULL, NULL, NULL, NULL)
void lf_hash_init2(LF_HASH *hash, uint element_size, uint flags,
                   uint key_offset, uint key_length,
                   hash_get_key_function get_key, CHARSET_INFO *charset,
                   lf_hash_func *hash_function, lf_allocator_func *ctor,
                   lf_allocator_func *dtor, lf_hash_init_func *init);
void lf_hash_destroy(LF_HASH *hash);
int lf_hash_insert(LF_HASH *hash, LF_PINS *pins, const void *data);
void *lf_hash_search(LF_HASH *hash, LF_PINS *pins, const void *key,
                     uint keylen);
int lf_hash_delete(LF_HASH *hash, LF_PINS *pins, const void *key, uint keylen);

static inline LF_PINS *lf_hash_get_pins(LF_HASH *hash) {
  return lf_pinbox_get_pins(&hash->alloc.pinbox);
}

static inline void lf_hash_put_pins(LF_PINS *pins) { lf_pinbox_put_pins(pins); }

static inline void lf_hash_search_unpin(LF_PINS *pins) { lf_unpin(pins, 2); }

typedef int lf_hash_match_func(const uchar *el);
void *lf_hash_random_match(LF_HASH *hash, LF_PINS *pins,
                           lf_hash_match_func *match, uint rand_val);

#endif
