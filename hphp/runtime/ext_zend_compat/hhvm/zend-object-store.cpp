/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext_zend_compat/hhvm/zend-object-store.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

IMPLEMENT_REQUEST_LOCAL(ZendObjectStore, ZendObjectStore::tl_instance);

void ZendObjectStore::requestShutdown() {
  m_store.clear();
  m_free_list_head = 0;
}

zend_object_handle ZendObjectStore::insertObject(void *object,
    zend_objects_store_dtor_t dtor,
    zend_objects_free_object_storage_t free_storage,
    zend_objects_store_clone_t clone)
{
  zend_object_handle index;
  if (m_free_list_head) {
    index = m_free_list_head;
    m_free_list_head = m_store.at(index).bucket.free_list.next;
  } else {
    index = m_store.size();
    // Don't use handle zero
    if (index == 0) {
      index++;
    }
    m_store.resize(index + 1);
  }

  zend_object_store_bucket & bucket = m_store.at(index);
  bucket.destructor_called = 0;
  bucket.valid = 1;
  bucket.apply_count = 0;

  auto & obj = bucket.bucket.obj;
  obj.refcount = 1; // unused
  obj.object = object;
  obj.dtor = dtor; // unused
  obj.free_storage = free_storage;
  obj.clone = clone;
  obj.handlers = nullptr; // unused
  return index;

}

void * ZendObjectStore::getObject(zend_object_handle handle) {
  return m_store.at(handle).bucket.obj.object;
}

void ZendObjectStore::freeObject(zend_object_handle handle) {
  zend_object_store_bucket & bucket = m_store.at(handle);
  assert(bucket.valid);
  auto & obj = bucket.bucket.obj;

  if (obj.free_storage) {
    TSRMLS_FETCH();
    HPHP::VMRegAnchor _;
    obj.free_storage(obj.object TSRMLS_CC);
  }

  bucket.bucket.free_list.next = m_free_list_head;
  m_free_list_head = handle;
  bucket.valid = 0;
}

zend_object_handle ZendObjectStore::cloneObject(zend_object_handle handle) {
  TSRMLS_FETCH();
  zend_object_store_bucket & bucket = m_store.at(handle);
  assert(bucket.valid);
  auto & obj = bucket.bucket.obj;
  if (obj.clone == nullptr) {
    return 0;
  }
  void * new_object = nullptr;
  obj.clone(obj.object, &new_object TSRMLS_CC);
  assert(bucket.valid);
  return insertObject(new_object, obj.dtor, obj.free_storage, obj.clone);
}

}
