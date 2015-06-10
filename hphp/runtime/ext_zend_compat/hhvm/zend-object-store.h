/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_ZEND_OBJECT_STORE_H_
#define incl_ZEND_OBJECT_STORE_H_

#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"
#include <vector>
#include "hphp/runtime/ext_zend_compat/hhvm/zend-request-local.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_objects_API.h"

namespace HPHP {

struct ZendObjectStore final : RequestEventHandler {
  static ZendObjectStore & getInstance() {
    return *tl_instance;
  }

  ZendObjectStore()
    : m_free_list_head(0)
  {}

  void requestInit() override {}
  void requestShutdown() override;

  // Defer shutdown until after other requestShutdown hooks are done
  // freeing their objects.
  int priority() const override {
    return 10;
  }

  zend_object_handle insertObject(void *object,
      zend_objects_store_dtor_t dtor,
      zend_objects_free_object_storage_t free_storage,
      zend_objects_store_clone_t clone);

  void * getObject(zend_object_handle handle);
  void freeObject(zend_object_handle handle);
  zend_object_handle cloneObject(zend_object_handle handle);

private:
  static __thread RequestLocal<ZendObjectStore> tl_instance;

  std::vector<zend_object_store_bucket> m_store;
  zend_object_handle m_free_list_head;
};

}

#endif
