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

__BEGIN_DECLS

#include <stdio.h>
#include "hphp/neo/neo_err.h"
#include "hphp/neo/neo_hash.h"

#define FORCE_HASH_AT 10

typedef struct _hdf HDF;

/* HDFFILELOAD is a callback function to intercept file load requests and
 * provide templates via another mechanism.  This way you can load templates
 * that you compiled-into your binary, from in-memory caches, or from a
 * zip file, etc.  The HDF is provided so you can choose to use the
 * hdf_search_path function to find the file.  contents should return
 * a full malloc copy of the contents of the file, which the parser will
 * own and free.  Use hdf_register_fileload to set this function for
 * your top level HDF node.
 * NOTE: Technically, we shouldn't need a separate copy for each parse, but
 * using the separate copy makes this equivalent to the CSFILELOAD function.  We
 * can change this if we really want to save that copy at the expense of
 * slightly more complicated code. */
typedef NEOERR* (*HDFFILELOAD)(void *ctx, HDF *hdf, const char *filename,
                              char **contents);

typedef struct _attr
{
  char *key;
  char *value;
  struct _attr *next;
} HDF_ATTR;

struct _hdf
{
  int link;
  int alloc_value;
  char *name;
  int name_len;
  char *value;
  struct _attr *attr;
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

  /* Should only be set on the head node, used to override the default file
   * load method */
  void *fileload_ctx;
  HDFFILELOAD fileload;

