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
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/unit-cache.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/type-constraint.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s_unresolved("[unresolved]");

namespace {

template<typename F>
bool cellInstanceOfImpl(const Cell* tv, F lookupClass) {
  assertx(!isRefType(tv->m_type));
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfResource:
    case KindOfRecord:
      return false;

    case KindOfClass: {
      auto const cls = lookupClass();
      if (cls && interface_supports_string(cls->name())) {
        classToStringHelper(tv->m_data.pclass); // maybe raise a warning
        return true;
      }
      return false;
    }

    case KindOfFunc: {
      auto const cls = lookupClass();
      if (cls && interface_supports_string(cls->name())) {
        funcToStringHelper(tv->m_data.pfunc); // maybe raise a warning
        return true;
      }
      return false;
    }

    case KindOfInt64: {
      auto const cls = lookupClass();
      return cls && interface_supports_int(cls->name());
    }

    case KindOfDouble: {
      auto const cls = lookupClass();
      return cls && interface_supports_double(cls->name());
    }

    case KindOfPersistentString:
    case KindOfString: {
      auto const cls = lookupClass();
      return cls && interface_supports_string(cls->name());
    }

    case KindOfPersistentVec:
    case KindOfVec: {
      auto const cls = lookupClass();
      return cls && interface_supports_vec(cls->name());
    }

    case KindOfPersistentDict:
    case KindOfDict: {
      auto const cls = lookupClass();
      return cls && interface_supports_dict(cls->name());
    }

    case KindOfPersistentKeyset:
    case KindOfKeyset: {
      auto const cls = lookupClass();
      return cls && interface_supports_keyset(cls->name());
    }

    case KindOfPersistentShape:
    case KindOfShape: {
      auto const cls = lookupClass();
      if (RuntimeOption::EvalHackArrDVArrs) {
        return cls && interface_supports_dict(cls->name());
      }
      return cls && interface_supports_array(cls->name());
    }

    case KindOfPersistentArray:
    case KindOfArray: {
      auto const cls = lookupClass();
      return cls && interface_supports_array(cls->name());
    }

    case KindOfObject: {
      auto const cls = lookupClass();
      return cls && tv->m_data.pobj->instanceof(cls);
    }

    case KindOfClsMeth: {
      auto const cls = lookupClass();
      if (cls && (RuntimeOption::EvalHackArrDVArrs ?
        interface_supports_vec(cls->name()) :
        interface_supports_array(cls->name()))) {
        return true;
      }
      return false;
    }

    case KindOfRef:
      break;
  }
  not_reached();
}

} // namespace

bool cellInstanceOf(const Cell* tv, const NamedEntity* ne) {
  return cellInstanceOfImpl(tv, [ne]() { return Unit::lookupClass(ne); });
}

bool cellInstanceOf(const Cell* tv, const Class* cls) {
  return cellInstanceOfImpl(tv, [cls]() { return cls; });
}

