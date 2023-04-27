/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _HASH_
#define _HASH_

#define SUCCESS 0
#define FAILURE 1

#include <sys/types.h>

#include "my_alloc.h"
#include "my_sys.h"

typedef struct _entry {
  const char *str;
  struct _entry *pNext;
} entry;

typedef struct bucket {
  uint h; /* Used for numeric indexing */
  const char *arKey;
  uint nKeyLength;
  uint count;
  entry *pData;
  struct bucket *pNext;
} Bucket;

typedef struct hashtable {
  uint nTableSize;
  uint initialized;
  MEM_ROOT mem_root;
  uint (*pHashFunction)(const char *arKey, uint nKeyLength);
  Bucket **arBuckets;
} HashTable;

extern int completion_hash_init(HashTable *ht, uint nSize);
extern int completion_hash_update(HashTable *ht, const char *arKey,
                                  uint nKeyLength, const char *str);
extern int hash_exists(HashTable *ht, char *arKey);
extern Bucket *find_all_matches(HashTable *ht, const char *str, uint length,
                                uint *res_length);
extern Bucket *find_longest_match(HashTable *ht, char *str, uint length,
                                  uint *res_length);
extern void add_word(HashTable *ht, const char *str);
extern void completion_hash_clean(HashTable *ht);
extern int completion_hash_exists(HashTable *ht, char *arKey, uint nKeyLength);
extern void completion_hash_free(HashTable *ht);

#endif /* _HASH_ */
