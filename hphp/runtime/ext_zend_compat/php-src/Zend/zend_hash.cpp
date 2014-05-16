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

#include "zend.h"
#include "zend_hash.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_qsort.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zval-helpers.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-wrap-func.h"
#include "hphp/util/safesort.h"
#include "hphp/util/assertions.h"

ZEND_API int _zend_hash_add_or_update(HashTable *ht, const char *arKey,
    uint nKeyLength, void *pData, uint nDataSize, void **pDest,
    int flag ZEND_FILE_LINE_DC)
{
  using namespace HPHP;

  if (nKeyLength <= 0) {
    return FAILURE;
  }
  assert(arKey[nKeyLength - 1] == '\0');
  String key(arKey, nKeyLength - 1, CopyString);
  if ((flag & HASH_ADD) && ht->exists(key)) {
    return FAILURE;
  }
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);
  proxy->proxySet(key.get(), pData, nDataSize, pDest);
  return SUCCESS;
}

ZEND_API int _zend_hash_quick_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  return _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, flag ZEND_FILE_LINE_CC);
}

ZEND_API int _zend_hash_index_update_or_next_insert(HashTable *ht, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  using namespace HPHP;
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);
  if (flag & HASH_NEXT_INSERT) {
    proxy->proxyAppend(pData, nDataSize, pDest);
    return SUCCESS;
  }

  if (flag & HASH_ADD && ht->exists(h)) {
    return FAILURE;
  }

  proxy->proxySet(h, pData, nDataSize, pDest);
  return SUCCESS;
}

ZEND_API void zend_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void * arg TSRMLS_DC) {
  using namespace HPHP;
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);
  for (auto pos = ht->iter_begin();
      pos != ArrayData::invalid_index;
      pos = ht->iter_advance(pos))
  {
    void * data = proxy->proxyGetValueRef(pos);
    int result = apply_func(data, arg TSRMLS_CC);

    if (result & ZEND_HASH_APPLY_REMOVE) {
      not_implemented();
    }
    if (result & ZEND_HASH_APPLY_STOP) {
      break;
    }
  }
}

ZEND_API void zend_hash_apply_with_arguments(HashTable *ht TSRMLS_DC, apply_func_args_t apply_func, int num_args, ...) {
  using namespace HPHP;
  va_list args;
  zend_hash_key hash_key;
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);

  for (auto pos = ht->iter_begin();
      pos != ArrayData::invalid_index;
      pos = ht->iter_advance(pos))
  {
    Variant key = ht->getKey(pos);
    int result;
    va_start(args, num_args);
    if (key.isInteger()) {
      hash_key.arKey = "";
      hash_key.nKeyLength = 0;
      hash_key.h = key.asInt64Val();
    } else {
      assert(key.isString());
      hash_key.arKey = key.asCStrRef().data();
      hash_key.nKeyLength = key.asCStrRef().size() + 1;
      hash_key.h = 0;
    }
    void * data = proxy->proxyGetValueRef(pos);
    result = apply_func(data TSRMLS_CC, num_args, args, &hash_key);

    if (result & ZEND_HASH_APPLY_REMOVE) {
      not_implemented();
    }
    if (result & ZEND_HASH_APPLY_STOP) {
      va_end(args);
      break;
    }
    va_end(args);
  }
}

ZEND_API int zend_hash_del_key_or_index(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, int flag) {
  using namespace HPHP;
  if (nKeyLength == 0) {
    ht->remove(h, false);
  } else {
    assert(arKey[nKeyLength - 1] == '\0');
    String key(arKey, nKeyLength - 1, CopyString);
    ht->remove(key.get(), false);
  }
  return SUCCESS;
}

ZEND_API ulong zend_get_hash_value(const char *arKey, uint nKeyLength)
{
  return zend_inline_hash_func(arKey, nKeyLength);
}

ZEND_API int zend_hash_find(const HashTable *ht, const char *arKey, uint nKeyLength, void **pData) {
  using namespace HPHP;
  if (nKeyLength <= 0) {
    return FAILURE;
  }
  always_assert(ht->isProxyArray());
  assert(arKey[nKeyLength - 1] == '\0');
  const ProxyArray * proxy = static_cast<const ProxyArray*>(ht);
  String key(arKey, nKeyLength - 1, CopyString);
  void * val = proxy->proxyGet(key.get());
  if (!val) {
    return FAILURE;
  }
  *pData = val;
  return SUCCESS;
}

ZEND_API int zend_hash_quick_find(const HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void **pData) {
  return zend_hash_find(ht, arKey, nKeyLength, pData);
}

ZEND_API int zend_hash_index_find(const HashTable *ht, ulong h, void **pData) {
  using namespace HPHP;
  always_assert(ht->isProxyArray());
  const ProxyArray * proxy = static_cast<const ProxyArray*>(ht);
  void * val = proxy->proxyGet(h);
  if (!val) {
    return FAILURE;
  }
  *pData = val;
  return SUCCESS;
}

ZEND_API int zend_hash_exists(const HashTable *ht, const char *arKey, uint nKeyLength)
{
  using namespace HPHP;
  assert(arKey[nKeyLength - 1] == '\0');
  return ht->exists(
    StrNR(String(arKey, nKeyLength - 1, CopyString).get())
  );
}

ZEND_API ulong zend_hash_next_free_element(const HashTable *ht) {
  return ht->iter_end() + 1;
}

