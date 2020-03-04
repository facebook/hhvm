/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_JIT_GUARD_CONSTRAINT_H_
#define incl_HPHP_JIT_GUARD_CONSTRAINT_H_

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
struct GuardConstraint {

  /*
   * Constructors.
   */
  /* implicit */ GuardConstraint(DataTypeCategory cat = DataTypeGeneric);
  explicit GuardConstraint(const Class* cls);
  explicit GuardConstraint(const RecordDesc* cls);

  /*
   * Stringify the GuardConstraint.
   */
  std::string toString() const;


  /////////////////////////////////////////////////////////////////////////////
  // Basic info.

  /*
   * Mark the GuardConstraint as weak; see documentation for `weak'.
   */
  GuardConstraint& setWeak(bool w = true);

  /*
   * Is this a trivial constraint?
   */
  bool empty() const;

  /*
   * Comparison.
   */
  bool operator==(GuardConstraint gc2) const;
  bool operator!=(GuardConstraint gc2) const;


  /////////////////////////////////////////////////////////////////////////////
  // Specialization.

  static constexpr uint8_t kWantArrayKind = 0x1;
  static constexpr uint8_t kWantVanillaArray = 0x2;
  static constexpr uint8_t kWantRecord = 0x4;
  static_assert(alignof(Class*) > kWantRecord,
                "Spec bits must fit in lower bits of pointers");

  /*
   * Is this GuardConstraint for a specialized type?
   */
  bool isSpecialized() const;

  /*
   * Constrain this type to a specialized array-like subtype. Guarding to kind
   * is strictly stronger than guarding to vanilla; it sets both flag bits.
   *
   * @requires: isSpecialized()
   */
  GuardConstraint& setWantArrayKind();
  GuardConstraint& setWantVanillaArray();
  bool wantArrayKind() const;
  bool wantVanillaArray() const;

  /*
   * Set, check, or return the specialized Class.
   *
   * @requires:
   *    setDesiredClass: isSpecialized()
   *                     desiredClass() is either nullptr, a parent of `cls',
   *                     or a child of `cls'
   *    desiredClass:    wantClass()
   */
  GuardConstraint& setDesiredClass(const Class* cls);
  bool wantClass() const;
  const Class* desiredClass() const;

  /*
   * Set, check, or return the specialized Record.
   *
   * @requires:
   *    setDesiredRecord: isSpecialized()
   *                      desiredRecord() is either nullptr, a parent of `rec',
   *                      or a child of `rec'
   *    desiredRecord:    wantRecord()
   */
  GuardConstraint& setDesiredRecord(const RecordDesc* rec);
  bool wantRecord() const;
  const RecordDesc* desiredRecord() const;


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
   * `m_specialized' holds a Class*, or a RecordDesc* with a 1 in its
   * second lowest bit, or a 1 in its low bit, indicating
   * that for a DataTypeSpecialized constraint, we require the specified class
   * or the specified record or an array kind, respectively.
   */
  uintptr_t m_specialized;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns true iff `t' is specific enough to fit `gc', meaning a consumer
 * constraining a value with `gc' would be satisfied with `t' as the value's
 * type after relaxation.
 */
bool typeFitsConstraint(Type t, GuardConstraint gc);

/*
 * relaxConstraint returns the least specific GuardConstraint 'gc' that doesn't
 * prevent the intersection of knownType and relaxType(toRelax, gc.category)
 * from satisfying origGc. It is used in IRBuilder::constrain*() functions to
 * determine how to constrain the typeParam and src values of Check
 * instructions, and the src values of Assert instructions.
 *
 * AssertType example:
 * t24:Obj<C> = AssertType<{Obj<C>|InitNull}> t4:Obj
 *
 * If constrainValue is called with (t24, DataTypeSpecialized), relaxConstraint
 * will be called with (DataTypeSpecialized, Obj<C>|InitNull, Obj). After a few
 * iterations it will determine that constraining Obj with
 * DataTypeCountness will still allow the result type of the AssertType
 * instruction to satisfy DataTypeSpecialized, because relaxType(Obj,
 * DataTypeCountness) == Obj.
 */
GuardConstraint relaxConstraint(GuardConstraint origGc,
                                Type knownType, Type toRelax);

/*
 * Return a copy of gc refined with any new information in newGc.
 */
GuardConstraint applyConstraint(GuardConstraint origGc, GuardConstraint newGc);

///////////////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/guard-constraint-inl.h"

#endif
