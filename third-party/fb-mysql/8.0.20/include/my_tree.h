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

#ifndef _tree_h
#define _tree_h

/**
  @file include/my_tree.h
*/

#include <stddef.h>
#include <sys/types.h>

#include "my_alloc.h" /* MEM_ROOT */
#include "my_base.h"  /* get 'enum ha_rkey_function' */
#include "my_inttypes.h"
#include "my_sys.h" /* qsort2_cmp */

/* Worst case tree is half full. This gives use 2^(MAX_TREE_HEIGHT/2) leafs */
#define MAX_TREE_HEIGHT 64

#define ELEMENT_KEY(tree, element)                                        \
  (tree->offset_to_key ? (void *)((uchar *)element + tree->offset_to_key) \
                       : *((void **)(element + 1)))

#define tree_set_pointer(element, ptr) \
  *((uchar **)(element + 1)) = ((uchar *)(ptr))

#define TREE_NO_DUPS 1

typedef enum { left_root_right, right_root_left } TREE_WALK;
typedef uint32 element_count;
typedef int (*tree_walk_action)(void *, element_count, void *);

typedef enum { free_init, free_free, free_end } TREE_FREE;
typedef void (*tree_element_free)(void *, TREE_FREE, const void *);

struct TREE_ELEMENT {
  TREE_ELEMENT() : count(0), colour(0) {}

  TREE_ELEMENT *left{nullptr}, *right{nullptr};
  uint32 count : 31, colour : 1; /* black is marked as 1 */
};

#define ELEMENT_CHILD(element, offs) \
  (*(TREE_ELEMENT **)((char *)element + offs))

struct TREE {
  TREE_ELEMENT *root{nullptr}, null_element;
  TREE_ELEMENT **parents[MAX_TREE_HEIGHT]{nullptr};
  uint offset_to_key{0}, elements_in_tree{0}, size_of_element{0};
  ulong memory_limit{0}, allocated{0};
  qsort2_cmp compare{nullptr};
  const void *custom_arg{nullptr};
  MEM_ROOT mem_root;
  bool with_delete{false};
  tree_element_free free{nullptr};
  uint flag{0};
};

/* Functions on whole tree */
void init_tree(TREE *tree, ulong memory_limit, int element_size,
               qsort2_cmp compare, bool with_delete,
               tree_element_free free_element, const void *custom_arg);
void delete_tree(TREE *);
void reset_tree(TREE *);
/* similar to delete tree, except we do not my_free() blocks in mem_root
 */
#define is_tree_inited(tree) ((tree)->root != 0)

/* Functions on leafs */
TREE_ELEMENT *tree_insert(TREE *tree, void *key, uint key_size,
                          const void *custom_arg);
void *tree_search(TREE *tree, void *key, const void *custom_arg);
int tree_walk(TREE *tree, tree_walk_action action, void *argument,
              TREE_WALK visit);
int tree_delete(TREE *tree, void *key, uint key_size, const void *custom_arg);
void *tree_search_key(TREE *tree, const void *key, TREE_ELEMENT **parents,
                      TREE_ELEMENT ***last_pos, enum ha_rkey_function flag,
                      const void *custom_arg);
void *tree_search_edge(TREE *tree, TREE_ELEMENT **parents,
                       TREE_ELEMENT ***last_pos, int child_offs);
void *tree_search_next(TREE *tree, TREE_ELEMENT ***last_pos, int l_offs,
                       int r_offs);
ha_rows tree_record_pos(TREE *tree, const void *key,
                        enum ha_rkey_function search_flag,
                        const void *custom_arg);

#define TREE_ELEMENT_EXTRA_SIZE (sizeof(TREE_ELEMENT) + sizeof(void *))

#endif
