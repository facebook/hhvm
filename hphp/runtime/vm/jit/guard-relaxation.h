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

#ifndef incl_HPHP_RUNTIME_VM_JIT_GUARD_RELAXATION_H_
#define incl_HPHP_RUNTIME_VM_JIT_GUARD_RELAXATION_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/block.h"

namespace HPHP { namespace jit {

struct GuardConstraints;
struct IRUnit;
struct SSATmp;

enum RelaxGuardsFlags {
  RelaxNormal =      0,
  RelaxSimple = 1 << 0,
  RelaxReflow = 1 << 1,
};

bool shouldHHIRRelaxGuards();

/*
 * Given a possibly null SSATmp*, determine if the type of that tmp may be
 * loosened by guard relaxation.
 */
bool typeMightRelax(const SSATmp* tmp);

bool relaxGuards(IRUnit&, const GuardConstraints& guards,
                 RelaxGuardsFlags flags);

/*
 * Returns true iff `t' is specific enough to fit `cat', meaning a consumer
 * constraining a value with `cat' would be satisfied with `t' as the value's
 * type after relaxation.
 */
bool typeFitsConstraint(Type t, TypeConstraint cat);

Type relaxType(Type t, TypeConstraint cat);
TypeConstraint relaxConstraint(const TypeConstraint origTc,
                               const Type knownType, const Type toRelax);
TypeConstraint applyConstraint(TypeConstraint origTc, TypeConstraint newTc);

} }

#endif
