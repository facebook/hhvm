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

#include "hphp/runtime/base/type-structure-helpers.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s_allows_unknown_fields("allows_unknown_fields");
const StaticString s_classname("classname");
const StaticString s_elem_types("elem_types");
const StaticString s_fields("fields");
const StaticString s_kind("kind");
const StaticString s_nullable("nullable");
const StaticString s_optional_shape_field("optional_shape_field");
const StaticString s_unresolved("[unresolved]");

bool cellInstanceOf(const Cell* tv, const NamedEntity* ne) {
  assertx(!isRefType(tv->m_type));
  Class* cls = nullptr;
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfResource:
    case KindOfFunc:
    case KindOfClass:
      return false;

    case KindOfInt64:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_int(cls->name());

    case KindOfDouble:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_double(cls->name());

    case KindOfPersistentString:
    case KindOfString:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_string(cls->name());

    case KindOfPersistentVec:
    case KindOfVec:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_vec(cls->name());

    case KindOfPersistentDict:
    case KindOfDict:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_dict(cls->name());

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_keyset(cls->name());

    case KindOfPersistentArray:
    case KindOfArray:
      cls = Unit::lookupClass(ne);
      return cls && interface_supports_array(cls->name());

    case KindOfObject:
      cls = Unit::lookupClass(ne);
      return cls && tv->m_data.pobj->instanceof(cls);

    case KindOfRef:
      break;
  }
  not_reached();
}

ALWAYS_INLINE
bool shapeAllowsUnknownFields(const Array& ts) {
  return
    ts.exists(s_allows_unknown_fields) &&
    ts[s_allows_unknown_fields].asBooleanVal();
}

ALWAYS_INLINE
bool isOptionalShapeField(const ArrayData* field) {
  auto const property = s_optional_shape_field.get();
  return field->exists(property) && tvCastToBoolean(field->at(property));
}

