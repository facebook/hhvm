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

#ifndef incl_HPHP_JIT_TYPE_CONSTRAINT_H_
#define incl_HPHP_JIT_TYPE_CONSTRAINT_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/jit/type.h"

#include <cstdint>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Class;

namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * Type information used by guard relaxation code to track the properties of a
 * type that consumers care about.
 */
struct TypeConstraint {

  /*
   * Constructors.
   */
  /* implicit */ TypeConstraint(DataTypeCategory cat = DataTypeGeneric);
  explicit TypeConstraint(const Class* cls);

  /*
   * Stringify the TypeConstraint.
   */
  std::string toString() const;


  /////////////////////////////////////////////////////////////////////////////
  // Basic info.

  /*
   * Mark the TypeConstraint as weak; see documentation for `weak'.
   */
  TypeConstraint& setWeak(bool w = true);

  /*
   * Is this a trivial constraint?
   */
  bool empty() const;

  /*
   * Comparison.
   */
  bool operator==(TypeConstraint tc2) const;
  bool operator!=(TypeConstraint tc2) const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialization.

  static constexpr uint8_t kWantArrayKind = 0x1;
  static constexpr uint8_t kWantArrayShape = 0x2;

  /*
   * Is this TypeConstraint for a specialized type?
   */
  bool isSpecialized() const;

  /*
   * Set or check the kWantArrayKind bit in `m_specialized'.
   *
   * @requires: isSpecialized()
   */
  TypeConstraint& setWantArrayKind();
  bool wantArrayKind() const;

  /*
   * Set or check the kWantArrayShape bit in 'm_specialized'.  kWantArrayShape
   * implies kWantArrayKind.
   *
   * @requires: isSpecialized()
   */
  TypeConstraint& setWantArrayShape();
  bool wantArrayShape() const;

  /*
   * Set, check, or return the specialized Class.
   *
   * @requires:
   *    setDesiredClass: isSpecialized()
   *                     desiredClass() is either nullptr, a parent of `cls',
   *                     or a child of `cls'
   *    desiredClass:    wantClass()
   */
  TypeConstraint& setDesiredClass(const Class* cls);
  bool wantClass() const;
  const Class* desiredClass() const;


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

  /*
   * `category' starts as DataTypeGeneric and is refined to more specific
   * values by consumers of the type.
   */
  DataTypeCategory category;

  /*
   * If weak is true, the consumer of the value being constrained doesn't
   * actually want to constrain the guard (if found).
   *
   * Most often used to figure out if a type can be used without further
   * constraining guards.
   */
  bool weak;

private:
  /*
   * `m_specialized' either holds a Class* or a 1 in its low bit, indicating
   * that for a DataTypeSpecialized constraint, we require the specified class
   * or an array kind, respectively.
   */
  uintptr_t m_specialized;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns true iff `t' is specific enough to fit `tc', meaning a consumer
 * constraining a value with `tc' would be satisfied with `t' as the value's
 * type after relaxation.
 */
bool typeFitsConstraint(Type t, TypeConstraint tc);

/*
 * relaxConstraint returns the least specific TypeConstraint 'tc' that doesn't
 * prevent the intersection of knownType and relaxType(toRelax, tc.category)
 * from satisfying origTc. It is used in IRBuilder::constrainValue and
 * IRBuilder::constrainStack to determine how to constrain the typeParam and
 * src values of CheckType/CheckStk instructions, and the src values of
 * AssertType/AssertStk instructions.
 *
 * AssertType example:
 * t24:Obj<C> = AssertType<{Obj<C>|InitNull}> t4:Obj
 *
 * If constrainValue is called with (t24, DataTypeSpecialized), relaxConstraint
 * will be called with (DataTypeSpecialized, Obj<C>|InitNull, Obj). After a few
 * iterations it will determine that constraining Obj with DataTypeCountness
 * will still allow the result type of the AssertType instruction to satisfy
 * DataTypeSpecialized, because relaxType(Obj, DataTypeCountness) == Obj.
 */
TypeConstraint relaxConstraint(const TypeConstraint origTc,
                               const Type knownType, const Type toRelax);

/*
 * Return a copy of tc refined with any new information in newTc.
 */
TypeConstraint applyConstraint(TypeConstraint origTc, TypeConstraint newTc);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/type-constraint-inl.h"

#endif
