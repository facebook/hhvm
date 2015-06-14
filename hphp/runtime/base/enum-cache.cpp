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

#include "hphp/runtime/base/enum-cache.h"
#include <memory>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// initialize the cache
static EnumCache s_cache;

const StaticString s_enumName("Enum");

const EnumCache::EnumValues* EnumCache::getValues(const Class* klass,
                                                  bool recurse) {
  if (UNLIKELY(klass->classVecLen() == 1 ||
               !s_enumName.get()->same(klass->classVec()[0]->name()))) {
    std::string msg;
    msg += klass->name()->data();
    msg += " must derive from Enum";
    EnumCache::failLookup(msg);
  }
  return s_cache.getEnumValues(klass, recurse);
}

const EnumCache::EnumValues* EnumCache::getValuesBuiltin(const Class* klass) {
  assert(isEnum(klass));
  return s_cache.getEnumValues(klass, false);
}

void EnumCache::deleteValues(const Class* klass) {
  // it's unlikely a class is in the cache so check first
  // without write lock
  if (s_cache.getEnumValuesIfDefined(getKey(klass, false)) != nullptr) {
    s_cache.deleteEnumValues(getKey(klass, false));
  }
  if (s_cache.getEnumValuesIfDefined(getKey(klass, true)) != nullptr) {
    s_cache.deleteEnumValues(getKey(klass, true));
  }
}

void EnumCache::failLookup(const Variant& msg) {
  SystemLib::throwExceptionObject(msg);
}

EnumCache::~EnumCache() {
  m_enumValuesMap.clear();
}

const EnumCache::EnumValues* EnumCache::loadEnumValues(const Class* klass,
                                                       bool recurse) {
  auto const numConstants = klass->numConstants();
  size_t foundOnClass = 0;
  Array values;
  Array names;
  auto const consts = klass->constants();
  for (size_t i = 0; i < numConstants; i++) {
    if (consts[i].isAbstract() || consts[i].isType()) {
      continue;
    }
    if (consts[i].m_class == klass) foundOnClass++;
    else if (!recurse) continue;
    Cell value = consts[i].m_val;
    // Handle dynamically set constants
    if (value.m_type == KindOfUninit) {
      value = klass->clsCnsGet(consts[i].m_name);
    }
    assert(value.m_type != KindOfUninit);
    if (UNLIKELY(!(IS_INT_TYPE(value.m_type) ||
        (tvIsString(&value) && value.m_data.pstr->isStatic())))) {
      // only int and string values allowed for enums. Moreover the strings
      // must be static
      std::string msg;
      msg += klass->name()->data();
      msg += " enum can only contain static string and int values";
      EnumCache::failLookup(msg);
    }
    values.set(StrNR(consts[i].m_name), cellAsCVarRef(value));
    names.set(cellAsCVarRef(value), VarNR(consts[i].m_name));
  }
  if (UNLIKELY(foundOnClass == 0)) {
    std::string msg;
    msg += klass->name()->data();
    msg += " enum must contain values";
    EnumCache::failLookup(msg);
  }

  {
    std::unique_ptr<EnumCache::EnumValues> enums(new EnumCache::EnumValues());
    enums->values = ArrayData::GetScalarArray(values.get());
    enums->names = ArrayData::GetScalarArray(names.get());

    intptr_t key = getKey(klass, recurse);
    EnumValuesMap::accessor acc;
    if (!m_enumValuesMap.insert(acc, key)) {
      return acc->second;
    }
    // add to the map the newly created values
    acc->second = enums.release();
    return acc->second;
  }
}

const EnumCache::EnumValues* EnumCache::getEnumValuesIfDefined(
  intptr_t key) const {
  EnumValuesMap::const_accessor acc;
  if (m_enumValuesMap.find(acc, key)) {
    return acc->second;
  }
  return nullptr;
}

const EnumCache::EnumValues* EnumCache::getEnumValues(const Class* klass,
                                                      bool recurse) {
  const EnumCache::EnumValues* values =
      getEnumValuesIfDefined(getKey(klass, recurse));
  if (values == nullptr) {
    values = loadEnumValues(klass, recurse);
  }
  return values;
}

void EnumCache::deleteEnumValues(intptr_t key) {
  EnumValuesMap::accessor acc;
  if (m_enumValuesMap.find(acc, key)) {
    delete acc->second;
    m_enumValuesMap.erase(acc);
  }
}

//////////////////////////////////////////////////////////////////////

}
