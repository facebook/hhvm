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

#ifndef incl_HPHP_TRACER_H
#define incl_HPHP_TRACER_H

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/trace.h"

#include <functional>
#include <queue>
#include <set>
#include <vector>

namespace HPHP {

template<typename Accum>
struct Tracer {
  typedef std::function<void(TypedValue *,             Accum&)> NodeFunc;
  typedef std::function<void(TypedValue *,TypedValue *,Accum&)> EdgeFunc;

  static void trace(TypedValue *self,
                    NodeFunc atNode,
                    EdgeFunc atEdge,
                    Accum& accumulator) {
    std::set<TypedValue *> visited;
    traceImpl(self, visited, atNode, atEdge, accumulator);
  }

  static void traceAll(std::vector<TypedValue *> &ts,
                       NodeFunc atNode,
                       EdgeFunc atEdge,
                       Accum& accumulator) {
    std::set<TypedValue *> visited;
    for (TypedValue *t : ts) {
      traceImpl(t, visited, atNode, atEdge, accumulator);
    }
  }

private:
  static void traceImpl(TypedValue *self,
                        std::set<TypedValue *> &visited,
                        NodeFunc atNode,
                        EdgeFunc atEdge,
                        Accum& accumulator) {
    std::queue<TypedValue *> tvs;
    const auto traceSingle = [&](TypedValue *parent, TypedValue *child) {
      atEdge(parent, child, accumulator);
      tvs.push(child);
    };
    const auto traceMultiple = [&](TypedValue *parent) {
      std::vector<TypedValue *> children;
      switch(parent->m_type) {
        case KindOfArray:
          parent->m_data.parr->getChildren(children);
          break;
        case KindOfObject:
          parent->m_data.pobj->getChildren(children);
          break;
        default:
          not_reached();
      }
      for (TypedValue *child : children) {
        traceSingle(parent, child);
      }
    };

    tvs.push(self);
    while (!tvs.empty()) {
      TypedValue *top = tvs.front();
      tvs.pop();
      if (visited.find(top) != visited.end()) {
        continue;
      }
      visited.insert(top);
      atNode(top, accumulator);
      switch (top->m_type) {
        case KindOfUninit:
        case KindOfNull:
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfStaticString:
        case KindOfString:
        case KindOfResource:
          break;
        case KindOfArray:
        case KindOfObject:
          traceMultiple(top);
          break;
        case KindOfRef:
          traceSingle(top, top->m_data.pref->tv());
          break;
        case KindOfIndirect:
          traceSingle(top, top->m_data.pind);
          break;
        default:
          not_reached();
      }
    }
  }
};

}

#undef TRACE_SINGLE_CHILD
#undef TRACE_MULTIPLE_CHILDREN

#endif
