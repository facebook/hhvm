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

#ifndef incl_HPHP_VM_JIT_PUNT_H_
#define incl_HPHP_VM_JIT_PUNT_H_

#include <stdexcept>

#include "hphp/runtime/base/types.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct Func;

//////////////////////////////////////////////////////////////////////
namespace jit {

struct FailedIRGen : std::runtime_error {
  const char* const file;
  const int         line;
  const char* const func;

  FailedIRGen(const char* _file, int _line, const char* _func);
};

struct FailedTraceGen : std::runtime_error {
  FailedTraceGen(const char* file, int line, const char* why);
};

struct FailedCodeGen : std::runtime_error {
  const char*   file;
  const int     line;
  const char*   func;
  const Offset  bcOff;
  const Func*   vmFunc;
  const bool    resumed;
  const TransID profTransId;

  FailedCodeGen(const char* _file,
                int _line,
                const char* _func,
                uint32_t _bcOff,
                const Func* _vmFunc,
                bool _resumed,
                TransID _profTransId);
};

//////////////////////////////////////////////////////////////////////

#define SPUNT(instr) do {                           \
  throw FailedIRGen(__FILE__, __LINE__, instr);     \
} while(0)

#define PUNT(instr) SPUNT(#instr)

#define TRACE_PUNT(why) do { \
  FTRACE(1, "punting: {}\n", why); \
  throw FailedTraceGen(__FILE__, __LINE__, why); \
} while(0)

//////////////////////////////////////////////////////////////////////
}}

#endif
