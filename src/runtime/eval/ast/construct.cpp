/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/eval/ast/construct.h>
#include <runtime/eval/parser/parser.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

Construct::TypePtrMap Construct::TypeHintTypes;
Construct::TypePtrMap Construct::HipHopTypeHintTypes;
Construct::TypePtrMap Construct::HipHopExperimentalTypeHintTypes;

const Construct::TypePtrMap &Construct::GetHipHopTypeHintTypes() {
  if (HipHopTypeHintTypes.empty()) {
    HipHopTypeHintTypes["bool"]    = KindOfBoolean;
    HipHopTypeHintTypes["boolean"] = KindOfBoolean;
    HipHopTypeHintTypes["int"]     = KindOfInt64;
    HipHopTypeHintTypes["integer"] = KindOfInt64;
    HipHopTypeHintTypes["real"]    = KindOfDouble;
    HipHopTypeHintTypes["double"]  = KindOfDouble;
    HipHopTypeHintTypes["float"]   = KindOfDouble;
    HipHopTypeHintTypes["string"]  = KindOfString;
  }
  return HipHopTypeHintTypes;
}

const Construct::TypePtrMap &Construct::GetHipHopExperimentalTypeHintTypes() {
  if (HipHopExperimentalTypeHintTypes.empty()) {
    HipHopExperimentalTypeHintTypes["vector"]  = KindOfArray;
    HipHopExperimentalTypeHintTypes["map"]     = KindOfArray;
    HipHopExperimentalTypeHintTypes["set"]     = KindOfArray;
  }
  return HipHopExperimentalTypeHintTypes;
}

const Construct::TypePtrMap &Construct::GetTypeHintTypes() {
  if (TypeHintTypes.empty()) {
    TypeHintTypes["array"] = KindOfArray;
    if (RuntimeOption::EnableHipHopExperimentalSyntax) {
      GetHipHopExperimentalTypeHintTypes();
      for (Construct::TypePtrMap::const_iterator iter =
           HipHopExperimentalTypeHintTypes.begin();
           iter != HipHopExperimentalTypeHintTypes.end(); ++iter) {
        TypeHintTypes[iter->first]  = iter->second;
      }
    }
    if (RuntimeOption::EnableHipHopSyntax) {
      GetHipHopTypeHintTypes();
      for (Construct::TypePtrMap::const_iterator iter =
           HipHopTypeHintTypes.begin();
           iter != HipHopTypeHintTypes.end(); ++iter) {
        TypeHintTypes[iter->first]  = iter->second;
      }
    }
  }
  return TypeHintTypes;
}

///////////////////////////////////////////////////////////////////////////////

Construct::Construct(CONSTRUCT_ARGS) : _count(0) {
  parser->getLocation(m_loc);
}

Construct::Construct(const Location *loc) : _count(0) {
  m_loc = *loc;
}

void Construct::resetLoc(Parser *parser) {
  parser->getLocation(m_loc);
}

void Construct::release() {
  delete this;
}

void Construct::dumpLoc() const {
  cerr << m_loc.file << ":" << m_loc.line1 << endl;
}

///////////////////////////////////////////////////////////////////////////////
}
}
