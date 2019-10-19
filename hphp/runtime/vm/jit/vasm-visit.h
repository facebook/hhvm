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

#ifndef incl_HPHP_JIT_VASM_VISIT_H_
#define incl_HPHP_JIT_VASM_VISIT_H_

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/type-traits.h"

#include <boost/dynamic_bitset.hpp>

#include <utility>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

template <class F, class A1, class A2>
auto invoke(F&& f, A1&& a1, A2 && /*a2*/)
  -> decltype(std::forward<F>(f)(std::forward<A1>(a1))) {
  return std::forward<F>(f)(std::forward<A1>(a1));
}

template<class F, class A1, class A2>
auto invoke(F&& f, A1&& a1, A2&& a2)
  -> decltype(std::forward<F>(f)(std::forward<A1>(a1), std::forward<A2>(a2))) {
  return std::forward<F>(f)(std::forward<A1>(a1), std::forward<A2>(a2));
}

}

///////////////////////////////////////////////////////////////////////////////

template<class R, class F>
typename std::enable_if<
  is_any<
    R,
    Vreg, VregSF, Vreg8, Vreg16, Vreg32, Vreg64, Vreg128, VregDbl, PhysReg
  >::value
>::type
visit(const Vunit&, R v, F f) {
  detail::invoke(f, v, width(v));
}

template<class F>
void visit(const Vunit&, Vptr p, F f) {
  if (p.base.isValid()) detail::invoke(f, p.base, width(p.base));
  if (p.index.isValid()) detail::invoke(f, p.index, width(p.index));
}

template<class F>
void visit(const Vunit& unit, Vtuple t, F f) {
  for (auto r : unit.tuples[t]) detail::invoke(f, r, width(r));
}

template<class F>
void visit(const Vunit& unit, VcallArgsId a, F f) {
  auto& args = unit.vcallArgs[a];
  for (auto r : args.args) detail::invoke(f, r, width(r));
  for (auto r : args.simdArgs) detail::invoke(f, r, width(r));
  for (auto r : args.stkArgs) detail::invoke(f, r, width(r));
}

template <class F>
void visit(const Vunit& /*unit*/, RegSet regs, F f) {
  regs.forEach([&](Vreg r) { detail::invoke(f, r, width(r)); });
}

///////////////////////////////////////////////////////////////////////////////

// NB: Unless otherwise stated, all these visitors will visit operands in the
// order they're defined in the VASM_OPCODES macro.

template<class Use>
void visitUses(const Vunit& unit, const Vinstr& inst, Use use) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      uses \
      break; \
    }
#define U(s)    visit(unit, i.s, use);
#define UA(s)   visit(unit, i.s, use);
#define UH(s,h) visit(unit, i.s, use);
#define UM(s)   visit(unit, i.s, use);
#define UW(s)   visit(unit, i.s, use);
#define Un
    VASM_OPCODES
#undef Un
#undef UW
#undef UM
#undef UH
#undef UA
#undef U
#undef O
  }
}

template<class Def>
void visitDefs(const Vunit& unit, const Vinstr& inst, Def def) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      defs \
      break; \
    }
#define D(d) visit(unit, i.d, def);
#define DH(d,h) visit(unit, i.d, def);
#define Dn
    VASM_OPCODES
#undef Dn
#undef DH
#undef D
#undef O
  }
}

template<typename Across>
void visitAcrosses(const Vunit& unit, const Vinstr& inst, Across across) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      uses \
      break; \
    }
#define U(s)
#define UA(s)   visit(unit, i.s, across);
#define UH(s,h)
#define UM(s)
#define UW(s)
#define Un
    VASM_OPCODES
#undef Un
#undef UW
#undef UM
#undef UH
#undef UA
#undef U
#undef O
  }
}

/*
 * visitOperands visits all operands of the given instruction, calling
 * visitor.imm(), visitor.use(), visitor.across(), and visitor.def() as defined
 * in the VASM_OPCODES macro.
 *
 * The template spew is necessary to support callers that only have a const
 * Vinstr& as well as callers with a Vinstr& that wish to mutate the
 * instruction in the visitor.
 */
template<class Tinstr, class Visitor>
typename maybe_const<Tinstr, Vinstr>::type
visitOperands(Tinstr& inst, Visitor& visitor) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      imms \
      uses \
      defs \
      break; \
    }
#define I(f) visitor.imm(i.f);
#define U(s) visitor.use(i.s);
#define UA(s) visitor.across(i.s);
#define UH(s,h) visitor.useHint(i.s, i.h);
#define UM(s) visitor.use(i.s);
#define UW(s) visitor.use(i.s);
#define D(d) visitor.def(i.d);
#define DH(d,h) visitor.defHint(i.d, i.h);
#define Inone
#define Un
#define Dn
    VASM_OPCODES
#undef Dn
#undef Un
#undef Inone
#undef DH
#undef D
#undef UW
#undef UM
#undef UH
#undef UA
#undef U
#undef I
#undef O
  }
}

///////////////////////////////////////////////////////////////////////////////

/* Build VregSets from uses/acrosses/defs of a given Vinstr */

inline VregSet usesSet(const Vunit& unit, const Vinstr& inst) {
  VregSet uses;
  visitUses(unit, inst, [&] (Vreg r) { uses.add(r); });
  return uses;
}

inline VregSet acrossesSet(const Vunit& unit, const Vinstr& inst) {
  VregSet acrosses;
  visitAcrosses(unit, inst, [&] (Vreg r) { acrosses.add(r); });
  return acrosses;
}

