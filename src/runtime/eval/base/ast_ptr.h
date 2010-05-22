/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_AST_PTR_H__
#define __HPHP_AST_PTR_H__

#include <runtime/base/util/smart_ptr.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_AST_PTR(cls)                            \
  class cls;                                            \
  typedef AstPtr<cls> cls##Ptr

template<class T>
class AstPtr : public SmartPtr<T> {
public:
  AstPtr() : SmartPtr<T>() {}
  template<class Y>
  AstPtr(Y v) : SmartPtr<T>(v) {}

  operator bool() const { return this->m_px; }

  template<class Y>
  AstPtr &operator=(Y px) {
    SmartPtr<T>::operator=(px);
    return *this;
  }
};

///////////////////////////////////////////////////////////////////////////////
}
}

#endif // __HPHP_AST_PTR_H__
