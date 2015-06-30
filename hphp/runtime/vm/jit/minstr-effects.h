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
#ifndef incl_HPHP_MINSTR_EFFECTS_H_
#define incl_HPHP_MINSTR_EFFECTS_H_

#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

struct FrameStateMgr;

//////////////////////////////////////////////////////////////////////

int minstrBaseIdx(Opcode opc);

//////////////////////////////////////////////////////////////////////

struct MInstrEffects {
  MInstrEffects(Opcode op, Type base);

  static bool supported(Opcode op);
  static bool supported(const IRInstruction* inst);

  Type baseType;
  bool baseTypeChanged;
  bool baseValChanged;
};

//////////////////////////////////////////////////////////////////////

}}


#endif
