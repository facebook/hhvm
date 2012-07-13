/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "extmap_xml_array.h"
#include <compiler/analysis/type.h>

///////////////////////////////////////////////////////////////////////////////

static const char *xml_array_extension_functions[] = {
#define S(n) (const char *)n
#define T(t) (const char *)HPHP::Type::KindOf ## t
#define EXT_TYPE 0
#include "xml_array.inc"
  NULL,
};
#undef EXT_TYPE

static const char *xml_array_extension_constants[] = {
#define EXT_TYPE 1
#include "xml_array.inc"
  NULL,
};
#undef EXT_TYPE

static const char *xml_array_extension_classes[] = {
#define EXT_TYPE 2
#include "xml_array.inc"
  NULL,
};
#undef EXT_TYPE

static const char *xml_array_extension_declared_dynamic[] = {
#define EXT_TYPE 3
#include "xml_array.inc"
  NULL,
};
#undef EXT_TYPE

///////////////////////////////////////////////////////////////////////////////

const char **xml_array_map[] = {
  xml_array_extension_functions,
  xml_array_extension_constants,
  xml_array_extension_classes,
  xml_array_extension_declared_dynamic
};