template <bool genErrorMessage>
bool checkTypeStructureMatchesCellImpl(
  const Array& ts,
  Cell c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
) {
  auto errOnLen = [&givenType](auto cell, auto len) {
    if (genErrorMessage) {
      givenType = folly::sformat("{} of length {}",
        describe_actual_type(&cell, true), len);
    }
  };

  auto errOnKey = [&errorKey](Cell key) {
    if (genErrorMessage) {
      std::string escapedKey;
      if (isStringType(key.m_type)) {
        escapedKey = folly::sformat("\"{}\"",
          folly::cEscape<std::string>(key.m_data.pstr->toCppString()));
      } else {
        assertx(isIntType(key.m_type));
        escapedKey = std::to_string(key.m_data.num);
      }
      errorKey = folly::sformat("[{}]{}", escapedKey, errorKey);
    }
  };

  bool result = false;
  auto type = c1.m_type;
  auto data = c1.m_data;
  if (ts.exists(s_nullable) &&
      ts[s_nullable].asBooleanVal() &&
      isNullType(type)) {
    return true;
  }
  assertx(ts.exists(s_kind));
  auto ts_kind = static_cast<TypeStructure::Kind>(ts[s_kind].toInt64Val());
  switch (ts_kind) {
    case TypeStructure::Kind::T_int:
      result = isIntType(type);
      break;
    case TypeStructure::Kind::T_bool:
      result = isBoolType(type);
      break;
    case TypeStructure::Kind::T_float:
      result = isDoubleType(type);
      break;
    case TypeStructure::Kind::T_string:
      if (isFuncType(type)) {
        if (RuntimeOption::EvalRaiseFuncConversionWarning) {
          raise_warning("Func to string conversion");
        }
        result = true;
        break;
      } else if (isClassType(type)) {
        if (RuntimeOption::EvalRaiseClassConversionWarning) {
          raise_warning("Class to string conversion");
        }
        result = true;
        break;
      }
      result = isStringType(type);
      break;
    case TypeStructure::Kind::T_resource:
      result =
        isResourceType(type) &&
        !reinterpret_cast<const Resource*>(&data.pres)->isInvalid();
      break;
    case TypeStructure::Kind::T_num:
      result = isIntType(type) || isDoubleType(type);
      break;
    case TypeStructure::Kind::T_arraykey:
      result = isIntType(type) || isStringType(type);
      break;
    case TypeStructure::Kind::T_dict:
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
        if (isArrayType(type) && data.parr->isDArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_DARR_IS_DICT);
        }
      }
      result = isDictType(type);
      break;
    case TypeStructure::Kind::T_vec:
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
        if (isArrayType(type) && data.parr->isVArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VARR_IS_VEC);
        }
      }
      result = isVecType(type);
      break;
    case TypeStructure::Kind::T_keyset:
      result = isKeysetType(type);
      break;
    case TypeStructure::Kind::T_vec_or_dict:
      result = isVecType(type) || isDictType(type);
      break;
    case TypeStructure::Kind::T_arraylike:
      result = isArrayType(type) || isVecType(type) ||
               isDictType(type) || isKeysetType(type);
      break;
    case TypeStructure::Kind::T_enum: {
      assertx(ts.exists(s_classname));
      auto const cls = Unit::lookupClass(ts[s_classname].asStrRef().get());
      result = cls && enumHasValue(cls, &c1);
      break;
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      assertx(ts.exists(s_classname));
      auto const ne = NamedEntity::get(ts[s_classname].asStrRef().get());
      result = cellInstanceOf(&c1, ne);
      break;
    }
    case TypeStructure::Kind::T_void:
      result = isNullType(type);
      break;
    case TypeStructure::Kind::T_noreturn:
      result = false;
      break;
    case TypeStructure::Kind::T_mixed:
      return true;
    case TypeStructure::Kind::T_nonnull:
      result = !isNullType(type);
      break;
    case TypeStructure::Kind::T_tuple: {
      if (!isArrayLikeType(type)) {
        result = false;
        break;
      }
      auto const elems = data.parr;
      if (!elems->isVecOrVArray()) {
        if (!RuntimeOption::EvalHackArrDVArrs && elems->isDArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow darrays
          // for tuples and log a warning. Fall through here.
        } else {
          result = false;
          break;
        }
      }
      assertx(ts.exists(s_elem_types));
      auto const tsElems = ts[s_elem_types].getArrayData();
      if (elems->size() != tsElems->size()) {
        errOnLen(c1, elems->size());
        result = false;
        break;
      }
      bool elemsDidMatch = true;
      bool keysDidMatch = true;
      int index = 0;
      IterateKV(
        elems,
        [&](Cell k, TypedValue elem) {
          if (!isIntType(k.m_type) || k.m_data.num != index++) {
            keysDidMatch = false;
            return true;
          }
          auto const& ts2 = asCArrRef(tsElems->rval(k.m_data.num));
          if (!checkTypeStructureMatchesCellImpl<genErrorMessage>(
              ts2, tvToCell(elem), givenType, expectedType, errorKey)) {
            elemsDidMatch = false;
            errOnKey(k);
            return true;
          }
          return false;
        }
      );
      if (!keysDidMatch) {
        result = false;
        break;
      }
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
        if (elemsDidMatch && elems->isDArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow darrays
          // for tuples and log a warning.
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_TUPLE_IS_DARR);
        }
      }
      return elemsDidMatch;
    }
    case TypeStructure::Kind::T_shape: {
      if (!isArrayLikeType(type)) {
        result = false;
        break;
      }
      auto const fields = data.parr;
      if (!fields->isDictOrDArray()) {
        if (!RuntimeOption::EvalHackArrDVArrs && fields->isVArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow varrays
          // for shapes and log a warning. Fall through here.
        } else {
          result = false;
          break;
        }
      }
      assertx(ts.exists(s_fields));
      auto const tsFields = ts[s_fields].getArrayData();
      auto const numDefinedFields = tsFields->size();
      auto const numFields = fields->size();
      auto numRequiredFields = 0;
      IterateV(
        tsFields,
        [&](TypedValue v) {
          assertx(isArrayLikeType(v.m_type));
          numRequiredFields += !isOptionalShapeField(v.m_data.parr);
        }
      );
      if (numFields < numRequiredFields) {
        errOnLen(c1, numFields);
        result = false;
        break;
      }
      auto const allowsUnknownFields = shapeAllowsUnknownFields(ts);
      if (!allowsUnknownFields && numFields > numDefinedFields) {
        errOnLen(c1, numFields);
        result = false;
        break;
      }
      auto fieldsDidMatch = true;
      auto numExpectedFields = 0;
      IterateKV(
        tsFields,
        [&](Cell k, TypedValue v) {
          assertx(isArrayLikeType(v.m_type));
          auto const tsFieldData = v.m_data.parr;
          if (!fields->exists(k)) {
            if (isOptionalShapeField(tsFieldData)) {
              return false;
            }
            fieldsDidMatch = false;
            errOnKey(k);
            return true;
          }
          auto const tsField = ArrNR(tsFieldData);
          auto const field = fields->at(k);
          if (!checkTypeStructureMatchesCellImpl<genErrorMessage>(
              tsField, tvToCell(field), givenType, expectedType, errorKey)) {
            fieldsDidMatch = false;
            errOnKey(k);
            return true;
          }
          numExpectedFields++;
          return false;
        }
      );
      if (!fieldsDidMatch) {
        result = false;
        break;
      }
      bool didSucceed = allowsUnknownFields || numFields == numExpectedFields;
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsArrayNotices)) {
        if (didSucceed && fields->isVArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow varrays
          // for shapes and log a warning.
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_SHAPE_IS_VARR);
        }
      }
      return didSucceed;
    }
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
      result = false;
      break;
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
      // Not supported, should have already thrown an error
      // on these during resolution
      always_assert(false);
  }
  if (genErrorMessage && !result) {
    if (givenType.empty()) givenType = describe_actual_type(&c1, true);
    if (expectedType.empty()) {
      expectedType = TypeStructure::toStringForDisplay(ts).toCppString();
    }
  }
  return result;
}

