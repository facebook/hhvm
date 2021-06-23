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

#include "hphp/runtime/base/annot-type.h"

#include <folly/MapUtil.h>
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/util/hash-map.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

using HhvmStrToTypeMap = hphp_hash_map<
  const StringData*, AnnotType, string_data_hash, string_data_isame
>;

using StdStrToTypeMap = hphp_string_imap<AnnotType>;

const StaticString
  s_HH_Traversable("HH\\Traversable"),
  s_HH_RX_Traversable("HH\\Rx\\Traversable"),
  s_HH_KeyedTraversable("HH\\KeyedTraversable"),
  s_HH_RX_KeyedTraversable("HH\\Rx\\KeyedTraversable"),
  s_HH_Container("HH\\Container"),
  s_HH_KeyedContainer("HH\\KeyedContainer"),
  s_XHPChild("XHPChild"),
  s_Stringish("Stringish");

MaybeDataType nameToMaybeDataType(const StringData* typeName) {
  auto const* type = nameToAnnotType(typeName);
  return type ? MaybeDataType(getAnnotDataType(*type)) : std::nullopt;
}

MaybeDataType nameToMaybeDataType(const std::string& typeName) {
  auto const* type = nameToAnnotType(typeName);
  return type ? MaybeDataType(getAnnotDataType(*type)) : std::nullopt;
}

/**
 * This is the authoritative map that determines which typehints require
 * special handling. Any typehint not on this list is assumed to be normal
 * "class-name" typehint.
 */
static const std::pair<HhvmStrToTypeMap, StdStrToTypeMap>& getAnnotTypeMaps() {
  static const std::pair<HhvmStrToTypeMap, StdStrToTypeMap> mapPair = []() {
    std::pair<HhvmStrToTypeMap, StdStrToTypeMap> mappedPairs;
    struct Pair {
      const char* name;
      AnnotType type;
    };
    std::vector<Pair> pairs = {
      { "HH\\nothing",  AnnotType::Nothing },
      { "HH\\noreturn", AnnotType::NoReturn },
      { "HH\\null",     AnnotType::Null },
      { "HH\\void",     AnnotType::Null },
      { "HH\\bool",     AnnotType::Bool },
      { "HH\\int",      AnnotType::Int },
      { "HH\\float",    AnnotType::Float },
      { "HH\\string",   AnnotType::String },
      { "HH\\resource", AnnotType::Resource },
      { "HH\\mixed",    AnnotType::Mixed },
      { "HH\\nonnull",  AnnotType::Nonnull },
      { "HH\\num",      AnnotType::Number },
      { "HH\\arraykey", AnnotType::ArrayKey },
      { "HH\\this",     AnnotType::This },
      { "self",         AnnotType::Self },
      { "parent",       AnnotType::Parent },
      { "callable",     AnnotType::Callable },
      { "HH\\vec",      AnnotType::Vec },
      { "HH\\dict",     AnnotType::Dict },
      { "HH\\keyset",   AnnotType::Keyset },
      { "HH\\varray",   AnnotType::Vec },
      { "HH\\darray",   AnnotType::Dict },
      { "HH\\varray_or_darray", AnnotType::VecOrDict },
      { "HH\\vec_or_dict", AnnotType::VecOrDict },
      { "HH\\AnyArray", AnnotType::ArrayLike },
    };
    if (RO::EvalClassPassesClassname) {
      pairs.push_back({ "HH\\classname", AnnotType::Classname });
    }
    for (unsigned i = 0; i < pairs.size(); ++i) {
      mappedPairs.first[makeStaticString(pairs[i].name)] = pairs[i].type;
      mappedPairs.second[pairs[i].name] = pairs[i].type;
    }
    return mappedPairs;
  }();
  return mapPair;
}

const AnnotType* nameToAnnotType(const StringData* typeName) {
  assertx(typeName);
  auto const& mapPair = getAnnotTypeMaps();
  return folly::get_ptr(mapPair.first, typeName);
}

const AnnotType* nameToAnnotType(const std::string& typeName) {
  auto const& mapPair = getAnnotTypeMaps();
  auto const* at = folly::get_ptr(mapPair.second, typeName);
  assertx(!at || *at != AnnotType::Object);
  return at;
}

namespace {

bool isame(folly::StringPiece a, folly::StringPiece b) {
  return a.size() == b.size() && !strcasecmp(a.data(), b.data());
}

}

bool interface_supports_non_objects(const StringData* s) {
  return interface_supports_non_objects(s->slice());
}

bool interface_supports_non_objects(folly::StringPiece s) {
  return interface_supports_arrlike(s) ||
         isame(s, s_Stringish.slice());
}

