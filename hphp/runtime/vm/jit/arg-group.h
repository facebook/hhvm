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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/phys-loc.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"

namespace HPHP { namespace jit {

class SSATmp;
struct IRInstruction;

namespace NativeCalls {
struct CallInfo;
}

//////////////////////////////////////////////////////////////////////

enum class DestType : unsigned {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  TV,    // return a TypedValue packed in two registers
  Dbl,   // return scalar double in a single FP register
  SIMD,  // return a TypedValue in one SIMD register
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

//////////////////////////////////////////////////////////////////////

namespace x64 {
struct CallDest {
  DestType type;
  Vreg reg0, reg1;
};
const CallDest kVoidDest { DestType::None };

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
  Vreg srcReg() const { return m_srcReg; }
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
    , m_imm64(imm)
  {}

  explicit ArgDesc(Kind kind, Vreg srcReg, Immed disp)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_disp32(disp)
  {}

  explicit ArgDesc(Kind kind)
    : m_kind(kind)
  {}

  explicit ArgDesc(SSATmp* tmp, Vloc, bool val = true);

private:
  Kind m_kind;
  Vreg m_srcReg;
  PhysReg m_dstReg;
  union {
    Immed64 m_imm64; // 64-bit plain immediate
    Immed m_disp32;  // 32-bit displacement
  };
  bool m_zeroExtend{false};
  bool m_done{false};
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
  typedef jit::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const IRInstruction* inst, const jit::vector<Vloc>& locs)
    : m_inst(inst), m_locs(locs), m_override(nullptr)
  {}

  size_t numGpArgs() const { return m_gpArgs.size(); }
  size_t numSimdArgs() const { return m_simdArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }

  ArgDesc& gpArg(size_t i) {
    assert(i < m_gpArgs.size());
    return m_gpArgs[i];
  }
  ArgDesc& simdArg(size_t i) {
    assert(i < m_simdArgs.size());
    return m_simdArgs[i];
  }
  ArgDesc& stkArg(size_t i) {
    assert(i < m_stkArgs.size());
    return m_stkArgs[i];
  }
  ArgDesc& operator[](size_t i) = delete;

