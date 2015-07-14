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
#include "hphp/hhbbc/interp-state.h"

#include <string>

#include <folly/Format.h>
#include <folly/Conv.h>

#include "hphp/util/match.h"
#include "hphp/hhbbc/analyze.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

template<class JoinOp>
bool merge_into(Iter& dst, const Iter& src, JoinOp join) {
  return match<bool>(
    dst,
    [&] (UnknownIter) { return false; },
    [&] (TrackedIter& diter) {
      return match<bool>(
        src,
        [&] (UnknownIter) {
          dst = UnknownIter {};
          return true;
        },
        [&] (const TrackedIter& siter) {
          auto k1 = join(diter.kv.first, siter.kv.first);
          auto k2 = join(diter.kv.second, siter.kv.second);
          auto const changed = k1 != diter.kv.first || k2 != diter.kv.second;
          diter.kv = std::make_pair(std::move(k1), std::move(k2));
          return changed;
        }
      );
    }
  );
}

std::string show(const Iter& iter) {
  return match<std::string>(
    iter,
    [&] (UnknownIter) { return "unk"; },
    [&] (const TrackedIter& ti) {
      return folly::format("{}, {}", show(ti.kv.first),
        show(ti.kv.second)).str();
    }
  );
}

}

//////////////////////////////////////////////////////////////////////

bool operator==(const ActRec& a, const ActRec& b) {
  auto const fsame =
    a.func.hasValue() != b.func.hasValue() ? false :
    a.func.hasValue() ? a.func->same(*b.func) :
    true;
  return a.kind == b.kind && fsame;
}

bool operator==(const State& a, const State& b) {
  return a.initialized == b.initialized &&
    a.thisAvailable == b.thisAvailable &&
    a.locals == b.locals &&
    a.stack == b.stack &&
    a.fpiStack == b.fpiStack;
}

bool operator!=(const ActRec& a, const ActRec& b) { return !(a == b); }
bool operator!=(const State& a, const State& b)   { return !(a == b); }

State without_stacks(const State& src) {
  auto ret          = State{};
  ret.initialized   = src.initialized;
  ret.thisAvailable = src.thisAvailable;
  ret.locals        = src.locals;
  ret.iters         = src.iters;
  return ret;
}

//////////////////////////////////////////////////////////////////////

PropertiesInfo::PropertiesInfo(const Index& index,
                               Context ctx,
                               ClassAnalysis* cls)
  : m_cls(cls)
{
  if (m_cls == nullptr && ctx.cls != nullptr) {
    m_privateProperties = index.lookup_private_props(ctx.cls);
    m_privateStatics    = index.lookup_private_statics(ctx.cls);
  }
}

PropState& PropertiesInfo::privateProperties() {
  if (m_cls != nullptr) {
    return m_cls->privateProperties;
  }
  return m_privateProperties;
}

PropState& PropertiesInfo::privateStatics() {
  if (m_cls != nullptr) {
    return m_cls->privateStatics;
  }
  return m_privateStatics;
}

const PropState& PropertiesInfo::privateProperties() const {
  return const_cast<PropertiesInfo*>(this)->privateProperties();
}

const PropState& PropertiesInfo::privateStatics() const {
  return const_cast<PropertiesInfo*>(this)->privateStatics();
}

//////////////////////////////////////////////////////////////////////

void merge_closure_use_vars_into(ClosureUseVarMap& dst,
                                 borrowed_ptr<php::Class> clo,
                                 std::vector<Type> types) {
  auto& current = dst[clo];
  if (current.empty()) {
    current = std::move(types);
    return;
  }

  assert(types.size() == current.size());
  for (auto i = uint32_t{0}; i < current.size(); ++i) {
    current[i] = union_of(std::move(current[i]), std::move(types[i]));
  }
}

bool widen_into(PropState& dst, const PropState& src) {
  assert(dst.size() == src.size());

  auto changed = false;

  auto dstIt = begin(dst);
  auto srcIt = begin(src);
  for (; dstIt != end(dst); ++dstIt, ++srcIt) {
    assert(srcIt != end(src));
    assert(srcIt->first == dstIt->first);
    auto const newT = widening_union(dstIt->second, srcIt->second);
    if (newT != dstIt->second) {
      changed = true;
      dstIt->second = newT;
    }
  }

  return changed;
}

bool merge_into(ActRec& dst, const ActRec& src) {
  if (dst.kind != src.kind) {
    dst = ActRec { FPIKind::Unknown };
    return true;
  }
  if (dst != src) {
    dst = ActRec { src.kind };
    return true;
  }
  return false;
}