namespace {

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

ALWAYS_INLINE
ArrayData* getShapeFieldElement(const TypedValue& v) {
  assertx(tvIsDictOrDArray(&v));
  auto const result = v.m_data.parr;
  auto const valueField = result->rval(s_value.get());
  if (!valueField.is_set()) return result;
  assertx(tvIsDictOrDArray(valueField));
  return valueField.val().parr;
}

ALWAYS_INLINE
folly::Optional<ArrayData*> getGenericTypesOpt(const ArrayData* ts) {
  auto const generics_field = ts->rval(s_generic_types.get());
  if (!generics_field.is_set()) return folly::none;
  assertx(isArrayType(generics_field.type()));
  return generics_field.val().parr;
}

ALWAYS_INLINE
bool isWildCard(const ArrayData* ts) {
  return get_ts_kind(ts) == TypeStructure::Kind::T_typevar &&
         ts->exists(s_name.get()) &&
         get_ts_name(ts)->equal(s_wildcard.get());
}

bool typeStructureIsTypeList(
  const ArrayData*,
  const ArrayData*,
  const StringData*,
  bool&,
  bool
);

/*
 * Checks whether the `input` type structure is of the type denoted by the type
 * structure called `type`
 *
 * For example:
 * typeStructureIsType(int, int) -> true
 * typeStructureIsType((int, string), (int, _)) -> true
 * typeStructureIsType((int, string), (_, int)) -> false
 *
 * This is similar to instanceof check but also works with wildcards but it
 * currently does not implement subtyping checks (TODO(T31677864))
 *
 * Strict parameter dictates whether to return false on erased generics and
 * whether to treat unresolved as a class name.
 * If class C takes an erased generic T, then
 * if strict, then typeStructureIsType(C<_>, C<int>) return false,
 * if not strict, then typeStructureIsType(C<_>, C<int>) returns true.
 */
bool typeStructureIsType(
  const ArrayData* input,
  const ArrayData* type,
  bool& warn,
  bool strict
) {
  assertx(input && type);
  if (input == type) return true; // shortcut
  if (!strict && isWildCard(input)) return true;
  auto tsKind = get_ts_kind(type);
  switch (tsKind) {
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike:
      if (is_ts_nullable(type)) {
        auto const inputT = get_ts_kind(input);
        return inputT == tsKind || inputT == TypeStructure::Kind::T_null;
      }
      if (!type->equal(input, true)) {
        if (is_ts_soft(type)) {
          auto ts = type->copy();
          // Let's try once again without the soft annotation
          auto const newType = ts->remove(s_soft.get());
          return newType->equal(input, true);
        }
        return false;
      }
      return true;
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      auto const classname_field = input->rval(s_classname.get());
      if (!classname_field.is_set()) return false;
      assertx(isStringType(classname_field.type()));
      auto const name = classname_field.val().pstr;
      if (!name->isame(get_ts_classname(type))) return false;
      auto const inputGenerics = getGenericTypesOpt(input);
      auto const typeGenerics = getGenericTypesOpt(type);
      if (!inputGenerics) {
        return !strict || (!typeGenerics && type->equal(input, true));
      }
      return typeGenerics && typeStructureIsTypeList(*inputGenerics,
                                                     *typeGenerics,
                                                     name,
                                                     warn,
                                                     strict);
    }
    case TypeStructure::Kind::T_tuple:
      return typeStructureIsTypeList(
        get_ts_elem_types(input),
        get_ts_elem_types(type),
        nullptr,
        warn,
        strict
      );
    case TypeStructure::Kind::T_shape: {
      auto const inputFields = get_ts_fields(input);
      auto const typeFields = get_ts_fields(type);
      if (inputFields->size() != typeFields->size()) return false;
      if (does_ts_shape_allow_unknown_fields(type) ^
          does_ts_shape_allow_unknown_fields(input)) {
        // either both needs to allow or neither
        return false;
      }
      auto result = true;
      // If any of the generics will warn, we need to keep track of it and
      // only warn if none of the other ones raise an error
      bool willWarn = false;
      IterateKV(
        typeFields,
        [&](Cell k, TypedValue v) {
          assertx(tvIsDictOrDArray(v));
          auto typeField = getShapeFieldElement(v);
          if (!inputFields->exists(k)) {
            result = false;
            return true; // short circuit
          }
          if (isOptionalShapeField(v.m_data.parr) ^
              isOptionalShapeField(inputFields->at(k).m_data.parr)) {
            // either both needs to be optional or neither
            result = false;
            return true; // short circuit
          }
          auto inputField = getShapeFieldElement(inputFields->at(k));
          // If this field is associated with the value, kill it
          // This is safe since we already checked this above
          auto cleanedInput = inputField->remove(s_optional_shape_field.get());
          auto cleanedType = typeField->remove(s_optional_shape_field.get());
          if (!typeStructureIsType(cleanedInput, cleanedType, strict, warn)) {
            if (warn || is_ts_soft(typeField)) {
              willWarn = true;
              warn = false;
              return false;
            }
            result = false;
            return true; // short circuit
          }
          return false;
        }
      );
      if (willWarn && result) {
        warn = true;
        result = false;
      }
      return result;
    }
    case TypeStructure::Kind::T_typevar:
      // Only true if the typevar is a wildcard
      return type->exists(s_name.get()) &&
        get_ts_name(type)->equal(s_wildcard.get());
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_reifiedtype:
    case TypeStructure::Kind::T_unresolved:
      raise_error("Invalid generics for type structure");
  }
  not_reached();
}

