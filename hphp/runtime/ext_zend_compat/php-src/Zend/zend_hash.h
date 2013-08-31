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

typedef HPHP::ArrayData HashTable;


typedef struct _zend_hash_key {
  const char *arKey;
  uint nKeyLength;
  ulong h;
} zend_hash_key;


typedef zend_bool (*merge_checker_func_t)(HashTable *target_ht, void *source_data, zend_hash_key *hash_key, void *pParam);

typedef int32_t HashPosition;

BEGIN_EXTERN_C()

/* startup/shutdown */
ZEND_API inline int _zend_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor, zend_bool persistent ZEND_FILE_LINE_DC) {
  ht->incRefCount();
  return SUCCESS;
}
ZEND_API int _zend_hash_init_ex(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor, zend_bool persistent, zend_bool bApplyProtection ZEND_FILE_LINE_DC);
ZEND_API inline void zend_hash_destroy(HashTable *ht) {
  decRefArr(ht);
}
ZEND_API void zend_hash_clean(HashTable *ht);
#define zend_hash_init(ht, nSize, pHashFunction, pDestructor, persistent)            _zend_hash_init((ht), (nSize), (pHashFunction), (pDestructor), (persistent) ZEND_FILE_LINE_CC)
#define zend_hash_init_ex(ht, nSize, pHashFunction, pDestructor, persistent, bApplyProtection)    _zend_hash_init_ex((ht), (nSize), (pHashFunction), (pDestructor), (persistent), (bApplyProtection) ZEND_FILE_LINE_CC)

/* additions/updates/changes */
ZEND_API int _zend_hash_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, zval **pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
#define zend_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)
#define zend_hash_add(ht, arKey, nKeyLength, pData, nDataSize, pDest) \
    _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, HASH_ADD ZEND_FILE_LINE_CC)

ZEND_API int _zend_hash_quick_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, zval **pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
#define zend_hash_quick_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest) \
    _zend_hash_quick_add_or_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest, HASH_UPDATE ZEND_FILE_LINE_CC)
#define zend_hash_quick_add(ht, arKey, nKeyLength, h, pData, nDataSize, pDest) \
    _zend_hash_quick_add_or_update(ht, arKey, nKeyLength, h, pData, nDataSize, pDest, HASH_ADD ZEND_FILE_LINE_CC)

ZEND_API int _zend_hash_index_update_or_next_insert(HashTable *ht, ulong h, zval **pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC);
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


/* Deletes */
ZEND_API int zend_hash_del_key_or_index(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, int flag);
#define zend_hash_del(ht, arKey, nKeyLength) \
    zend_hash_del_key_or_index(ht, arKey, nKeyLength, 0, HASH_DEL_KEY)
#define zend_hash_quick_del(ht, arKey, nKeyLength, h) \
    zend_hash_del_key_or_index(ht, arKey, nKeyLength, h, HASH_DEL_KEY_QUICK)
#define zend_hash_index_del(ht, h) \
    zend_hash_del_key_or_index(ht, NULL, 0, h, HASH_DEL_INDEX)

/* Data retreival */
ZEND_API int zend_hash_find(const HashTable *ht, const char *arKey, uint nKeyLength, zval ***pData);

/* traversing */
#define zend_hash_has_more_elements_ex(ht, pos) \
  (zend_hash_get_current_key_type_ex(ht, pos) == HASH_KEY_NON_EXISTENT ? FAILURE : SUCCESS)
ZEND_API int zend_hash_move_forward_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_move_backwards_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_get_current_key_ex(const HashTable *ht, char **str_index, uint *str_length, ulong *num_index, zend_bool duplicate, HashPosition *pos);
ZEND_API int zend_hash_get_current_key_type_ex(HashTable *ht, HashPosition *pos);
ZEND_API int zend_hash_get_current_data_ex(HashTable *ht, zval ***pData, HashPosition *pos);
ZEND_API void zend_hash_internal_pointer_reset_ex(HashTable *ht, HashPosition *pos);

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

/* Copying, merging and sorting */
ZEND_API void _zend_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, void *tmp, uint size, int overwrite ZEND_FILE_LINE_DC);
#define zend_hash_merge(target, source, pCopyConstructor, tmp, size, overwrite)          \
  _zend_hash_merge(target, source, pCopyConstructor, tmp, size, overwrite ZEND_FILE_LINE_CC)

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

inline int zend_symtable_update(HashTable *ht, const char *arKey, uint nKeyLength, zval **pData, uint nDataSize, void **pDest)          \
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_update(ht, idx, pData, nDataSize, pDest));
  return zend_hash_update(ht, arKey, nKeyLength, pData, nDataSize, pDest);
}


inline int zend_symtable_del(HashTable *ht, const char *arKey, uint nKeyLength)
{
  ZEND_HANDLE_NUMERIC(arKey, nKeyLength, zend_hash_index_del(ht, idx));
  return zend_hash_del(ht, arKey, nKeyLength);
}

#endif              /* ZEND_HASH_H */
