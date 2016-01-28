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

#ifndef incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_
#define incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/irlower.h"

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <vector>

namespace HPHP { namespace jit {

struct IRInstruction;

namespace NativeCalls {

// Information about C++ helpers called by translated code, used by
// CodeGenerator and LinearScan. See nativecalls.cpp for a full
// description of the types and enums.

struct FuncPtr {
  FuncPtr() {}
  explicit FuncPtr(TCA) = delete;

  template<class Ret, class... Args>
  /* implicit */ FuncPtr(Ret (*fp)(Args...))
    : call(CallSpec::direct(fp))
  {}

  template<class Ret, class Cls, class... Args>
  /* implicit */ FuncPtr(Ret (Cls::*fp)(Args...))
    : call(CallSpec::method(fp))
  {}

  template<class Ret, class Cls, class... Args>
  /* implicit */ FuncPtr(Ret (Cls::*fp)(Args...) const)
    : call(CallSpec::method(fp))
  {}

  /*
   * Create FuncPtrs to array data "rotated" vtables.  For example, in
   * native-calls.cpp:
   *
   *   {NvGetInt, &g_array_funcs.nvGetInt, DSSA, SNone, {{SSA, 0}, {SSA, 1}}},
   *
   */
  template<class Ret, class... Args>
  /* implicit */ FuncPtr(Ret (*const (*p)[ArrayData::kNumKinds])(Args...))
    : call(CallSpec::array(p))
  {
    always_assert(0 && "This code needs to be conditional on "
                       "deltaFits(p, sz::dword) before using it");
  }

  union { CallSpec call; };
};

enum class ArgType : unsigned {
  SSA,
  TV,
  ExtraImm,
  Imm,
};

// Function that extracts the bits for an immediate value from extra
// data.
typedef std::function<uintptr_t (const IRInstruction*)> ExtraDataBits;

struct Arg {
  Arg(ArgType type, intptr_t ival) : type(type), ival(ival) {}

  explicit Arg(ExtraDataBits&& func)
    : type(ArgType::ExtraImm)
    , ival(-1)
    , extraFunc(std::move(func))
  {}

  ArgType type;
  intptr_t ival;
  ExtraDataBits extraFunc;
};

struct CallInfo {
  Opcode op;
  FuncPtr func;
  DestType dest;
  irlower::SyncOptions sync;
  std::vector<Arg> args;
};

typedef std::initializer_list<CallInfo> CallInfoList;
typedef hphp_hash_map<Opcode, CallInfo, std::hash<Opcode>> CallInfoMap;

struct CallMap {
  explicit CallMap(CallInfoList infos);

  static bool hasInfo(Opcode op);
  static const CallInfo& info(Opcode op);

private:
  CallInfoMap m_map;
};

}}}

#endif
