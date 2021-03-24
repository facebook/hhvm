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

#include "hphp/runtime/vm/jit/arg-group.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

const char* destTypeName(DestType dt) {
  switch (dt) {
    case DestType::None:     return "None";
    case DestType::Indirect: return "Indirect";
    case DestType::SSA:      return "SSA";
    case DestType::Byte:     return "Byte";
    case DestType::TV:       return "TV";
    case DestType::Dbl:      return "Dbl";
    case DestType::SIMD:     return "SIMD";
  }
  not_reached();
}

ArgDesc::ArgDesc(SSATmp* tmp,
                 Vloc loc,
                 bool val,
                 folly::Optional<AuxUnion> aux) {
  assertx(IMPLIES(aux, !val));

  auto const setTypeImm = [&] {
    static_assert(offsetof(TypedValue, m_type) % 8 == 0, "");

    if (aux) {
      auto const dt = static_cast<std::make_unsigned<data_type_t>::type>(
        tmp->type().toDataType()
      );
      static_assert(std::numeric_limits<decltype(dt)>::digits <= 32, "");
      m_imm64 = dt | auxToMask(*aux);
      m_kind = Kind::Imm;
    } else {
      m_typeImm = tmp->type().toDataType();
      m_kind = Kind::TypeImm;
    }
  };

  if (tmp->hasConstVal()) {
    // tmp is a constant
    if (val) {
      m_imm64 = tmp->rawVal();
      m_kind = Kind::Imm;
    } else {
      setTypeImm();
    }
    return;
  }

  if (val) {
    assertx(loc.reg(0) != InvalidReg);
    m_srcReg = loc.reg(0);
    m_kind = Kind::Reg;
    // zero extend any boolean value that we pass to the helper in case
    // the helper expects it (e.g., as TypedValue)
    if (tmp->isA(TBool)) m_zeroExtend = true;
    return;
  }

  if (tmp->numWords() > 1) {
    assertx(loc.reg(1) != InvalidReg);
    // val is false so we're passing tmp's type.
    m_srcReg = loc.reg(1);
    m_kind = Kind::Reg;
    m_aux = aux;
    return;
  }

  setTypeImm();
}

///////////////////////////////////////////////////////////////////////////////

ArgGroup& ArgGroup::ssa(int i, bool allowFP) {
  auto const s = m_inst->src(i);
  auto const loc = m_locs[s];

  if (s->isA(TDbl) && allowFP) {
    push_SIMDarg(ArgDesc{s, loc}, s->type());
  } else {
    if (s->isA(TLvalToCell) && !s->isA(TBottom)) {
      // If there's exactly one register argument slot left, the whole tv_lval
      // goes on the stack instead of being split between a register and the
      // stack.
      if (m_gpArgs.size() == num_arg_regs() - 1) m_override = &m_stkArgs;
      SCOPE_EXIT { m_override = nullptr; };

      push_arg(ArgDesc{ArgDesc::Kind::Reg, loc.reg(0), -1}, s->type());
      push_arg(ArgDesc{ArgDesc::Kind::Reg, loc.reg(1), -1});
    } else {
      push_arg(ArgDesc{s, loc}, s->type());
    }
  }
  return *this;
}

ArgGroup& ArgGroup::typedValue(int i, folly::Optional<AuxUnion> aux) {
  // If there's exactly one register argument slot left, the whole TypedValue
  // goes on the stack instead of being split between a register and the stack.
  if (m_gpArgs.size() == num_arg_regs() - 1) m_override = &m_stkArgs;

  // On x86, if we have one free GP register left, we'll use it for the next
  // int argument, but on ARM, we'll just spill all later int arguments.
  #ifndef __aarch64__
    SCOPE_EXIT { m_override = nullptr; };
  #endif

  static_assert(offsetof(TypedValue, m_data) == 0, "");
  static_assert(offsetof(TypedValue, m_type) == 8, "");
  ssa(i, false).type(i, aux);
  return *this;
}

void ArgGroup::push_arg(const ArgDesc& arg, Type t) {
  // If m_override is set, use it unconditionally. Otherwise, select
  // m_gpArgs or m_stkArgs depending on how many args we've already pushed.
  ArgVec* args = m_override;
  if (!args) {
    if (arg.kind() == ArgDesc::Kind::IndRet && num_arg_regs_ind_ret() > 0) {
      args = &m_indRetArgs;
    } else {
      args = m_gpArgs.size() < num_arg_regs() ? &m_gpArgs : &m_stkArgs;
    }
  }
  args->push_back(arg);
  m_argTypes.emplace_back(t);
}

void ArgGroup::push_SIMDarg(const ArgDesc& arg, Type t) {
  // See push_arg above
  ArgVec* args = m_override;
  if (!args) {
    args = m_simdArgs.size() < num_arg_regs_simd()
         ? &m_simdArgs : &m_stkArgs;
  }
  args->push_back(arg);
  m_argTypes.emplace_back(t);
}

///////////////////////////////////////////////////////////////////////////////

}}
