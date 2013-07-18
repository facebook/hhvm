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

#ifndef incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_
#define incl_HPHP_VM_TRANSL_HOPT_NATIVECALLS_H_

#include <initializer_list>
#include <functional>
#include <vector>

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-gen.h"

namespace HPHP {  namespace JIT { namespace NativeCalls {

// Information about C++ helpers called by translated code, used by
// CodeGenerator and LinearScan. See nativecalls.cpp for a full
// description of the types and enums.

enum FuncType : unsigned {
  FPtr,
  FSSA,
};

struct FuncPtr {
  FuncPtr() {}
  /* implicit */ FuncPtr(TCA f) : type(FPtr), ptr(f) {}
  FuncPtr(FuncType t, uint64_t i) : type(t), srcIdx(i) { assert(t == FSSA); }

  FuncType type;
  union {
    TCA ptr;
    uint64_t srcIdx;
  };
};

enum ArgType : unsigned {
  SSA,
  TV,
  VecKeyS,
  VecKeyIS,
  ExtraImm,
};

// Function that extracts the bits for an immediate value from extra
// data.
typedef std::function<uintptr_t (IRInstruction*)> ExtraDataBits;

struct Arg {
  Arg(ArgType type, unsigned srcIdx) : type(type), srcIdx(srcIdx) {}

  explicit Arg(ExtraDataBits&& func)
    : type(ExtraImm)
    , srcIdx(-1u)
    , extraFunc(std::move(func))
  {}

  ArgType type;
  unsigned srcIdx;
  ExtraDataBits extraFunc;
};

struct CallInfo {
  Opcode op;
  FuncPtr func;
  DestType dest;
  SyncOptions sync;
  std::vector<Arg> args;
};

typedef std::initializer_list<CallInfo> CallInfoList;
typedef hphp_hash_map<Opcode, CallInfo, int64_hash> CallInfoMap;

class CallMap {
public:
  explicit CallMap(CallInfoList infos);

  static bool hasInfo(Opcode op);
  static const CallInfo& info(Opcode op);

private:
  CallInfoMap m_map;
};

} } }

#endif
