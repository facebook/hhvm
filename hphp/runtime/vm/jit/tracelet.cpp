/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/tracelet.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace JIT {

Tracelet::Tracelet() :
    m_stackChange(0),
    m_arState(),
    m_analysisFailed(false),
    m_inliningFailed(false){ }

Tracelet::~Tracelet() { }

NormalizedInstruction* Tracelet::newNormalizedInstruction() {
  // Note: we're relying on the () here to zero-initialize the arg
  // union, etc.
  NormalizedInstruction* ni = new NormalizedInstruction();
  m_instrs.push_back(ni);
  return ni;
}

DynLocation* Tracelet::newDynLocation(Location l, DataType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation(Location l, RuntimeType t) {
  DynLocation* dl = new DynLocation(l, t);
  m_dynlocs.push_back(dl);
  return dl;
}

DynLocation* Tracelet::newDynLocation() {
  DynLocation* dl = new DynLocation();
  m_dynlocs.push_back(dl);
  return dl;
}

void Tracelet::print() const {
  print(std::cerr);
}

void Tracelet::print(std::ostream& out) const {
  const NormalizedInstruction* i = m_instrStream.first;
  if (i == nullptr) {
    out << "<empty>\n";
    return;
  }

  out << "Guards:\n";
  for (auto const& dep : m_dependencies) {
    out << "  " << dep.second->pretty() << '\n';
  }
  out << "Static types:\n";
  for (auto const& dep : m_resolvedDeps) {
    out << "  " << dep.second->pretty() << '\n';
  }
  out << show(i->source) << '\n';

  auto blockStart = i->source;
  for (; i; i = i->next) {
    auto nextSk = i->source.advanced(i->unit());
    if (!i->next || i->next->source != nextSk) {
      auto opts = Unit::PrintOpts().range(blockStart.offset(), nextSk.offset())
                                   .indent(2);
      blockStart.unit()->prettyPrint(out, opts);
      if (i->next) blockStart = i->next->source;
    }
  }
}

std::string Tracelet::toString() const {
  std::ostringstream out;
  print(out);
  return out.str();
}

SrcKey Tracelet::nextSk() const {
  return m_instrStream.last->nextSk();
}

const Func* Tracelet::func() const {
  return m_sk.func();
}

} } // HPHP::JIT
