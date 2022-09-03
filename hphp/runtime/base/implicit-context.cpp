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

Object ImplicitContext::setByValue(Object&& obj) {
  auto const prev = *ImplicitContext::activeCtx;
  *ImplicitContext::activeCtx = obj.detach();
  return Object::attach(prev);
}

std::string ImplicitContext::stateToString(ImplicitContext::State state) {
  switch (state) {
    case ImplicitContext::State::Value:
      return "a value";
    case ImplicitContext::State::Inaccessible:
      return "an inaccessible";
    case ImplicitContext::State::SoftInaccessible:
      return "a soft inaccessible";
    case ImplicitContext::State::SoftSet:
      return "a soft set";
  }
  not_reached();
}

bool ImplicitContext::isStateSoft(ImplicitContext::State state) {
  switch (state) {
    case ImplicitContext::State::Value:
    case ImplicitContext::State::Inaccessible:
      return false;
    case ImplicitContext::State::SoftInaccessible:
    case ImplicitContext::State::SoftSet:
      return true;
  }
  not_reached();
}

Variant ImplicitContext::getBlameVectors() {
  auto const obj = *ImplicitContext::activeCtx;
  if (!obj) return init_null_variant;
  auto const context = Native::data<ImplicitContext>(obj);
  if (context->m_blameFromSoftInaccessible.empty() &&
      context->m_blameFromSoftSet.empty()) {
    // Optimization to not waste memory in case they are both empty
    return init_null_variant;
  }

  VecInit ret(2);
  {
    VecInit inaccessible(context->m_blameFromSoftInaccessible.size());
    for (auto const& s : context->m_blameFromSoftInaccessible) {
      inaccessible.append(String{const_cast<StringData*>(s)});
    }
    ret.append(inaccessible.toArray());
  }
  {
    VecInit set(context->m_blameFromSoftSet.size());
    for (auto const& s : context->m_blameFromSoftSet) {
      set.append(String{const_cast<StringData*>(s)});
    }
    ret.append(set.toArray());
  }
  return ret.toArray();
}

} // namespace HPHP
