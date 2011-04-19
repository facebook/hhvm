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

const Construct::TypePtrMap &Construct::GetTypeHintTypes() {
  if (TypeHintTypes.empty()) {
    TypeHintTypes["array"] = KindOfArray;
    if (RuntimeOption::EnableHipHopExperimentalSyntax) {
      TypeHintTypes["vector"]  = KindOfArray;
      TypeHintTypes["map"]     = KindOfArray;
      TypeHintTypes["set"]     = KindOfArray;
    }
    if (RuntimeOption::EnableHipHopSyntax) {
      TypeHintTypes["bool"]    = KindOfBoolean;
      TypeHintTypes["boolean"] = KindOfBoolean;
      TypeHintTypes["int"]     = KindOfInt64;
      TypeHintTypes["integer"] = KindOfInt64;
      TypeHintTypes["real"]    = KindOfDouble;
      TypeHintTypes["double"]  = KindOfDouble;
      TypeHintTypes["float"]   = KindOfDouble;
      TypeHintTypes["string"]  = KindOfString;
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
