/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_NAMED_ENTITY_DEFS_H_
#define incl_HPHP_VM_NAMED_ENTITY_DEFS_H_

#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"

namespace HPHP {

template<class Fn>
void NamedEntity::foreach_name(Fn fn) {
  if (auto table = NamedEntity::table()) {
    for (auto n = table->begin(); n != table->end(); ++n) {
      fn(n->second);
    }
  }
}

template<class Fn>
void NamedEntity::foreach_class(Fn fn) {
  foreach_name([&](NamedEntity& name) {
    for (auto cls = name.clsList(); cls; cls = cls->m_nextClass) {
      for (auto const& clone : cls->scopedClones()) {
        fn(clone.second.get());
      }
      fn(cls);
    }
  });
}

template<class Fn>
void NamedEntity::foreach_cached_class(Fn fn) {
  foreach_name([&](NamedEntity& name) {
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
void NamedEntity::foreach_cached_func(Fn fn) {
  foreach_name([&](NamedEntity& name) {
    if (auto func = name.getCachedFunc()) {
      fn(func);
    }
  });
}

}
#endif // incl_HPHP_VM_NAMED_ENTITY_DEFS_H_
