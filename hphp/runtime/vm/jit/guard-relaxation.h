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
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/block.h"

namespace HPHP { namespace JIT {

struct SSATmp;
struct IRUnit;

enum RelaxGuardsFlags {
  RelaxNormal =      0,
  RelaxSimple = 1 << 0,
  RelaxReflow = 1 << 1,
};

IRInstruction* guardForLocal(uint32_t locId, SSATmp* fp);
bool shouldHHIRRelaxGuards();

/*
 * Given a possibly null SSATmp*, determine if the type of that tmp may be
 * loosened by guard relaxation.
 */
bool typeMightRelax(const SSATmp* tmp);

bool relaxGuards(IRUnit&, const GuardConstraints& guards,
                 RelaxGuardsFlags flags);

typedef std::function<void(const RegionDesc::Location&, Type)> VisitGuardFn;
void visitGuards(IRUnit&, const VisitGuardFn& func);

bool typeFitsConstraint(Type t, TypeConstraint cat);
Type relaxType(Type t, TypeConstraint cat);
void incCategory(DataTypeCategory& c);
TypeConstraint relaxConstraint(const TypeConstraint origTc,
                               const Type knownType, const Type toRelax);
TypeConstraint applyConstraint(TypeConstraint origTc, TypeConstraint newTc);

} }

#endif
