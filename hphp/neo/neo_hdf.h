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

#ifndef incl_HPHP_NEO_HDF_H_
#define incl_HPHP_NEO_HDF_H_ 1


#include <stdio.h>
#include "hphp/neo/neo_err.h"
#include "hphp/neo/neo_hash.h"

__BEGIN_DECLS
#define FORCE_HASH_AT 10

typedef struct _hdf HDF;

struct _hdf
{
  int alloc_value;
  char *name;
  int name_len;
  char *value;
  struct _hdf *top;
  struct _hdf *next;
  struct _hdf *child;

  /* the following fields are used to implement a cache */
  struct _hdf *last_hp;
  struct _hdf *last_hs;

  /* the following HASH is used when we reach more than FORCE_HASH_AT
   * elements */
  NE_HASH *hash;
  /* When using the HASH, we need to know where to append new children */
  struct _hdf *last_child;

  int visited;
  int is_wildcard;
};

/*
 * Function: hdf_init - Initialize an HDF data set
 * Description: hdf_init initializes an HDF data set and returns the
 *              pointer to the top node in the data set.
 * Input: hdf - pointer to an HDF pointer
 * Output: hdf - allocated hdf node
 * Returns: NERR_NOMEM - unable to allocate memory for dataset
 */
NEOERR* hdf_init (HDF **hdf);

/*
 * Function: hdf_destroy - deallocate an HDF data set
 * Description: hdf_destroy is used to deallocate all memory associated
 *              with an hdf data set.  Although you can pass an HDF node
 *              as an argument to this function, you are likely to cause
 *              a segfault if you continue to access the data set.  In
 *              the future, we may restrict hdf_destroy so it only works
 *              on the top level node.
 * Input: hdf - pointer to an HDF data set allocated with hdf_init
 * Output: None
 * Returns: None
 */
void hdf_destroy (HDF **hdf);

/*
 * Function: hdf_get_obj - return the HDF data set node at a named location
 * Description: hdf_get_obj walks the dataset given by hdf to the node
 *              named name, and then returns the pointer to that node
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk to
 * Output: None
 * Returns: the pointer to the named node, or NULL if it doesn't exist
 */
HDF* hdf_get_obj (HDF *hdf, const char *name, NEOERR** err);

/*
 * Function: hdf_get_node - Similar to hdf_get_obj except all the nodes
 *           are created if the don't exist.
 * Description: hdf_get_node is similar to hdf_get_obj, except instead
 *              of stopping if it can't find a node in the tree, it will
 *              create all of the nodes necessary to hand you back the
 *              node you ask for.  Nodes are created with no value.
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk to
 * Output: ret -> the dataset node you asked for
 * Returns: NERR_NOMEM - unable to allocate new nodes
 */
NEOERR * hdf_get_node (HDF *hdf, const char *name, HDF **ret);

/*
 * Function: hdf_get_child - return the first child of the named node
 * Description: hdf_get_child will walk the dataset starting at hdf to
 *              name, and return the first child of that node
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk to
 * Output: None
 * Returns: The first child of the named dataset node or NULL if the
 *          node is not found (or it has no children)
 */
HDF* hdf_get_child (HDF *hdf, const char *name, NEOERR** err);

/*
 * Function: hdf_set_visited - Mark a node visited or not
 */
void hdf_set_visited (HDF *hdf, int visited);

/*
 * Function: hdf_is_visited - Return a node visited or not
 */
int hdf_is_visited (HDF *hdf);

/*
 * Function: hdf_is_wildcard - Returns if a node's name is from a wildcard
 */
int hdf_is_wildcard (HDF *hdf);

/*
 * Function: hdf_obj_child - Return the first child of a dataset node
 * Description: hdf_obj_child and the other hdf_obj_ functions are
 *              accessors to the HDF dataset.  Although we do not
 *              currently "hide" the HDF struct implementation, we
 *              recommend you use the accessor functions instead of
 *              accessing the values directly.
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The pointer to the first child, or NULL if there is none
 */
HDF* hdf_obj_child (HDF *hdf, NEOERR**);

/*
 * Function: hdf_obj_next - Return the next node of a dataset level
 * Description: hdf_obj_next is an accessor function for the HDF struct
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The pointer to the next node, or NULL if there is none
 */
HDF* hdf_obj_next (HDF *hdf);

