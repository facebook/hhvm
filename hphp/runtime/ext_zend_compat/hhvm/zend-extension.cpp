/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "folly/AtomicHashMap.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_modules.h"

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