bool interface_supports_arrlike(const StringData* s) {
  return interface_supports_arrlike(s->slice());
}

bool interface_supports_arrlike(folly::StringPiece s) {
  return isame(s, s_HH_Traversable.slice()) ||
         isame(s, s_HH_KeyedTraversable.slice()) ||
         isame(s, s_HH_RX_Traversable.slice()) ||
         isame(s, s_HH_RX_KeyedTraversable.slice()) ||
         isame(s, s_HH_Container.slice()) ||
         isame(s, s_HH_KeyedContainer.slice()) ||
         isame(s, s_XHPChild.slice());
}

bool interface_supports_string(const StringData* s) {
  return interface_supports_string(s->slice());
}

bool interface_supports_string(folly::StringPiece s) {
  return isame(s, s_XHPChild.slice()) ||
         isame(s, s_Stringish.slice());
}

bool interface_supports_int(const StringData* s) {
  return interface_supports_int(s->slice());
}

bool interface_supports_int(folly::StringPiece s) {
  return isame(s, s_XHPChild.slice());
}

bool interface_supports_double(const StringData* s) {
  return interface_supports_double(s->slice());
}

bool interface_supports_double(folly::StringPiece s) {
  return isame(s, s_XHPChild.slice());
}

///////////////////////////////////////////////////////////////////////////////

TypedValue annotDefaultValue(AnnotType at) {
  switch (at) {
    case AnnotType::Mixed:
    case AnnotType::Self:
    case AnnotType::Parent:
    case AnnotType::This:
    case AnnotType::Callable:
    case AnnotType::Resource:
    case AnnotType::Object:
    case AnnotType::Nothing:
    case AnnotType::Record:
    case AnnotType::NoReturn:
    case AnnotType::Classname:
    case AnnotType::Null:     return make_tv<KindOfNull>();
    case AnnotType::Nonnull:
    case AnnotType::Number:
    case AnnotType::ArrayKey:
    case AnnotType::Int:      return make_tv<KindOfInt64>(0);
    case AnnotType::Bool:     return make_tv<KindOfBoolean>(false);
    case AnnotType::Float:    return make_tv<KindOfDouble>(0);
    case AnnotType::ArrayLike:
    case AnnotType::VecOrDict:
    case AnnotType::Vec:
      return make_tv<KindOfPersistentVec>(staticEmptyVec());
    case AnnotType::String:
      return make_tv<KindOfPersistentString>(staticEmptyString());
    case AnnotType::Dict:
      return make_tv<KindOfPersistentDict>(staticEmptyDictArray());
    case AnnotType::Keyset:
      return make_tv<KindOfPersistentKeyset>(staticEmptyKeysetArray());
  }
  always_assert(false);
}

