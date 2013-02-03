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

#include "folly/Conv.h"
#include "folly/Format.h"
#include "folly/experimental/Gen.h"

#include "util/trace.h"
#include "runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace VM { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

Type::Tag outputType(const IRInstruction* inst) {

#define D(type)   return Type::type;
#define DofS(n)   return inst->getSrc(n)->getType();
#define DUnbox(n) return Type::unbox(inst->getSrc(n)->getType());
#define DBox(n)   return Type::box(inst->getSrc(n)->getType());
#define DParam    return inst->getTypeParam();
#define NA        assert(0 && "outputType requires inst->hasDst()");

#define O(name, dstinfo, srcinfo, flags) case name: dstinfo not_reached();

  switch (inst->getOpcode()) {
  IR_OPCODES
  default: not_reached();
  }

#undef O

#undef D
#undef DofS
#undef DUnbox
#undef DBox
#undef DParam
#undef NA

}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * subtypeAny(ssa, types...)
 *
 * Returns whether ssa->getType() is a subtype of any of the
 * Type::Tags in the variable-length list.
 */

bool subtypeAny(const SSATmp* s, Type::Tag t) {
  return s->isA(t);
}
template<class... Args>
bool subtypeAny(const SSATmp* s, Type::Tag t, Args... args) {
  return s->isA(t) || subtypeAny(s, args...);
}

// Return a renderable string for a union of types that was specified.
// Used when a type assertion fails to produce an error message.
template<class... Args>
std::string expectedStr(Args... t) {
  using namespace folly::gen;
  return folly::join(
    '|',
    from({t...}) | mapped([](Type::Tag t) { return Type::Strings[t]; })
                 | as<std::vector>()
  );
}

}

/*
 * Runtime typechecking for IRInstruction operands.
 *
 * This is generated using the table in ir.h.  We instantiate
 * IR_OPCODES after defining all the various source forms to do type
 * assertions according to their form (see ir.h for documentation on
 * the notation).  The checkers appear in argument order, so each one
 * increments curSrc, and at the end we can check that the argument
 * count was also correct.
 */
void assertOperandTypes(const IRInstruction* inst) {
  if (!debug) return;

  int curSrc = 0;

  auto bail = [&] (const std::string& msg) {
    FTRACE(1, "{}", msg);
    if (!::HPHP::Trace::moduleEnabled(::HPHP::Trace::hhir, 1)) {
      fprintf(stderr, "%s\n", msg.c_str());
    }
    always_assert(false && "instruction operand type check failure");
  };

  auto getSrc = [&]() -> SSATmp* {
    if (curSrc < inst->getNumSrcs()) {
      return inst->getSrc(curSrc);
    }

    bail(folly::format(
      "Internal error: instruction had too few operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
    not_reached();
  };

  auto check = [&] (bool cond, const std::string& expected) {
    if (cond) return;

    bail(folly::format(
      "Error: failed type check on operand {}\n"
      "   instruction: {}\n"
      "   was expecting: {}\n"
      "   received: {}\n",
        curSrc,
        inst->toString(),
        expected,
        Type::Strings[inst->getSrc(curSrc)->getType()]
      ).str()
    );
  };

  auto checkNoArgs = [&]{
    if (inst->getNumSrcs() == 0) return;
    bail(folly::format(
      "Error: instruction expected no operands\n"
      "   instruction: {}\n",
        inst->toString()
      ).str()
    );
  };

  auto countCheck = [&]{
    if (inst->getNumSrcs() == curSrc) return;
    bail(folly::format(
      "Error: instruction had too many operands\n"
      "   instruction: {}\n"
      "   expected {} arguments\n",
        inst->toString(),
        curSrc
      ).str()
    );
  };

  using namespace Type;

#define NA       return checkNoArgs();
#define S(...)   check(subtypeAny(getSrc(), __VA_ARGS__), \
                       expectedStr(__VA_ARGS__));         \
                   ++curSrc;
#define C(type)  check(getSrc()->isConst() &&       \
                       getSrc()->isA(Type::type),   \
                       "constant " #type);          \
                  ++curSrc;
#define CStr     C(StaticStr)
#define SNum     S(Int,Bool)
#define SUnk     return;

#define O(opcode, dstinfo, srcinfo, flags)      \
  case opcode: srcinfo countCheck(); return;

  switch (inst->getOpcode()) {
    IR_OPCODES
  default: assert(0);
  }

#undef O

#undef NA
#undef S
#undef C
#undef CStr
#undef SNum
#undef SUnk

}

//////////////////////////////////////////////////////////////////////

}}}

