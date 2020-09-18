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
#pragma once

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/execution-context.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

inline void php_global_set(const StaticString& name, Variant var) {
  variant_ref lval{g_context->m_globalNVTable->lookupAdd(name.get())};
  lval = std::move(var);
}

inline Variant php_global_exchange(const StaticString& name, Variant newV) {
  variant_ref lval{g_context->m_globalNVTable->lookupAdd(name.get())};
  Variant ret{lval};
  lval = std::move(newV);
  return ret;
}

inline Variant php_global(const StaticString& name) {
  auto const tv = g_context->m_globalNVTable->lookup(name.get());
  // Note: Variant is making unnecessary KindOfUninit checks here.
  return tv ? Variant{variant_ref{tv}} : uninit_null();
}

//////////////////////////////////////////////////////////////////////

}


