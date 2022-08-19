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

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/tree.cc
  Code for handling red-black (balanced) binary trees.
  key in tree is allocated according to following:

  1) If size < 0 then tree will not allocate keys and only a pointer to
     each key is saved in tree.
     compare and search functions uses and returns key-pointer

  2) If size == 0 then there are two options:
       - key_size != 0 to tree_insert: The key will be stored in the tree.
       - key_size == 0 to tree_insert:  A pointer to the key is stored.
     compare and search functions uses and returns key-pointer.

  3) if key_size is given to init_tree then each node will continue the
     key and calls to insert_key may increase length of key.
     if key_size > sizeof(pointer) and key_size is a multiple of 8 (double
     align) then key will be put on a 8 aligned address. Else
     the key will be on address (element+1). This is transparent for user
     compare and search functions uses a pointer to given key-argument.

  - If you use a free function for tree-elements and you are freeing
    the element itself, you should use key_size = 0 to init_tree and
    tree_search

  The actual key in TREE_ELEMENT is saved as a pointer or after the
  TREE_ELEMENT struct.
  If one uses only pointers in tree one can use tree_set_pointer() to
  change address of data.
*/

/*
  NOTE:
  tree->compare function should be ALWAYS called as
    (*tree->compare)(custom_arg, ELEMENT_KEY(tree,element), key)
  and not other way around, as
    (*tree->compare)(custom_arg, key, ELEMENT_KEY(tree,element))

  ft_boolean_search.c (at least) relies on that.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "my_tree.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"

#define BLACK 1
#define RED 0
#define DEFAULT_ALLOC_SIZE 8192

static void delete_tree_element(TREE *, TREE_ELEMENT *);
static int tree_walk_left_root_right(TREE *, TREE_ELEMENT *, tree_walk_action,
                                     void *);
static int tree_walk_right_root_left(TREE *, TREE_ELEMENT *, tree_walk_action,
                                     void *);
static void left_rotate(TREE_ELEMENT **parent, TREE_ELEMENT *leaf);
static void right_rotate(TREE_ELEMENT **parent, TREE_ELEMENT *leaf);
static void rb_insert(TREE *tree, TREE_ELEMENT ***parent, TREE_ELEMENT *leaf);
static void rb_delete_fixup(TREE *tree, TREE_ELEMENT ***parent);

/* The actuall code for handling binary trees */

#ifndef DBUG_OFF
static int test_rb_tree(TREE_ELEMENT *element);
#endif

void init_tree(TREE *tree, ulong memory_limit, int element_size,
               qsort2_cmp compare, bool with_delete,
               tree_element_free free_element, const void *custom_arg) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("tree: %p  element_size: %d", tree, element_size));

  new (&tree->null_element) TREE_ELEMENT();
  tree->root = &tree->null_element;
  tree->compare = compare;
  tree->size_of_element =
      element_size > 0 ? static_cast<uint>(element_size) : 0;
  tree->memory_limit = memory_limit;
  tree->free = free_element;
  tree->allocated = 0;
  tree->elements_in_tree = 0;
  tree->custom_arg = custom_arg;
  tree->null_element.colour = BLACK;
  tree->null_element.left = tree->null_element.right = nullptr;
  tree->flag = 0;
  if (!free_element && element_size >= 0 &&
      (static_cast<uint>(element_size) <= sizeof(void *) ||
       (static_cast<uint>(element_size) & (sizeof(void *) - 1)))) {
    /*
      We know that the data doesn't have to be aligned (like if the key
      contains a double), so we can store the data combined with the
      TREE_ELEMENT.
    */
    tree->offset_to_key = sizeof(TREE_ELEMENT); /* Put key after element */
  } else {
    tree->offset_to_key = 0; /* use key through pointer */
    tree->size_of_element += sizeof(void *);
  }
  if (!(tree->with_delete = with_delete)) {
    init_alloc_root(key_memory_TREE, &tree->mem_root, DEFAULT_ALLOC_SIZE, 0);
  }
}

