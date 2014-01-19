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
#ifndef incl_HHBBC_BC_H_
#define incl_HHBBC_BC_H_

#include <vector>
#include <utility>

#include <boost/mpl/has_xxx.hpp>

#include "hphp/util/tiny-vector.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/hhbbc/src-loc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP {
struct StringData;
struct ArrayData;
}

namespace HPHP { namespace HHBBC {

struct Bytecode;

namespace php {
  struct Block;
  struct Local;
  struct Iter;
}

//////////////////////////////////////////////////////////////////////

/*
 * The following creates a struct for each bytecode using the opcode
 * table.  Each opcode will be named bc::opcode, and has a single
 * constructor that takes its immediate types in order.
 *
 * E.g.
 *
 *   auto fpass = bc::FPassC { 2 };  // FPassC for arg 2
 */

//////////////////////////////////////////////////////////////////////

struct MElem {
  MemberCode mcode;
  union {
    SString immStr;
    int64_t immInt;
    borrowed_ptr<php::Local> immLoc;
  };
};

struct MVector {
  LocationCode lcode;
  borrowed_ptr<php::Local> locBase;
  std::vector<MElem> mcodes;
};

inline uint32_t numVecPops(const MVector& mvec) {
  uint32_t ret = numLocationCodeStackVals(mvec.lcode);
  for (auto& mc : mvec.mcodes) {
    ret += mcodeStackVals(mc.mcode);
  }
  return ret;
}

using IterTabEnt    = std::pair<IterKind,borrowed_ptr<php::Iter>>;
using IterTab       = std::vector<IterTabEnt>;

using SwitchTab     = std::vector<borrowed_ptr<php::Block>>;

// The final entry in the SSwitchTab is the default case, it will
// always have a nullptr for the string.
using SSwitchTabEnt = std::pair<SString,borrowed_ptr<php::Block>>;
using SSwitchTab    = std::vector<SSwitchTabEnt>;

//////////////////////////////////////////////////////////////////////

namespace bc {

#define IMM_TY_MA       MVector
#define IMM_TY_BLA      SwitchTab
#define IMM_TY_SLA      SSwitchTab
#define IMM_TY_ILA      IterTab
#define IMM_TY_IVA      int32_t
#define IMM_TY_I64A     int64_t
#define IMM_TY_LA       borrowed_ptr<php::Local>
#define IMM_TY_IA       borrowed_ptr<php::Iter>
#define IMM_TY_DA       double
#define IMM_TY_SA       SString
#define IMM_TY_AA       SArray
#define IMM_TY_BA       borrowed_ptr<php::Block>
#define IMM_TY_OA(type) type
#define IMM_TY_VSA      std::vector<SString>

#define IMM_NAME_MA(n)      mvec
#define IMM_NAME_BLA(n)     targets
#define IMM_NAME_SLA(n)     targets
#define IMM_NAME_ILA(n)     iterTab
#define IMM_NAME_IVA(n)     arg##n
#define IMM_NAME_I64A(n)    arg##n
#define IMM_NAME_LA(n)      loc##n
#define IMM_NAME_IA(n)      iter##n
#define IMM_NAME_DA(n)      dbl##n
#define IMM_NAME_SA(n)      str##n
#define IMM_NAME_AA(n)      arr##n
#define IMM_NAME_BA(n)      target
#define IMM_NAME_OA_IMPL(n) subop
#define IMM_NAME_OA(type)   IMM_NAME_OA_IMPL
#define IMM_NAME_VSA(n)     keys

#define IMM_EXTRA_MA
#define IMM_EXTRA_BLA
#define IMM_EXTRA_SLA
#define IMM_EXTRA_ILA
#define IMM_EXTRA_IVA
#define IMM_EXTRA_I64A
#define IMM_EXTRA_LA
#define IMM_EXTRA_IA
#define IMM_EXTRA_DA
#define IMM_EXTRA_SA
#define IMM_EXTRA_AA
#define IMM_EXTRA_BA        using has_target_flag = std::true_type;
#define IMM_EXTRA_OA(x)
#define IMM_EXTRA_VSA

#define IMM_MEM(which, n)          IMM_TY_##which IMM_NAME_##which(n)
#define IMM_MEM_NA
#define IMM_MEM_ONE(x)             IMM_MEM(x, 1);
#define IMM_MEM_TWO(x, y)          IMM_MEM(x, 1); IMM_MEM(y, 2);
#define IMM_MEM_THREE(x, y, z)     IMM_MEM(x, 1); IMM_MEM(y, 2);  \
                                   IMM_MEM(z, 3);
#define IMM_MEM_FOUR(x, y, z, l)   IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                   IMM_MEM(z, 3); IMM_MEM(l, 4);

#define IMM_EXTRA_NA
#define IMM_EXTRA_ONE(x)           IMM_EXTRA_##x
#define IMM_EXTRA_TWO(x,y)         IMM_EXTRA_ONE(x)       IMM_EXTRA_ONE(y)
#define IMM_EXTRA_THREE(x,y,z)     IMM_EXTRA_TWO(x,y)     IMM_EXTRA_ONE(z)
#define IMM_EXTRA_FOUR(x,y,z,l)    IMM_EXTRA_THREE(x,y,z) IMM_EXTRA_ONE(l)

#define IMM_CTOR(which, n)         IMM_TY_##which IMM_NAME_##which(n)
#define IMM_CTOR_NA
#define IMM_CTOR_ONE(x)            IMM_CTOR(x, 1)
#define IMM_CTOR_TWO(x, y)         IMM_CTOR(x, 1), IMM_CTOR(y, 2)
#define IMM_CTOR_THREE(x, y, z)    IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                   IMM_CTOR(z, 3)
#define IMM_CTOR_FOUR(x, y, z, l)  IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                   IMM_CTOR(z, 3), IMM_CTOR(l, 4)

#define IMM_INIT(which, n)         IMM_NAME_##which(n) ( IMM_NAME_##which(n) )
#define IMM_INIT_NA
#define IMM_INIT_ONE(x)            : IMM_INIT(x, 1)
#define IMM_INIT_TWO(x, y)         : IMM_INIT(x, 1), IMM_INIT(y, 2)
#define IMM_INIT_THREE(x, y, z)    : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                     IMM_INIT(z, 3)
#define IMM_INIT_FOUR(x, y, z, l)  : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                     IMM_INIT(z, 3), IMM_INIT(l, 4)


#define POP_UV  if (i == 0) return Flavor::U
#define POP_CV  if (i == 0) return Flavor::C
#define POP_AV  if (i == 0) return Flavor::A
#define POP_VV  if (i == 0) return Flavor::V
#define POP_FV  if (i == 0) return Flavor::F
#define POP_RV  if (i == 0) return Flavor::R

#define POP_NOV             uint32_t numPop() const { return 0; } \
                            Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_ONE(x)          uint32_t numPop() const { return 1; } \
                            Flavor popFlavor(uint32_t i) const {  \
                              POP_##x; not_reached();             \
                            }

#define POP_TWO(x, y)       uint32_t numPop() const { return 2; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##x; not_reached(); \
                            }

#define POP_THREE(x, y, z)  uint32_t numPop() const { return 3; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##y; --i; POP_##z;  \
                              not_reached();                        \
                            }

#define POP_MMANY   uint32_t numPop() const { return 0 + numVecPops(mvec); } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_C_MMANY uint32_t numPop() const { return 1 + numVecPops(mvec); } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_R_MMANY uint32_t numPop() const { return 1 + numVecPops(mvec); } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_V_MMANY uint32_t numPop() const { return 1 + numVecPops(mvec); } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_CMANY   uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::C;                       \
                    }

