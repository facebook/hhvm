/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef ZEND_HASH_H
#define ZEND_HASH_H

#include <sys/types.h>
#include "zend.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/array-iterator.h"

#define HASH_KEY_IS_STRING 1
#define HASH_KEY_IS_LONG 2
#define HASH_KEY_NON_EXISTENT 3
#define HASH_KEY_NON_EXISTANT HASH_KEY_NON_EXISTENT /* Keeping old define (with typo) for backward compatibility */

#define HASH_UPDATE     (1<<0)
#define HASH_ADD      (1<<1)
#define HASH_NEXT_INSERT  (1<<2)

#define HASH_DEL_KEY 0
#define HASH_DEL_INDEX 1
#define HASH_DEL_KEY_QUICK 2

#define HASH_UPDATE_KEY_IF_NONE    0
#define HASH_UPDATE_KEY_IF_BEFORE  1
#define HASH_UPDATE_KEY_IF_AFTER   2
#define HASH_UPDATE_KEY_ANYWAY     3

typedef ulong (*hash_func_t)(const char *arKey, uint nKeyLength);
typedef int  (*compare_func_t)(const void *, const void * TSRMLS_DC);
typedef void (*sort_func_t)(void *, size_t, register size_t, compare_func_t TSRMLS_DC);
typedef void (*dtor_func_t)(void *pDest);
typedef void (*copy_ctor_func_t)(void *pElement);
typedef void (*copy_ctor_param_func_t)(void *pElement, void *pParam);

struct _hashtable;

typedef struct bucket {
  ulong h;            /* Used for numeric indexing */
  uint nKeyLength;
  void *pData;
  void *pDataPtr;
  struct bucket *pListNext;
  struct bucket *pListLast;
  struct bucket *pNext;
  struct bucket *pLast;
  const char *arKey;
} Bucket;

#ifdef HHVM
typedef HPHP::ArrayData HashTable;
#else
typedef struct _hashtable {
  uint nTableSize;
  uint nTableMask;
  uint nNumOfElements;
  ulong nNextFreeElement;
  Bucket *pInternalPointer;  /* Used for element traversal */
  Bucket *pListHead;
  Bucket *pListTail;
  Bucket **arBuckets;
  dtor_func_t pDestructor;
  zend_bool persistent;
  unsigned char nApplyCount;
  zend_bool bApplyProtection;
#if ZEND_DEBUG
  int inconsistent;
#endif
} HashTable;
#endif

typedef struct _zend_hash_key {
  const char *arKey;
  uint nKeyLength;
  ulong h;
} zend_hash_key;


typedef zend_bool (*merge_checker_func_t)(HashTable *target_ht, void *source_data, zend_hash_key *hash_key, void *pParam);

#ifdef HHVM
typedef HPHP::MArrayIter HashPosition;
#else
typedef Bucket* HashPosition;
#endif

BEGIN_EXTERN_C()

/* startup/shutdown */
ZEND_API int _zend_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor, zend_bool persistent ZEND_FILE_LINE_DC);
ZEND_API int _zend_hash_init_ex(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor, zend_bool persistent, zend_bool bApplyProtection ZEND_FILE_LINE_DC);
ZEND_API void zend_hash_destroy(HashTable *ht);
ZEND_API void zend_hash_clean(HashTable *ht);
#define zend_hash_init(ht, nSize, pHashFunction, pDestructor, persistent)            _zend_hash_init((ht), (nSize), (pHashFunction), (pDestructor), (persistent) ZEND_FILE_LINE_CC)
#define zend_hash_init_ex(ht, nSize, pHashFunction, pDestructor, persistent, bApplyProtection)    _zend_hash_init_ex((ht), (nSize), (pHashFunction), (pDestructor), (persistent), (bApplyProtection) ZEND_FILE_LINE_CC)

/* additions/updates/changes */
ZEND_API int _zend_hash_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
#define zend_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)
#define zend_hash_add(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_ADD ZEND_FILE_LINE_CC)

ZEND_API int _zend_hash_quick_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
#define zend_hash_quick_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest) \
    _zend_hash_quick_add_or_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)
#define zend_hash_quick_add(ht, arKey, nKeyLength, h, pData, nDataSize, pDest) \
    _zend_hash_quick_add_or_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest, HASH_ADD ZEND_FILE_LINE_CC)

