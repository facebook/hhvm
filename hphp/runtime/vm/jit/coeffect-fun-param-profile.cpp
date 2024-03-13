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

#include "hphp/runtime/vm/jit/coeffect-fun-param-profile.h"

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/tv-type.h"

#include "hphp/runtime/vm/func.h"

namespace HPHP::jit {

std::vector<CoeffectFunParamProfile::OptType>
CoeffectFunParamProfile::order() const {
  using OptType = CoeffectFunParamProfile::OptType;
  std::vector<std::pair<double, OptType>> elems;

  auto const add = [&] (uint32_t val, OptType t) {
    double d = (double) val / m_total;
    if (d < RO::EvalCoeffectFunParamProfileThreshold) return;
    elems.push_back({d, t});
  };
  add(m_null, OptType::Null);
  add(m_closure, OptType::Closure);
  add(m_methcaller, OptType::MethCaller);
  add(m_func, OptType::Func);
  add(m_clsmeth, OptType::ClsMeth);

  // Sort descending order
  std::sort(elems.begin(), elems.end(),
            [&](auto& a, auto& b) { return a.first > b.first; });

  std::vector<OptType> result;
  for (auto const& e : elems) result.push_back(e.second);
  return result;
}

void CoeffectFunParamProfile::update(const TypedValue* tv) {
  assertx(tv);
  if (tvIsNull(tv)) {
    m_null++;
  } else if (tvIsObject(tv)) {
    auto const cls = tv->m_data.pobj->getVMClass();
    if (cls->isClosureClass()) {
      m_closure++;
    } else if (cls == SystemLib::getMethCallerHelperClass()) {
      m_methcaller++;
    }
  } else if (tvIsFunc(tv)) {
    m_func++;

  } else if (tvIsClsMeth(tv)) {
    m_clsmeth++;
  }
  m_total++;
}

void CoeffectFunParamProfile::reduce(CoeffectFunParamProfile& l,
                                     const CoeffectFunParamProfile& r) {
  l.m_null += r.m_null;
  l.m_closure += r.m_closure;
  l.m_methcaller += r.m_methcaller;
  l.m_func += r.m_func;
  l.m_clsmeth += r.m_clsmeth;
  l.m_total += r.m_total;
}

std::string CoeffectFunParamProfile::toString() const {
  return folly::to<std::string>(
    "null:", m_null, ",",
    "closure:", m_closure, ",",
    "methcaller:", m_methcaller, ",",
    "func:", m_func, ",",
    "clsmeth:", m_clsmeth, ",",
    "total:", m_total
  );
}

folly::dynamic CoeffectFunParamProfile::toDynamic() const {
  return folly::dynamic::object("null", m_null)
                               ("closure", m_closure)
                               ("methcaller", m_methcaller)
                               ("func", m_func)
                               ("clsmeth", m_clsmeth)
                               ("total", m_total)
                               ;
}

} // HPHP::jit
