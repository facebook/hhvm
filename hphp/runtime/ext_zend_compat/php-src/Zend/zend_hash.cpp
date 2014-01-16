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
#include "hphp/runtime/ext_hhvm/ext_zend_compat.h"
#include "hphp/util/safesort.h"
#include "hphp/util/assertions.h"

ZEND_API int _zend_hash_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  if (nKeyLength <= 0) {
    return FAILURE;
  }
  assert(nDataSize == sizeof(void*));
  assert(arKey[nKeyLength - 1] == '\0');

  if ((flag & HASH_ADD) && zend_hash_exists(ht, arKey, nKeyLength)) {
    return FAILURE;
  }
  HPHP::String key(arKey, nKeyLength - 1, HPHP::CopyString);
  ht->zSet(key.get(), (*(zval**)pData));

  if (pDest) {
    *pDest = pData;
  }
  return SUCCESS;
}

ZEND_API int _zend_hash_quick_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  return _zend_hash_add_or_update(ht, arKey, nKeyLength, pData, nDataSize, pDest, flag ZEND_FILE_LINE_CC);
}

ZEND_API int _zend_hash_index_update_or_next_insert(HashTable *ht, ulong h, void *pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  assert(nDataSize == sizeof(void*));
  if (flag & HASH_NEXT_INSERT) {
    ht->zAppend(*(zval**)pData);
    return SUCCESS;
  }

  if (flag & HASH_ADD && zend_hash_index_exists(ht, h)) {
    return FAILURE;
  }

  ht->zSet(h, (*(zval**)pData));
  return SUCCESS;
}

ZEND_API void zend_hash_apply_with_argument(HashTable *ht, apply_func_arg_t apply_func, void * arg TSRMLS_DC) {
  for (HPHP::ArrayIter it(ht); it; ++it) {
    zval* data = it.zSecond();
		int result = apply_func(&data, arg);

		if (result & ZEND_HASH_APPLY_REMOVE) {
      not_implemented();
		}
		if (result & ZEND_HASH_APPLY_STOP) {
			break;
		}
	}
}

ZEND_API void zend_hash_apply_with_arguments(HashTable *ht TSRMLS_DC, apply_func_args_t apply_func, int num_args, ...) {
	va_list args;
	zend_hash_key hash_key;

  for (HPHP::ArrayIter it(ht); it; ++it) {
		int result;
		va_start(args, num_args);
    if (it.first().isInteger()) {
      hash_key.arKey = "";
      hash_key.nKeyLength = 0;
      hash_key.h = it.first().asInt64Val();
    } else {
      assert(it.first().isString());
      hash_key.arKey = it.first().asCStrRef().data();
      hash_key.nKeyLength = it.first().asCStrRef().size() + 1;
      hash_key.h = 0;
    }
    zval* data = it.zSecond();
		result = apply_func(&data, num_args, args, &hash_key);

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
  if (nKeyLength == 0) {
    ht->remove(h, false);
  } else {
    assert(arKey[nKeyLength - 1] == '\0');
    HPHP::String key(arKey, nKeyLength - 1, HPHP::CopyString);
    ht->remove(key.get(), false);
  }
  return SUCCESS;
}

ZEND_API ulong zend_get_hash_value(const char *arKey, uint nKeyLength)
{
  return zend_inline_hash_func(arKey, nKeyLength);
}

/**
 * Something to try and keep these TypedValue**s alive.
 */
class TypedValueHolder {
  public:
    HPHP::TypedValue** getNext() {
      return &m_data[m_pos++ & 15];
    };
  private:
    uint8_t m_pos;
    HPHP::TypedValue* m_data[16];
};

ZEND_API int zend_hash_find(const HashTable *ht, const char *arKey, uint nKeyLength, void **pData) {
  if (nKeyLength <= 0) {
    return FAILURE;
  }
  assert(arKey[nKeyLength - 1] == '\0');
  HPHP::String key(arKey, nKeyLength - 1, HPHP::CopyString);
  auto val = ht->nvGet(key.get());
  if (!val) {
    return FAILURE;
  }
  HPHP::zBoxAndProxy(val);
  auto p = (zval***)pData;
  *p = &val->m_data.pref;
  return SUCCESS;
}

ZEND_API int zend_hash_quick_find(const HashTable *ht, const char *arKey, uint nKeyLength, ulong h, void **pData) {
  return zend_hash_find(ht, arKey, nKeyLength, pData);
}

ZEND_API int zend_hash_index_find(const HashTable *ht, ulong h, void **pData) {
  auto val = ht->nvGet(h);
  if (!val) {
    return FAILURE;
  }
  HPHP::zBoxAndProxy(val);
  auto p = (zval***)pData;
  *p = &val->m_data.pref;
  return SUCCESS;
}

ZEND_API int zend_hash_exists(const HashTable *ht, const char *arKey, uint nKeyLength)
{
  assert(arKey[nKeyLength - 1] == '\0');
  return ht->exists(
    HPHP::StrNR(HPHP::String(arKey, nKeyLength - 1, HPHP::CopyString).get())
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
  HashPosition hp = ht->getPosition();
  if (pos) {
    hp = *pos;
  }
  if (hp == ht->invalid_index) {
    return HASH_KEY_NON_EXISTENT;
  }

  HPHP::Variant key = ht->getKey(hp);
  if (key.isString()) {
    HPHP::String keyStr = key.toString();
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
  HashPosition hp = ht->getPosition();
  if (pos) {
    hp = *pos;
  }
  if (hp == ht->invalid_index) {
    return HASH_KEY_NON_EXISTENT;
  }

  HPHP::Variant key = ht->getKey(hp);
  HPHP::DataType type = key.getType();
  switch (type) {
    case HPHP::KindOfInt64:
      return HASH_KEY_IS_LONG;
    case HPHP::KindOfString:
    case HPHP::KindOfStaticString:
      return HASH_KEY_IS_STRING;
    default:
      not_reached();
  }
}

ZEND_API int zend_hash_get_current_data_ex(HashTable *ht, void **pData, HashPosition *pos) {
  HashPosition hp = ht->getPosition();
  if (pos) {
    hp = *pos;
  }
  if (hp == ht->invalid_index) {
    return FAILURE;
  }
  auto val = ht->nvGetValueRef(hp);
  if (!val) {
    return FAILURE;
  }
  HPHP::zBoxAndProxy(val);
  auto p = (zval***)pData;
  *p = &val->m_data.pref;
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
  target->plusEq(source); // XXX: can this COW?
  for (HPHP::ArrayIter it(source); !it.end(); it.next()) {
    auto tv = (HPHP::TypedValue*)it.secondRef().asTypedValue();
    HPHP::zBoxAndProxy(tv);
    pCopyConstructor((void*)(&tv->m_data.pref));
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
  target->merge(source);
  for (HPHP::ArrayIter it(source); !it.end(); it.next()) {
    const void *tv = it.secondRef().asTypedValue();
    pCopyConstructor(const_cast<void*>(tv));
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

ZEND_API int _zend_hash_init(HashTable *ht, uint nSize, hash_func_t pHashFunction, dtor_func_t pDestructor, zend_bool persistent ZEND_FILE_LINE_DC) {
  ht->incRefCount();
  return SUCCESS;
}

ZEND_API void zend_hash_destroy(HashTable *ht) {
  decRefArr(ht);
}
