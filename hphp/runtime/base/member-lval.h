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

#ifndef incl_HPHP_MEMB_LVAL_H_
#define incl_HPHP_MEMB_LVAL_H_

#include "hphp/runtime/base/datatype.h"

#include <cstddef>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct HeapObject;
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
  union ptr_union {
    friend member_lval;

    ptr_union(TypedValue* tv) : tv{tv} {}
    ptr_union(Value* val) : val{val} {}
    ptr_union(std::nullptr_t) : tv{nullptr} {}

  private:
    TypedValue* tv;
    Value* val;
  };

  member_lval(HeapObject* base, ptr_union ptr);
  member_lval(HeapObject* base, TypedValue* elem);

  /*
   * The base type which logically contains the referenced value and type.
   */
  HeapObject* base() const;
  ArrayData* arr_base() const;

  /*
   * Whether this member_lval contains a valid reference to a value and type.
   */
  bool has_ref() const;

  /*
   * References to the value and type.
   *
   * @requires: has_ref()
   */
  const Value& val() const;
        Value& val();
  const DataType& type() const;
        DataType& type();

  /*
   * Get a pointer to the
   */
  TypedValue* tv() const;

  /*
   * Opaque element pointer.
   *
   * This should only be used for constructing new member_lval objects from
   * existing ones.
   */
  ptr_union elem() const;

private:
  union {
    HeapObject* m_base;
    ArrayData* m_arr;
  };
  ptr_union m_ptr;
};

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/member-lval-inl.h"

#endif
