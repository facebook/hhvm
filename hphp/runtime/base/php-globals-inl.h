/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_PHP_GLOBALS_INL_H_
#define incl_HPHP_PHP_GLOBALS_INL_H_

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void php_global_set(const StaticString& name, Variant var) {
  auto& lval = tvAsVariant(g_context->m_globalVarEnv->lookupAdd(name.get()));
  lval = std::move(var);
}

inline void php_global_bind(const StaticString& name, Variant& v) {
  auto to = g_context->m_globalVarEnv->lookupAdd(name.get());
  tvBind(v.asTypedValue(), to);
}

inline Variant php_global_exchange(const StaticString& name, Variant newV) {
  Variant ret;
  auto& lval = tvAsVariant(g_context->m_globalVarEnv->lookupAdd(name.get()));
  ret = lval;
  lval = std::move(newV);
  return ret;
}

inline Variant php_global(const StaticString& name) {
  auto const tv = g_context->m_globalVarEnv->lookup(name.get());
  // Note: Variant is making unnecessary KindOfUninit checks here.
  return tv ? tvAsCVarRef(tv) : Variant();
}

//////////////////////////////////////////////////////////////////////

}


#endif