#define POP_SMANY   uint32_t numPop() const { return keys.size(); }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::C;                       \
                    }

#define POP_FMANY   uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assert(i < numPop());                   \
                      return Flavor::F;                       \
                    }

#define POP_CVMANY  uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      not_reached();                          \
                    }

#define POP_CVUMANY uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      not_reached();                          \
                    }

#define PUSH_UV  if (i == 0) return TUninit
#define PUSH_CV  if (i == 0) return TInitCell
#define PUSH_AV  if (i == 0) return TCls
#define PUSH_VV  if (i == 0) return TRef
#define PUSH_FV  if (i == 0) return TInitGen
#define PUSH_RV  if (i == 0) return TInitGen

#define PUSH_NOV          uint32_t numPush() const { return 0; } \
                          Type pushType(uint32_t i) const { not_reached(); }

#define PUSH_ONE(x)       uint32_t numPush() const { return 1; }  \
                          Type pushType(uint32_t i) const {       \
                            PUSH_##x; not_reached();              \
                          }

#define PUSH_TWO(x, y)    uint32_t numPush() const { return 2; }    \
                          Type pushType(uint32_t i) const {         \
                            PUSH_##x; --i; PUSH_##y; not_reached(); \
                          }

#define PUSH_INS_1(...)   uint32_t numPush() const { return 1; }        \
                          Type pushType(uint32_t i) const { not_reached(); }

