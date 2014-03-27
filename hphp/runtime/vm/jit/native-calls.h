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

#ifndef incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_
#define incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_

#include <initializer_list>
#include <functional>
#include <vector>
#include <algorithm>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/arg-group.h"

namespace HPHP { namespace JIT {

struct IRInstruction;

namespace NativeCalls {

// Information about C++ helpers called by translated code, used by
// CodeGenerator and LinearScan. See nativecalls.cpp for a full
// description of the types and enums.

enum class FuncType : unsigned {
  Call,
  SSA,
};

struct FuncPtr {
  FuncPtr() {}
  explicit FuncPtr(TCA f) : type(FuncType::Call), call(f) {}

  template<class Ret, class... Args>
  /* implicit */ FuncPtr(Ret (*fp)(Args...))
    : type(FuncType::Call)
    , call(fp)
  {}

  FuncPtr(FuncType t, uint64_t i) : type(t), srcIdx(i) {
    assert(t == FuncType::SSA);
  }

  FuncType type;
  union {
    CppCall call;
    uint64_t srcIdx;
  };
};

enum class ArgType : unsigned {
  SSA,
  TV,
  MemberKeyS,
  MemberKeyIS,
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
  SyncOptions sync;
  std::vector<Arg> args;

  ArgGroup toArgGroup(const RegAllocInfo &regs,
                      const IRInstruction* inst) const;
};

typedef std::initializer_list<CallInfo> CallInfoList;
typedef hphp_hash_map<Opcode, CallInfo, std::hash<Opcode>> CallInfoMap;

class CallMap {
public:
  explicit CallMap(CallInfoList infos);

  static bool hasInfo(Opcode op);
  static const CallInfo& info(Opcode op);

private:
  CallInfoMap m_map;
};

}}}

#endif
