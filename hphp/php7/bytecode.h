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

#include "hphp/runtime/vm/hhbc.h"

#include <boost/variant.hpp>

#include <unordered_map>
#include <vector>

namespace HPHP { namespace php7 {

struct Block;

enum Flavor {
  Drop,
  Cell,
  Ref,
  Return,
  FuncParam,
};

namespace bc {

struct StringOffsetVector {
  std::unordered_map<std::string, Block*> branches;
  Block* defaultBranch{nullptr};
};

// to disambiguate between string literals and local names
struct NamedLocal {
  std::string name;
};

struct UniqueLocal {
  explicit UniqueLocal()
    : id(std::make_shared<uint32_t>(-1)) {}

  std::shared_ptr<uint32_t> id;
};

typedef boost::variant<
  NamedLocal,
  UniqueLocal
> Local;

enum MemberType {
  Property,
  Element,
};

struct LocalMember {
  MemberType type;
  Local local;
};

struct CellMember {
  MemberType type;
  uint32_t location;
};

struct ImmMember {
  MemberType type;
  std::string name;
};

struct ImmIntElem {
  int64_t val;
};

struct NewElem {};

typedef boost::variant<
  CellMember,
  LocalMember,
  ImmMember,
  ImmIntElem,
  NewElem
  // null-safe access (MQT) is left out since PHP doesn't have this feature
> MemberKey;

struct ClassrefSlot {
  explicit ClassrefSlot()
    : id(std::make_shared<int32_t>(-1)) {}

  bool allocated() const {
    return *id >= 0;
  }

  std::shared_ptr<int32_t> id;
};

struct ReadClassref {
  /* implicit */ ReadClassref(const ClassrefSlot& slot)
    : slot(slot) {}

  ClassrefSlot slot;
};

struct WriteClassref {
  /* implicit */ WriteClassref(const ClassrefSlot& slot)
    : slot(slot) {}

  ClassrefSlot slot;
};

// void* immediate types just aren't being used right now
#define IMM_TYPE_BLA std::vector<Block*>
#define IMM_TYPE_SLA StringOffsetVector
#define IMM_TYPE_ILA void*
#define IMM_TYPE_IVA uint32_t
#define IMM_TYPE_I64A int64_t
#define IMM_TYPE_LA Local
#define IMM_TYPE_IA int32_t
#define IMM_TYPE_CAR ReadClassref
#define IMM_TYPE_CAW WriteClassref
#define IMM_TYPE_DA double
#define IMM_TYPE_SA std::string
#define IMM_TYPE_AA int
#define IMM_TYPE_RATA void*
#define IMM_TYPE_BA Block*
#define IMM_TYPE_OA(subtype) subtype
#define IMM_TYPE_KA MemberKey
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
    static const char* name() { \
      return #opcode; \
    }\
    IMM_ ## imms \
    \
    template<class Visitor> \
    void visit_imms(Visitor&& v) const { \
      IMM_VISIT_ ## imms \
    } \
    \
    template<class Visitor> \
    void visit_imms(Visitor&& v) { \
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

} // namespace bc

// too many opcodes for us to use boost::variant so we'll roll our own :)
struct Bytecode {
  Bytecode()
    : code(Op::Nop),
      Nop() {}

#define O(opcode, imms, inputs, outputs, flags) \
  /* implicit */ Bytecode(const bc::opcode& data) \
    : code(Op::opcode) { \
    new (&opcode) bc::opcode(data); \
  }
OPCODES
#undef O

  Bytecode(const Bytecode& b) {
    code = b.code;
    switch(code) {
#define O(opcode, ...) \
      case Op::opcode: new (&opcode) bc::opcode(b.opcode); break;
      OPCODES
#undef O
    }
  }

  Bytecode(Bytecode&& b) {
    code = b.code;
    switch (code) {
#define O(opcode, ...) \
      case Op::opcode: new (&opcode) bc::opcode(std::move(b.opcode)); break;
      OPCODES
#undef O
    }
  }

  Bytecode& operator=(const Bytecode& b) {
    destroy_op();
    code = b.code;
    switch(code) {
#define O(opcode, ...) case Op::opcode: opcode = b.opcode; break;
      OPCODES
#undef O
    }
    return *this;
  }

  Bytecode& operator=(Bytecode&& b) {
    destroy_op();
    code = b.code;
    switch(code) {
#define O(opcode, ...) case Op::opcode: opcode = std::move(b.opcode); break;
      OPCODES
#undef O
    }
    return *this;
  }

  ~Bytecode() {
    destroy_op();
  }

  template<class Visitor>
  void visit(Visitor&& visit) const {
    switch (code) {
#define O(opcode, ...) case Op::opcode: visit.bytecode(opcode); break;
      OPCODES
#undef O
    }
  }

  template<class Visitor>
  void visit(Visitor&& visit) {
    switch (code) {
#define O(opcode, ...) case Op::opcode: visit.bytecode(opcode); break;
      OPCODES
#undef O
    }
  }

 private:
  void destroy_op() {
    using namespace bc;
    switch(code) {
#define O(opcode, ...) case Op::opcode: opcode.~opcode(); break;
      OPCODES
#undef O
    }
  }

  Op code;
  union {
#define O(opcode, imms, inputs, outputs, flags) \
  bc::opcode opcode;
    OPCODES
#undef O
  };
};

// these are the last instructions in the block, they must be jumps or leave
// the current function i.e. only these instructions:
#define EXIT_OPS \
  EXIT(Jmp) \
  EXIT(JmpNS) \
  EXIT(JmpZ) \
  EXIT(JmpNZ) \
  EXIT(Switch) \
  EXIT(SSwitch) \
  EXIT(RetC) \
  EXIT(RetV) \
  EXIT(Unwind) \
  EXIT(Throw) \
  EXIT_LAST(Fatal)

#define EXIT(name) bc::name,
#define EXIT_LAST(name) bc::name
typedef boost::variant<
  EXIT_OPS
> ExitOp;
#undef EXIT
#undef EXIT_LAST

}}  // HPHP::php7

#endif // _incl_HPHP_PHP7_BYTECODE_H