bool typeStructureIsTypeList(
  const ArrayData* inputL,
  const ArrayData* typeL,
  const StringData* clsname,
  bool& warn,
  bool strict
) {
  assertx(inputL && typeL);
  auto const size = typeL->size();
  if (inputL->size() != size) return false;
  std::vector<TypeParamInfo> tpinfo;
  bool found = false;
  if (!strict && clsname) {
    auto const ne = NamedEntity::get(clsname);
    if (auto const cls = Unit::lookupClass(ne)) {
      found = true;
      tpinfo = cls->getReifiedGenericsInfo().m_typeParamInfo;
      if (tpinfo.size() == 0) return true;
      if (tpinfo.size() != size) return false;
    }
  }
  // If any of the generics will warn, we need to keep track of it and only warn
  // if none of the other ones raise an error
  bool willWarn = false;
  for (size_t i = 0; i < size; ++i) {
    if (found && !tpinfo[i].m_isReified) continue;
    auto const inputElem = inputL->rval(i);
    auto const typeElem = typeL->rval(i);
    assertx(tvIsDictOrDArray(inputElem) && tvIsDictOrDArray(typeElem));
    if (!typeStructureIsType(inputElem.val().parr, typeElem.val().parr,
                             warn, strict)) {
      if (warn || (found && (tpinfo[i].m_isWarn ||
                             is_ts_soft(typeElem.val().parr)))) {
        willWarn = true;
        warn = false;
        continue;
      }
      return false;
    }
  }
  if (!willWarn) return true;
  warn = true;
  return false;
}

ALWAYS_INLINE
bool checkReifiedGenericsMatch(
  const Array& ts,
  Cell c1,
  const NamedEntity* ne,
  bool& warn,
  bool strict // whether to return false on erased generics
) {
  if (!ts.exists(s_generic_types)) return true;
  // TODO(T31677864): Handle non KindOfObject types
  if (c1.m_type != KindOfObject) return true;
  auto const obj = c1.m_data.pobj;
  auto const cls = Unit::lookupClass(ne);
  assertx(cls);
  if (!cls->hasReifiedGenerics()) {
    if (!strict) return true;
    // Before returning false, lets check if all the generics are wildcards
    // If not all wildcard, since this is not a reified class, then it is false
    return isTSAllWildcards(ts.get());
  }
  auto const obj_generics = getClsReifiedGenericsProp(cls, obj);
  auto const generics = ts[s_generic_types].getArrayData();
  auto const size = obj_generics->size();
  if (size != generics->size()) return false;

  // If any of the generics will warn, we need to keep track of it and only warn
  // if none of the other ones raise an error
  bool willWarn = false;
  for (size_t i = 0; i < size; ++i) {
    auto const objrg = obj_generics->rval(i);
    auto const rg = generics->rval(i);
    assertx(tvIsDictOrDArray(objrg) && tvIsDictOrDArray(rg));
    auto const tsvalue = rg.val().parr;
    if (get_ts_kind(tsvalue) == TypeStructure::Kind::T_typevar &&
        tsvalue->exists(s_name.get()) &&
        get_ts_name(tsvalue)->equal(s_wildcard.get())) {
      continue;
    }
    if (!typeStructureIsType(objrg.val().parr, tsvalue, warn, strict)) {
      auto const tpinfo = cls->getReifiedGenericsInfo().m_typeParamInfo;
      assertx(tpinfo.size() == size);
      if (warn || tpinfo[i].m_isWarn || is_ts_soft(tsvalue)) {
        willWarn = true;
        warn = false;
        continue;
      }
      return false;
    }
  }
  if (!willWarn) return true;
  warn = true;
  return false;
}

} // namespace

