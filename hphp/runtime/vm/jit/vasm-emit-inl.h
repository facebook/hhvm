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

#include <algorithm>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////
// Vout.

inline Vout& Vout::operator=(const Vout& v) {
  assertx(&v.m_unit == &m_unit);
  m_block = v.m_block;
  m_origin = v.m_origin;
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
  return m_unit.blocks[m_block].area;
}

inline Vpoint Vout::makePoint() {
  return Vpoint{m_unit.next_point++};
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
  return m_unit.makeConst(v);
}

///////////////////////////////////////////////////////////////////////////////
// Vasm.

inline Vasm::Area& Vasm::area(AreaIndex i) {
  assertx((unsigned)i < m_areas.size());
  return m_areas[(unsigned)i];
}

///////////////////////////////////////////////////////////////////////////////
}}