ZEND_API int _zend_hash_index_update_or_next_insert(HashTable *ht, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
#define zend_hash_index_update(ht, h, pData, nDataSize, pDest) \
    _zend_hash_index_update_or_next_insert(ht, h, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)
#define zend_hash_next_index_insert(ht, pData, nDataSize, pDest) \
    _zend_hash_index_update_or_next_insert(ht, 0, pData, nDataSize, pDest, HASH_NEXT_INSERT ZEND_FILE_LINE_CC)

ZEND_API int zend_hash_add_empty_element(HashTable *ht, const char *arKey, uint nKeyLength);


#define ZEND_HASH_APPLY_KEEP        0
#define ZEND_HASH_APPLY_REMOVE        1<<0
#define ZEND_HASH_APPLY_STOP        1<<1

typedef int (*apply_func_t)(void *pDest TSRMLS_DC);
typedef int (*apply_func_arg_t)(void *pDest, void *argument TSRMLS_DC);
typedef int (*apply_func_args_t)(void *pDest TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key);

ZEND_API void zend_hash_graceful_destroy(HashTable *ht);
ZEND_API void zend_hash_graceful_reverse_destroy(HashTable *ht);
ZEND_API void zend_hash_apply(HashTable *ht, apply_func_t apply_func TSRMLS_DC);
ZEND_API void zend_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void * TSRMLS_DC);
ZEND_API void zend_hash_apply_with_arguments(HashTable *ht TSRMLS_DC, apply_func_args_t apply_func, int, ...);

/* This function should be used with special care (in other words,
 * it should usually not be used).  When used with the ZEND_HASH_APPLY_STOP
 * return value, it assumes things about the order of the elements in the hash.
 * Also, it does not provide the same kind of reentrancy protection that
 * the standard apply functions do.
 */
ZEND_API void zend_hash_reverse_apply(HashTable *ht, apply_func_t apply_func TSRMLS_DC);


/* Deletes */
ZEND_API int zend_hash_del_key_or_index(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, int flag);
#define zend_hash_del(ht, arKey, nKeyLength) \
    zend_hash_del_key_or_index(ht, arKey, nKeyLength, 0, HASH_DEL_KEY)
#define zend_hash_quick_del(ht, arKey, nKeyLength, h) \
    zend_hash_del_key_or_index(ht, arKey, nKeyLength, h, HASH_DEL_KEY_QUICK)
#define zend_hash_index_del(ht, h) \
    zend_hash_del_key_or_index(ht, NULL, 0, h, HASH_DEL_INDEX)

ZEND_API ulong zend_get_hash_value(const char *arKey, uint nKeyLength);

/* Data retreival */
ZEND_API int zend_hash_find(const HashTable *ht, const char *arKey, uint nKeyLength, void **pData);
ZEND_API int zend_hash_quick_find(const HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void **pData);
ZEND_API int zend_hash_index_find(const HashTable *ht, ulong h, void **pData);

/* Misc */
ZEND_API int zend_hash_exists(const HashTable *ht, const char *arKey, uint nKeyLength);
ZEND_API int zend_hash_quick_exists(const HashTable *ht, const char *arKey, uint nKeyLength, ulong h);
ZEND_API int zend_hash_index_exists(const HashTable *ht, ulong h);
ZEND_API ulong zend_hash_next_free_element(const HashTable *ht);

/* traversing */
#define zend_hash_has_more_elements_ex(ht, pos) \
  (zend_hash_get_current_key_type_ex(ht, pos) == HASH_KEY_NON_EXISTENT ? FAILURE : SUCCESS)
ZEND_API int zend_hash_move_forward_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_move_backwards_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_get_current_key_ex(const HashTable *ht, char **str_index, uint *str_length, ulong *num_index, zend_bool duplicate, HashPosition *pos);
ZEND_API void zend_hash_get_current_key_zval_ex(const HashTable *ht, zval *key, HashPosition *pos);
ZEND_API int zend_hash_get_current_key_type_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_get_current_data_ex(HashTable *ht, void **pData, HashPosition *pos);
ZEND_API void zend_hash_internal_pointer_reset_ex(HashTable *ht, HashPosition *pos);
ZEND_API void zend_hash_internal_pointer_end_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_update_current_key_ex(HashTable *ht, int key_type, const char *str_index, uint str_length, ulong num_index, int mode, HashPosition *pos);

