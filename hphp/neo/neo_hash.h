/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#ifndef incl_HPHP_NEO_HASH_H_
#define incl_HPHP_NEO_HASH_H_ 1

__BEGIN_DECLS

#include <stdlib.h>
#include "hphp/neo/neo_misc.h"

typedef UINT32 (*NE_HASH_FUNC)(const void *);
typedef int (*NE_COMP_FUNC)(const void *, const void *);

typedef struct _NE_HASHNODE
{
  void *key;
  void *value;
  UINT32 hashv;
  struct _NE_HASHNODE *next;
} NE_HASHNODE;

typedef struct _HASH
{
  UINT32 size;
  UINT32 num;

  NE_HASHNODE **nodes;
  NE_HASH_FUNC hash_func;
  NE_COMP_FUNC comp_func;
} NE_HASH;

NEOERR *ne_hash_init (NE_HASH **hash, NE_HASH_FUNC hash_func, NE_COMP_FUNC comp_func);
void ne_hash_destroy (NE_HASH **hash);
NEOERR *ne_hash_insert(NE_HASH *hash, void *key, void *value);
void *ne_hash_lookup(NE_HASH *hash, void *key);
int ne_hash_has_key(NE_HASH *hash, void *key);
void *ne_hash_remove(NE_HASH *hash, void *key);
void *ne_hash_next(NE_HASH *hash, void **key);

int ne_hash_str_comp(const void *a, const void *b);
UINT32 ne_hash_str_hash(const void *a);

int ne_hash_int_comp(const void *a, const void *b);
UINT32 ne_hash_int_hash(const void *a);

__END_DECLS

#endif /* incl_HPHP_NEO_HASH_H_ */
