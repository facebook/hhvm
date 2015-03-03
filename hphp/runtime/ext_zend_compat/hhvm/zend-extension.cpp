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
#include <folly/AtomicHashMap.h>
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_modules.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_API.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/util/text-util.h"

#include <atomic>

namespace HPHP {

// Initial size of the map
#define APPROXIMATE_STATIC_ZEND_MODULES 10
static std::atomic_int s_zend_next_module(1);
static folly::AtomicHashMap<int, ZendExtension*> *s_zend_extensions;

ZendExtension::ZendExtension(const char* name) : Extension(name) {
  if (!s_zend_extensions) {
    // No need to worry about races, this all happens during pre-main
    // initialization
    s_zend_extensions = new folly::AtomicHashMap<int, ZendExtension*>(
      APPROXIMATE_STATIC_ZEND_MODULES
    );
  }
}

ZendExtension* ZendExtension::GetByModuleNumber(int module_number) {
  if (!s_zend_extensions) return nullptr;
  auto iter = s_zend_extensions->find(module_number);
  if (iter != s_zend_extensions->end()) {
    return iter->second;
  }
  return nullptr;
}

bool ZendExtension::moduleEnabled() const {
  return RuntimeOption::EnableZendCompat;
}

void ZendExtension::moduleInit() {
  if (!RuntimeOption::EnableZendCompat) {
    return;
  }
  // Give it a module number
  zend_module_entry* module = getEntry();
  module->module_number = s_zend_next_module++;
  s_zend_extensions->insert(module->module_number, this);

  ZendObject::registerNativeData();
  // Allocate globals
  if (module->globals_size) {
    ts_allocate_id(module->globals_id_ptr, module->globals_size,
        (ts_allocate_ctor) module->globals_ctor,
        (ts_allocate_dtor) module->globals_dtor);
  }
  // Register global functions
  if (module->functions) {
    const zend_function_entry * fe = module->functions;
    while (fe->fname) {
      assert(fe->handler);
      Native::registerBuiltinZendFunction(fe->fname, fe->handler);
      fe++;
    }
  }
  // Call MINIT
  if (module->module_startup_func) {
    TSRMLS_FETCH();
    module->module_startup_func(1, module->module_number TSRMLS_CC);
  }
  // The systemlib name must match the name used by the build process. For
  // in-tree builds this is the directory name, which is typically the same
  // as the extension name converted to lower case.
  std::string slName = toLower(std::string(getName()));
  loadSystemlib(slName);
}

}
