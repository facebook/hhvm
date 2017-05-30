/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_ZEND_EXCEPTION_STORE
#define incl_HPHP_ZEND_EXCEPTION_STORE

#include <exception>
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {

struct ZendExceptionStore final : RequestEventHandler {
  static ZendExceptionStore& getInstance() {
    return *tl_instance;
  }

  void set(Object&& e) {
    m_obj = std::move(e);
  }
  void clear() {
    m_obj.reset();
  }
  void requestInit() override {
  }
  void requestShutdown() override {
    clear();
  }
  bool empty() {
    return !m_obj;
  }
  void rethrow() {
    if (m_obj) {
      throw_object(std::move(m_obj));
    }
  }

private:
  Object m_obj;
  DECLARE_STATIC_REQUEST_LOCAL(ZendExceptionStore, tl_instance);
};

}
#endif
