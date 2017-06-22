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

#ifndef _incl_HPHP_PHP7_BYTECODE_H
#define _incl_HPHP_PHP7_BYTECODE_H

#include <hphp/runtime/vm/hhbc.h>
#include <boost/variant.hpp>

namespace HPHP { namespace PHP7 {

namespace BC {

// void* immediate types just aren't being used right now
#define IMM_TYPE_BLA void*
#define IMM_TYPE_SLA void*
#define IMM_TYPE_ILA void*
#define IMM_TYPE_IVA void*
#define IMM_TYPE_I64A int64_t
#define IMM_TYPE_LA int32_t
#define IMM_TYPE_IA int32_t
#define IMM_TYPE_CAR int32_t
#define IMM_TYPE_CAW int32_t
#define IMM_TYPE_DA double
#define IMM_TYPE_SA int
#define IMM_TYPE_AA int
#define IMM_TYPE_RATA void*
#define IMM_TYPE_BA uint64_t // offset (basic block id)
#define IMM_TYPE_OA(subtype) subtype
#define IMM_TYPE_KA void*
#define IMM_TYPE_LAR void*
#define IMM_TYPE_VSA void*

#define IMM(type, n) IMM_TYPE_ ## type imm ## n
#define IMM_NA
#define IMM_ONE(a)           IMM(a, 1);
#define IMM_TWO(a, b)        IMM(a, 1); IMM(b, 2);
#define IMM_THREE(a, b, c)   IMM(a, 1); IMM(b, 2); IMM(c, 3);
#define IMM_FOUR(a, b, c, d) IMM(a, 1); IMM(b, 2); IMM(c, 3); IMM(d, 4);

#define IMM_VISIT(n) v.imm(imm ## n)
#define IMM_VISIT_NA
#define IMM_VISIT_ONE(a)           IMM_VISIT(1);
#define IMM_VISIT_TWO(a, b)        IMM_VISIT_ONE(a)         IMM_VISIT(2);
#define IMM_VISIT_THREE(a, b, c)   IMM_VISIT_TWO(a, b)      IMM_VISIT(3);
#define IMM_VISIT_FOUR(a, b, c, d) IMM_VISIT_THREE(a, b, c) IMM_VISIT(4);

#define O(opcode, imms, inputs, outputs, flags) \
  struct opcode { \
    static constexpr Op code = Op::opcode; \
    static inline const char* name() { \
      return #opcode; \
    }\
    IMM_ ## imms \
    \
    template<class Visitor> \
    void inline visit_imms(Visitor&& v) const { \
      IMM_VISIT_ ## imms \
    } \
  };
OPCODES
#undef O

#undef IMM_TYPE_BLA
#undef IMM_TYPE_SLA
#undef IMM_TYPE_ILA
#undef IMM_TYPE_IVA
#undef IMM_TYPE_I64A
#undef IMM_TYPE_LA
#undef IMM_TYPE_IA
#undef IMM_TYPE_CAR
#undef IMM_TYPE_CAW
#undef IMM_TYPE_DA
#undef IMM_TYPE_SA
#undef IMM_TYPE_AA
#undef IMM_TYPE_RATA
#undef IMM_TYPE_BA
#undef IMM_TYPE_OA
#undef IMM_TYPE_KA
#undef IMM_TYPE_LAR
#undef IMM_TYPE_VSA
#undef IMM_VISIT
#undef IMM_VISIT_NA
#undef IMM_VISIT_ONE
#undef IMM_VISIT_TWO
#undef IMM_VISIT_THREE
#undef IMM_VISIT_FOUR
#undef IMM
#undef IMM_NA
#undef IMM_ONE
#undef IMM_TWO
#undef IMM_THREE
#undef IMM_FOUR

} // namespace BC

// too many opcodes for us to use boost::variant so we'll roll our own :)
struct Bytecode {
  Bytecode()
    : code(Op::Nop) {
      op.Nop = BC::Nop{};
  }


#define O(opcode, imms, inputs, outputs, flags) \
  /* implicit */ Bytecode(const BC::opcode& data) \
    : code(Op::opcode) { \
    new (&op) BC::opcode(data); \
  }
OPCODES
#undef O

  template<class Opcode>
  inline Opcode get() const {
    assert(Opcode::code == code);
    return *static_cast<Opcode*>(&op);
  }

  template<class Visitor>
  inline void visit(Visitor&& visit) const {
    switch (code) {
#define O(opcode, ...) case Op::opcode: visit.bytecode(op.opcode); break;
      OPCODES
#undef O
    }
  }

 private:
  Op code;
  union {
#define O(opcode, imms, inputs, outputs, flags) \
  BC::opcode opcode;
    OPCODES
#undef O
  } op;
};


}}  // HPHP::PHP7

#endif // _incl_HPHP_PHP7_BYTECODE_H
