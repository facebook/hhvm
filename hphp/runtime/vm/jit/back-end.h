/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_JIT_BACK_END_H
#define incl_HPHP_JIT_BACK_END_H

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/util/data-block.h"

namespace HPHP {

struct ActRec;
struct SrcKey;

///////////////////////////////////////////////////////////////////////////////

namespace jit {

struct AsmInfo;
struct IRUnit;
struct PhysReg;

///////////////////////////////////////////////////////////////////////////////

struct BackEnd {
  virtual ~BackEnd();

  virtual void enterTCHelper(TCA start, ActRec* stashedAR) = 0;

  virtual void streamPhysReg(std::ostream& os, PhysReg reg) = 0;
  virtual void disasmRange(std::ostream& os, int indent, bool dumpIR,
                           TCA begin, TCA end) = 0;

protected:
  BackEnd() {}
};

std::unique_ptr<BackEnd> newBackEnd();

///////////////////////////////////////////////////////////////////////////////

}}

#endif