static void free_tree(TREE *tree, myf free_flags) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("tree: %p", tree));

  if (tree->root) /* If initialized */
  {
    if (tree->with_delete)
      delete_tree_element(tree, tree->root);
    else {
      if (tree->free) {
        if (tree->memory_limit)
          (*tree->free)(nullptr, free_init, tree->custom_arg);
        delete_tree_element(tree, tree->root);
        if (tree->memory_limit)
          (*tree->free)(nullptr, free_end, tree->custom_arg);
      }
      free_root(&tree->mem_root, free_flags);
    }
  }
  tree->root = &tree->null_element;
  tree->elements_in_tree = 0;
  tree->allocated = 0;
}

void delete_tree(TREE *tree) {
  free_tree(tree, MYF(0)); /* my_free() mem_root if applicable */
}

void reset_tree(TREE *tree) {
  /* do not free mem_root, just mark blocks as free */
  free_tree(tree, MYF(MY_MARK_BLOCKS_FREE));
}

static void delete_tree_element(TREE *tree, TREE_ELEMENT *element) {
  if (element != &tree->null_element) {
    delete_tree_element(tree, element->left);
    if (tree->free)
      (*tree->free)(ELEMENT_KEY(tree, element), free_free, tree->custom_arg);
    delete_tree_element(tree, element->right);
    if (tree->with_delete) my_free(element);
  }
}

/*
  insert, search and delete of elements

  The following should be true:
    parent[0] = & parent[-1][0]->left ||
    parent[0] = & parent[-1][0]->right

  @returns
    NULL     OOM or duplicate
    non-null inserted element
*/

TREE_ELEMENT *tree_insert(TREE *tree, void *key, uint key_size,
                          const void *custom_arg) {
  int cmp;
  TREE_ELEMENT *element, ***parent;

  parent = tree->parents;
  *parent = &tree->root;
  element = tree->root;
  for (;;) {
    if (element == &tree->null_element ||
        (cmp = (*tree->compare)(custom_arg, ELEMENT_KEY(tree, element), key)) ==
            0)
      break;
    if (cmp < 0) {
      *++parent = &element->right;
      element = element->right;
    } else {
      *++parent = &element->left;
      element = element->left;
    }
  }
  if (element == &tree->null_element) {
    uint alloc_size = sizeof(TREE_ELEMENT) + key_size + tree->size_of_element;
    tree->allocated += alloc_size;

    if (tree->memory_limit && tree->elements_in_tree &&
        tree->allocated > tree->memory_limit) {
      reset_tree(tree);
      return tree_insert(tree, key, key_size, custom_arg);
    }

    key_size += tree->size_of_element;
    if (tree->with_delete)
      element =
          (TREE_ELEMENT *)my_malloc(key_memory_TREE, alloc_size, MYF(MY_WME));
    else
      element = (TREE_ELEMENT *)tree->mem_root.Alloc(alloc_size);
    if (!element) return (nullptr);
    **parent = element;
    element->left = element->right = &tree->null_element;
    if (!tree->offset_to_key) {
      if (key_size == sizeof(void *)) /* no length, save pointer */
        *((void **)(element + 1)) = key;
      else {
        *((void **)(element + 1)) = (void *)((void **)(element + 1) + 1);
        memcpy((uchar *)*((void **)(element + 1)), key,
               (size_t)(key_size - sizeof(void *)));
      }
    } else
      memcpy((uchar *)element + tree->offset_to_key, key, (size_t)key_size);
    element->count = 1;
    tree->elements_in_tree++;
    rb_insert(tree, parent, element); /* rebalance tree */
  } else {
    if (tree->flag & TREE_NO_DUPS) return (nullptr);
    element->count++;
    /* Avoid a wrap over of the count. */
    if (!element->count) element->count--;
  }
  DBUG_EXECUTE("check_tree", test_rb_tree(tree->root););
  return element;
}

