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
 *      1-byte m_extra_hi8 field for the fields byte
 *      2-byte bespoke::LayoutIndex (only high byte is used)
 *  8-byte m_raw_positions field
 *  1-byte m_kind field
 *  8-byte m_alias field
 *  8-byte m_typevars field
 *  8-byte m_typevar_types field
 *  { additional fields }
 *
 */

namespace HPHP::bespoke {

using Kind = HPHP::TypeStructure::Kind;

#define TYPE_STRUCTURE_KINDS                \
  X(void, TypeStructure)                    \
  X(int, TypeStructure)                     \
  X(bool, TypeStructure)                    \
  X(float, TypeStructure)                   \
  X(string, TypeStructure)                  \
  X(num, TypeStructure)                     \
  X(arraykey, TypeStructure)                \
  X(noreturn, TypeStructure)                \
  X(mixed, TypeStructure)                   \
  X(nonnull, TypeStructure)                 \
  X(null, TypeStructure)                    \
  X(nothing, TypeStructure)                 \
  X(dynamic, TypeStructure)

#define TYPE_STRUCTURE_CHILDREN_KINDS       \
  X(shape,            TSShape)              \
  X(tuple,            TSTuple)              \
  X(fun,              TSFun)                \
  X(typevar,          TSTypevar)            \
  X(class,            TSWithClassishTypes)  \
  X(interface,        TSWithClassishTypes)  \
  X(enum,             TSWithClassishTypes)  \
  X(trait,            TSWithClassishTypes)  \
  X(dict,             TSWithGenericTypes)   \
  X(vec,              TSWithGenericTypes)   \
  X(keyset,           TSWithGenericTypes)   \
  X(vec_or_dict,      TSWithGenericTypes)   \
  X(darray,           TSWithGenericTypes)   \
  X(varray,           TSWithGenericTypes)   \
  X(varray_or_darray, TSWithGenericTypes)   \
  X(any_array,        TSWithGenericTypes)

/*
 * This macro describes some properties for each field in TypeStructure.
 *  X(Field, FieldString, DataType)
 *   - Field : name of the method to call to access the field in TypeStructure
 *   - FieldString : static string name of the field
 *   - DataType : DataType for the field as a TypedValue
 *   - Pos : the position value of the field for m_raw_positions
 */

#define TYPE_STRUCTURE_FIELDS                                     \
  X(nullable,           nullable,             KindOfBoolean,  1)  \
  X(soft,               soft,                 KindOfBoolean,  2)  \
  /* position 3 unused */                                         \
  X(opaque,             opaque,               KindOfBoolean,  4)  \
  X(optionalShapeField, optional_shape_field, KindOfBoolean,  5)  \
  X(kind,               kind,                 KindOfInt64,    6)  \
  X(alias,              alias,                KindOfString,   7)  \
  X(typevars,           typevars,             KindOfString,   8)  \
  X(typevarTypes,       typevar_types,        KindOfDict,     9)

/*
 * The following _FIELDS macros describe properties for each field belonging
 * specifically to a TypeStructure child struct
 *
 *  X(Field, Type, DataType)
 *   - Field : "m_Field" is the name of the field in the struct
 *   - Type : type of the field
 *   - DataType : DataType for the field as a TypedValue
 *   - Struct : child struct the field belongs to
 *   - FieldsByteOffset : the offset that indicates the field group in kFieldsByte
 *   - Pos : the position value of the field for m_raw_positions
 */

#define TSSHAPE_FIELDS(X)                                                   \
  X(fields,                 ArrayData*,   KindOfDict,     TSShape, 1, 10)       \
  X(allows_unknown_fields,  bool,         KindOfBoolean,  TSShape, 1, 11)

#define TSTUPLE_FIELDS(X)                                                   \
  X(elem_types,             ArrayData*,   KindOfVec,      TSTuple, 2, 10)       \

#define TSFUN_FIELDS(X)                                                     \
  X(param_types,            ArrayData*,   KindOfVec,      TSFun, 3, 10)         \
  X(return_type,            ArrayData*,   KindOfDict,     TSFun, 3, 11)         \
  X(variadic_type,          ArrayData*,   KindOfDict,     TSFun, 3, 12)         \

#define TSTYPEVAR_FIELDS(X)                                                 \
  X(name,             StringData*,  KindOfString,   TSTypevar, 4, 10)

#define TSGENERIC_FIELDS(X)                                                 \
  X(generic_types,    ArrayData*,   KindOfVec,      TSWithGenericTypes, 5, 10)  \

#define TSCLASSISH_FIELDS(X)                                                \
  X(generic_types,    ArrayData*,   KindOfVec,      TSWithClassishTypes, 5, 10) \
  X(classname,        StringData*,  KindOfString,   TSWithClassishTypes, 6, 11) \
  X(exact,            bool,         KindOfBoolean,  TSWithClassishTypes, 6, 12)

#define TYPE_STRUCTURE_CHILDREN_FIELDS                    \
  TSSHAPE_FIELDS(X)                                       \
  TSTUPLE_FIELDS(X)                                       \
  TSFUN_FIELDS(X)                                         \
  TSTYPEVAR_FIELDS(X)                                     \
  TSCLASSISH_FIELDS(X)                                    \
  TSGENERIC_FIELDS(X)

#define TSCHILDREN_METHODS(T)                                           \
  void incRefFields();                                                  \
  void decRefFields();                                                  \
  bool containsField(const StringData* k) const;                        \
  bool checkInvariants() const;                                         \
  bool checkBespokeChildren() const;                                    \
  static TypedValue tsNvGetStr(const T* tad, const StringData* k);      \
  static void scan(const T* tad, type_scan::Scanner& scanner);          \
  static void initializeFields(T* tad);                                 \
  static bool setField(T* tad, StringData* k, TypedValue v, bool nested); \
  static void convertToUncounted(T* tad, const MakeUncountedEnv& env);  \
  static void releaseUncounted(T* tad);                                 \
  static TypedValue getKeyFromPositionValue(const T* tad, int value);   \
  static TypedValue getValFromPositionValue(const T* tad, int value);   \
  static size_t getFieldOffset(const StringData* k);                    \
  static int countFields(const ArrayData* ad);                          \
  static void onSetEvalScalar(T* tad);

/*
 * The FieldsByte dictates the specific fields that should exist on each
 * bespoke type struct. Each bit is set iff the field(s) at that offset could
 * exist in the struct. The bit that represents each field is defined in
 * the _FIELDS macros as an offset, and these are generally separated according
 * to the different child structs.
 *
 * Fields in the base TypeStructure always exist and are represented by the offset 0.
 *
 * Child structs generally contain distinct fields. The only exception is the
 * field generic_types, which can exist for both TSWithClassishTypes and
 * TSWithGenericTypes, so the bit is set for both.
 */
constexpr uint8_t kTypeStructureFieldsByte    = 0b00000001;
constexpr uint8_t kTSShapeFieldsByte          = 0b00000011;
constexpr uint8_t kTSTupleFieldsByte          = 0b00000101;
constexpr uint8_t kTSFunFieldsByte            = 0b00001001;
constexpr uint8_t kTSTypevarFieldsByte        = 0b00010001;
constexpr uint8_t kTSGenericTypesFieldsByte   = 0b00100001;
constexpr uint8_t kTSClassishTypesFieldsByte  = 0b01100001;

//////////////////////////////////////////////////////////////////////////////

struct TypeStructureLayout;

/*
 * TypeStructure should only contain the kinds specified in TYPE_STRUCTURE_KINDS
 */
struct TypeStructure : BespokeArray {
  static void InitializeLayouts();
  static LayoutIndex GetLayoutIndex();
  static bool isBespokeTypeStructure(const ArrayData* ad);
  static bool isFullyBespokeTypeStructure(const ArrayData* ad);

