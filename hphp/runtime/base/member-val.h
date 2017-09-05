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

#ifndef incl_HPHP_MEMB_VAL_H_
#define incl_HPHP_MEMB_VAL_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

#include <cstddef>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct ObjectData;
struct HeapObject;
struct RefData;
struct StringData;
struct TypedValue;
union Value;

///////////////////////////////////////////////////////////////////////////////

/*
 * Encapsulated minstr lval reference to a container and the value and type tag
 * of one of its members.
 */
struct member_lval {
  member_lval();

  /*
   * Opaque union of element pointer types.
   */
  union ptr_u {
    friend member_lval;

    ptr_u(TypedValue* tv) : tv{tv} {}
    ptr_u(Value* val) : val{val} {}
    ptr_u(std::nullptr_t) : tv{nullptr} {}

    explicit operator bool() const { return !!tv; }

  private:
    TypedValue* tv;
    Value* val;
  };

  member_lval(HeapObject* base, ptr_u ptr);
  member_lval(HeapObject* base, TypedValue* elem);

  /*
   * The base type which logically contains the referenced value and type.
   */
  HeapObject* base() const;
  ArrayData* arr_base() const;

  /*
   * Whether this member_lval contains a valid reference to a value and type.
   */
  explicit operator bool() const;
  bool has_ref() const;

  /*
   * References to the value and type.
   *
   * @requires: has_val()
   */
  const Value& val() const;
        Value& val();
  const DataType& type() const;
        DataType& type();

  /*
   * Get a pointer to the referenced TypedValue.
   *
   * The behavior is undefined if the base() does not actually store the value
   * and type as a TypedValue.  Currently, this never happens.
   *
   * TODO(#9077255): Make it happen for monomorphic arrays.
   */
  TypedValue* tv_ptr() const;

  /*
   * Opaque element pointer.
   *
   * This should only be used for constructing new member_lval objects from
   * existing ones.
   */
  ptr_u elem() const;

private:
  union {
    HeapObject* m_base;
    ArrayData* m_arr;
  };
  ptr_u m_ptr;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Encapsulated minstr rval reference to a container and the value and type tag
 * of one of its members.
 *
 * Similar to member_lval, except all the pointers and references are const.
 */
struct member_rval {
  member_rval();

  /*
   * Opaque union of element pointer types.
   */
  union ptr_u {
    friend member_rval;

    ptr_u(const TypedValue* tv) : tv{tv} {}
    ptr_u(const Value* val) : val{val} {}
    ptr_u(std::nullptr_t) : tv{nullptr} {}

    explicit operator bool() const { return !!tv; }

  private:
    const TypedValue* tv;
    const Value* val;
  };

  member_rval(const HeapObject* base, ptr_u ptr);
  member_rval(const HeapObject* base, const TypedValue* elem);

  /*
   * Whether this member_rval contains a valid reference to a value and type.
   */
  explicit operator bool() const;
  bool has_val() const;

  /*
   * References to the value and type.
   *
   * @requires: has_val()
   */
  Value val() const;
  DataType type() const;

  /*
   * Get a pointer to the referenced TypedValue.
   *
   * The behavior is undefined if the base() does not actually store the value
   * and type as a TypedValue.  Currently, this never happens.
   *
   * TODO(#9077255): Make it happen for monomorphic arrays.
   */
  const TypedValue* tv_ptr() const;

  /*
   * Get a copy of the referenced value and type as a TypedValue.
   *
   * @requires: has_val()
   */
  TypedValue tv() const;

  /*
   * Opaque element pointer.
   *
   * This should only be used for constructing new member_rval objects from
   * existing ones.
   */
  ptr_u elem() const;

private:
  union {
    const HeapObject* m_base;
    const ArrayData* m_arr;
    const ObjectData* m_obj;
    const StringData* m_str;
    const RefData* m_ref;
  };
  ptr_u m_ptr;
};

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/member-val-inl.h"

#endif
