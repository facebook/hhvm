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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-class-entry.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/ext_zend_compat/hhvm/ZendRequestLocal.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"

#include <unordered_map>

static std::unordered_map<const HPHP::StringData*, zend_class_entry> s_internal_class_entries;
ZEND_REQUEST_LOCAL_MAP(HPHP::Class*, zend_class_entry, s_class_cache);

zend_class_entry* zend_hphp_class_to_class_entry(HPHP::Class* cls) {
  // Check to see if it is in the request-local cache
  auto & cache = s_class_cache.get()->get();
  zend_class_entry * ce;
  auto cacheIter = cache.find(cls);
  if (cacheIter != cache.end()) {
    return &cacheIter->second;
  }

  // Was it internal to zend compat? If so, we have a thread-independent CE
  auto internalIter = s_internal_class_entries.find(cls->name());
  if (internalIter != s_internal_class_entries.end()) {
    ce = &internalIter->second;
    ce->hphp_class = cls;
    return ce;
  }

  // Construct default zend_class_entry using operator[]
  ce = &cache[cls];
  ce->hphp_class = cls;
  ce->name = cls->name()->data();
  ce->name_length = cls->name()->size();
  return ce;
}

HPHP::Class * zend_hphp_class_entry_to_class(const zend_class_entry * ce)
{
  if (ce->hphp_class) {
    return ce->hphp_class;
  }
  const HPHP::StringData * sd = HPHP::makeStaticString(ce->name, ce->name_length);
  HPHP::Class * cls = HPHP::Unit::lookupClass(sd);
  if (!cls) {
    return nullptr;
  }
  ce->hphp_class = cls;
  return cls;
}

zend_class_entry* zend_hphp_get_internal_class_entry(const HPHP::StringData* name)
{
  auto it = s_internal_class_entries.find(name);
  if (it == s_internal_class_entries.end()) {
    return nullptr;
  } else {
    return &it->second;
  }
}

zend_class_entry* zend_hphp_register_internal_class_entry(HPHP::StringData* name)
{
  zend_class_entry * ce = &s_internal_class_entries[name];
  ce->hphp_class = nullptr;
  ce->name = name->data();
  ce->name_length = name->size();
  return ce;
}

const HPHP::Class::SProp* zce_find_static_prop(const zend_class_entry* ce,
                                               const char* name,
                                               size_t len) {
  auto const& sprops = ce->hphp_class->staticProperties();
  for (size_t i = 0; i < ce->hphp_class->numStaticProperties(); ++i) {
    auto const& sprop = sprops[i];
    if (sprop.m_name->isame(HPHP::StringData::Make(name, len, HPHP::CopyString))) {
      return &sprop;
    }
  }
  return nullptr;
}