typedef struct _HashPointer {
  HashPosition pos;
  ulong h;
} HashPointer;

ZEND_API int zend_hash_get_pointer(const HashTable *ht, HashPointer *ptr);
ZEND_API int zend_hash_set_pointer(HashTable *ht, const HashPointer *ptr);

#define zend_hash_has_more_elements(ht) \
  zend_hash_has_more_elements_ex(ht, NULL)
#define zend_hash_move_forward(ht) \
  zend_hash_move_forward_ex(ht, NULL)
#define zend_hash_move_backwards(ht) \
  zend_hash_move_backwards_ex(ht, NULL)
#define zend_hash_get_current_key(ht, str_index, num_index, duplicate) \
  zend_hash_get_current_key_ex(ht, str_index, NULL, num_index, duplicate, NULL)
#define zend_hash_get_current_key_zval(ht, key) \
  zend_hash_get_current_key_zval_ex(ht, key, NULL)
#define zend_hash_get_current_key_type(ht) \
  zend_hash_get_current_key_type_ex(ht, NULL)
#define zend_hash_get_current_data(ht, pData) \
  zend_hash_get_current_data_ex(ht, pData, NULL)
#define zend_hash_internal_pointer_reset(ht) \
  zend_hash_internal_pointer_reset_ex(ht, NULL)
#define zend_hash_internal_pointer_end(ht) \
  zend_hash_internal_pointer_end_ex(ht, NULL)
#define zend_hash_update_current_key(ht, key_type, str_index, str_length, num_index) \
  zend_hash_update_current_key_ex(ht, key_type, str_index, str_length, num_index, HASH_UPDATE_KEY_ANYWAY, NULL)

/* Copying, merging and sorting */
ZEND_API void zend_hash_copy(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, void *tmp, uint size);
ZEND_API void _zend_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, void *tmp, uint size, int overwrite ZEND_FILE_LINE_DC);
ZEND_API void zend_hash_merge_ex(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, uint size, merge_checker_func_t pMergeSource, void *pParam);
ZEND_API int zend_hash_sort(HashTable *ht, sort_func_t sort_func, compare_func_t compare_func, int renumber TSRMLS_DC);
ZEND_API int zend_hash_compare(HashTable *ht1, HashTable *ht2, compare_func_t compar, zend_bool ordered TSRMLS_DC);
ZEND_API int zend_hash_minmax(const HashTable *ht, compare_func_t compar, int flag, void **pData TSRMLS_DC);

#define zend_hash_merge(target, source, pCopyConstructor, tmp, size, overwrite)          \
  _zend_hash_merge(target, source, pCopyConstructor, tmp, size, overwrite ZEND_FILE_LINE_CC)

ZEND_API int zend_hash_num_elements(const HashTable *ht);

ZEND_API int zend_hash_rehash(HashTable *ht);

/*
 * DJBX33A (Daniel J. Bernstein, Times 33 with Addition)
 *
 * This is Daniel J. Bernstein's popular `times 33' hash function as
 * posted by him years ago on comp.lang.c. It basically uses a function
 * like ``hash(i) = hash(i-1) * 33 + str[i]''. This is one of the best
 * known hash functions for strings. Because it is both computed very
 * fast and distributes very well.
 *
 * The magic of number 33, i.e. why it works better than many other
 * constants, prime or not, has never been adequately explained by
 * anyone. So I try an explanation: if one experimentally tests all
 * multipliers between 1 and 256 (as RSE did now) one detects that even
 * numbers are not useable at all. The remaining 128 odd numbers
 * (except for the number 1) work more or less all equally well. They
 * all distribute in an acceptable way and this way fill a hash table
 * with an average percent of approx. 86%.
 *
 * If one compares the Chi^2 values of the variants, the number 33 not
 * even has the best value. But the number 33 and a few other equally
 * good numbers like 17, 31, 63, 127 and 129 have nevertheless a great
 * advantage to the remaining numbers in the large set of possible
 * multipliers: their multiply operation can be replaced by a faster
 * operation based on just one shift plus either a single addition
 * or subtraction operation. And because a hash function has to both
 * distribute good _and_ has to be very fast to compute, those few
 * numbers should be preferred and seems to be the reason why Daniel J.
 * Bernstein also preferred it.
 *
 *
 *                  -- Ralf S. Engelschall <rse@engelschall.com>
 */

