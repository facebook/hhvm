/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/thrift/spec-holder.h"

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

namespace HPHP { namespace thrift {

Array get_tspec(const Class* cls) {
  auto lookup = cls->clsCnsGet(s_SPEC.get());
  if (lookup.m_type == KindOfUninit) {
    thrift_error(
      folly::sformat("Class {} does not have a property named {}",
                     cls->name(), s_SPEC),
      ERR_INVALID_DATA);
  }
  Variant structSpec = tvAsVariant(&lookup);
  if (!structSpec.isArray()) {
    thrift_error("invalid type of spec", ERR_INVALID_DATA);
  }
  return structSpec.toArray();
}

bool SpecHolder::typeSatisfiesConstraint(const TypeConstraint& tc,
                                         TType type,
                                         const Array& spec,
                                         bool isBinary) {
  switch (type) {
    case T_STOP:
    case T_VOID:
      return tc.alwaysPasses(KindOfNull);
    case T_STRUCT: {
      auto const className = spec.rval(s_class);
      if (isNullType(className.type())) return false;
      auto const classNameString = tvCastToString(className.tv());
      // Binary deserializing can assign a null to an object, while compact
      // won't (this might be a bug).
      return tc.alwaysPasses(classNameString.get()) &&
        (!isBinary || tc.alwaysPasses(KindOfNull));
    }
    case T_BOOL:
      return tc.alwaysPasses(KindOfBoolean);
    case T_BYTE:
    case T_I16:
    case T_I32:
    case T_I64:
    case T_U64:
      return tc.alwaysPasses(KindOfInt64);
    case T_DOUBLE:
    case T_FLOAT:
      return tc.alwaysPasses(KindOfDouble);
    case T_UTF8:
    case T_UTF16:
    case T_STRING:
      return tc.alwaysPasses(KindOfString);
    case T_MAP: {
      auto const format = tvCastToString(
        spec.rval(s_format, AccessFlags::None).tv()
      );
      if (format.equal(s_harray)) {
        return tc.alwaysPasses(KindOfDict);
      } else if (format.equal(s_collection)) {
        return tc.alwaysPasses(c_Map::classof()->name());
      } else {
        return tc.alwaysPasses(KindOfArray);
      }
      break;
    }
    case T_LIST: {
      auto const format = tvCastToString(
        spec.rval(s_format, AccessFlags::None).tv()
      );
      if (format.equal(s_harray)) {
        return tc.alwaysPasses(KindOfVec);
      } else if (format.equal(s_collection)) {
        return tc.alwaysPasses(c_Vector::classof()->name());
      } else {
        return tc.alwaysPasses(KindOfArray);
      }
      break;
    }
    case T_SET: {
      auto const format = tvCastToString(
        spec.rval(s_format, AccessFlags::None).tv()
      );
      if (format.equal(s_harray)) {
        return tc.alwaysPasses(KindOfKeyset);
      } else if (format.equal(s_collection)) {
        return tc.alwaysPasses(c_Set::classof()->name());
      } else {
        return tc.alwaysPasses(KindOfArray);
      }
      break;
    }
  }
  return false;
}

SpecCacheMap SpecHolder::s_specCacheMap(1000);

}}
