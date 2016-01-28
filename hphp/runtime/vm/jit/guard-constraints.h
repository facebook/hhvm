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

#ifndef incl_HPHP_VM_GUARD_CONSTRAINTS_H_
#define incl_HPHP_VM_GUARD_CONSTRAINTS_H_

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type-source.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

struct IRInstruction;

/*
 * GuardConstraints holds state that is collected during initial IR generation
 * and needed by the guard relaxation pass.
 */
struct GuardConstraints {
  /*
   * Maps guard instructions (CheckLoc, CheckStk, etc.) to TypeConstraints. The
   * TypeConstraints for a guard start out fully generic and are tightened
   * appropriately when a value's type is used.
   */
  jit::hash_map<const IRInstruction*, TypeConstraint> guards;

  /*
   * Maps certain instructions dealing with locals to the source of the
   * local's type coming into the instruction: usually either a guard or the
   * last known value of the local.
   */
  jit::hash_map<const IRInstruction*, TypeSourceSet> typeSrcs;

  /*
   * Maps AssertLoc/CheckLoc instructions to the type of the local coming into
   * the instruction. It is needed to compute the type of the local after the
   * guard.
   */
  jit::hash_map<const IRInstruction*, Type> prevTypes;
};

}}

#endif
