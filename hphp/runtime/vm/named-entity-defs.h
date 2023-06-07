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

#pragma once

#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/preclass.h"
#include "hphp/runtime/ext/core/ext_core_closure.h"

namespace HPHP {

template<class Fn>
void NamedType::foreach_name(Fn fn) {
  if (auto table = NamedType::types()) {
    for (auto n = table->begin(); n != table->end(); ++n) {
      fn(n->second);
    }
  }
}

template<class Fn>
void NamedType::foreach_class(Fn fn) {
  foreach_name([&](NamedType& name) {
    for (auto cls = name.clsList(); cls; cls = cls->m_next) {
      for (auto const& clone : cls->scopedClones()) {
        fn(clone.second.get());
      }
      fn(cls);
    }
  });
}

template<class Fn>
void NamedType::foreach_cached_class(Fn fn) {
  foreach_name([&](NamedType& name) {
    if (auto cls = name.clsList()) {
      if (auto cached = cls->getCached()) {
        if (cls->parent() != c_Closure::classof()) {
          fn(cached);
        }
      }
    }
  });
}

template<class Fn>
void NamedFunc::foreach_name(Fn fn) {
  if (auto table = NamedFunc::funcs()) {
    for (auto n = table->begin(); n != table->end(); ++n) {
      fn(n->second);
    }
  }
}

template<class Fn>
void NamedFunc::foreach_cached_func(Fn fn) {
  foreach_name([&](NamedFunc& name) {
    if (auto func = name.getCachedFunc()) {
      fn(func);
    }
  });
}

template<class T>
const char* NamedType::checkSameName() {
  if (!std::is_same<T, PreTypeAlias>::value && getCachedTypeAlias()) {
    return "type";
  } else if (!std::is_same<T, PreClass>::value && getCachedClass()) {
    return "class";
  }
  return nullptr;
}

}