int tree_delete(TREE *tree, void *key, uint key_size, const void *custom_arg) {
  int cmp, remove_colour;
  TREE_ELEMENT *element, ***parent, ***org_parent, *nod;
  if (!tree->with_delete) return 1; /* not allowed */

  parent = tree->parents;
  *parent = &tree->root;
  element = tree->root;
  for (;;) {
    if (element == &tree->null_element) return 1; /* Was not in tree */
    if ((cmp = (*tree->compare)(custom_arg, ELEMENT_KEY(tree, element), key)) ==
        0)
      break;
    if (cmp < 0) {
      *++parent = &element->right;
      element = element->right;
    } else {
      *++parent = &element->left;
      element = element->left;
    }
  }
  if (element->left == &tree->null_element) {
    (**parent) = element->right;
    remove_colour = element->colour;
  } else if (element->right == &tree->null_element) {
    (**parent) = element->left;
    remove_colour = element->colour;
  } else {
    org_parent = parent;
    *++parent = &element->right;
    nod = element->right;
    while (nod->left != &tree->null_element) {
      *++parent = &nod->left;
      nod = nod->left;
    }
    (**parent) = nod->right; /* unlink nod from tree */
    remove_colour = nod->colour;
    org_parent[0][0] = nod; /* put y in place of element */
    org_parent[1] = &nod->right;
    nod->left = element->left;
    nod->right = element->right;
    nod->colour = element->colour;
  }
  if (remove_colour == BLACK) rb_delete_fixup(tree, parent);
  if (tree->free)
    (*tree->free)(ELEMENT_KEY(tree, element), free_free, tree->custom_arg);
  tree->allocated -= sizeof(TREE_ELEMENT) + tree->size_of_element + key_size;
  my_free(element);
  tree->elements_in_tree--;
  return 0;
}

void *tree_search(TREE *tree, void *key, const void *custom_arg) {
  int cmp;
  TREE_ELEMENT *element = tree->root;

  for (;;) {
    if (element == &tree->null_element) return (void *)nullptr;
    if ((cmp = (*tree->compare)(custom_arg, ELEMENT_KEY(tree, element), key)) ==
        0)
      return ELEMENT_KEY(tree, element);
    if (cmp < 0)
      element = element->right;
    else
      element = element->left;
  }
}

void *tree_search_key(TREE *tree, const void *key, TREE_ELEMENT **parents,
                      TREE_ELEMENT ***last_pos, enum ha_rkey_function flag,
                      const void *custom_arg) {
  int cmp;
  TREE_ELEMENT *element = tree->root;
  TREE_ELEMENT **last_left_step_parent = nullptr,
               **last_right_step_parent = nullptr;
  TREE_ELEMENT **last_equal_element = nullptr;

  /*
    TODO: support for HA_READ_KEY_OR_PREV, HA_READ_PREFIX flags if needed.
  */

  *parents = &tree->null_element;
  while (element != &tree->null_element) {
    *++parents = element;
    if ((cmp = (*tree->compare)(custom_arg, ELEMENT_KEY(tree, element), key)) ==
        0) {
      switch (flag) {
        case HA_READ_KEY_EXACT:
        case HA_READ_KEY_OR_NEXT:
        case HA_READ_BEFORE_KEY:
          last_equal_element = parents;
          cmp = 1;
          break;
        case HA_READ_AFTER_KEY:
          cmp = -1;
          break;
        case HA_READ_PREFIX_LAST:
        case HA_READ_PREFIX_LAST_OR_PREV:
          last_equal_element = parents;
          cmp = -1;
          break;
        default:
          return nullptr;
      }
    }
    if (cmp < 0) /* element < key */
    {
      last_right_step_parent = parents;
      element = element->right;
    } else {
      last_left_step_parent = parents;
      element = element->left;
    }
  }
  switch (flag) {
    case HA_READ_KEY_EXACT:
    case HA_READ_PREFIX_LAST:
      *last_pos = last_equal_element;
      break;
    case HA_READ_KEY_OR_NEXT:
      *last_pos =
          last_equal_element ? last_equal_element : last_left_step_parent;
      break;
    case HA_READ_AFTER_KEY:
      *last_pos = last_left_step_parent;
      break;
    case HA_READ_PREFIX_LAST_OR_PREV:
      *last_pos =
          last_equal_element ? last_equal_element : last_right_step_parent;
      break;
    case HA_READ_BEFORE_KEY:
      *last_pos = last_right_step_parent;
      break;
    default:
      return nullptr;
  }
  return *last_pos ? ELEMENT_KEY(tree, **last_pos) : nullptr;
}

