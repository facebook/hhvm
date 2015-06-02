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
#ifndef incl_HPHP_LOCAL_EFFECTS_H_
#define incl_HPHP_LOCAL_EFFECTS_H_

#include <cstdint>

#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/type-source.h"

namespace HPHP { namespace jit {

struct FrameStateMgr;
struct SSATmp;
struct IRInstruction;

//////////////////////////////////////////////////////////////////////

/*
 * This module reports effects of IR instructions on locals, by making
 * callbacks through the LocalStateHook interface.
 */

//////////////////////////////////////////////////////////////////////

struct LocalStateHook {
  virtual void clearLocals() = 0;
  virtual void setLocalValue(uint32_t id, SSATmp* value) = 0;
  virtual void refineLocalValues(SSATmp* oldVal, SSATmp* newVal) = 0;
  virtual void killLocalsForCall(bool callDestroysLocals) = 0;
  virtual void dropLocalRefsInnerTypes() = 0;
  virtual void refineLocalType(uint32_t id, Type, TypeSource) = 0;
  virtual void setLocalPredictedType(uint32_t id, Type) = 0;
  virtual void setLocalType(uint32_t id, Type) = 0;
  virtual void setBoxedLocalPrediction(uint32_t id, Type) = 0;
  virtual void updateLocalRefPredictions(SSATmp*, SSATmp*) = 0;
  virtual void setLocalTypeSource(uint32_t id, TypeSource) = 0;

protected:
  ~LocalStateHook() = default;
};

//////////////////////////////////////////////////////////////////////

void local_effects(const FrameStateMgr&,
                   const IRInstruction*,
                   LocalStateHook&);

//////////////////////////////////////////////////////////////////////

}}

#endif