bool isTSAllWildcards(const ArrayData* ts) {
  if (!ts->exists(s_generic_types.get())) return true;
  auto const generics = ts->at(s_generic_types.get());
  assertx(isArrayLikeType(generics.m_type));
  auto allWildcard = true;
  IterateV(
    generics.m_data.parr,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      allWildcard &= isWildCard(v.m_data.parr);
      // if there is at least one not wildcard, then we can short circuit
      return !allWildcard;
    }
  );
  return allWildcard;
}

// This function will always be called after `VerifyParamType` instruction, so
// we can make the assumption that if `param` is an object, outermost type in
// `type_` is correct.
// This function will only be called when either `param` is an object or `type_`
// is a primitive reified type parameter
bool verifyReifiedLocalType(
  const ArrayData* type_,
  const TypedValue* param,
  bool isTypeVar,
  bool& warn
) {
  if (tvIsNull(param) && is_ts_nullable(type_)) return true;
  Array type = ArrNR(type_);
  auto const isObj = tvIsObject(param);
  // If it is not coming from a reified type variable and it is an object,
  // we do not need to run the check if the object in question does not have
  // reified generics since `VerifyParamType` should have taken care of it
  if (!isTypeVar && isObj) {
    auto const obj = param->m_data.pobj;
    auto const objcls = obj->getVMClass();
    // Since we already checked that the class matches in VerifyParamType using
    // the type annotation
    if (!objcls->hasReifiedGenerics() && !objcls->hasReifiedParent()) {
      return true;
    }
  }
  // We only need to resolve the type structure if the param is an object and
  // outmost type is unresolved, since there is no way for the outmost type to
  // be resolved but not the inner types due to assumption above.
  assertx(get_ts_kind(type_) != TypeStructure::Kind::T_unresolved || isObj);
  if (isObj && get_ts_kind(type_) == TypeStructure::Kind::T_unresolved) {
    try {
      bool persistent = true;
      type = TypeStructure::resolve(type, nullptr, nullptr,
                                    req::vector<Array>(), persistent);
    } catch (Exception& e) {
      return false;
    } catch (Object& e) {
      return false;
    }
  }
  return checkTypeStructureMatchesCell(type, *param, warn);
}

/*
 * Sets the warn flag if the type parameter is denoted as soft either through
 * an annotation at the declaration site or by soft type hint at the generic
 * level
 * If isOrAsOp flag is set, this means we are running this check for is
 * type testing or as type assertion operations. For these operations,
 * we reject comparisons over {v,d,}array since we do want not users to be able
 * distinguish {v,d,}arrays while HackArrayMigration is in progress.
 * Being able to tell between them cripples the ability to transparently
 * switch between them in userland.
 * When isOrAsOp is not set, we only allow being able to check {v,d,}arrays as
 * an ArrLike, i.e. we do not allow finer checks.
 */
