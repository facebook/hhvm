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

#include "hphp/runtime/ext_zend_compat/hhvm/ZendRequestLocal.h"
#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"

ZEND_REQUEST_LOCAL_MAP(HPHP::Class*, zend_class_entry, s_class_map);
zend_class_entry *zend_hphp_class_to_class_entry(HPHP::Class* cls) {
  zend_class_entry& cache = s_class_map.get()->get()[cls];
  if (!cache.hphp_class) {
    cache.hphp_class = cls;
    cache.name = cls->name()->data();
    cache.name_length = cls->name()->size();
  }
  return &cache;
}
