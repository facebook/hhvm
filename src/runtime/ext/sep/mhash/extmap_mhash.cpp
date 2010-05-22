/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "extmap_mhash.h"
#include <compiler/analysis/type.h>

///////////////////////////////////////////////////////////////////////////////

static const char *mhash_extension_functions[] = {
#define S(n) (const char *)n
#define T(t) (const char *)HPHP::Type::KindOf ## t
#define EXT_TYPE 0
#include "mhash.inc"
  NULL,
};
#undef EXT_TYPE

static const char *mhash_extension_constants[] = {
#define EXT_TYPE 1
#include "mhash.inc"
  NULL,
};
#undef EXT_TYPE

static const char *mhash_extension_classes[] = {
#define EXT_TYPE 2
#include "mhash.inc"
  NULL,
};
#undef EXT_TYPE

static const char *mhash_extension_declared_dynamic[] = {
#define EXT_TYPE 3
#include "mhash.inc"
  NULL,
};
#undef EXT_TYPE

///////////////////////////////////////////////////////////////////////////////

const char **mhash_map[] = {
  mhash_extension_functions,
  mhash_extension_constants,
  mhash_extension_classes,
  mhash_extension_declared_dynamic
};
