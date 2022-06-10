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

#pragma once

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/layout.h"

/*
 * TypeStructure layout:
 * This should store fields that could appear on any kind of type structure.
 * More specific fields are defined in T<Kind> children structs that inherit
 * from TypeStructure.
 *
 *  16-byte ArrayData header:
 *      8-byte HeapObject header:
 *           4-byte refcount
 *           1-byte ArrayKind
 *           1-byte GC bits
 *           1-byte aux bits
 *           1-byte size index
 *      4-byte m_size field
 *      1-byte m_extra_lo8 field for boolean fields
 *      1-byte m_extra_hi8 field for kind
 *      2-byte bespoke::LayoutIndex (only high byte is used)
 *  8-byte m_alias field
 *  8-byte m_typevars field
 *  8-byte m_typevar_types field
 *
 */

namespace HPHP::bespoke {

using Kind = HPHP::TypeStructure::Kind;

/*
 * This macro describes some properties for each field in TypeStructure.
 *  X(Field, FieldString, DataType)
 *   - Field : name of the method to call to access the field in TypeStructure
 *   - FieldString : static string name of the field
 *   - DataType : DataType for the field as a TypedValue
 */
#define TYPE_STRUCTURE_FIELDS                                \
  X(nullable,           nullable,             KindOfBoolean) \
  X(soft,               soft,                 KindOfBoolean) \
  X(like,               like,                 KindOfBoolean) \
  X(opaque,             opaque,               KindOfBoolean) \
  X(optionalShapeField, optional_shape_field, KindOfBoolean) \
  X(kind,               kind,                 KindOfInt64)   \
  X(alias,              alias,                KindOfString)  \
  X(typevars,           typevars,             KindOfString)  \
  X(typevarTypes,       typevar_types,        KindOfVec)

//////////////////////////////////////////////////////////////////////////////

struct TypeStructureLayout;

struct TypeStructure : BespokeArray {
  static LayoutIndex GetLayoutIndex();
  static void InitializeLayouts();

  static bool isValidTypeStructure(ArrayData* ad);
  static TypeStructure* MakeFromVanilla(ArrayData* ad);
  static const TypeStructure* As(const ArrayData* ad);
  static TypeStructure* As(ArrayData* ad);

  template<bool Static>
  static TypeStructure* MakeReserve(
    bool legacy, HPHP::TypeStructure::Kind kind);

  bool checkInvariants() const;
  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  TypeStructure* copy() const;
  Kind typeKind() const { return static_cast<Kind>(kind()); }

  enum BitFieldOffsets : uint8_t {
    kNullableOffset = 0,
    kSoftOffset = 1,
    kLikeOffset = 2,
    kOpaqueOffset = 3,
    kOptionalShapeFieldOffset = 4
  };

  static auto constexpr kNumFields = 9;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(TypeStructure)
#undef X

  static void setField(TypeStructure* tad, StringData* k, TypedValue v);
  static ssize_t numFields(Kind);

  bool nullable() const { return m_extra_lo8 & (1 << kNullableOffset); }
  bool soft() const { return m_extra_lo8 & (1 << kSoftOffset); }
  bool like() const { return m_extra_lo8 & (1 << kLikeOffset); }
  bool opaque() const { return m_extra_lo8 & (1 << kOpaqueOffset); }
  bool optionalShapeField() const {
    return m_extra_lo8 & (1 << kOptionalShapeFieldOffset);
  }
  int8_t kind() const { return m_extra_hi8; }
  StringData* alias() const { return m_alias; }
  StringData* typevars() const { return m_typevars; }
  ArrayData* typevarTypes() const { return m_typevar_types; }

private:
  static size_t sizeIndex(Kind);
  void incRefFields();
  void decRefFields();

  void clearBitField(BitFieldOffsets offset) {
    m_extra_lo8 &= ~(1 << offset);
  }
  void setBitField(uint8_t val, BitFieldOffsets offset) {
    clearBitField(offset);
    m_extra_lo8 |= val << offset;
  }
  bool containsField(const StringData* k) const;

  StringData* m_alias;
  StringData* m_typevars;
  ArrayData* m_typevar_types;
};

struct TSVoid : TypeStructure {};
struct TSInt : TypeStructure {};
struct TSBool : TypeStructure {};
struct TSFloat : TypeStructure {};
struct TSString : TypeStructure {};
struct TSNum : TypeStructure {};
struct TSArraykey : TypeStructure {};
struct TSNoreturn : TypeStructure {};
struct TSMixed : TypeStructure {};
struct TSNonnull : TypeStructure {};
struct TSNull : TypeStructure {};
struct TSNothing : TypeStructure {};
struct TSDynamic : TypeStructure {};

struct TSShape : TypeStructure {
private:
  bool m_allows_unknown_fields;
  ArrayData* m_fields;
};

struct TSTuple : TypeStructure {
private:
  ArrayData* m_elem_types;
};

struct TSFun : TypeStructure {
private:
  ArrayData* m_param_types;
  ArrayData* m_return_type;
  ArrayData* m_variadic_type;
};

struct TSTypevar : TypeStructure {
private:
  StringData* m_name;
};

struct TypeStructureWithClassishTypes : TypeStructure {
private:
  StringData* m_classname;
  bool m_exact;
};
struct TSClass : TypeStructureWithClassishTypes {};
struct TSInterface : TypeStructureWithClassishTypes {};
struct TSEnum : TypeStructureWithClassishTypes {};
struct TSTrait : TypeStructureWithClassishTypes {};

struct TypeStructureWithGenericTypes : TypeStructure {
private:
  ArrayData* m_generic_types;
};
struct TSDict : TypeStructureWithGenericTypes {};
struct TSVec : TypeStructureWithGenericTypes {};
struct TSKeyset : TypeStructureWithGenericTypes {};
struct TSVecOrDict : TypeStructureWithGenericTypes {};
struct TSDarray : TypeStructureWithGenericTypes {};
struct TSVarray : TypeStructureWithGenericTypes {};
struct TSVarrayOrDarray : TypeStructureWithGenericTypes {};
struct TSAnyArray : TypeStructureWithGenericTypes {};


//////////////////////////////////////////////////////////////////////////////
// Layouts

struct TypeStructureLayout : ConcreteLayout {
  TypeStructureLayout();

  ArrayLayout appendType(Type val) const override;
  ArrayLayout removeType(Type key) const override;
  ArrayLayout setType(Type key, Type val) const override;
  std::pair<Type, bool> elemType(Type key) const override;
  std::pair<Type, bool> firstLastType(bool isFirst, bool isKey) const override;
  Type iterPosType(Type pos, bool isKey) const override;
};

// todo: layouts for specific TStructs

}
