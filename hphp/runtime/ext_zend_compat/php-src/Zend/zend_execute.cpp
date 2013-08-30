/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "zend_execute.h"

ZEND_API int zend_lookup_class(const char *name, int name_length, zend_class_entry ***ce TSRMLS_DC) {
  HPHP::StringData *class_name = HPHP::StringData::GetStaticString(name, name_length);
  **ce = HPHP::Unit::loadClass(class_name);
  return **ce == nullptr ? FAILURE : SUCCESS;
}

ZEND_API const char *get_active_class_name(const char **space TSRMLS_DC) {
  HPHP::Class *cls = HPHP::liveClass();
  if (!cls) {
    if (space) {
      *space = "";
    }
    return "";
  }
  if (space) {
    *space = "::";
  }
  return cls->name()->data();
}