/*
 * Function: hdf_obj_name - Return the name of a node
 * Description: hdf_obj_name is an accessor function for a datset node
 *              which returns the name of the node.  This is just the
 *              local name, and not the full path.
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The name of the node.  If this is the top node, the name is
 * NULL.
 */
char* hdf_obj_name (HDF *hdf);

/*
 * Function: hdf_obj_value - Return the value of a node
 * Description: hdf_obj_value is an accessor function for a dataset node
 *              which returns the value of the node, or NULL if the node
 *              has no value.  This is not a copy of the value, so the
 *              node retains ownership of the value
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The value of the node, or NULL if it has no value
 */
char* hdf_obj_value (HDF *hdf, NEOERR**);

/*
 * Function: hdf_set_value - Set the value of a named node
 * Description: hdf_set_value will set the value of a named node.  All
 *              of the interstitial nodes which don't exist will be
 *              created with a value of NULL.  Existing nodes are not
 *              modified.  New nodes are created at the end of the list.
 *              If a list of nodes exceeds FORCE_HASH_AT, then a HASH
 *              will be created at that level and all of the nodes will
 *              be added to the hash for faster lookup times.
 *              The copy of the value will be made which the dataset
 *              will own.
 * Input: hdf -> the pointer to the hdf dataset
 *        name -> the named node to walk to
 *        value -> the value to set the node to
 * Output: None
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_set_value (HDF *hdf, const char *name, const char *value);

/*
 * Function: hdf_read_file - read an HDF data file
 * Description:
 * Input:
 * Output:
 * Returns: NERR_IO, NERR_NOMEM, NERR_PARSE
 */
NEOERR* hdf_read_file (HDF *hdf, const char *path);

/*
 * Function: hdf_write_file - write an HDF data file
 * Description:
 * Input:
 * Output:
 * Returns: NERR_IO
 */
NEOERR* hdf_write_file (HDF *hdf, const char *path);

/*
 * Function: hdf_read_string - read an HDF string
 * Description:
 * Input:
 * Output:
 * Returns: NERR_NOMEM, NERR_PARSE
 */
NEOERR* hdf_read_string (HDF *hdf, const char *s);

/*
 * Function: hdf_write_string - serialize an HDF dataset to a string
 * Description:
 * Input:
 * Output:
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_write_string (HDF *hdf, char **s);

/*
 * Function: hdf_dump_format - dump an HDF dataset to FILE *fp
 * Description:
 * Input:
 * Output:
 * Returns:
 */
NEOERR* hdf_dump_format (HDF *hdf, int lvl, FILE *fp);

/*
 * Function: hdf_dump_str - dump an HDF dataset to NEOSTRING
 * Description:
 * Input:
 * Output:
 * Returns:
 */
NEOERR* hdf_dump_str(HDF *hdf, const char *prefix, int compact, NEOSTRING *str);

/*
 * Function: hdf_remove_tree - delete a subtree of an HDF dataset
 * Description:
 * Input:
 * Output:
 * Returns:
 */
NEOERR* hdf_remove_tree (HDF *hdf, const char *name);

/*
 * Function: hdf_copy - copy part of an HDF dataset to another
 * Description: hdf_copy is a deep copy of an HDF tree pointed to by
 *              src to the named node of dest.  dest and src need not be
 *              part of the same data set
 * Input: dest_hdf -> the destination dataset
 *        name -> the name of the destination node
 *        src -> the hdf dataset to copy to the destination
 * Output: None
 * Returns: NERR_NOMEM, NERR_NOT_FOUND
 */
NEOERR* hdf_copy (HDF *dest_hdf, const char *name, HDF *src);

/*
 * Function: hdf_search_path - Find a file given a search path in HDF
 * Description: hdf_search_path is a convenience/utility function that
 *              searches for relative filenames in a search path.  The
 *              search path is the list given by the children of
 *              hdf.loadpaths.
 * Input: hdf -> the hdf dataset to use
 *        path -> the relative path
 *        full -> a pointer to a buffer
 *        full_len -> size of full buffer
 * Output: full -> the full path of the file
 * Returns: NERR_NOT_FOUND if the file wasn't found in the search path
 */
NEOERR* hdf_search_path (HDF *hdf, const char *path, char *full, int full_len);

__END_DECLS

#endif /* incl_HPHP_NEO_HDF_H_ */