#define PUSH_INS_2(...)   uint32_t numPush() const { return 1; }        \
                          Type pushType(uint32_t i) const { not_reached(); }

#define O(opcode, imms, inputs, outputs, flags) \
  struct opcode {                               \
    static constexpr Op op = Op::opcode;        \
    explicit opcode ( IMM_CTOR_##imms )         \
      IMM_INIT_##imms                           \
    {}                                          \
    IMM_MEM_##imms                              \
    IMM_EXTRA_##imms                            \
    POP_##inputs                                \
    PUSH_##outputs                              \
  };
OPCODES
#undef O

#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_INS_1
#undef PUSH_INS_2

#undef PUSH_UV
#undef PUSH_CV
#undef PUSH_AV
#undef PUSH_VV
#undef PUSH_FV
#undef PUSH_RV

#undef POP_UV
#undef POP_CV
#undef POP_AV
#undef POP_VV
#undef POP_FV
#undef POP_RV

#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_MMANY
#undef POP_C_MMANY
#undef POP_R_MMANY
#undef POP_V_MMANY
#undef POP_CMANY
#undef POP_SMANY
#undef POP_FMANY
#undef POP_CVMANY
#undef POP_CVUMANY

#undef IMM_TY_MA
#undef IMM_TY_BLA
#undef IMM_TY_SLA
#undef IMM_TY_ILA
#undef IMM_TY_IVA
#undef IMM_TY_I64A
#undef IMM_TY_LA
#undef IMM_TY_IA
#undef IMM_TY_DA
#undef IMM_TY_SA
#undef IMM_TY_AA
#undef IMM_TY_BA
#undef IMM_TY_OA
#undef IMM_TY_VSA

// These are deliberately not undefined, so they can be used in other
// places.
// #undef IMM_NAME_MA
// #undef IMM_NAME_BLA
// #undef IMM_NAME_SLA
// #undef IMM_NAME_ILA
// #undef IMM_NAME_IVA
// #undef IMM_NAME_I64A
// #undef IMM_NAME_LA
// #undef IMM_NAME_IA
// #undef IMM_NAME_DA
// #undef IMM_NAME_SA
// #undef IMM_NAME_AA
// #undef IMM_NAME_BA
// #undef IMM_NAME_OA
// #undef IMM_NAME_OA_IMPL

#undef IMM_EXTRA_MA
#undef IMM_EXTRA_BLA
#undef IMM_EXTRA_SLA
#undef IMM_EXTRA_ILA
#undef IMM_EXTRA_IVA
#undef IMM_EXTRA_I64A
#undef IMM_EXTRA_LA
#undef IMM_EXTRA_IA
#undef IMM_EXTRA_DA
#undef IMM_EXTRA_SA
#undef IMM_EXTRA_AA
#undef IMM_EXTRA_BA
#undef IMM_EXTRA_OA

#undef IMM_MEM

#undef IMM_MEM_NA
#undef IMM_MEM_ONE
#undef IMM_MEM_TWO
#undef IMM_MEM_THREE
#undef IMM_MEM_FOUR

#undef IMM_CTOR
#undef IMM_CTOR_NA
#undef IMM_CTOR_ONE
#undef IMM_CTOR_TWO
#undef IMM_CTOR_THREE
#undef IMM_CTOR_FOUR

#undef IMM_INIT
#undef IMM_INIT_NA
#undef IMM_INIT_ONE
#undef IMM_INIT_TWO
#undef IMM_INIT_THREE
#undef IMM_INIT_FOUR

}

