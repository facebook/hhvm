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

#include "hphp/runtime/vm/jit/cg-meta.h"

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////
// Vout.

inline Vout& Vout::operator=(const Vout& v) {
  assertx(&v.m_unit == &m_unit);
  m_block = v.m_block;
  m_irctx = v.m_irctx;
  return *this;
}

inline Vout& Vout::operator=(Vlabel b) {
  m_block = b;
  return *this;
}

inline Vout::operator Vlabel() const {
  return m_block;
}

inline AreaIndex Vout::area() const {
  return m_unit.blocks[m_block].area_idx;
}

inline Vreg Vout::makeReg() {
  return m_unit.makeReg();
}

inline Vtuple Vout::makeTuple(const VregList& regs) const {
  return m_unit.makeTuple(regs);
}

inline Vtuple Vout::makeTuple(VregList&& regs) const {
  return m_unit.makeTuple(std::move(regs));
}

inline VcallArgsId Vout::makeVcallArgs(VcallArgs&& args) const {
  return m_unit.makeVcallArgs(std::move(args));
}

template<class T>
inline Vreg Vout::cns(T v) {
  return m_unit.makeConst(Vconst{v});
}

template<class T, class... Args>
T* Vout::allocData(Args&&... args) {
  return m_unit.allocData<T>(std::forward<Args>(args)...);
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

template<class GenFunc>
TCA vwrap_impl(CodeBlock& main, CodeBlock& cold, DataBlock& data,
               CGMeta* meta, GenFunc gen, CodeKind kind, bool relocate) {
  CGMeta dummy_meta;

  auto const start = main.frontier();

  { // Finish emitting in this scope so that we can assert about `dummy_meta'.
    Vauto vauto { main, cold, data, meta ? *meta : dummy_meta, kind, relocate };
    gen(vauto.main(), vauto.cold());
  }

  dummy_meta.process_literals();
  assertx(dummy_meta.empty());

  return start;
}

}

///////////////////////////////////////////////////////////////////////////////

}}