  // returns whether an array that is not a bespoke type structure can be
  // converted into the bespoke version
  static bool isValidTypeStructure(const ArrayData* ad);

  static TypeStructure* MakeFromVanilla(ArrayData* ad, bool forceNonStatic = false);
  static TypeStructure* MakeFromVanillaStatic(ArrayData* ad, bool nested);
  // MakeFromVanillaNested always creates a non-static array
  static ArrayData* MakeFromVanillaNested(ArrayData* ad);
  static const TypeStructure* As(const ArrayData* ad);
  static TypeStructure* As(ArrayData* ad);
  static void OnSetEvalScalar(TypeStructure* tad);
  static ArrayData* CopyStatic(const TypeStructure* tad);

  template<bool Static>
  static TypeStructure* MakeReserve(
    bool legacy, HPHP::TypeStructure::Kind kind);
  template<bool Static>
  TypeStructure* copy() const;

  bool checkInvariants() const;
  ArrayData* escalateWithCapacity(size_t capacity, const char* reason) const;
  Kind typeKind() const { return static_cast<Kind>(kind()); }

  enum BitFieldOffsets : uint8_t {
    kNullableOffset = 0,
    kSoftOffset = 1,
    // offset 2 unused
    kOpaqueOffset = 3,
    kOptionalShapeFieldOffset = 4
  };

  static constexpr uint8_t kFieldsByte = kTypeStructureFieldsByte;
  static constexpr uint8_t kMaxPositionValue = 9;
  static constexpr uint8_t kMaxPossibleFields = 12;

  static constexpr size_t kindOffset() {
    static_assert(folly::kIsLittleEndian);
    return offsetof(TypeStructure, m_kind);
  }
  static constexpr size_t bitFieldOffset() {
    return offsetof(TypeStructure, m_extra_lo8);
  }
  static constexpr size_t fieldsByteOffset() {
    return offsetof(TypeStructure, m_extra_hi8);
  }

  // returns the datatype of key and the bit offset for key in kFieldsByte
  static std::pair<DataType, uint8_t> getFieldPair(const StringData* key);

  static size_t getFieldOffset(const StringData* key);
  static uint8_t getBooleanBitOffset(const StringData* key);

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS(TypeStructure)
#undef X