inline VregSet defsSet(const Vunit& unit, const Vinstr& inst) {
  VregSet defs;
  visitDefs(unit, inst, [&] (Vreg r) { defs.add(r); });
  return defs;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename U, typename D,
          typename RU, typename RD>
struct MutableRegVisitor {
  Vunit& unit;

  U u;
  D d;
  RU ru;
  RD rd;

  template <typename T> void imm(const T&) const {}
  template <typename T> void across(T& r) { use(r); }
  template <typename T1, typename T2> void useHint(T1& r, const T2&) { use(r); }
  template <typename T1, typename T2> void defHint(T1& r, const T2&) { def(r); }

  void use(RegSet r) { r = ru(r); }
  void use(Vtuple uses) { for (auto& r : unit.tuples[uses]) use(r); }
  void use(Vptr& m) {
    if (m.base.isValid())  use(m.base);
    if (m.index.isValid()) use(m.index);
  }
  void use(VcallArgsId a) {
    auto& args = unit.vcallArgs[a];
    for (auto& r : args.args)       use(r);
    for (auto& r : args.simdArgs)   use(r);
    for (auto& r : args.stkArgs)    use(r);
    for (auto& r : args.indRetArgs) use(r);
  }
  template<typename W> void use(Vr<W>& m) { Vreg r = m; use(r); m = r; }
  void use(Vreg& r) { r = u((Vreg)r); }

  void def(RegSet r) const { r = rd(r); }
  void def(Vtuple defs) { for (auto& r : unit.tuples[defs]) def(r); }
  template<typename W> void def(Vr<W>& m) { Vreg r = m; def(r); m = r; }
  void def(Vreg& r) { r = d((Vreg)r); }
};

}

/*
 * Visit all the register operands of a Vinstr, calling 'u' for uses, 'd' for
 * defs, 'ru' for use RegSets, 'rd' for def RegSets. The callables return a new
 * Vreg/RegSet, causing the Vinstr to be rewritten with that Vreg/RegSet.
 */
template <typename U, typename D,
          typename RU, typename RD>
void visitRegsMutable(Vunit& unit,
                      Vinstr& instr,
                      U&& u,
                      D&& d,
                      RU&& ru,
                      RD&& rd) {
  detail::MutableRegVisitor<U, D, RU, RD> visitor{
    unit,
    std::forward<U>(u),
    std::forward<D>(d),
    std::forward<RU>(ru),
    std::forward<RD>(rd)
  };
  visitOperands(instr, visitor);
}

/*
 * Overload for when you don't care about RegSets (they'll be left unchanged).
 */
template <typename U, typename D>
void visitRegsMutable(Vunit& unit,
                      Vinstr& instr,
                      U&& u,
                      D&& d) {
  visitRegsMutable(
    unit,
    instr,
    std::forward<U>(u),
    std::forward<D>(d),
    [](RegSet r) { return r; },
    [](RegSet r) { return r; }
  );
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename D> struct DefsWithHintsVisitor {
  const Vunit& unit;
  D d;

  template <typename T> void imm(const T&) const {}
  template <typename T> void use(const T&) {}
  template <typename T> void across(const T&) {}
  template <typename T1, typename T2> void useHint(const T1&, const T2&) {}

  void defHint(Vreg r1, Vreg r2) { d(r1, r2); }
  void defHint(Vtuple v1, Vtuple v2) {
    auto const& t1 = unit.tuples[v1];
    auto const& t2 = unit.tuples[v2];
    assertx(t1.size() == t2.size());
    for (size_t i = 0; i < t1.size(); ++i) d(t1[i], t2[i]);
  }

  void def(const RegSet& s) { s.forEach([&] (Vreg r) { d(r, Vreg{}); }); }
  void def(Vtuple defs) {
    for (auto const& r : unit.tuples[defs]) d(r, Vreg{});
  }
  void def(Vreg r) { d(r, Vreg{}); }
};

}

/*
 * Visit a Vinstr's defs, calling 'd' with the Vreg and its matching hint (if
 * any). If there's no hint, an invalid Vreg will be passed as the second
 * parameter.
 */
template <typename D>
void visitDefsWithHints(const Vunit& unit, const Vinstr& instr, D&& d) {
  detail::DefsWithHintsVisitor<D> visitor{unit, std::forward<D>(d)};
  visitOperands(instr, visitor);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Visit reachable blocks, calling `pre' and `post' on each one.
 */
struct DfsWalker {
  explicit DfsWalker(const Vunit& u)
    : unit(u)
    , visited(u.blocks.size())
  {}

  template<class Pre, class Post>
  void dfs(Vlabel b, Pre pre, Post post) {
    if (visited.test(b)) return;
    visited.set(b);

    pre(b);
    for (auto s : succs(unit.blocks[b])) {
      dfs(s, pre, post);
    }
    post(b);
  }

  template<class Pre, class Post>
  void dfs(Pre pre, Post post) {
    dfs(unit.entry, pre, post);
  }

private:
  const Vunit& unit;
  boost::dynamic_bitset<> visited;
};

/*
 * Visit reachable blocks in postorder, calling `fn' on each one.
 *
 * Guaranteed not to use standard iterators on u.blocks, because several passes
 * (e.g., vlower) forbid it.
 */
struct PostorderWalker {
  explicit PostorderWalker(const Vunit& u) : m_dfs{u} {}

  template<class Post>
  void dfs(Post post) {
    m_dfs.dfs([](Vlabel){}, post);
  }

private:
  DfsWalker m_dfs;
};

///////////////////////////////////////////////////////////////////////////////

using PredVector = jit::vector<TinyVector<Vlabel, 3>>;
PredVector computePreds(const Vunit& unit);

}}

#endif