bool checkTypeStructureMatchesCell(const Array& ts, Cell c1) {
  std::string givenType, expectedType, errorKey;
  return checkTypeStructureMatchesCellImpl<false>(
    ts, c1, givenType, expectedType, errorKey);
}

bool checkTypeStructureMatchesCell(
  const Array& ts,
  Cell c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
) {
  return checkTypeStructureMatchesCellImpl<true>(
    ts, c1, givenType, expectedType, errorKey);
}

void errorOnIsAsExpressionInvalidTypes(const Array&);

ALWAYS_INLINE
void errorOnIsAsExpressionInvalidTypesList(const ArrayData* tsFields) {
  IterateV(
    tsFields,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      errorOnIsAsExpressionInvalidTypes(ArrNR(v.m_data.parr));
    }
  );
}

void errorOnIsAsExpressionInvalidTypes(const Array& ts) {
  assertx(ts.exists(s_kind));
  auto ts_kind = static_cast<TypeStructure::Kind>(ts[s_kind].toInt64Val());
  switch (ts_kind) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_xhp:
      return;
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
      raise_error("\"is\" and \"as\" operators cannot be used with an array");
    case TypeStructure::Kind::T_fun:
      raise_error("\"is\" and \"as\" operators cannot be used with a function");
    case TypeStructure::Kind::T_typevar:
      raise_error(
        "\"is\" and \"as\" operators cannot be used with a generic type");
    case TypeStructure::Kind::T_trait:
      raise_error("\"is\" and \"as\" operators cannot be used with a trait");
    case TypeStructure::Kind::T_tuple: {
      assertx(ts.exists(s_elem_types));
      auto const elemsArr = ts[s_elem_types].getArrayData();
      errorOnIsAsExpressionInvalidTypesList(elemsArr);
      return;
    }
    case TypeStructure::Kind::T_shape: {
      assertx(ts.exists(s_fields));
      auto const tsFields = ts[s_fields].getArrayData();
      errorOnIsAsExpressionInvalidTypesList(tsFields);
      return;
    }
  }
  not_reached();
}

/**
 * Returns whether the type structure may not be able to be resolved statically,
 * i.e. if it contains `this` references.
 */
bool typeStructureCouldBeNonStatic(const Array& ts) {
  assertx(ts.exists(s_kind));
  switch (static_cast<TypeStructure::Kind>(ts[s_kind].toInt64Val())) {
    case TypeStructure::Kind::T_tuple:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_shape:
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
      return true;
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_xhp:
      return false;
  }
  not_reached();
}

Array resolveAndVerifyTypeStructure(
  const Array& ts,
  const Class* declaringCls,
  const Class* calledCls,
  bool suppress
) {
  assertx(!ts.empty());
  assertx(ts.isDictOrDArray());
  Array resolved;
  try {
    bool persistent = true;
    resolved = TypeStructure::resolve(ts, calledCls, declaringCls, persistent);
  } catch (Exception& e) {
    // Catch and throw again so we get a line number
    auto const errMsg = e.getMessage();
    if (!suppress) raise_error(errMsg);
    if (RuntimeOption::EvalIsExprEnableUnresolvedWarning) raise_warning(errMsg);
    // Lets just return an unresolved array instead
    resolved = Array::CreateDArray();
    resolved.add(s_kind,
                 Variant(static_cast<uint8_t>(
                         TypeStructure::Kind::T_unresolved)));
    resolved.add(s_classname, Variant(s_unresolved));
  }
  assertx(!resolved.empty());
  assertx(resolved.isDictOrDArray());
  errorOnIsAsExpressionInvalidTypes(resolved);
  return resolved;
}

void throwTypeStructureDoesNotMatchCellException(
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
) {
  assertx(!givenType.empty());
  assertx(!expectedType.empty());
  std::string error;
  if (errorKey.empty()) {
    error = folly::sformat("Expected {}, got {}", expectedType, givenType);
  } else {
    error = folly::sformat("Expected {} at {}, got {}",
      expectedType, errorKey, givenType);
  }
  SystemLib::throwTypeAssertionExceptionObject(error);
}

} // namespace HPHP