/*
  Search first (the most left) or last (the most right) tree element
*/
void *tree_search_edge(TREE *tree, TREE_ELEMENT **parents,
                       TREE_ELEMENT ***last_pos, int child_offs) {
  TREE_ELEMENT *element = tree->root;

  *parents = &tree->null_element;
  while (element != &tree->null_element) {
    *++parents = element;
    element = ELEMENT_CHILD(element, child_offs);
  }
  *last_pos = parents;
  return **last_pos != &tree->null_element ? ELEMENT_KEY(tree, **last_pos)
                                           : nullptr;
}

void *tree_search_next(TREE *tree, TREE_ELEMENT ***last_pos, int l_offs,
                       int r_offs) {
  TREE_ELEMENT *x = **last_pos;

  if (ELEMENT_CHILD(x, r_offs) != &tree->null_element) {
    x = ELEMENT_CHILD(x, r_offs);
    *++*last_pos = x;
    while (ELEMENT_CHILD(x, l_offs) != &tree->null_element) {
      x = ELEMENT_CHILD(x, l_offs);
      *++*last_pos = x;
    }
    return ELEMENT_KEY(tree, x);
  } else {
    TREE_ELEMENT *y = *--*last_pos;
    while (y != &tree->null_element && x == ELEMENT_CHILD(y, r_offs)) {
      x = y;
      y = *--*last_pos;
    }
    return y == &tree->null_element ? nullptr : ELEMENT_KEY(tree, y);
  }
}

/*
  Expected that tree is fully balanced
  (each path from root to leaf has the same length)
*/
ha_rows tree_record_pos(TREE *tree, const void *key, enum ha_rkey_function flag,
                        const void *custom_arg) {
  int cmp;
  TREE_ELEMENT *element = tree->root;
  double left = 1;
  double right = tree->elements_in_tree;

  while (element != &tree->null_element) {
    if ((cmp = (*tree->compare)(custom_arg, ELEMENT_KEY(tree, element), key)) ==
        0) {
      switch (flag) {
        case HA_READ_KEY_EXACT:
        case HA_READ_BEFORE_KEY:
          cmp = 1;
          break;
        case HA_READ_AFTER_KEY:
          cmp = -1;
          break;
        default:
          return HA_POS_ERROR;
      }
    }
    if (cmp < 0) /* element < key */
    {
      element = element->right;
      left = (left + right) / 2;
    } else {
      element = element->left;
      right = (left + right) / 2;
    }
  }
  switch (flag) {
    case HA_READ_KEY_EXACT:
    case HA_READ_BEFORE_KEY:
      return (ha_rows)right;
    case HA_READ_AFTER_KEY:
      return (ha_rows)left;
    default:
      return HA_POS_ERROR;
  }
}

int tree_walk(TREE *tree, tree_walk_action action, void *argument,
              TREE_WALK visit) {
  switch (visit) {
    case left_root_right:
      return tree_walk_left_root_right(tree, tree->root, action, argument);
    case right_root_left:
      return tree_walk_right_root_left(tree, tree->root, action, argument);
  }
  return 0; /* Keep gcc happy */
}

static int tree_walk_left_root_right(TREE *tree, TREE_ELEMENT *element,
                                     tree_walk_action action, void *argument) {
  int error;
  if (element->left) /* Not null_element */
  {
    if ((error = tree_walk_left_root_right(tree, element->left, action,
                                           argument)) == 0 &&
        (error = (*action)(ELEMENT_KEY(tree, element),
                           (element_count)element->count, argument)) == 0)
      error = tree_walk_left_root_right(tree, element->right, action, argument);
    return error;
  }
  return 0;
}

static int tree_walk_right_root_left(TREE *tree, TREE_ELEMENT *element,
                                     tree_walk_action action, void *argument) {
  int error;
  if (element->right) /* Not null_element */
  {
    if ((error = tree_walk_right_root_left(tree, element->right, action,
                                           argument)) == 0 &&
        (error = (*action)(ELEMENT_KEY(tree, element),
                           (element_count)element->count, argument)) == 0)
      error = tree_walk_right_root_left(tree, element->left, action, argument);
    return error;
  }
  return 0;
}

/* Functions to fix up the tree after insert and delete */

static void left_rotate(TREE_ELEMENT **parent, TREE_ELEMENT *leaf) {
  TREE_ELEMENT *y;

  y = leaf->right;
  leaf->right = y->left;
  parent[0] = y;
  y->left = leaf;
}

