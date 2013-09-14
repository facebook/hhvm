#include "zend_hash.h"

ZEND_API int _zend_hash_add_or_update(HashTable *ht, const char *arKey, uint nKeyLength, zval **pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  if (nKeyLength <= 0) {
    return FAILURE;
  }
  assert(arKey[nKeyLength - 1] == '\0');
  auto key = HPHP::makeStaticString(arKey, nKeyLength - 1);

  if ((flag & HASH_ADD) && ht->exists(key)) {
    return FAILURE;
  }
  ht->set(key, tvAsVariant(*pData), false);
  return SUCCESS;
}

ZEND_API int _zend_hash_index_update_or_next_insert(HashTable *ht, ulong h, zval **pData, uint nDataSize, void **pDest, int flag ZEND_FILE_LINE_DC) {
  if (flag & HASH_NEXT_INSERT) {
    ht->append(tvAsVariant(*pData), false);
    return SUCCESS;
  }

  if (flag & HASH_ADD && ht->exists(h)) {
    return FAILURE;
  }

  ht->set(h, tvAsVariant(*pData), false);
  return SUCCESS;
}

ZEND_API int zend_hash_del_key_or_index(HashTable *ht, const char *arKey, uint nKeyLength, ulong h, int flag) {
  if (nKeyLength == 0) {
    ht->remove(h, false);
  } else {
    assert(arKey[nKeyLength - 1] == '\0');
    auto key = HPHP::makeStaticString(arKey, nKeyLength - 1);
    ht->remove(key, false);
  }
  return SUCCESS;
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
static __thread TypedValueHolder tvHolder;

ZEND_API int zend_hash_find(const HashTable *ht, const char *arKey, uint nKeyLength, zval ***pData) {
  if (nKeyLength <= 0) {
    return FAILURE;
  }
  assert(arKey[nKeyLength - 1] == '\0');
  auto key = HPHP::makeStaticString(arKey, nKeyLength - 1);

  *pData = tvHolder.getNext();
  **pData = ht->nvGet(key);
  return **pData == nullptr ? FAILURE : SUCCESS;
}

ZEND_API int zend_hash_index_find(const HashTable *ht, ulong h, void **pData) {
  *pData = ht->nvGet(h);
  return *pData == nullptr ? FAILURE : SUCCESS;
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
      *str_length = keyStr.size();
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

ZEND_API int zend_hash_get_current_data_ex(HashTable *ht, zval ***pData, HashPosition *pos) {
  HashPosition hp = ht->getPosition();
  if (pos) {
    hp = *pos;
  }
  if (hp == ht->invalid_index) {
    return FAILURE;
  }
  *pData = tvHolder.getNext();
  **pData = ht->nvGetValueRef(hp);
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
  target->plus(source, false);
  for (HPHP::ArrayIter it(source); !it.end(); it.next()) {
    const void *tv = it.secondRef().asTypedValue();
    pCopyConstructor(const_cast<void*>(tv));
  }
}
