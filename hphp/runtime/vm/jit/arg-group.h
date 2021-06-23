/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#pragma once

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/arch.h"

namespace HPHP { namespace jit {

struct SSATmp;
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
  None,      // return void (no valid registers)
  Indirect,  // return struct/object to the address in the first arg
  SSA,       // return an SSA value in 1 or 2 integer registers
  Byte,      // return a single-byte register value
  TV,        // return a TypedValue packed in two registers
  Dbl,       // return scalar double in a single FP register
  SIMD,      // return a TypedValue in one SIMD register
};
const char* destTypeName(DestType);

struct CallDest {
  CallDest(DestType t, Type vt, Vreg r0 = Vreg{}, Vreg r1 = Vreg{})
    : valueType{vt}, reg0{r0}, reg1{r1}, type{t}
  {}

  CallDest(DestType t, Vreg r0 = Vreg{}, Vreg r1 = Vreg{})
    : CallDest(t, TTop, r0, r1)
  {}

  Type valueType;
  Vreg reg0, reg1;
  DestType type;
};
UNUSED const CallDest kVoidDest { DestType::None };
UNUSED const CallDest kIndirectDest { DestType::Indirect };

struct ArgDesc {
  enum class Kind {
    Reg,     // Normal register
    Imm,     // 64-bit Immediate
    TypeImm, // DataType Immediate
    Addr,    // Address (register plus 32-bit displacement)
    DataPtr, // Pointer to data section
    IndRet,  // Indirect Return Address (register plus 32-bit displacement)
    SpilledTV, // Address of TypedValue pushed onto the stack
  };

  Vreg srcReg() const { return m_srcReg; }
  Kind kind() const { return m_kind; }
  Immed64 imm() const {
    assertx(m_kind == Kind::Imm || m_kind == Kind::DataPtr);
    return m_imm64;
  }
  DataType typeImm() const {
    assertx(m_kind == Kind::TypeImm);
    return m_typeImm;
  }
  Immed disp() const {
    assertx(m_kind == Kind::Addr ||
            m_kind == Kind::IndRet);
    return m_disp32;
  }
  Vreg srcReg2() const {
    assertx(m_kind == Kind::SpilledTV);
    return m_srcReg2;
  }

  bool isZeroExtend() const { return m_zeroExtend; }
  Optional<AuxUnion> aux() const { return m_aux; }

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

  explicit ArgDesc(Kind kind, Vreg srcReg1, Vreg srcReg2)
    : m_kind(kind)
    , m_srcReg(srcReg1)
    , m_srcReg2(srcReg2)
  {}

  explicit ArgDesc(SSATmp* tmp,
                   Vloc,
                   bool val = true,
                   Optional<AuxUnion> aux = std::nullopt);

private:
  Kind m_kind;
  Vreg m_srcReg;
  union {
    Immed64 m_imm64; // 64-bit plain immediate
    Immed m_disp32;  // 32-bit displacement
    DataType m_typeImm;
    Vreg m_srcReg2;
  };
  Optional<AuxUnion> m_aux;
  bool m_zeroExtend{false};
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
    : m_inst(inst), m_locs(locs)
  {}

  size_t numGpArgs() const { return m_gpArgs.size(); }
  size_t numSimdArgs() const { return m_simdArgs.size(); }
  size_t numStackArgs() const { return m_stkArgs.size(); }
  size_t numIndRetArgs() const { return m_indRetArgs.size(); }

  const std::vector<Type>& argTypes() const {
    return m_argTypes;
  }
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
  const ArgDesc& indRetArg(size_t i) const {
    assertx(i < m_indRetArgs.size());
    return m_indRetArgs[i];
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

  template<class T> ArgGroup& dataPtr(const T* ptr) {
    push_arg(ArgDesc{ArgDesc::Kind::DataPtr, ptr});
    return *this;
  }

  ArgGroup& reg(Vreg reg) {
    push_arg(ArgDesc(ArgDesc::Kind::Reg, reg, -1));
    return *this;
  }

  ArgGroup& addr(Vreg base, Immed off) {
    push_arg(ArgDesc(ArgDesc::Kind::Addr, base, off));
    return *this;
  }

  /*
   * indRet args are similar to simple addr args, but are used specifically to
   * pass the address that the native call will use for indirect returns. If a
   * platform has no dedicated registers for indirect returns, then it uses
   * the first general purpose argument register.
   */
  ArgGroup& indRet(Vreg base, Immed off) {
    push_arg(ArgDesc(ArgDesc::Kind::IndRet, base, off));
    return *this;
  }

  ArgGroup& ssa(int i, bool allowFP = true);

  /*
   * Pass tmp as a TypedValue passed by value.
   */
  ArgGroup& typedValue(int i, Optional<AuxUnion> aux = std::nullopt);

  ArgGroup& memberKeyIS(int i) {
    return memberKeyImpl(i, true);
  }

  ArgGroup& memberKeyS(int i) {
    return memberKeyImpl(i, false);
  }

  /*
   * Push a TypedValue onto the stack and pass its address. This is
   * needed, for example, if you want to pass a tv_lval to a function
   * expecting a const Variant&.
   */
  ArgGroup& constPtrToTV(Vreg type, Vreg data) {
    push_arg(ArgDesc(ArgDesc::Kind::SpilledTV, type, data));
    return *this;
  }

  const IRInstruction* inst() const {
    return m_inst;
  }

private:
  void push_arg(const ArgDesc& arg, Type t = TBottom);
  void push_SIMDarg(const ArgDesc& arg, Type t = TBottom);

  /*
   * For passing the m_type field of a TypedValue.
   */
  ArgGroup& type(int i, Optional<AuxUnion> aux) {
    auto s = m_inst->src(i);
    push_arg(ArgDesc(s, m_locs[s], false, aux));
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
  ArgVec* m_override{nullptr}; // force args to go into a specific ArgVec
  ArgVec m_indRetArgs; // Indirect Return Address
  ArgVec m_gpArgs; // INTEGER class args
  ArgVec m_simdArgs; // SSE class args
  ArgVec m_stkArgs; // Overflow
  jit::vector<Type> m_argTypes;
};

ArgGroup toArgGroup(const NativeCalls::CallInfo&,
                    const StateVector<SSATmp,Vloc>& locs,
                    const IRInstruction*);

}}