//////////////////////////////////////////////////////////////////////

/*
 * Bytecode is a tagged-union that can hold any HHBC opcode struct
 * defined above.  You can visit a bytecode with a static_visitor
 * using visit(), defined below.
 *
 * Each Bytecode also carries a corresponding SrcLoc that indicates
 * which line of PHP code the bytecode was generated for.
 */
struct Bytecode {
  // Default construction creates a Nop.
  Bytecode()
    : op(Op::Nop)
    , Nop(bc::Nop{})
  {}

#define O(opcode, ...)                          \
  /* implicit */ Bytecode(bc::opcode data)      \
    : op(Op::opcode)                            \
  {                                             \
    new (&opcode) bc::opcode(std::move(data));  \
  }

  OPCODES

#undef O

  // Note: assuming bc::Nop is empty and has trivial dtor/ctor.

  Bytecode(const Bytecode& o) : op(Op::Nop) { *this = o; }
  Bytecode(Bytecode&& o) : op(Op::Nop) { *this = std::move(o); }

  Bytecode& operator=(const Bytecode& o) {
    destruct();
    op = Op::Nop;
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(o.opcode); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  Bytecode& operator=(Bytecode&& o) {
    destruct();
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(std::move(o.opcode)); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  ~Bytecode() { destruct(); }

  uint32_t numPop() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPop();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  Flavor popFlavor(uint32_t i) const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.popFlavor(i);
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  uint32_t numPush() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPush();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  Op op;
  php::SrcLoc srcLoc;

#define O(opcode, ...) bc::opcode opcode;
  union { OPCODES };
#undef O

private:
  void destruct() {
    switch (op) {
#define O(opcode, ...)                          \
      case Op::opcode:                          \
        { typedef bc::opcode X;                 \
          this->opcode.~X(); }                  \
        break;
      OPCODES
#undef O
    }
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * Helper for making a Bytecode with a given srcLoc.
 *
 * Ex:
 *    auto b = bc_with_loc(something.srcLoc, bc::Nop {});
 */
template<class T> Bytecode bc_with_loc(php::SrcLoc loc, const T& t) {
  Bytecode b = t;
  b.srcLoc = loc;
  return b;
}

//////////////////////////////////////////////////////////////////////

/*
 * Visit a bytecode using a StaticVisitor, similar to
 * boost::apply_visitor or match().
 *
 * The `v' argument should be a function object that accepts a call
 * operator for all the bc::Foo types, with a nested member typedef
 * called result_type that indicates the return type of the call.
 */
template<class Visit>
typename Visit::result_type visit(const Bytecode& b, Visit v) {
#define O(opcode, ...) case Op::opcode: return v(b.opcode);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_target, has_target_flag, false);

//////////////////////////////////////////////////////////////////////

std::string show(const Bytecode& bc);

//////////////////////////////////////////////////////////////////////

}

}

#endif
