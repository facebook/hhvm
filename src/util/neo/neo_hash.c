/*
 * Copyright 2003-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#include "cs_config.h"

#include <stdlib.h>
#include <string.h>

#include "neo_misc.h"
#include "neo_err.h"
#include "neo_hash.h"

static NEOERR *_hash_resize(NE_HASH *hash);
static NE_HASHNODE **_hash_lookup_node (NE_HASH *hash, void *key, UINT32 *hashv);

NEOERR *ne_hash_init (NE_HASH **hash, NE_HASH_FUNC hash_func, NE_COMP_FUNC comp_func)
{
  NE_HASH *my_hash = NULL;

  my_hash = (NE_HASH *) calloc(1, sizeof(NE_HASH));
  if (my_hash == NULL)
    return nerr_raise(NERR_NOMEM, "Unable to allocate memory for NE_HASH");

  my_hash->size = 256;
  my_hash->num = 0;
  my_hash->hash_func = hash_func;
  my_hash->comp_func = comp_func;

  my_hash->nodes = (NE_HASHNODE **) calloc (my_hash->size, sizeof(NE_HASHNODE *));
  if (my_hash->nodes == NULL)
  {
    free(my_hash);
    return nerr_raise(NERR_NOMEM, "Unable to allocate memory for NE_HASHNODES");
  }

  *hash = my_hash;

  return STATUS_OK;
}

void ne_hash_destroy (NE_HASH **hash)
{
  NE_HASH *my_hash;
  NE_HASHNODE *node, *next;
  int x;

  if (hash == NULL || *hash == NULL)
    return;

  my_hash = *hash;

  for (x = 0; x < my_hash->size; x++)
  {
    node = my_hash->nodes[x];
    while (node)
    {
      next = node->next;
      free(node);
      node = next;
    }
  }
  free(my_hash->nodes);
  my_hash->nodes = NULL;
  free(my_hash);
  *hash = NULL;
}

NEOERR *ne_hash_insert(NE_HASH *hash, void *key, void *value)
{
  UINT32 hashv;
  NE_HASHNODE **node;

  node = _hash_lookup_node(hash, key, &hashv);

  if (*node)
  {
    (*node)->value = value;
  }
  else
  {
    *node = (NE_HASHNODE *) malloc(sizeof(NE_HASHNODE));
    if (node == NULL)
      return nerr_raise(NERR_NOMEM, "Unable to allocate NE_HASHNODE");

    (*node)->hashv = hashv;
    (*node)->key = key;
    (*node)->value = value;
    (*node)->next = NULL;
  }
  hash->num++;

  return _hash_resize(hash);
}

void *ne_hash_lookup(NE_HASH *hash, void *key)
{
  NE_HASHNODE *node;

  node = *_hash_lookup_node(hash, key, NULL);

  return (node) ? node->value : NULL;
}

void *ne_hash_remove(NE_HASH *hash, void *key)
{
  NE_HASHNODE **node, *rem;
  void *value = NULL;

  node = _hash_lookup_node(hash, key, NULL);
  if (*node)
  {
    rem = *node;
    *node = rem->next;
    value = rem->value;
    free(rem);
    hash->num--;
  }
  return value;
}

int ne_hash_has_key(NE_HASH *hash, void *key)
{
  NE_HASHNODE *node;

  node = *_hash_lookup_node(hash, key, NULL);

  if (node) return 1;
  return 0;
}

void *ne_hash_next(NE_HASH *hash, void **key)
{
  NE_HASHNODE **node = 0;
  UINT32 hashv, bucket;

  if (*key)
  {
    node = _hash_lookup_node(hash, key, NULL);

    if (*node)
    {
      bucket = (*node)->hashv & (hash->size - 1);
    }
    else
    {
      hashv = hash->hash_func(*key);
      bucket = hashv & (hash->size - 1);
    }
  }
  else
  {
    bucket = 0;
  }

  if (*node)
  {
    if ((*node)->next)
    {
      *key = (*node)->next->key;
      return (*node)->next->value;
    }
    bucket++;
  }

  while (bucket < hash->size)
  {
    if (hash->nodes[bucket])
    {
      *key = hash->nodes[bucket]->key;
      return hash->nodes[bucket]->value;
    }
    bucket++;
  }

  return NULL;
}

static NE_HASHNODE **_hash_lookup_node (NE_HASH *hash, void *key, UINT32 *o_hashv)
{
  UINT32 hashv, bucket;
  NE_HASHNODE **node;

  hashv = hash->hash_func(key);
  if (o_hashv) *o_hashv = hashv;
  bucket = hashv & (hash->size - 1);
  /* ne_warn("Lookup %s %d %d", key, hashv, bucket); */

  node = &(hash->nodes[bucket]);

  if (hash->comp_func)
  {
    while (*node && !(hash->comp_func((*node)->key, key)))
      node = &(*node)->next;
  }
  else
  {
    /* No comp_func means we're doing pointer comparisons */
    while (*node && (*node)->key != key)
      node = &(*node)->next;
  }

  /* ne_warn("Node %x", node); */
  return node;
}

/* Ok, we're doing some weirdness here... */
static NEOERR *_hash_resize(NE_HASH *hash)
{
  NE_HASHNODE **new_nodes;
  NE_HASHNODE *entry, *prev;
  int x, next_bucket;
  int orig_size = hash->size;
  UINT32 hash_mask;

  if (hash->size > hash->num)
    return STATUS_OK;

  /* We always double in size */
  new_nodes = (NE_HASHNODE **) realloc (hash->nodes, (hash->size*2) * sizeof(NE_HASHNODE));
  if (new_nodes == NULL)
    return nerr_raise(NERR_NOMEM, "Unable to allocate memory to resize NE_HASH");

  hash->nodes = new_nodes;
  orig_size = hash->size;
  hash->size = hash->size*2;

  /* Initialize new parts */
  for (x = orig_size; x < hash->size; x++)
  {
    hash->nodes[x] = NULL;
  }

  hash_mask = hash->size - 1;

  for (x = 0; x < orig_size; x++)
  {
    prev = NULL;
    next_bucket = x + orig_size;
    for (entry = hash->nodes[x];
	 entry;
	 entry = prev ? prev->next : hash->nodes[x])
    {
      if ((entry->hashv & hash_mask) != x)
      {
	if (prev)
	{
	  prev->next = entry->next;
	}
	else
	{
	  hash->nodes[x] = entry->next;
	}
	entry->next = hash->nodes[next_bucket];
	hash->nodes[next_bucket] = entry;
      }
      else
      {
	prev = entry;
      }
    }
  }

  return STATUS_OK;
}

int ne_hash_str_comp(const void *a, const void *b)
{
  return !strcmp((const char *)a, (const char *)b);
}

UINT32 ne_hash_str_hash(const void *a)
{
  return ne_crc((unsigned char *)a, strlen((const char *)a));
}

int ne_hash_int_comp(const void *a, const void *b)
{
  if (a == b) return 1;
  return 0;
}

UINT32 ne_hash_int_hash(const void *a)
{
  return (UINT32)(long)(a);
}
