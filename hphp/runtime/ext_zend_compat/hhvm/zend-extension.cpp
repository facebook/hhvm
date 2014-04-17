/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext_zend_compat/hhvm/zend-extension.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-object.h"
#include "folly/AtomicHashMap.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_modules.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_API.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/text-util.h"

#include <atomic>

// Initial size of the map
#define APPROXIMATE_STATIC_ZEND_MODULES 10
static std::atomic_int s_zend_next_module(1);
static folly::AtomicHashMap<int, ZendExtension*> *s_zend_extensions;

ZendExtension::ZendExtension(const char* name) : HPHP::Extension(name) {
  if (!s_zend_extensions) {
    // No need to worry about races, this all happens during pre-main
    // initialization
    s_zend_extensions = new folly::AtomicHashMap<int, ZendExtension*>(
      APPROXIMATE_STATIC_ZEND_MODULES
    );
  }
  zend_module_entry* module = this->getEntry();
  module->module_number = s_zend_next_module++;
  s_zend_extensions->insert(module->module_number, this);
}

ZendExtension* ZendExtension::GetByModuleNumber(int module_number) {
  if (!s_zend_extensions) return nullptr;
  auto iter = s_zend_extensions->find(module_number);
  if (iter != s_zend_extensions->end()) {
    return iter->second;
  }
  return nullptr;
}

void ZendExtension::moduleInit() {
  if (!HPHP::RuntimeOption::EnableZendCompat) {
    return;
  }
  HPHP::ZendObject::registerNativeData();
  // Allocate globals
  zend_module_entry* entry = getEntry();
  if (entry->globals_size) {
    ts_allocate_id(entry->globals_id_ptr, entry->globals_size,
        (ts_allocate_ctor) entry->globals_ctor,
        (ts_allocate_dtor) entry->globals_dtor);
  }
  // Register global functions
  const zend_function_entry * fe = entry->functions;
  while (fe->fname) {
    assert(fe->handler);
    HPHP::Native::registerBuiltinFunction(HPHP::makeStaticString(fe->fname),
                                          fe->handler);
    fe++;
  }
  // Call MINIT
  if (entry->module_startup_func) {
    TSRMLS_FETCH();
    entry->module_startup_func(1, entry->module_number TSRMLS_CC);
  }
  // The systemlib name must match the name used by the build process. For
  // in-tree builds this is the directory name, which is typically the same
  // as the extension name converted to lower case.
  std::string slName = HPHP::toLower(std::string(getName()));
  loadSystemlib(slName);
}