template <bool genErrorMessage>
bool checkTypeStructureMatchesCellImpl(
  const Array& ts,
  Cell c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey,
  bool& warn,
  bool isOrAsOp
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
        if (RuntimeOption::EvalIsStringNotices) {
          raise_notice("Func used in is_string");
        }
        result = true;
        break;
      } else if (isClassType(type)) {
        if (RuntimeOption::EvalIsStringNotices) {
          raise_notice("Class used in is_string");
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
      result = isDictOrShapeType(type);
      break;
    case TypeStructure::Kind::T_vec:
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatIsVecDictNotices)) {
        if (isArrayType(type) && data.parr->isVArray()) {
          raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_VARR_IS_VEC);
        }
      }
      if (isClsMethType(type)) {
        if (RuntimeOption::EvalHackArrDVArrs) {
          if (RuntimeOption::EvalIsVecNotices) {
            raise_notice(Strings::CLSMETH_COMPAT_IS_VEC);
          }
          result = true;
        } else {
          result = false;
        }
        break;
      }
      result = isVecType(type);
      break;
    case TypeStructure::Kind::T_keyset:
      result = isKeysetType(type);
      break;
    case TypeStructure::Kind::T_vec_or_dict:
      if (isClsMethType(type)) {
        if (RuntimeOption::EvalHackArrDVArrs) {
          if (RuntimeOption::EvalIsVecNotices) {
            raise_notice(Strings::CLSMETH_COMPAT_IS_VEC);
          }
          result = true;
        } else {
          result = false;
        }
        break;
      }
      result = isVecType(type) || isDictOrShapeType(type);
      break;
    case TypeStructure::Kind::T_arraylike:
      if (isClsMethType(type)) {
        if (RuntimeOption::EvalIsVecNotices) {
          raise_notice(Strings::CLSMETH_COMPAT_IS_ANY_ARR);
        }
        result = true;
        break;
      }
      result = isArrayType(type) || isVecType(type) ||
               isDictType(type) || isShapeType(type) || isKeysetType(type);
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
      if (result) result &= checkReifiedGenericsMatch(ts, c1, ne, warn,
                                                      isOrAsOp);
      break;
    }
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
      result = isNullType(type);
      break;
    case TypeStructure::Kind::T_nothing:
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
        if (!RuntimeOption::EvalHackArrDVArrs && elems->isPHPArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow darrays
          // and plain PHP arrays for tuples and log a warning.
          // Fall through here.
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
          auto thisElemWarns = false;
          if (!checkTypeStructureMatchesCellImpl<genErrorMessage>(
              ts2, tvToCell(elem), givenType, expectedType,
              errorKey, thisElemWarns, isOrAsOp)) {
            errOnKey(k);
            if (thisElemWarns) {
              warn = true;
              return false;
            }
            elemsDidMatch = false;
            return true;
          }
          return false;
        }
      );
      // If there is an error, ignore warn
      if (!elemsDidMatch || !keysDidMatch) warn = false;
      if (!keysDidMatch) {
        result = false;
        break;
      }
      if (elemsDidMatch && warn) elemsDidMatch = false;
      if (UNLIKELY(
        RuntimeOption::EvalHackArrCompatIsArrayNotices &&
        elemsDidMatch &&
        elems->isPHPArray()
      )) {
        if (elems->isDArray()) {
          raise_hackarr_compat_is_operator("darray", "tuple");
        } else if (!elems->isVArray()) {
          raise_hackarr_compat_is_operator("array", "tuple");
        }
      }
      result = elemsDidMatch;
      break;
    }
    case TypeStructure::Kind::T_shape: {
      if (!isArrayLikeType(type)) {
        result = false;
        break;
      }
      auto const fields = data.parr;
      if (!fields->isDictOrDArray()) {
        if (!RuntimeOption::EvalHackArrDVArrs && fields->isPHPArray()) {
          // TODO(T29967020) If this is pre-migration, we should allow varrays
          // and plain PHP arrays for shapes and log a warning.
          // Fall through here.
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
          auto const tsField = getShapeFieldElement(v);
          auto const field = fields->at(k);
          bool thisFieldWarns = false;
          numExpectedFields++;
          if (!checkTypeStructureMatchesCellImpl<genErrorMessage>(
              ArrNR(tsField), tvToCell(field), givenType,
              expectedType, errorKey, thisFieldWarns, isOrAsOp)) {
            errOnKey(k);
            if (thisFieldWarns) {
              warn = true;
              return false;
            }
            fieldsDidMatch = false;
            return true;
          }
          return false;
        }
      );
      if (!fieldsDidMatch ||
          !(allowsUnknownFields || numFields == numExpectedFields)) {
        warn = false;
        result = false;
        break;
      }
      if (UNLIKELY(
        RuntimeOption::EvalHackArrCompatIsArrayNotices &&
        !warn &&
        fields->isPHPArray()
      )) {
        if (fields->isVArray()) {
          raise_hackarr_compat_is_operator("varray", "shape");
        } else if (!fields->isDArray()) {
          raise_hackarr_compat_is_operator("array", "shape");
        }
      }
      result = !warn;
      break;
    }
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
      result = !isOrAsOp && isArrayLikeType(type);
      break;
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
      result = false;
      break;
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_reifiedtype:
      // Not supported, should have already thrown an error
      // on these during resolution
      always_assert(false);
  }
  if (!warn && is_ts_soft(ts.get())) warn = true;
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
  bool warn = false;
  return checkTypeStructureMatchesCellImpl<false>(
    ts, c1, givenType, expectedType, errorKey, warn, true);
}

