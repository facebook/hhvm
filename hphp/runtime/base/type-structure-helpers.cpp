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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/tv-comparisons.h"
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
bool tvInstanceOfImpl(const TypedValue* tv, F lookupClass) {
  switch (tv->m_type) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfResource:
    case KindOfRecord:
    case KindOfRFunc:
    case KindOfRClsMeth:
      return false;

    case KindOfClass:
    case KindOfLazyClass: {
      auto const cls = lookupClass();
      if (cls && interface_supports_string(cls->name())) {
        if (RuntimeOption::EvalRaiseClassConversionWarning) {
          raise_warning(Strings::CLASS_TO_STRING);
        }
        return true;
      }
      return false;
    }

    case KindOfFunc: {
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
    case KindOfVec:
    case KindOfPersistentDict:
    case KindOfDict:
    case KindOfPersistentKeyset:
    case KindOfKeyset:
    case KindOfPersistentDArray:
    case KindOfDArray:
    case KindOfPersistentVArray:
    case KindOfVArray: {
      auto const cls = lookupClass();
      return cls && interface_supports_arrlike(cls->name());
    }

    case KindOfObject: {
      auto const cls = lookupClass();
      return cls && tv->m_data.pobj->instanceof(cls);
    }

    case KindOfClsMeth: {
      auto const cls = lookupClass();
      if (cls && interface_supports_arrlike(cls->name())) {
        if (RO::EvalIsVecNotices) {
          raise_notice("Implicit clsmeth to %s conversion",
                       cls->name()->data());
        }
        return true;
      }
      return false;
    }
  }
  not_reached();
}

} // namespace

bool tvInstanceOf(const TypedValue* tv, const NamedEntity* ne) {
  return tvInstanceOfImpl(tv, [ne]() { return Class::lookup(ne); });
}

bool tvInstanceOf(const TypedValue* tv, const Class* cls) {
  return tvInstanceOfImpl(tv, [cls]() { return cls; });
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
  assertx(tvIsHAMSafeDArray(&v));
  auto const result = v.m_data.parr;
  auto const valueField = result->get(s_value.get());
  if (!valueField.is_init()) return result;
  assertx(tvIsHAMSafeDArray(valueField));
  return valueField.val().parr;
}