template<class JoinOp>
bool merge_impl(State& dst, const State& src, JoinOp join) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  assert(src.initialized);
  assert(dst.locals.size() == src.locals.size());
  assert(dst.iters.size() == src.iters.size());
  assert(dst.stack.size() == src.stack.size());
  assert(dst.fpiStack.size() == src.fpiStack.size());

  if (src.unreachable) {
    // If we're coming from unreachable code and the dst is already
    // initialized, it doesn't change the dst (whether it is reachable or not).
    return false;
  }
  if (dst.unreachable) {
    // If we're going to code currently believed to be unreachable, take the
    // src state, and consider the dest state changed only if the source state
    // was reachable.
    dst = src;
    return !src.unreachable;
  }

  auto changed = false;

  auto const available = dst.thisAvailable && src.thisAvailable;
  if (available != dst.thisAvailable) {
    changed = true;
    dst.thisAvailable = available;
  }

  for (auto i = size_t{0}; i < dst.stack.size(); ++i) {
    auto newT = join(dst.stack[i], src.stack[i]);
    if (dst.stack[i] != newT) {
      changed = true;
      dst.stack[i] = std::move(newT);
    }
  }

  for (auto i = size_t{0}; i < dst.locals.size(); ++i) {
    auto newT = join(dst.locals[i], src.locals[i]);
    if (dst.locals[i] != newT) {
      changed = true;
      dst.locals[i] = std::move(newT);
    }
  }

  for (auto i = size_t{0}; i < dst.iters.size(); ++i) {
    if (merge_into(dst.iters[i], src.iters[i], join)) {
      changed = true;
    }
  }

  for (auto i = size_t{0}; i < dst.fpiStack.size(); ++i) {
    if (merge_into(dst.fpiStack[i], src.fpiStack[i])) {
      changed = true;
    }
  }

  return changed;
}

bool merge_into(State& dst, const State& src) {
  return merge_impl(dst, src, union_of);
}

bool widen_into(State& dst, const State& src) {
  return merge_impl(dst, src, widening_union);
}

//////////////////////////////////////////////////////////////////////

static std::string fpiKindStr(FPIKind k) {
  switch (k) {
  case FPIKind::Unknown:     return "unk";
  case FPIKind::CallableArr: return "arr";
  case FPIKind::Func:        return "func";
  case FPIKind::Ctor:        return "ctor";
  case FPIKind::ObjMeth:     return "objm";
  case FPIKind::ClsMeth:     return "clsm";
  case FPIKind::ObjInvoke:   return "invoke";
  }
  not_reached();
}

std::string show(const ActRec& a) {
  return folly::to<std::string>(
    "ActRec { ",
    fpiKindStr(a.kind),
    a.cls || a.func ? ": " : "",
    a.cls ? show(*a.cls) : "",
    a.cls && a.func ? "::" : "",
    a.func ? show(*a.func) : "",
    " }"
  );
}

std::string state_string(const php::Func& f, const State& st) {
  std::string ret;

  if (!st.initialized) {
    ret = "state: uninitialized\n";
    return ret;
  }

  folly::format(&ret, "state{}:\n", st.unreachable ? " (unreachable)" : "");
  if (f.cls) {
    folly::format(&ret, "thisAvailable({})\n", st.thisAvailable);
  }

  for (auto i = size_t{0}; i < st.locals.size(); ++i) {
    folly::format(&ret, "{: <8} :: {}\n",
      local_string(borrow(f.locals[i])),
      show(st.locals[i])
    );
  }

  for (auto i = size_t{0}; i < st.iters.size(); ++i) {
    folly::format(&ret, "iter {: <2}   :: {}\n", i, show(st.iters[i]));
  }

  for (auto i = size_t{0}; i < st.stack.size(); ++i) {
    folly::format(&ret, "stk[{:02}] :: {}\n",
      i,
      show(st.stack[i])
    );
  }

  return ret;
}

std::string property_state_string(const PropertiesInfo& props) {
  std::string ret;

  for (auto& kv : props.privateProperties()) {
    ret += folly::format("$this->{: <14} :: {}\n",
      kv.first,
      show(kv.second)
    ).str();
  }
  for (auto& kv : props.privateStatics()) {
    ret += folly::format("self::${: <14} :: {}\n",
      kv.first,
      show(kv.second)
    ).str();
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