bool checkTypeStructureMatchesCell(
  const Array& ts,
  Cell c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
) {
  bool warn = false;
  return checkTypeStructureMatchesCellImpl<true>(
    ts, c1, givenType, expectedType, errorKey, warn, true);
}

bool checkTypeStructureMatchesCell(
  const Array& ts,
  Cell c1,
  bool& warn
) {
  std::string givenType, expectedType, errorKey;
  return checkTypeStructureMatchesCellImpl<false>(
    ts, c1, givenType, expectedType, errorKey, warn, false);
}

ALWAYS_INLINE
void errorOnIsAsExpressionInvalidTypesList(const ArrayData* tsFields) {
  IterateV(
    tsFields,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      auto arr = v.m_data.parr;
      auto const value_field = arr->rval(s_value.get());
      if (value_field.is_set()) {
        assertx(isArrayType(value_field.type()));
        arr = value_field.val().parr;
      }
      errorOnIsAsExpressionInvalidTypes(ArrNR(arr));
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
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_nothing:
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
    case TypeStructure::Kind::T_reifiedtype:
      raise_error("\"is\" and \"as\" operators cannot be used with "
                  "incomplete type structures");
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
    case TypeStructure::Kind::T_reifiedtype:
      return true;
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_int:
    case TypeStructure::Kind::T_bool:
    case TypeStructure::Kind::T_float:
    case TypeStructure::Kind::T_string:
    case TypeStructure::Kind::T_resource:
    case TypeStructure::Kind::T_num:
    case TypeStructure::Kind::T_arraykey:
    case TypeStructure::Kind::T_nothing:
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

template <bool IsOrAsOp>
Array resolveAndVerifyTypeStructure(
  const Array& ts,
  const Class* declaringCls,
  const Class* calledCls,
  const req::vector<Array>& tsList,
  bool suppress
) {
  assertx(!ts.empty());
  assertx(ts.isDictOrDArray());
  auto const handleResolutionException = [&](auto const& errMsg) {
    if (!suppress || !IsOrAsOp) raise_error(errMsg);
    if (RuntimeOption::EvalIsExprEnableUnresolvedWarning) raise_warning(errMsg);
    auto unresolved = Array::CreateDArray();
    unresolved.set(
      s_kind,
      Variant(static_cast<uint8_t>(TypeStructure::Kind::T_unresolved))
    );
    unresolved.set(s_classname, Variant(s_unresolved));
    return unresolved;
  };
  Array resolved;
  try {
    bool persistent = true;
    resolved =
      TypeStructure::resolve(ts, calledCls, declaringCls, tsList, persistent);
  } catch (Exception& e) {
    // Catch and throw again so we get a line number
    resolved = handleResolutionException(e.getMessage());
  } catch (Object& e) {
    std::string errMsg = "unable to resolve anonymous type structure";
    resolved = handleResolutionException(errMsg);
  }
  assertx(!resolved.empty());
  assertx(resolved.isDictOrDArray());
  if (IsOrAsOp) errorOnIsAsExpressionInvalidTypes(resolved);
  return resolved;
}

template Array resolveAndVerifyTypeStructure<true>(
  const Array&, const Class*, const Class*, const req::vector<Array>&, bool);

template Array resolveAndVerifyTypeStructure<false>(
  const Array&, const Class*, const Class*, const req::vector<Array>&, bool);

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
