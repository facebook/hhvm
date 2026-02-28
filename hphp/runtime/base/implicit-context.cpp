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

#include "hphp/runtime/base/implicit-context.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/type-object.h"

#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

rds::Link<ObjectData*, rds::Mode::Normal> ImplicitContext::activeCtx;

rds::Link<ObjectData*, rds::Mode::Normal> ImplicitContext::emptyCtx;

void ImplicitContext::setActive(Object&& ctx) {
  assertx(*ImplicitContext::activeCtx);
  (*ImplicitContext::activeCtx)->decRefCount();
  *ImplicitContext::activeCtx = ctx.detach();
}

ImplicitContext::Saver::Saver() {
  m_context = *ImplicitContext::activeCtx;
  setActive(Object{*ImplicitContext::emptyCtx});
}

ImplicitContext::Saver::~Saver() {
  setActive(Object{m_context});
}

Variant ImplicitContext::getBlameVectors() {
  /*
   * TODO: once confirmed by privacy org that we don't need
   * any blame mechanism, rip out this function
  */
  return init_null_variant;
}

} // namespace HPHP
