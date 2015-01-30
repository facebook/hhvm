/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

/*
 * DO NOT INCLUDE THIS FILE.
 *
 * WHAT YOU PROBABLY WANT IS TO INCLUDE type-string.h, type-array.h, or
 * type-variant.h
 *
 * IF YOU REALLY NEED array-data-defs.h, THEN PLEASE INCLUDE IT MANUALLY.
 */

#ifndef incl_HPHP_COMPLEX_TYPES_H_
#define incl_HPHP_COMPLEX_TYPES_H_

#include "hphp/runtime/base/typed-value.h"

#define incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-helpers.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/array-data-defs.h"

#undef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_

#endif // incl_HPHP_COMPLEX_TYPES_H_
