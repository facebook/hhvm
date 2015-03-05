/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Optional.h>
#include <folly/MapUtil.h>
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

typedef hphp_hash_map<const StringData*, AnnotType, string_data_hash,
  string_data_isame> HhvmStrToTypeMap;

typedef hphp_string_imap<AnnotType> StdStrToTypeMap;

const StaticString
  s_HH_Traversable("HH\\Traversable"),
  s_HH_KeyedTraversable("HH\\KeyedTraversable"),
  s_HH_Container("HH\\Container"),
  s_HH_KeyedContainer("HH\\KeyedContainer"),
  s_Indexish("Indexish"),
  s_XHPChild("XHPChild"),
  s_Stringish("Stringish");

MaybeDataType nameToMaybeDataType(const StringData* typeName) {
  auto const* type = nameToAnnotType(typeName);
  return type ? MaybeDataType(getAnnotDataType(*type)) : folly::none;
}

MaybeDataType nameToMaybeDataType(const std::string& typeName) {
  auto const* type = nameToAnnotType(typeName);
  return type ? MaybeDataType(getAnnotDataType(*type)) : folly::none;
}

/**
 * This is the authoritative map that determines which typehints require
 * special handling. Any typehint not on this list is assumed to be normal
 * "class-name" typehint.
 */
static const std::pair<HhvmStrToTypeMap, StdStrToTypeMap>& getAnnotTypeMaps() {
  static const std::pair<HhvmStrToTypeMap, StdStrToTypeMap> mapPair = []() {
    std::pair<HhvmStrToTypeMap, StdStrToTypeMap> mapPair;
    const struct Pair {
      const char* name;
      AnnotType type;
    } pairs[] = {
      { "HH\\noreturn", AnnotType::Uninit },
      { "HH\\void",     AnnotType::Null },
      { "HH\\bool",     AnnotType::Bool },
      { "HH\\int",      AnnotType::Int },
      { "HH\\float",    AnnotType::Float },
      { "HH\\string",   AnnotType::String },
      { "array",        AnnotType::Array },
      { "HH\\resource", AnnotType::Resource },
      { "HH\\mixed",    AnnotType::Mixed },
      { "HH\\num",      AnnotType::Number },
      { "HH\\arraykey", AnnotType::ArrayKey },
      { "self",         AnnotType::Self },
      { "parent",       AnnotType::Parent },
      { "callable",     AnnotType::Callable },
    };
    for (unsigned i = 0; i < sizeof(pairs) / sizeof(Pair); ++i) {
      mapPair.first[makeStaticString(pairs[i].name)] = pairs[i].type;
      mapPair.second[pairs[i].name] = pairs[i].type;
    }
    return mapPair;
  }();
  return mapPair;
}

const AnnotType* nameToAnnotType(const StringData* typeName) {
  assert(typeName);
  auto const& mapPair = getAnnotTypeMaps();
  return folly::get_ptr(mapPair.first, typeName);
}

const AnnotType* nameToAnnotType(const std::string& typeName) {
  auto const& mapPair = getAnnotTypeMaps();
  auto const* at = folly::get_ptr(mapPair.second, typeName);
  assert(!at || *at != AnnotType::Object);
  return at;
}

bool interface_supports_non_objects(const StringData* s) {
  return (s->isame(s_HH_Traversable.get()) ||
          s->isame(s_HH_KeyedTraversable.get()) ||
          s->isame(s_HH_Container.get()) ||
          s->isame(s_HH_KeyedContainer.get()) ||
          s->isame(s_Indexish.get()) ||
          s->isame(s_XHPChild.get()) ||
          s->isame(s_Stringish.get()));
}

bool interface_supports_array(const StringData* s) {
  return (s->isame(s_HH_Traversable.get()) ||
          s->isame(s_HH_KeyedTraversable.get()) ||
          s->isame(s_HH_Container.get()) ||
          s->isame(s_HH_KeyedContainer.get()) ||
          s->isame(s_Indexish.get()) ||
          s->isame(s_XHPChild.get()));
}

bool interface_supports_array(const std::string& n) {
  const char* s = n.c_str();
  return ((n.size() == 14 && !strcasecmp(s, "HH\\Traversable")) ||
          (n.size() == 19 && !strcasecmp(s, "HH\\KeyedTraversable")) ||
          (n.size() == 12 && !strcasecmp(s, "HH\\Container")) ||
          (n.size() == 17 && !strcasecmp(s, "HH\\KeyedContainer")) ||
          (n.size() == 8 && !strcasecmp(s, "Indexish")) ||
          (n.size() == 8 && !strcasecmp(s, "XHPChild")));
}

bool interface_supports_string(const StringData* s) {
  return s->isame(s_XHPChild.get())
    || s->isame(s_Stringish.get());
}

bool interface_supports_string(const std::string& n) {
  const char *s = n.c_str();
  return (n.size() == 8 && !strcasecmp(s, "XHPChild"))
    || (n.size() == 9 && !strcasecmp(s, "Stringish"));
}

bool interface_supports_int(const StringData* s) {
  return (s->isame(s_XHPChild.get()));
}

bool interface_supports_int(const std::string& n) {
  const char *s = n.c_str();
  return (n.size() == 8 && !strcasecmp(s, "XHPChild"));
}

bool interface_supports_double(const StringData* s) {
  return (s->isame(s_XHPChild.get()));
}

bool interface_supports_double(const std::string& n) {
  const char *s = n.c_str();
  return (n.size() == 8 && !strcasecmp(s, "XHPChild"));
}

///////////////////////////////////////////////////////////////////////////////

}
