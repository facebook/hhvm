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

#ifndef incl_HPHP_ZEND_CLASS_TO_CLASS_ENTRY
#define incl_HPHP_ZEND_CLASS_TO_CLASS_ENTRY

typedef struct _zend_class_entry zend_class_entry;
struct _zend_class_entry;

zend_class_entry *zend_hphp_class_to_class_entry(HPHP::Class* cls);

#endif // incl_HPHP_ZEND_CLASS_TO_CLASS_ENTRY