ALWAYS_INLINE
folly::Optional<ArrayData*> getGenericTypesOpt(const ArrayData* ts) {
  auto const generics_field = ts->get(s_generic_types.get());
  if (!generics_field.is_init()) return folly::none;
  assertx(tvIsHAMSafeVArray(generics_field));
  return generics_field.val().parr;
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
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_dict:
    case TypeStructure::Kind::T_vec:
    case TypeStructure::Kind::T_keyset:
    case TypeStructure::Kind::T_vec_or_dict:
    case TypeStructure::Kind::T_arraylike: {
      if (is_ts_nullable(type)) {
        auto const inputT = get_ts_kind(input);
        return inputT == tsKind || inputT == TypeStructure::Kind::T_null;
      }
      auto const soft = is_ts_soft(type);
      auto const aliasInType = type->exists(s_alias);
      auto const aliasInInput = input->exists(s_alias);
      if (type->size() + aliasInInput != input->size() + soft + aliasInType) {
        return false;
      }
      bool result = true;
      IterateKV(
        input,
        [&](TypedValue k, TypedValue v1) {
          assertx(tvIsString(k));
          if (k.m_data.pstr->isame(s_alias.get())) return false;
          auto const v2 = type->get(k.m_data.pstr);
          if (v2.is_init() && tvEqual(v1, v2)) return false;
          result = false;
          return true; // short circuit
        }
      );
      return result;
    }
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      auto const classname_field = input->get(s_classname.get());
      if (!classname_field.is_init()) return false;
      assertx(isStringType(classname_field.type()));
      auto const name = classname_field.val().pstr;
      if (!name->isame(get_ts_classname(type))) return false;
      auto const inputGenerics = getGenericTypesOpt(input);
      auto const typeGenerics = getGenericTypesOpt(type);
      if (!inputGenerics) {
        return !strict || (!typeGenerics && type->same(input));
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
        [&](TypedValue k, TypedValue v) {
          assertx(tvIsHAMSafeDArray(v));
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
          if (!typeStructureIsType(cleanedInput, cleanedType, warn, strict)) {
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
    case TypeStructure::Kind::T_fun: {
      auto const inputR = get_ts_return_type(input);
      auto const typeR = get_ts_return_type(type);
      if (!typeStructureIsType(inputR, typeR, warn, strict)) {
        return false;
      }
      auto const inputP = get_ts_param_types(input);
      auto const typeP = get_ts_param_types(type);
      if (!typeStructureIsTypeList(inputP, typeP, nullptr, warn, strict)) {
        return false;
      }
      auto const inputV = get_ts_variadic_type_opt(input);
      auto const typeV = get_ts_variadic_type_opt(type);
      return inputV && typeV
        ? typeStructureIsType(inputV, typeV, warn, strict)
        : inputV == typeV;
    }
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_typeaccess:
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
    if (auto const cls = Class::lookup(ne)) {
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
    auto const inputElem = inputL->get(i);
    auto const typeElem = typeL->get(i);
    assertx(tvIsHAMSafeDArray(inputElem) && tvIsHAMSafeDArray(typeElem));
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
  TypedValue c1,
  const NamedEntity* ne,
  bool& warn,
  bool strict // whether to return false on erased generics
) {
  if (!ts.exists(s_generic_types)) return true;
  // TODO(T31677864): Handle non KindOfObject types
  if (c1.m_type != KindOfObject) return true;
  auto const obj = c1.m_data.pobj;
  auto const cls = Class::lookup(ne);
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
    auto const objrg = obj_generics->get(i);
    auto const rg = generics->get(i);
    assertx(tvIsHAMSafeDArray(objrg) && tvIsHAMSafeDArray(rg));
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
  tv_rval param,
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
    auto const obj = val(param).pobj;
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
      if (is_ts_soft(type_)) warn = true;
      return false;
    } catch (Object& e) {
      if (is_ts_soft(type_)) warn = true;
      return false;
    }
  }
  return checkTypeStructureMatchesTV(type, *param, warn);
}

/*
 * Shared implementation for checkTypeStructureMatchesTV().
 *
 * If `isOrAsOp` is set, we are running this check for "is type" testing or "as
 * type" assertion operations.  For these operations, we reject comparisons
 * over {v,d,}array since we do want not users to be able distinguish
 * {v,d,}arrays while HAM is in progress (i.e., we only allow checking
 * {,v,d}arrays as ArrLike, not at any finer granularity).  Being able to tell
 * between them cripples the ability to transparently switch between them in
 * userland.
 */
template <bool gen_error>
bool checkTypeStructureMatchesTVImpl(
  const Array& ts,
  TypedValue c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey,
  bool& warn,
  bool isOrAsOp
) {
  auto const errOnLen = [&givenType](auto cell, auto len) {
    if (!gen_error) return;
    givenType = folly::sformat("{} of length {}",
      describe_actual_type(&cell), len);
  };

  auto const errOnKey = [&errorKey](TypedValue key) {
    if (!gen_error) return;
    auto const escapedKey = [key] {
      if (isStringType(type(key))) {
        return folly::sformat("\"{}\"",
          folly::cEscape<std::string>(val(key).pstr->toCppString()));
      }
      assertx(isIntType(type(key)));
      return std::to_string(val(key).num);
    }();
    errorKey = folly::sformat("[{}]{}", escapedKey, errorKey);
  };

  auto const type = c1.m_type;
  auto const data = c1.m_data;

  if (ts.exists(s_nullable) &&
      ts[s_nullable].asBooleanVal() &&
      isNullType(type)) {
    return true;
  }
  assertx(ts.exists(s_kind));

  auto const ts_kind = static_cast<TypeStructure::Kind>(
    ts[s_kind].toInt64Val()
  );

  auto const result = [&] {
    switch (ts_kind) {
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_null:
      return isNullType(type);
    case TypeStructure::Kind::T_nonnull:
      return !isNullType(type);
    case TypeStructure::Kind::T_int:
      return isIntType(type);
    case TypeStructure::Kind::T_bool:
      return isBoolType(type);
    case TypeStructure::Kind::T_float:
      return isDoubleType(type);
    case TypeStructure::Kind::T_num:
      return isIntType(type) || isDoubleType(type);
    case TypeStructure::Kind::T_arraykey:
      return isIntType(type) || isStringType(type);

    case TypeStructure::Kind::T_string:
      if (isClassType(type)) {
        if (RO::EvalClassIsStringNotices) {
          raise_notice("Class used in is_string");
        }
        return true;
      }
      return isStringType(type);

    case TypeStructure::Kind::T_resource:
      return isResourceType(type) &&
             !reinterpret_cast<const Resource*>(&data.pres)->isInvalid();

    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
      return !isOrAsOp && is_php_array(&c1);

    case TypeStructure::Kind::T_dict:
      return is_dict(&c1);

    case TypeStructure::Kind::T_vec:
      return is_vec(&c1);

    case TypeStructure::Kind::T_vec_or_dict:
      return is_vec(&c1) || is_dict(&c1);

    case TypeStructure::Kind::T_keyset:
      return is_keyset(&c1);

    case TypeStructure::Kind::T_arraylike:
      if (isClsMethType(type)) {
        if (RuntimeOption::EvalIsVecNotices) {
          raise_notice(Strings::CLSMETH_COMPAT_IS_ANY_ARR);
        }
        return true;
      }
      return isArrayLikeType(type);

    case TypeStructure::Kind::T_enum: {
      assertx(ts.exists(s_classname));
      auto const cls = Class::lookup(ts[s_classname].asStrRef().get());
      if (!isOrAsOp) {
        if (auto const dt = cls ? cls->enumBaseTy() : folly::none) {
          return equivDataTypes(*dt, type);
        }
        return isIntType(type) || isStringType(type);
      }
      return cls && enumHasValue(cls, &c1);
    }

    case TypeStructure::Kind::T_trait:
      // For is/as, we will not get here since we'll throw an error on the
      // resolution pass.
      // For parameter/return type verification, we treat it as a class type.
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_xhp: {
      assertx(ts.exists(s_classname));
      auto const ne = NamedEntity::get(ts[s_classname].asStrRef().get());
      return tvInstanceOf(&c1, ne) &&
             checkReifiedGenericsMatch(ts, c1, ne, warn, isOrAsOp);
    }

    case TypeStructure::Kind::T_tuple: {
      if (RO::EvalIsCompatibleClsMethType) {
        if (isClsMethType(type)) {
          if (RO::EvalIsVecNotices) {
            raise_notice(Strings::CLSMETH_COMPAT_IS_TUPLE);
          }
          auto const arr = clsMethToVecHelper(data.pclsmeth);
          return checkTypeStructureMatchesTVImpl<gen_error>(
            ts,
            make_array_like_tv(arr.get()),
            givenType,
            expectedType,
            errorKey,
            warn,
            isOrAsOp
          );
        }
      }

      if (!isArrayLikeType(type)) return false;

      /*
       * There are two behaviors we're trying to deal with these notices:
       *
       *   1. The reified generic path for tuple checks allows darrays to pass
       *      these checks. We want to restrict to only varrays. See isOrAsOp.
       *
       *   2. If any Hack array makes it to one of these check sites, we want
       *      to lift the HAM observation behavior into an is_php_array call.
       *
       * The two behaviors interact; if we can complete item 1 early, then we
       * can restrict logging for item 2 to vecs alone.
       */
      auto const ad = data.parr;
      auto const maybe_raise_notice = [&] (bool const result) {
        if (!gen_error /* avoid double logging */ && result &&
            RO::EvalHackArrIsShapeTupleNotices && !ad->isVArray()) {
          auto const dt = getDataTypeString(ad->toDataType());
          raise_hackarr_compat_notice(folly::sformat("{} is tuple", dt));
        }
        return result && (isArrayType(type) || RO::EvalHackArrDVArrs);
      };

      if (!isOrAsOp) return maybe_raise_notice(!isKeysetType(type));

      if (!ad->isVArray() && !ad->isVecType()) return false;

      assertx(ts.exists(s_elem_types));
      auto const ts_arr = ts[s_elem_types].getArrayData();
      if (ad->size() != ts_arr->size()) {
        errOnLen(c1, ad->size());
        return false;
      }

      auto const is_tuple_like = [&] {
        bool keys_match = true; // are keys contiguous and zero-indexed?
        bool vals_match = true; // do vals match the type structure types?
        int index = 0;

        IterateKV(ad,
          [&](TypedValue k, TypedValue v) {
            if (!isIntType(k.m_type) || k.m_data.num != index++) {
              keys_match = false;
              return true;
            }
            auto const tv = ts_arr->get(k.m_data.num);
            auto const& ts2 = asCArrRef(&tv);
            auto thisElemWarns = false;
            if (!checkTypeStructureMatchesTVImpl<gen_error>(
              ts2, v, givenType, expectedType, errorKey, thisElemWarns, isOrAsOp
            )) {
              errOnKey(k);
              if (thisElemWarns) {
                warn = true;
                return false;
              }
              vals_match = false;
              return true;
            }
            return false;
          }
        );
        // If there is an error, ignore `warn`.
        if (!vals_match || !keys_match) warn = false;

        if (!keys_match || warn) return false;
        return vals_match;
      }();

      return maybe_raise_notice(is_tuple_like);
    }

    case TypeStructure::Kind::T_shape: {
      if (!RO::EvalHackArrDVArrs && RO::EvalIsCompatibleClsMethType) {
        if (isClsMethType(type)) {
          if (RO::EvalIsVecNotices) {
            raise_notice(Strings::CLSMETH_COMPAT_IS_SHAPE);
          }
          auto const arr = clsMethToVecHelper(data.pclsmeth);
          return checkTypeStructureMatchesTVImpl<gen_error>(
            ts,
            make_array_like_tv(arr.get()),
            givenType,
            expectedType,
            errorKey,
            warn,
            isOrAsOp
          );
        }
      }

      if (!isArrayLikeType(type)) return false;

      /*
       * By analogy with the tuple case (see above), we're trying to restrict
       * reified generics to darrays, and also logging when a Hack array would
       * haved passed one of these checks.
       */
      auto const ad = data.parr;
      auto const maybe_raise_notice = [&] (bool const result) {
        if (!gen_error /* avoid double logging */ && result &&
            RO::EvalHackArrIsShapeTupleNotices && !ad->isDArray()) {
          auto const dt = getDataTypeString(ad->toDataType());
          raise_hackarr_compat_notice(folly::sformat("{} is shape", dt));
        }
        return result && (isArrayType(type) || RO::EvalHackArrDVArrs);
      };

      if (!isOrAsOp) return maybe_raise_notice(!isKeysetType(type));

      if (!ad->isDArray() && !ad->isDictType()) return false;

      assertx(ts.exists(s_fields));
      auto const ts_arr = ts[s_fields].getArrayData();

      auto const numRequiredFields = [&] {
        auto n = 0;
        IterateV(
          ts_arr,
          [&](TypedValue v) {
            assertx(isArrayLikeType(v.m_type));
            n += !isOptionalShapeField(v.m_data.parr);
          }
        );
        return n;
      }();

      if (ad->size() < numRequiredFields) {
        errOnLen(c1, ad->size());
        return false;
      }

      auto const allowsUnknownFields = shapeAllowsUnknownFields(ts);
      if (!allowsUnknownFields && ad->size() > ts_arr->size()) {
        errOnLen(c1, ad->size());
        return false;
      }

      auto fieldsDidMatch = true;
      auto numExpectedFields = 0;

      IterateKV(
        ts_arr,
        [&](TypedValue k, TypedValue v) {
          assertx(isArrayLikeType(v.m_type));
          auto const tsFieldData = v.m_data.parr;
          if (!ad->exists(k)) {
            if (isOptionalShapeField(tsFieldData)) {
              return false;
            }
            fieldsDidMatch = false;
            errOnKey(k);
            return true;
          }
          auto const tsField = getShapeFieldElement(v);
          auto const field = ad->at(k);
          bool thisFieldWarns = false;
          numExpectedFields++;
          if (!checkTypeStructureMatchesTVImpl<gen_error>(
            ArrNR(tsField), field, givenType,
            expectedType, errorKey, thisFieldWarns, isOrAsOp
          )) {
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
          !(allowsUnknownFields || ad->size() == numExpectedFields)) {
        warn = false;
        return false;
      }
      return maybe_raise_notice(!warn);
    }

    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
      return false;

    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
      return true;

    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
      // For is/as, we will not get here since we'll throw an error on the
      // resolution pass.
      // For parameter/return type verification, we don't check these types, so
      // just return true.
      return true;

    case TypeStructure::Kind::T_reifiedtype:
      // This type should have been removed in the resolution phase.
      always_assert(false);
    }
    not_reached();
  }();
  if (!warn && is_ts_soft(ts.get())) warn = true;
  if (gen_error && !result) {
    if (givenType.empty()) givenType = describe_actual_type(&c1);
    if (expectedType.empty()) {
      expectedType =
        TypeStructure::toString(ts,
          TypeStructure::TSDisplayType::TSDisplayTypeUser).toCppString();
    }
  }
  return result;
}

bool checkTypeStructureMatchesTV(const Array& ts, TypedValue c1) {
  std::string givenType, expectedType, errorKey;
  bool warn = false;
  return checkTypeStructureMatchesTVImpl<false>(
    ts, c1, givenType, expectedType, errorKey, warn, true);
}

bool checkTypeStructureMatchesTV(
  const Array& ts,
  TypedValue c1,
  std::string& givenType,
  std::string& expectedType,
  std::string& errorKey
) {
  bool warn = false;
  return checkTypeStructureMatchesTVImpl<true>(
    ts, c1, givenType, expectedType, errorKey, warn, true);
}

bool checkTypeStructureMatchesTV(
  const Array& ts,
  TypedValue c1,
  bool& warn
) {
  std::string givenType, expectedType, errorKey;
  return checkTypeStructureMatchesTVImpl<false>(
    ts, c1, givenType, expectedType, errorKey, warn, false);
}

ALWAYS_INLINE
bool errorOnIsAsExpressionInvalidTypesList(const ArrayData* tsFields,
                                           bool dryrun, bool allowWildcard) {
  bool willError = false;
  IterateV(
    tsFields,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      auto arr = v.m_data.parr;
      auto const value_field = arr->get(s_value.get());
      if (value_field.is_init()) {
        assertx(tvIsHAMSafeDArray(value_field));
        arr = value_field.val().parr;
      }
      if (!errorOnIsAsExpressionInvalidTypes(ArrNR(arr), dryrun,
                                             allowWildcard)) {
        return false;
      }
      willError = true;
      return true; // short-circuit
    }
  );
  return willError;
}

bool errorOnIsAsExpressionInvalidTypes(const Array& ts, bool dryrun,
                                       bool allowWildcard) {
  auto const err = [&](const char* errMsg) {
    if (dryrun) return true;
    raise_error("\"is\" and \"as\" operators cannot be used with %s", errMsg);
  };
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
    case TypeStructure::Kind::T_null:
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_xhp:
      return false;
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
      if (ts.exists(s_generic_types)) {
        auto genericsArr = ts[s_generic_types].getArrayData();
        return errorOnIsAsExpressionInvalidTypesList(genericsArr, dryrun,
                                                     true);
      }
      return false;
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
      return err("an array");
    case TypeStructure::Kind::T_fun:
      return err("a function");
    case TypeStructure::Kind::T_typevar:
      if (allowWildcard && isWildCard(ts.get())) return false;
      return err("a generic type");
    case TypeStructure::Kind::T_trait:
      return err("a trait");
    case TypeStructure::Kind::T_reifiedtype:
      return err("incomplete type structures");
    case TypeStructure::Kind::T_tuple: {
      assertx(ts.exists(s_elem_types));
      auto const elemsArr = ts[s_elem_types].getArrayData();
      return errorOnIsAsExpressionInvalidTypesList(elemsArr, dryrun,
                                                   allowWildcard);
    }
    case TypeStructure::Kind::T_shape: {
      assertx(ts.exists(s_fields));
      auto const tsFields = ts[s_fields].getArrayData();
      return errorOnIsAsExpressionInvalidTypesList(tsFields, dryrun,
                                                   allowWildcard);
    }
  }
  not_reached();
}

/**
 * Returns whether the type structure may not be able to be resolved statically,
 * i.e. if it contains `this` references.
 */
bool typeStructureCouldBeNonStatic(const ArrayData* ts) {
  switch (get_ts_kind(ts)) {
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
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_reifiedtype:
      return true;
    case TypeStructure::Kind::T_unresolved: {
      if (get_ts_classname(ts)->isame(s_hh_this.get())) return true;
      bool genericsCouldBeNonStatic = false;
      auto const generics = ts->get(s_generic_types.get());
      if (generics.is_init()) {
        assertx(isArrayLikeType(generics.type()));
        IterateV(
          generics.val().parr,
          [&](TypedValue v) {
            assertx(isArrayLikeType(v.m_type));
            genericsCouldBeNonStatic |=
              typeStructureCouldBeNonStatic(v.m_data.parr);
            // If at least one generic could be non-static, short circuit
            return genericsCouldBeNonStatic;
          }
        );
      }
      return genericsCouldBeNonStatic;
    }
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
    case TypeStructure::Kind::T_dynamic:
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
  assertx(ts.isHAMSafeDArray());
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
  assertx(resolved.isHAMSafeDArray());
  if (IsOrAsOp) errorOnIsAsExpressionInvalidTypes(resolved, false);
  return resolved;
}

template Array resolveAndVerifyTypeStructure<true>(
  const Array&, const Class*, const Class*, const req::vector<Array>&, bool);

template Array resolveAndVerifyTypeStructure<false>(
  const Array&, const Class*, const Class*, const req::vector<Array>&, bool);

void throwTypeStructureDoesNotMatchTVException(
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

bool doesTypeStructureContainTUnresolved(const ArrayData* ts) {
  bool result = false;
  IterateKV(
    ts,
    [&] (TypedValue k, TypedValue v) {
      if (tvIsInt(v) && tvIsString(k) && k.m_data.pstr->isame(s_kind.get()) &&
          static_cast<TypeStructure::Kind>(v.m_data.num) ==
          TypeStructure::Kind::T_unresolved) {
        result = true;
        return true; // short circuit
      }
      if (!isArrayLikeType(type(v))) return false;
      assertx(tvIsHAMSafeVArray(v) || tvIsHAMSafeDArray(v));
      result |= doesTypeStructureContainTUnresolved(v.m_data.parr);
      return result; // short circuit if true
    }
  );
  return result;
}

} // namespace HPHP