  static bool setField(
    TypeStructure* tad, StringData* k, TypedValue v, bool nested);

  bool nullable() const { return m_extra_lo8 & (1 << kNullableOffset); }
  bool soft() const     { return m_extra_lo8 & (1 << kSoftOffset); }
  bool opaque() const   { return m_extra_lo8 & (1 << kOpaqueOffset); }
  bool optionalShapeField() const {
    return m_extra_lo8 & (1 << kOptionalShapeFieldOffset);
  }
  uint8_t kind() const { return m_kind; }
  StringData* alias() const { return m_alias; }
  StringData* typevars() const { return m_typevars; }
  ArrayData* typevarTypes() const { return m_typevar_types; }
  uint8_t fieldsByte() const { return m_extra_hi8; }
  int8_t getPositionValue(int pos) const {
    if (pos > kMaxPossibleFields) return 0;
    return m_raw_positions >> (pos * 4) & 0xf;
  }
  void setIterationPosition(StringData* field, int i) {
    auto pos = getPositionValueFromField(field);
    assertx(0 <= pos && pos < 0xf);
    m_raw_positions |= pos << (i * 4);
  }

private:
  static size_t sizeIndex(Kind);
  void incRefFields();
  void decRefFields();

  void clearBitField(BitFieldOffsets offset) { m_extra_lo8 &= ~(1 << offset); }
  void setBitField(TypedValue v, BitFieldOffsets offset);
  bool containsField(const StringData* k) const;

  static TypedValue tsNvGetStr(const TypeStructure* tad, const StringData* k);
  static int8_t getPositionValueFromField(StringData* field);

  /*
   * m_raw_positions is used to store the order in which fields are added to
   * the type structure. This order needs to be preserved for iteration and
   * escalation purposes.
   *
   * m_raw_positions is divided into 4-bit chunks, and the value in each chunk
   * represents a field on the type structure. The position value that maps to
   * each field is defined in the _FIELDS macros. The insertion order is stored
   * from the lowest to highest bit, so the lowest bit represents the field
   * that was added first.
   *
   * This is just a packed integer array for memory efficiency.
   *
   * m_raw_positions  ->
   *   |  ...  |  0111  |  0001  |  0110  |
   *                         ^        ^
   *                         |       lowest bit, field was inserted first
   *                         |       value is 0110, so inserted field is 'kind'
   *                         |
   *                        field inserted second
   *
   * A value of 0 indicates nothing was inserted at that position.
   */
  int64_t m_raw_positions;

  uint8_t m_kind;
  StringData* m_alias;
  StringData* m_typevars;
  ArrayData* m_typevar_types;
};

struct TSShape : TypeStructure {
  TSCHILDREN_METHODS(TSShape)
  static constexpr uint8_t kFieldsByte = kTSShapeFieldsByte;
  ArrayData* fields() const { return m_fields; };
  bool allowsUnknownFields() const { return m_allows_unknown_fields; }
private:
  ArrayData* m_fields;
  bool m_allows_unknown_fields;
};

struct TSTuple : TypeStructure {
  TSCHILDREN_METHODS(TSTuple)
  static constexpr uint8_t kFieldsByte = kTSTupleFieldsByte;
  ArrayData* elemTypes() const { return m_elem_types; }
private:
  ArrayData* m_elem_types;
};

struct TSFun : TypeStructure {
  TSCHILDREN_METHODS(TSFun)
  static constexpr uint8_t kFieldsByte = kTSFunFieldsByte;
  ArrayData* paramTypes() const { return m_param_types; }
  ArrayData* returnType() const { return m_return_type; }
  ArrayData* variadicType() const { return m_variadic_type; }
private:
  ArrayData* m_param_types;
  ArrayData* m_return_type;
  ArrayData* m_variadic_type;
};

struct TSTypevar : TypeStructure {
  TSCHILDREN_METHODS(TSTypevar)
  static constexpr uint8_t kFieldsByte = kTSTypevarFieldsByte;
  StringData* name() const { return m_name; }
private:
  StringData* m_name;
};

/*
 * TSWithGenericTypes should only contain the kinds as specified
 * in TYPE_STRUCTURE_CHILDREN_KINDS
 */
struct TSWithGenericTypes : TypeStructure {
  TSCHILDREN_METHODS(TSWithGenericTypes)
  static constexpr uint8_t kFieldsByte = kTSGenericTypesFieldsByte;
  ArrayData* genericTypes() const { return m_generic_types; }
protected:
  ArrayData* m_generic_types;
};

/*
 * TSWithClassishTypes should only contain the kinds as specified
 * in TYPE_STRUCTURE_CHILDREN_KINDS
 */
struct TSWithClassishTypes : TSWithGenericTypes {
  TSCHILDREN_METHODS(TSWithClassishTypes)
  static constexpr uint8_t kFieldsByte = kTSClassishTypesFieldsByte;
  StringData* classname() const { return m_classname; }
  bool exact() const { return m_exact; }
private:
  StringData* m_classname;
  bool m_exact;
};

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

}
