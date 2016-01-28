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

#ifndef incl_ZEND_PROPERTY_INFO_
#define incl_ZEND_PROPERTY_INFO_

#include "Zend/zend_compile.h"
#include "Zend/zend_types.h"

namespace HPHP {

template <class T>
void prop_to_zpi(zend_class_entry* ce, const T* prop, zend_property_info* info);

}
#endif
