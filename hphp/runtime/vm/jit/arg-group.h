/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {

class SSATmp;
struct IRInstruction;

namespace NativeCalls { struct CallInfo; }

//////////////////////////////////////////////////////////////////////

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

enum class DestType : uint8_t {
  None,  // return void (no valid registers)
  SSA,   // return a single-register value
  Byte,  // return a single-byte register value
  TV,    // return a TypedValue packed in two registers
  Dbl,   // return scalar double in a single FP register
  SIMD,  // return a TypedValue in one SIMD register
};
const char* destTypeName(DestType);

struct CallDest {
  DestType type;
  Vreg reg0, reg1;
};
UNUSED const CallDest kVoidDest { DestType::None };

struct ArgDesc {
  enum class Kind {
    Reg,     // Normal register
    Imm,     // 64-bit Immediate
    TypeImm, // DataType Immediate
    Addr,    // Address (register plus 32-bit displacement)
  };

  PhysReg dstReg() const { return m_dstReg; }
  Vreg srcReg() const { return m_srcReg; }
  Kind kind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Immed64 imm() const { assertx(m_kind == Kind::Imm); return m_imm64; }
  DataType typeImm() const {
    assertx(m_kind == Kind::TypeImm);
    return m_typeImm;
  }
  Immed disp() const { assertx(m_kind == Kind::Addr); return m_disp32; }
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
    DataType m_typeImm;
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
 *   assertx(args.size() == 3);
 */
struct ArgGroup {
  typedef jit::vector<ArgDesc> ArgVec;

  explicit ArgGroup(const IRInstruction* inst,
                    const StateVector<SSATmp,Vloc>& locs)
    : m_inst(inst), m_locs(locs), m_override(nullptr)
  {}

  size_t numGpArgs() const { return m_gpArgs.size(); }
  size_t numSimdArgs() const { return m_simdArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }

  ArgDesc& gpArg(size_t i) {
    assertx(i < m_gpArgs.size());
    return m_gpArgs[i];
  }
  const ArgDesc& gpArg(size_t i) const {
    return const_cast<ArgGroup*>(this)->gpArg(i);
  }
  const ArgDesc& simdArg(size_t i) const {
    assertx(i < m_simdArgs.size());
    return m_simdArgs[i];
  }
  const ArgDesc& stkArg(size_t i) const {
    assertx(i < m_stkArgs.size());
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
    auto s = m_inst->src(i);
    ArgDesc arg(s, m_locs[s]);
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
  ArgGroup& typedValue(int i);

  ArgGroup& memberKeyIS(int i) {
    return memberKeyImpl(i, true);
  }

  ArgGroup& memberKeyS(int i) {
    return memberKeyImpl(i, false);
  }

  const IRInstruction* inst() const {
    return m_inst;
  }

private:
  void push_arg(const ArgDesc& arg);
  void push_SIMDarg(const ArgDesc& arg);

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(int i) {
    auto s = m_inst->src(i);
    push_arg(ArgDesc(s, m_locs[s], false));
    return *this;
  }

  ArgGroup& memberKeyImpl(int i, bool allowInt) {
    auto key = m_inst->src(i);
    if (key->isA(TStr) || (allowInt && key->isA(TInt))) {
      return ssa(i);
    }
    return typedValue(i);
  }

private:
  const IRInstruction* m_inst;
  const StateVector<SSATmp,Vloc>& m_locs;
  ArgVec* m_override; // used to force args to go into a specific ArgVec
  ArgVec m_gpArgs; // INTEGER class args
  ArgVec m_simdArgs; // SSE class args
  ArgVec m_stkArgs; // Overflow
};

ArgGroup toArgGroup(const NativeCalls::CallInfo&,
                    const StateVector<SSATmp,Vloc>& locs,
                    const IRInstruction*);

}}
#endif