static void right_rotate(TREE_ELEMENT **parent, TREE_ELEMENT *leaf) {
  TREE_ELEMENT *x;

  x = leaf->left;
  leaf->left = x->right;
  parent[0] = x;
  x->right = leaf;
}

static void rb_insert(TREE *tree, TREE_ELEMENT ***parent, TREE_ELEMENT *leaf) {
  TREE_ELEMENT *y, *par, *par2;

  leaf->colour = RED;
  while (leaf != tree->root && (par = parent[-1][0])->colour == RED) {
    if (par == (par2 = parent[-2][0])->left) {
      y = par2->right;
      if (y->colour == RED) {
        par->colour = BLACK;
        y->colour = BLACK;
        leaf = par2;
        parent -= 2;
        leaf->colour = RED; /* And the loop continues */
      } else {
        if (leaf == par->right) {
          left_rotate(parent[-1], par);
          par = leaf; /* leaf is now parent to old leaf */
        }
        par->colour = BLACK;
        par2->colour = RED;
        right_rotate(parent[-2], par2);
        break;
      }
    } else {
      y = par2->left;
      if (y->colour == RED) {
        par->colour = BLACK;
        y->colour = BLACK;
        leaf = par2;
        parent -= 2;
        leaf->colour = RED; /* And the loop continues */
      } else {
        if (leaf == par->left) {
          right_rotate(parent[-1], par);
          par = leaf;
        }
        par->colour = BLACK;
        par2->colour = RED;
        left_rotate(parent[-2], par2);
        break;
      }
    }
  }
  tree->root->colour = BLACK;
}

static void rb_delete_fixup(TREE *tree, TREE_ELEMENT ***parent) {
  TREE_ELEMENT *x, *w, *par;

  x = **parent;
  while (x != tree->root && x->colour == BLACK) {
    if (x == (par = parent[-1][0])->left) {
      w = par->right;
      if (w->colour == RED) {
        w->colour = BLACK;
        par->colour = RED;
        left_rotate(parent[-1], par);
        parent[0] = &w->left;
        *++parent = &par->left;
        w = par->right;
      }
      if (w->left->colour == BLACK && w->right->colour == BLACK) {
        w->colour = RED;
        x = par;
        parent--;
      } else {
        if (w->right->colour == BLACK) {
          w->left->colour = BLACK;
          w->colour = RED;
          right_rotate(&par->right, w);
          w = par->right;
        }
        w->colour = par->colour;
        par->colour = BLACK;
        w->right->colour = BLACK;
        left_rotate(parent[-1], par);
        x = tree->root;
        break;
      }
    } else {
      w = par->left;
      if (w->colour == RED) {
        w->colour = BLACK;
        par->colour = RED;
        right_rotate(parent[-1], par);
        parent[0] = &w->right;
        *++parent = &par->right;
        w = par->left;
      }
      if (w->right->colour == BLACK && w->left->colour == BLACK) {
        w->colour = RED;
        x = par;
        parent--;
      } else {
        if (w->left->colour == BLACK) {
          w->right->colour = BLACK;
          w->colour = RED;
          left_rotate(&par->left, w);
          w = par->left;
        }
        w->colour = par->colour;
        par->colour = BLACK;
        w->left->colour = BLACK;
        right_rotate(parent[-1], par);
        x = tree->root;
        break;
      }
    }
  }
  x->colour = BLACK;
}

#ifndef DBUG_OFF

/* Test that the proporties for a red-black tree holds */

static int test_rb_tree(TREE_ELEMENT *element) {
  int count_l, count_r;

  if (!element->left) return 0; /* Found end of tree */
  if (element->colour == RED &&
      (element->left->colour == RED || element->right->colour == RED)) {
    printf("Wrong tree: Found two red in a row\n");
    return -1;
  }
  count_l = test_rb_tree(element->left);
  count_r = test_rb_tree(element->right);
  if (count_l >= 0 && count_r >= 0) {
    if (count_l == count_r) return count_l + (element->colour == BLACK);
    printf("Wrong tree: Incorrect black-count: %d - %d\n", count_l, count_r);
  }
  return -1;
}
#endif
