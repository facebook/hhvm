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
#ifndef incl_HPHP_JIT_ARG_GROUP_H
#define incl_HPHP_JIT_ARG_GROUP_H

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/phys-loc.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"

namespace HPHP { namespace JIT {

class SSATmp;
struct IRInstruction;

//////////////////////////////////////////////////////////////////////

enum class DestType : unsigned {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  SSA2,  // return a two-register value (pair)
  TV     // return a TypedValue packed in two registers
};

/*
 * CallDest is the destination specification for a cgCallHelper
 * invocation.
 *
 * The DestType describes the return type of native helper calls,
 * particularly register assignments.
 *
 * These are created using the callDest() member functions.
 */
struct CallDest {
  DestType type;
  PhysReg reg0;
  PhysReg reg1;
};

const CallDest kVoidDest { DestType::None, InvalidReg, InvalidReg };

//////////////////////////////////////////////////////////////////////

class ArgDesc {
public:
  enum class Kind {
    Reg,     // Normal register
    TypeReg, // TypedValue's m_type field. Might need arch-specific
             // mangling before call depending on TypedValue's layout.
    Imm,     // 64-bit Immediate
    Addr,    // Address (register plus 32-bit displacement)
    None,    // Nothing: register will contain garbage
  };

  PhysReg dstReg() const { return m_dstReg; }
  PhysReg srcReg() const { return m_srcReg; }
  Kind kind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Immed64 imm() const { assert(m_kind == Kind::Imm); return m_imm64; }
  Immed disp() const { assert(m_kind == Kind::Addr); return m_disp32; }
  bool isZeroExtend() const { return m_zeroExtend; }
  bool done() const { return m_done; }
  void markDone() { m_done = true; }

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, Immed64 imm)
    : m_kind(kind)
    , m_srcReg(InvalidReg)
    , m_dstReg(reg::noreg)
    , m_imm64(imm)
    , m_zeroExtend(false)
    , m_done(false)
  {}

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed disp)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_dstReg(reg::noreg)
    , m_disp32(disp)
    , m_zeroExtend(false)
    , m_done(false)
  {}

  explicit ArgDesc(SSATmp* tmp, const PhysLoc&, bool val = true);

private:
  Kind m_kind;
  PhysReg m_srcReg;
  PhysReg m_dstReg;
  union {
    Immed64 m_imm64; // 64-bit plain immediate
    Immed m_disp32;  // 32-bit displacement
  };
  bool m_zeroExtend;
  bool m_done;
};

//////////////////////////////////////////////////////////////////////

/*
 * Bag of ArgDesc for use with cgCallHelper.
 *
 * You can create this using function chaining.  Example:
 *
 *   ArgGroup args;
 *   args.imm(0)
 *       .reg(rax)
 *       .immPtr(makeStaticString("Yo"))
 *       ;
 *   assert(args.size() == 3);
 */
struct ArgGroup {
  typedef smart::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const IRInstruction* inst, const RegAllocInfo::RegMap& regs)
    : m_inst(inst), m_regs(regs), m_override(nullptr)
  {}

  size_t numRegArgs() const { return m_regArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }

  ArgDesc& reg(size_t i) {
    assert(i < m_regArgs.size());
    return m_regArgs[i];
  }
  ArgDesc& operator[](size_t i) {
    return reg(i);
  }
  ArgDesc& stk(size_t i) {
    assert(i < m_stkArgs.size());
    return m_stkArgs[i];
  }

  ArgGroup& imm(Immed64 imm) {
    push_arg(ArgDesc(ArgDesc::Kind::Imm, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& immPtr(std::nullptr_t) { return imm(0); }

  ArgGroup& reg(PhysReg reg) {
    push_arg(ArgDesc(ArgDesc::Kind::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& addr(PhysReg base, Immed off) {
    push_arg(ArgDesc(ArgDesc::Kind::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(int i) {
    push_arg(ArgDesc(m_inst->src(i), m_regs.src(i)));
    return *this;
  }

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(int i) {
    // If there's exactly one register argument slot left, the whole TypedValue
    // goes on the stack instead of being split between a register and the
    // stack.
    if (m_regArgs.size() == X64::kNumRegisterArgs - 1) {
      m_override = &m_stkArgs;
    }
    packed_tv ? type(i).ssa(i) : ssa(i).type(i);
    m_override = nullptr;
    return *this;
  }

  ArgGroup& memberKeyIS(int i) {
    return memberKeyImpl(i, true);
  }

  ArgGroup& memberKeyS(int i) {
    return memberKeyImpl(i, false);
  }

private:
  void push_arg(const ArgDesc& arg) {
    // If m_override is set, use it unconditionally. Otherwise, select
    // m_regArgs or m_stkArgs depending on how many args we've already pushed.
    ArgVec* args = m_override;
    if (!args) {
      args = m_regArgs.size() < X64::kNumRegisterArgs ? &m_regArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(int i) {
    push_arg(ArgDesc(m_inst->src(i), m_regs.src(i), false));
    return *this;
  }

  ArgGroup& none() {
    push_arg(ArgDesc(ArgDesc::Kind::None, InvalidReg, -1));
    return *this;
  }

  ArgGroup& memberKeyImpl(int i, bool allowInt) {
    auto key = m_inst->src(i);
    if (key->isA(Type::Str) || (allowInt && key->isA(Type::Int))) {
      return ssa(i);
    }
    return typedValue(i);
  }

private:
  const IRInstruction* m_inst;
  const RegAllocInfo::RegMap& m_regs;
  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_regArgs;
  ArgVec m_stkArgs;
};

}}
#endif