static inline ulong zend_inline_hash_func(const char *arKey, uint nKeyLength)
{
  register ulong hash = 5381;

  /* variant with the hash unrolled eight times */
  for (; nKeyLength >= 8; nKeyLength -= 8) {
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
    hash = ((hash << 5) + hash) + *arKey++;
  }
  switch (nKeyLength) {
    case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
    case 1: hash = ((hash << 5) + hash) + *arKey++; break;
    case 0: break;
EMPTY_SWITCH_DEFAULT_CASE()
  }
  return hash;
}


ZEND_API ulong zend_hash_func(const char *arKey, uint nKeyLength);

#if ZEND_DEBUG
/* debug */
void zend_hash_display_pListTail(const HashTable *ht);
void zend_hash_display(const HashTable *ht);
#endif

END_EXTERN_C()

#define ZEND_INIT_SYMTABLE(ht)                \
  ZEND_INIT_SYMTABLE_EX(ht, 2, 0)

#define ZEND_INIT_SYMTABLE_EX(ht, n, persistent)      \
  zend_hash_init(ht, n, NULL, ZVAL_PTR_DTOR, persistent)

#define ZEND_HANDLE_NUMERIC_EX(key, length, idx, func) do {          \
  register const char *tmp = key;                      \
                                      \
  if (*tmp == '-') {                            \
    tmp++;                                \
  }                                    \
  if (*tmp >= '0' && *tmp <= '9') { /* possibly a numeric index */    \
    const char *end = key + length - 1;                  \
                                      \
    if ((*end != '\0') /* not a null terminated string */        \
     || (*tmp == '0' && length > 2) /* numbers with leading zeros */  \
     || (end - tmp > MAX_LENGTH_OF_LONG - 1) /* number too long */    \
     || (SIZEOF_LONG == 4 &&                      \
         end - tmp == MAX_LENGTH_OF_LONG - 1 &&              \
         *tmp > '2')) { /* overflow */                  \
      break;                              \
    }                                  \
    idx = (*tmp - '0');                          \
    while (++tmp != end && *tmp >= '0' && *tmp <= '9') {        \
      idx = (idx * 10) + (*tmp - '0');                \
    }                                  \
    if (tmp == end) {                          \
      if (*key == '-') {                        \
        if (idx-1 > LONG_MAX) { /* overflow */            \
          break;                          \
        }                              \
        idx = 0 - idx;                                 \
      } else if (idx > LONG_MAX) { /* overflow */            \
        break;                            \
      }                                \
      func;                              \
    }                                  \
  }                                    \
} while (0)

#define ZEND_HANDLE_NUMERIC(key, length, func) do {              \
  ulong idx;                                \
                                      \
  ZEND_HANDLE_NUMERIC_EX(key, length, idx, return func);          \
} while (0)

static inline int zend_symtable_update(HashTable *ht, const char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest)          \
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_update(ht, idx, pData, nDataSize, pDest));
  return zend_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest);
}


static inline int zend_symtable_del(HashTable *ht, const char *arKey, uint nKeyLength)
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_del(ht, idx));
  return zend_hash_del(ht, arKey, nKeyLength);
}


static inline int zend_symtable_find(HashTable *ht, const char *arKey, uint nKeyLength, void **pData)
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_find(ht, idx, pData));
  return zend_hash_find(ht, arKey, nKeyLength, pData);
}


static inline int zend_symtable_exists(HashTable *ht, const char *arKey, uint nKeyLength)
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_exists(ht, idx));
  return zend_hash_exists(ht, arKey, nKeyLength);
}

static inline int zend_symtable_update_current_key_ex(HashTable *ht, const char *arKey, uint nKeyLength, int mode, HashPosition *pos)
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_update_current_key_ex(ht, HASH_KEY_IS_LONG, NULL, 0, idx, mode, pos));
  return zend_hash_update_current_key_ex(ht, HASH_KEY_IS_STRING, arKey, nKeyLength, 0, mode, pos);
}
#define zend_symtable_update_current_key(ht,arKey,nKeyLength,mode) \
  zend_symtable_update_current_key_ex(ht, arKey, nKeyLength, mode, NULL)


#endif              /* ZEND_HASH_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
