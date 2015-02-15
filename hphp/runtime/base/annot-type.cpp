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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

typedef hphp_hash_map<const StringData*, AnnotType, string_data_hash,
  string_data_isame> AnnotTypeMap;

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

static const AnnotTypeMap& getAnnotTypeMap() {
  static const AnnotTypeMap atMap = []() {
    AnnotTypeMap atMap;
    const struct Pair {
      const StringData* name;
      AnnotType type;
    } pairs[] = {
      { makeStaticString("HH\\bool"),     AnnotType::Bool },
      { makeStaticString("HH\\int"),      AnnotType::Int },
      { makeStaticString("HH\\float"),    AnnotType::Float },
      { makeStaticString("HH\\string"),   AnnotType::String },
      { makeStaticString("array"),        AnnotType::Array },
      { makeStaticString("HH\\resource"), AnnotType::Resource },
      { makeStaticString("HH\\num"),      AnnotType::Number },
      { makeStaticString("HH\\arraykey"), AnnotType::ArrayKey },
      { makeStaticString("self"),         AnnotType::Self },
      { makeStaticString("parent"),       AnnotType::Parent },
      { makeStaticString("callable"),     AnnotType::Callable },
    };
    for (unsigned i = 0; i < sizeof(pairs) / sizeof(Pair); ++i) {
      atMap[pairs[i].name] = pairs[i].type;
    }
    return atMap;
  }();
  return atMap;
}

const AnnotType* nameToAnnotType(const StringData* typeName) {
  assert(typeName);
  const AnnotTypeMap& atMap = getAnnotTypeMap();
  return folly::get_ptr(atMap, typeName);
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