  int visited;
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
 * Function: hdf_get_int_value - Return the integer value of a point in
 *           the data set
 * Description: hdf_get_int_value walks the HDF data set pointed to by
 *              hdf to name, and returns the value of that node
 *              converted to an integer.  If that node does not exist,
 *              or it does not contain a number, the defval is returned.
 * Input: hdf -> a node in an HDF data set
 *        name -> the name of a node to walk to in the data set
 *        defval -> value to return in case of error or if the node
 *                  doesn't exist
 * Output: None
 * Returns: The integer value of the node, or the defval
 */
int hdf_get_int_value (HDF *hdf, const char *name, int defval);

/*
 * Function: hdf_get_value - Return the value of a node in the data set
 * Description: hdf_get_value walks the data set pointed to by hdf via
 *              name and returns the string value located there, or
 *              defval if the node doesn't exist
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk the data set to
 *        defval -> the default value to return if the node doesn't
 *                  exist
 * Output: None
 * Returns: A pointer to the string stored in the data set, or defval.
 *          The data set maintains ownership of the string, if you want
 *          a copy you either have to call strdup yourself, or use
 *          hdf_get_copy
 */
char *hdf_get_value (HDF *hdf, const char *name, const char *defval);

/*
 * Function: hdf_get_valuevf - Return the value of a node in the data set
 * Description: hdf_get_valuevf walks the data set pointed to by hdf via
 *              namefmt printf expanded with varargs ap, and returns the
 *              string value located there, or NULL if it doesn't exist.
 *              This differs from hdf_get_value in that there is no
 *              default value possible.
 * Input: hdf -> the dataset node to start from
 *        namefmt -> the format string
 *        ap -> va_list of varargs
 * Output: None
 * Returns: A pointer to the string stored in the data set, or NULL.
 *          The data set maintains ownership of the string, if you want
 *          a copy you either have to call strdup yourself.
 */
char* hdf_get_valuevf (HDF *hdf, const char *namefmt, va_list ap)
                       ATTRIBUTE_PRINTF(2,0);

/*
 * Function: hdf_get_valuef - Return the value of a node in the data set
 * Description: hdf_get_valuef walks the data set pointed to by hdf via
 *              namefmt printf expanded with varargs, and returns the
 *              string value located there, or NULL if it doesn't exist.
 *              This differs from hdf_get_value in that there is no
 *              default value possible.
 * Input: hdf -> the dataset node to start from
 *        namefmt -> the printf-style format string
 *        ... -> arguments to fill out namefmt
 * Output: None
 * Returns: A pointer to the string stored in the data set, or NULL.
 *          The data set maintains ownership of the string, if you want
 *          a copy you either have to call strdup yourself.
 */
char* hdf_get_valuef (HDF *hdf, const char *namefmt, ...)
                      ATTRIBUTE_PRINTF(2,3);

/*
 * Function: hdf_get_copy - Returns a copy of a string in the HDF data set
 * Description: hdf_get_copy is similar to hdf_get_value, except that it
 *              returns an malloc'd copy of the string.
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk the data set to
 *        defval -> the default value to return if the node doesn't
 *                  exist
 * Output: value -> the allocated string (if defval = NULL, then value
 *                  will be NULL if defval is used)
 * Returns: NERR_NOMEM if unable to allocate the new copy
 */
NEOERR* hdf_get_copy (HDF *hdf, const char *name, char **value,
                      const char *defval);

/*
 * Function: hdf_get_obj - return the HDF data set node at a named location
 * Description: hdf_get_obj walks the dataset given by hdf to the node
 *              named name, and then returns the pointer to that node
 * Input: hdf -> the dataset node to start from
 *        name -> the name to walk to
 * Output: None
 * Returns: the pointer to the named node, or NULL if it doesn't exist
 */
HDF* hdf_get_obj (HDF *hdf, const char *name);

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
HDF* hdf_get_child (HDF *hdf, const char *name);

/*
 * Function: hdf_get_attr -
 * Description:
 * Input:
 * Output:
 * Returns:
 */
HDF_ATTR* hdf_get_attr (HDF *hdf, const char *name);

/*
 * Function: hdf_set_attr -
 * Description:
 * Input:
 * Output:
 * Returns:
 */
NEOERR* hdf_set_attr (HDF *hdf, const char *name, const char *key,
                      const char *value);

/*
 * Function: hdf_set_visited - Mark a node visited or not
 */
void hdf_set_visited (HDF *hdf, int visited);

/*
 * Function: hdf_is_visited - Return a node visited or not
 */
int hdf_is_visited (HDF *hdf);

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
HDF* hdf_obj_child (HDF *hdf);

/*
 * Function: hdf_obj_next - Return the next node of a dataset level
 * Description: hdf_obj_next is an accessor function for the HDF struct
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The pointer to the next node, or NULL if there is none
 */
HDF* hdf_obj_next (HDF *hdf);

/*
 * Function: hdf_obj_top - Return the pointer to the top dataset node
 * Description: hdf_obj_top is an accessor function which returns a
 *              pointer to the top of the dataset, the node which was
 *              returned by hdf_init.  This is most useful for
 *              implementations of language wrappers where individual
 *              nodes are tied garbage colletion wise to the top node of
 *              the data set
 * Input: hdf -> the hdf dataset node
 * Output: None
 * Returns: The pointer to the top node
 */
HDF* hdf_obj_top (HDF *hdf);

/*
 * Function: hdf_obj_attr - Return the HDF Attributes for a node
 * Description:
 * Input:
 * Output:
 * Returns:
 */
HDF_ATTR* hdf_obj_attr (HDF *hdf);

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
char* hdf_obj_value (HDF *hdf);

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
 * Function: hdf_set_valuef - Set the value of a named node
 * Description: hdf_set_valuef is a convenience function that wraps
 *              hdf_set_value.  Due to limitations of C, the fmt is in
 *              the format "name=value", where we will first format the
 *              entire string, and then break it at the first (from the
 *              left) equal sign (=) and use the left portion as the
 *              name and the right portion as the value.  This function
 *              is somewhat inefficient in that it first allocates the
 *              full name=value, and then the call to hdf_set_value
 *              duplicates the value portion, and then we free the
 *              name=value.
 *              Currently, we don't strip whitespace from the key or
 *              value.  In the future, this function might work more
 *              like reading a single line of an HDF string or file,
 *              allowing for attributes and symlinks to be specified...
 *              maybe.
 * Input: hdf -> the pointer to the hdf dataset
 *        fmt -> the name=value printf(3) format string
 * Output: None
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_set_valuef (HDF *hdf, const char *fmt, ...)
                        ATTRIBUTE_PRINTF(2,3);
NEOERR* hdf_set_valuevf (HDF *hdf, const char *fmt, va_list ap)
                         ATTRIBUTE_PRINTF(2,0);

/*
 * Function: hdf_set_int_value - Set the value of a named node to a number
 * Description: hdf_set_int_value is a helper function that maps an
 *              integer to a string, and then calls hdf_set_value with
 *              that string
 * Input: hdf -> the pointer to the hdf dataset
 *        name -> the named node to walk to
 *        value -> the value to set the node to
 * Output: None
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_set_int_value (HDF *hdf, const char *name, int value);

/*
 * Function: hdf_set_copy - Copy a value from one location in the
 *           dataset to another
 * Description: hdf_set_copy first walks the hdf dataset to the named src
 *              node, and then copies that value to the named dest node.
 *              If the src node is not found, an error is raised.
 * Input: hdf -> the pointer to the dataset node
 *        dest -> the name of the destination node
 *        src -> the name of the source node
 * Output: None
 * Returns: NERR_NOMEM, NERR_NOT_FOUND
 */
NEOERR* hdf_set_copy (HDF *hdf, const char *dest, const char *src);

/*
 * Function: hdf_set_buf - Set the value of a node without duplicating
 *           the value
 * Description: hdf_set_buf is similar to hdf_set_value, except the
 *              dataset takes ownership of the value instead of making a
 *              copy of it.  The dataset assumes that value was
 *              malloc'd, since it will attempt to free it when
 *              hdf_destroy is called
 * Input: hdf -> the hdf dataset node
 *        name -> the name to walk to
 *        value -> the malloc'd value
 * Output: None
 * Returns: NERR_NOMEM - unable to allocate a node
 */

NEOERR* hdf_set_buf (HDF *hdf, const char *name, char *value);

/*
 * Function: hdf_set_symlink - Set part of the tree to link to another
 * Description: hdf_set_symlink creates a link between two sections of
 *              an HDF dataset.  The link is "by name" hence the term
 *              "symlink".  This means that the destination node does
 *              not need to exist.  Any attempt to access the source
 *              node will cause the function to walk to the dest node,
 *              and then continue walking from there.  Using symlinks
 *              can "hide" values in the dataset since you won't be able
 *              to access any children of the linked node directly,
 *              though dumps and other things which access the data
 *              structure directly will bypass the symlink.  Use this
 *              feature sparingly as its likely to surprise you.
 * Input: hdf -> the dataset node
 *        src -> the source node name
 *        dest -> the destination node name (from the top of the
 *        dataset, not relative names)
 * Output: None
 * Returns: NERR_NOMEM
 */
NEOERR *hdf_set_symlink (HDF *hdf, const char *src, const char *dest);

/*
 * Function: hdf_sort_obj - sort the children of an HDF node
 * Description: hdf_sort_obj will sort the children of an HDF node,
 *              based on the given comparison function.
 *              This function works by creating an array of the pointers
 *              for each child object of h, using qsort to sort that
 *              array, and then re-ordering the linked list of children
 *              to the new order.  The qsort compare function uses a
 *              pointer to the value in the array, which in our case is
 *              a pointer to an HDF struct, so your comparison function
 *              should work on HDF ** pointers.
 * Input: h - HDF node
 *        compareFunc - function which returns 1,0,-1 depending on some
 *                      criteria.  The arguments to this sort function
 *                      are pointers to pointers to HDF elements.  For
 *                      example:
 *                      int sortByName(const void *a, const void *b) {
 *                        HDF **ha = (HDF **)a;
 *                        HDF **hb = (HDF **)b;
 *
 *                      return strcasecmp(hdf_obj_name(*ha), hdf_obj_name(*hb));
 *                      }
 *
 * Output: None (h children will be sorted)
 * Return: NERR_NOMEM
 */
NEOERR *hdf_sort_obj(HDF *h, int (*compareFunc)(const void *, const void *));

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
 * Function: hdf_write_file_atomic - write an HDF data file atomically
 * Description: hdf_write_file_atomic is similar to hdf_write_file,
 *              except the new file is created with a unique name and
 *              then rename(2) is used to atomically replace the old
 *              file with the new file
 * Input:
 * Output:
 * Returns: NERR_IO
 */
NEOERR* hdf_write_file_atomic (HDF *hdf, const char *path);

/*
 * Function: hdf_read_string - read an HDF string
 * Description:
 * Input:
 * Output:
 * Returns: NERR_NOMEM, NERR_PARSE
 */
NEOERR* hdf_read_string (HDF *hdf, const char *s);

/*
 * Function: hdf_read_string_ignore - Read an HDF string and ignore errors
 * Description:
 * Input:
 * Output:
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_read_string_ignore (HDF *hdf, const char *s, int ignore);

/*
 * Function: hdf_write_string - serialize an HDF dataset to a string
 * Description:
 * Input:
 * Output:
 * Returns: NERR_NOMEM
 */
NEOERR* hdf_write_string (HDF *hdf, char **s);

/*
 * Function: hdf_dump - dump an HDF dataset to stdout
 * Description:
 * Input:
 * Output:
 * Returns:
 */
NEOERR* hdf_dump (HDF *hdf, const char *prefix);

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

/*
 * Function: hdf_register_fileload - register a fileload function
 * Description: hdf_register_fileload registers a fileload function that
 *              overrides the built-in function.  The built-in function
 *              uses hdf_search_path and ne_file_load (based on stat/open/read)
 *              to find and load the file on every hdf_read_file (including
 *              #include).  You can override this function if you wish to provide
 *              other file search functions, or load the hdf file
 *              from an in-memory cache, etc.
 * Input: hdf - pointer to a head HDF node
 *        ctx - pointer that is passed to the HDFFILELOAD function when called
 *        fileload - a HDFFILELOAD function
 * Output: None
 * Return: None
 *
 */

void hdf_register_fileload(HDF *hdf, void *ctx, HDFFILELOAD fileload);

__END_DECLS

#endif /* incl_HPHP_NEO_HDF_H_ */