AnnotAction
annotCompat(DataType dt, AnnotType at, const StringData* annotClsName) {
  assertx(IMPLIES(at == AnnotType::Object, annotClsName != nullptr));

  auto const metatype = getAnnotMetaType(at);
  switch (metatype) {
    case AnnotMetaType::Mixed:
      return AnnotAction::Pass;
    case AnnotMetaType::Nonnull:
      return (dt == KindOfNull) ? AnnotAction::Fail : AnnotAction::Pass;
    case AnnotMetaType::Number:
      return (isIntType(dt) || isDoubleType(dt))
        ? AnnotAction::Pass : AnnotAction::Fail;
    case AnnotMetaType::ArrayKey:
      if (isClassType(dt)) {
        return RuntimeOption::EvalClassStringHintNotices
          ? AnnotAction::WarnClass : AnnotAction::ConvertClass;
      }
      if (isLazyClassType(dt)) {
        return RuntimeOption::EvalClassStringHintNotices
          ? AnnotAction::WarnLazyClass : AnnotAction::ConvertLazyClass;
      }
      return (isIntType(dt) || isStringType(dt))
        ? AnnotAction::Pass : AnnotAction::Fail;
    case AnnotMetaType::Self:
    case AnnotMetaType::Parent:
      // For "self" and "parent", if `dt' is not an object we know
      // it's not compatible, otherwise more checks are required
      return (dt == KindOfObject)
        ? AnnotAction::ObjectCheck : AnnotAction::Fail;
    case AnnotMetaType::This:
      return (dt == KindOfObject)
        ? AnnotAction::ObjectCheck
        : AnnotAction::Fail;
    case AnnotMetaType::Callable:
      // For "callable", if `dt' is not string/array/object/func we know
      // it's not compatible, otherwise more checks are required
      return (isStringType(dt) || isVecType(dt) || isDictType(dt) ||
              isFuncType(dt) || dt == KindOfObject || isClsMethType(dt) ||
              isRFuncType(dt) || isRClsMethType(dt))
        ? AnnotAction::CallableCheck : AnnotAction::Fail;
    case AnnotMetaType::VecOrDict:
      if (isClsMethType(dt) && RO::EvalIsCompatibleClsMethType) {
        return AnnotAction::ClsMethCheck;
      }
      return (isVecType(dt) || isDictType(dt))
        ? AnnotAction::Pass
        : AnnotAction::Fail;
    case AnnotMetaType::ArrayLike:
      if (isClsMethType(dt) && RO::EvalIsCompatibleClsMethType) {
        return AnnotAction::ClsMethCheck;
      }
      return isArrayLikeType(dt) ? AnnotAction::Pass : AnnotAction::Fail;
    case AnnotMetaType::Classname:
      if (isStringType(dt)) return AnnotAction::Pass;
      if (isClassType(dt) || isLazyClassType(dt)) {
        return RO::EvalClassnameNotices ?
          AnnotAction::WarnClassname : AnnotAction::Pass;
      }
      return AnnotAction::Fail;
    case AnnotMetaType::Nothing:
    case AnnotMetaType::NoReturn:
      return AnnotAction::Fail;
    case AnnotMetaType::Precise:
      break;
  }

  assertx(metatype == AnnotMetaType::Precise);
  if (at == AnnotType::String && dt == KindOfClass) {
    return RuntimeOption::EvalClassStringHintNotices
      ? AnnotAction::WarnClass : AnnotAction::ConvertClass;
  }
  if (at == AnnotType::String && dt == KindOfLazyClass) {
    return RuntimeOption::EvalClassStringHintNotices
      ? AnnotAction::WarnLazyClass : AnnotAction::ConvertLazyClass;
  }
  if (isClsMethType(dt) && RO::EvalIsCompatibleClsMethType) {
    auto const resolve = [] (bool okay) {
      return okay ? AnnotAction::ClsMethCheck : AnnotAction::Fail;
    };
    if (at == AnnotType::Vec)       return resolve(true);
    if (at == AnnotType::ArrayLike) return resolve(true);
  }

  if (at == AnnotType::Record) {
    return dt == KindOfRecord ? AnnotAction::RecordCheck : AnnotAction::Fail;
  }

  if (at != AnnotType::Object) {
    // If `at' is "bool", "int", "float", "string", "array", or "resource",
    // then equivDataTypes() can definitively tell us whether or not `dt'
    // is compatible.
    return equivDataTypes(getAnnotDataType(at), dt)
      ? AnnotAction::Pass : AnnotAction::Fail;
  }

  // If `dt' is not an object, check for "magic" interfaces that
  // support non-object datatypes
  if (dt != KindOfObject && interface_supports_non_objects(annotClsName)) {
    switch (dt) {
      case KindOfInt64:
        return interface_supports_int(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfDouble:
        return interface_supports_double(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentString:
      case KindOfString:
        return interface_supports_string(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfPersistentVec:
      case KindOfVec:
      case KindOfPersistentDict:
      case KindOfDict:
      case KindOfPersistentKeyset:
      case KindOfKeyset:
        return interface_supports_arrlike(annotClsName)
          ? AnnotAction::Pass : AnnotAction::Fail;
      case KindOfClass:
        if (interface_supports_string(annotClsName)) {
          return RuntimeOption::EvalClassStringHintNotices
            ? AnnotAction::WarnClass : AnnotAction::ConvertClass;
        }
        return AnnotAction::Fail;
      case KindOfLazyClass:
        if (interface_supports_string(annotClsName)) {
          return RuntimeOption::EvalClassStringHintNotices
            ? AnnotAction::WarnLazyClass : AnnotAction::ConvertLazyClass;
        }
        return AnnotAction::Fail;
      case KindOfClsMeth:
        return RO::EvalIsCompatibleClsMethType &&
               interface_supports_arrlike(annotClsName) ?
          AnnotAction::ClsMethCheck : AnnotAction::Fail;
      case KindOfFunc:
      case KindOfRClsMeth:
      case KindOfRFunc:
      case KindOfUninit:
      case KindOfNull:
      case KindOfBoolean:
      case KindOfResource:
      case KindOfRecord:
        return AnnotAction::Fail;
      case KindOfObject:
        not_reached();
        break;
    }
  }

  return AnnotAction::ObjectCheck;
}

///////////////////////////////////////////////////////////////////////////////

}
