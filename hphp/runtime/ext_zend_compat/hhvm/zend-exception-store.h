/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

struct ZendExceptionStore final : RequestEventHandler {
  static ZendExceptionStore& getInstance() {
    return *tl_instance;
  }

  template <class E> void set(E e) {
    m_ptr = std::make_exception_ptr(e);
  }
  void setPointer(std::exception_ptr ptr) {
    m_ptr = ptr;
  }
  void clear() {
    m_ptr = nullptr;
  }
  void requestInit() override {
  }
  void requestShutdown() override {
    clear();
  }
  std::exception_ptr get() {
    return m_ptr;
  }
  bool empty() {
    return !m_ptr;
  }
  void rethrow() {
    if (m_ptr) {
      std::exception_ptr p = get();
      clear();
      std::rethrow_exception(p);
    }
  }

private:
  std::exception_ptr m_ptr;
  DECLARE_STATIC_REQUEST_LOCAL(ZendExceptionStore, tl_instance);
};

}
#endif
