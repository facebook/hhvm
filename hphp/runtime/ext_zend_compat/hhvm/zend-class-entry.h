/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_ZEND_CLASS_ENTRY_H_
#define incl_ZEND_CLASS_ENTRY_H_

#include "hphp/runtime/vm/class.h"

typedef struct _zend_class_entry zend_class_entry;
struct _zend_class_entry;

namespace HPHP {

struct StringData;

zend_class_entry* zend_hphp_class_to_class_entry(Class* cls);
Class * zend_hphp_class_entry_to_class(const zend_class_entry * ce);
zend_class_entry* zend_hphp_get_internal_class_entry(const StringData* name);
zend_class_entry* zend_hphp_register_internal_class_entry(StringData* name);

const Class::SProp* zce_find_static_prop(const zend_class_entry* ce,
                                               const char* name,
                                               size_t len);

}

#endif