  ArgGroup& imm(Immed64 imm) {
    push_arg(ArgDesc(ArgDesc::Kind::Imm, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& immPtr(std::nullptr_t) { return imm(0); }

  ArgGroup& reg(Vreg reg) {
    push_arg(ArgDesc(ArgDesc::Kind::Reg, reg, -1));
    return *this;
  }

  ArgGroup& addr(Vreg base, Immed off) {
    push_arg(ArgDesc(ArgDesc::Kind::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(int i, bool isFP = false) {
    ArgDesc arg(m_inst->src(i), m_locs[i]);
    if (isFP) {
      push_SIMDarg(arg);
    } else {
      push_arg(arg);
    }
    return *this;
  }

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(int i) {
    // If there's exactly one register argument slot left, the whole TypedValue
    // goes on the stack instead of being split between a register and the
    // stack.
    if (m_gpArgs.size() == x64::kNumRegisterArgs - 1) {
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
    // m_gpArgs or m_stkArgs depending on how many args we've already pushed.
    ArgVec* args = m_override;
    if (!args) {
      args = m_gpArgs.size() < x64::kNumRegisterArgs ? &m_gpArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  void push_SIMDarg(const ArgDesc& arg) {
    // See push_arg above
    ArgVec* args = m_override;
    if (!args) {
      args = m_simdArgs.size() < x64::kNumSIMDRegisterArgs
           ? &m_simdArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(int i) {
    push_arg(ArgDesc(m_inst->src(i), m_locs[i], false));
    return *this;
  }

  ArgGroup& none() {
    push_arg(ArgDesc(ArgDesc::Kind::None));
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
  const jit::vector<Vloc>& m_locs;
  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_gpArgs; // INTEGER class args
  ArgVec m_simdArgs; // SSE class args
  ArgVec m_stkArgs; // Overflow
};

ArgGroup toArgGroup(const NativeCalls::CallInfo&, const jit::vector<Vloc>& locs,
                    const IRInstruction*);
} // X64

namespace arm {
struct CallDest {
  DestType type;
  PhysReg reg0;
  PhysReg reg1;
};
const CallDest kVoidDest { DestType::None, InvalidReg, InvalidReg };
class ArgDesc {
public:
  enum class Kind {
    Reg,     // Normal register
    TypeReg, // TypedValue's m_type field. Might need arch-specific
             // mangling before call depending on TypedValue's layout.
    Imm,     // 64-bit Immediate
    Addr,    // Address (register plus 32-bit displacement)
    IpRel,   // ip-relative address
    None,    // Nothing: register will contain garbage
  };

  PhysReg dstReg() const { return m_dstReg; }
  PhysReg srcReg() const { return m_srcReg; }
  Kind kind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Immed64 imm() const {
    assert(m_kind == Kind::Imm || m_kind == Kind::IpRel);
    return m_imm64;
  }
  Immed disp() const { assert(m_kind == Kind::Addr); return m_disp32; }
  bool isZeroExtend() const { return m_zeroExtend; }
  bool done() const { return m_done; }
  void markDone() { m_done = true; }

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, Immed64 imm)
    : m_kind(kind)
    , m_imm64(imm)
  {}

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed disp)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_disp32(disp)
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
  bool m_zeroExtend{false};
  bool m_done{false};
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
  typedef jit::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const IRInstruction* inst, const RegAllocInfo::RegMap& regs)
    : m_inst(inst), m_regs(regs), m_override(nullptr)
  {}

  size_t numGpArgs() const { return m_gpArgs.size(); }
  size_t numSimdArgs() const { return m_simdArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }

  ArgDesc& gpArg(size_t i) {
    assert(i < m_gpArgs.size());
    return m_gpArgs[i];
  }
  ArgDesc& simdArg(size_t i) {
    assert(i < m_simdArgs.size());
    return m_simdArgs[i];
  }
  ArgDesc& stkArg(size_t i) {
    assert(i < m_stkArgs.size());
    return m_stkArgs[i];
  }

  ArgGroup& imm(Immed64 imm) {
    push_gp(ArgDesc(ArgDesc::Kind::Imm, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& immPtr(std::nullptr_t) { return imm(0); }

  ArgGroup& reg(PhysReg reg) {
    push_gp(ArgDesc(ArgDesc::Kind::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& addr(PhysReg base, Immed off) {
    push_gp(ArgDesc(ArgDesc::Kind::Addr, base, off));
    return *this;
  }

  ArgGroup& ssa(int i, bool isFP = false) {
    ArgDesc arg(m_inst->src(i), m_regs.src(i));
    if (isFP) {
      push_simd(arg);
    } else {
      push_gp(arg);
    }
    return *this;
  }

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(int i) {
    // If there's exactly one register argument slot left, the whole TypedValue
    // goes on the stack instead of being split between a register and the
    // stack.
    if (m_gpArgs.size() == x64::kNumRegisterArgs - 1) {
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

  template<typename T>
  ArgGroup& ipRel(const T* ptr) {
    push_gp(ArgDesc(ArgDesc::Kind::IpRel, (uintptr_t)ptr));
    return *this;
  }

private:
  void push_gp(const ArgDesc& arg) {
    // If m_override is set, use it unconditionally. Otherwise, select
    // m_gpArgs or m_stkArgs depending on how many args we've already pushed.
    ArgVec* args = m_override;
    if (!args) {
      args = m_gpArgs.size() < x64::kNumRegisterArgs ? &m_gpArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  void push_simd(const ArgDesc& arg) {
    // See push_gp above
    ArgVec* args = m_override;
    if (!args) {
      args = m_simdArgs.size() < x64::kNumSIMDRegisterArgs
           ? &m_simdArgs : &m_stkArgs;
    }
    args->push_back(arg);
  }

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(int i) {
    push_gp(ArgDesc(m_inst->src(i), m_regs.src(i), false));
    return *this;
  }

  ArgGroup& none() {
    push_gp(ArgDesc(ArgDesc::Kind::None, InvalidReg, -1));
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
  ArgVec m_gpArgs; // INTEGER class args
  ArgVec m_simdArgs; // SSE class args
  ArgVec m_stkArgs; // Overflow
};

ArgGroup toArgGroup(const NativeCalls::CallInfo&, const RegAllocInfo& regs,
                    const IRInstruction*);
} // ARM

}}
#endif
