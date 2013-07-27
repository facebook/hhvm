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
namespace Transl {

Tracelet::Tracelet() :
    m_stackChange(0),
    m_arState(),
    m_analysisFailed(false),
    m_inliningFailed(false){ }

Tracelet::~Tracelet() { }

NormalizedInstruction* Tracelet::newNormalizedInstruction() {
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

  out << i->unit()->filepath()->data() << ':'
            << i->unit()->getLineNumber(i->offset()) << std::endl;
  for (; i; i = i->next) {
    out << "  " << i->offset() << ": " << i->toString() << std::endl;
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

} } // HPHP::Transl