ZEND_API int zend_hash_move_forward_ex(HashTable *ht, HashPosition *pos) {
  if (pos) {
    *pos = ht->iter_advance(*pos);
    return *pos != ht->invalid_index;
  }
  ht->next();
  return ht->isInvalid() ? FAILURE : SUCCESS;
}
ZEND_API int zend_hash_move_backwards_ex(HashTable *ht, HashPosition *pos) {
  if (pos) {
    *pos = ht->iter_rewind(*pos);
    return *pos != ht->invalid_index;
  }
  ht->prev();
  return ht->isInvalid() ? FAILURE : SUCCESS;
}
ZEND_API int zend_hash_get_current_key_ex(const HashTable *ht, char **str_index, uint *str_length, ulong *num_index, zend_bool duplicate, HashPosition *pos) {
  using namespace HPHP;
  HashPosition hp = pos ? *pos : ht->getPosition();
  if (hp == ht->invalid_index) {
    return HASH_KEY_NON_EXISTENT;
  }

  Variant key = ht->getKey(hp);
  if (key.isString()) {
    String keyStr = key.toString();
    if (duplicate) {
      *str_index = estrndup(keyStr.data(), keyStr.size());
    } else {
      *str_index = const_cast<char*>(keyStr.data());
    }
    if (str_length) {
      *str_length = keyStr.size() + 1;
      assert((*str_index)[*str_length - 1] == '\0');
    }
    return HASH_KEY_IS_STRING;
  }

  assert(key.isInteger());
  *num_index = key.toInt64();
  return HASH_KEY_IS_LONG;
}
ZEND_API int zend_hash_get_current_key_type_ex(HashTable *ht, HashPosition *pos) {
  using namespace HPHP;
  HashPosition hp = pos ? *pos : ht->getPosition();
  if (hp == ht->invalid_index) {
    return HASH_KEY_NON_EXISTENT;
  }

  Variant key = ht->getKey(hp);
  DataType type = key.getType();
  switch (type) {
    case KindOfInt64:
      return HASH_KEY_IS_LONG;
    case KindOfString:
    case KindOfStaticString:
      return HASH_KEY_IS_STRING;
    default:
      not_reached();
  }
}

ZEND_API int zend_hash_get_current_data_ex(HashTable *ht, void **pData, HashPosition *pos) {
  using namespace HPHP;
  HashPosition hp = pos ? *pos : ht->getPosition();
  if (hp == ht->invalid_index) {
    return FAILURE;
  }
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);
  void * val = proxy->proxyGetValueRef(hp);
  if (!val) {
    return FAILURE;
  }
  *pData = val;
  return SUCCESS;
}

ZEND_API void zend_hash_internal_pointer_reset_ex(HashTable *ht, HashPosition *pos) {
  if (pos) {
    *pos = ht->iter_begin();
  } else {
    ht->reset();
  }
}

ZEND_API void _zend_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, void *tmp, uint size, int overwrite ZEND_FILE_LINE_DC) {
  using namespace HPHP;
  always_assert(source->isProxyArray());
  ProxyArray * proxy_source = static_cast<ProxyArray*>(source);
  target->plusEq(source);
  if (pCopyConstructor) {
    for (auto pos = source->iter_begin();
        pos != ArrayData::invalid_index;
        pos = source->iter_advance(pos))
    {
      pCopyConstructor(proxy_source->proxyGetValueRef(pos));
    }
  }
}

ZEND_API int zend_hash_index_exists(const HashTable *ht, ulong h) {
  return ht->exists(h);
}

ZEND_API int zend_hash_num_elements(const HashTable *ht) {
  return ht->size();
}

ZEND_API void zend_hash_clean(HashTable *ht) {
  for (HPHP::ArrayIter it(ht); !it.end(); it.next()) {
    ht->remove(it.secondRef(), false);
  }
}

ZEND_API void zend_hash_copy(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, void *tmp, uint size) {
  using namespace HPHP;
  always_assert(source->isProxyArray());
  ProxyArray * source_proxy = static_cast<ProxyArray*>(source);
  target->merge(source);
  if (pCopyConstructor) {
    for (auto pos = source->iter_begin();
        pos != ArrayData::invalid_index;
        pos = source->iter_advance(pos))
    {
      pCopyConstructor(source_proxy->proxyGetValueRef(pos));
    }
  }
}

ZEND_API ulong zend_hash_func(const char *arKey, uint nKeyLength)
{
  return zend_inline_hash_func(arKey, nKeyLength);
}

ZEND_API int zend_hash_sort(HashTable *ht, sort_func_t sort_func,
              compare_func_t compar, int renumber TSRMLS_DC) {
  assert(sort_func == zend_qsort);
  // TODO figure out how to use compar
  ht->ksort(0, true);
  return SUCCESS;
}

ZEND_API int _zend_hash_init(HashTable *ht, uint nSize,
    hash_func_t pHashFunction, dtor_func_t pDestructor,
    zend_bool persistent ZEND_FILE_LINE_DC)
{
  using namespace HPHP;
  always_assert(ht->isProxyArray());
  ProxyArray * proxy = static_cast<ProxyArray*>(ht);
  proxy->proxyInit(nSize, pDestructor, persistent);
  proxy->incRefCount();
  return SUCCESS;
}

ZEND_API void zend_hash_destroy(HashTable *ht) {
  decRefArr(ht);
}
