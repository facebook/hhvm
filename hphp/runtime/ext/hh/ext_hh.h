/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
bool HHVM_FUNCTION(could_include, const String& file);
TypedValue HHVM_FUNCTION(serialize_memoize_param, TypedValue param);
void HHVM_FUNCTION(set_frame_metadata, const Variant& metadata);

TypedValue serialize_memoize_param_set(ArrayData*);
TypedValue serialize_memoize_param_arr(ArrayData*);
TypedValue serialize_memoize_param_obj(ObjectData*);
TypedValue serialize_memoize_param_col(ObjectData*);
TypedValue serialize_memoize_param_str(StringData*);
TypedValue serialize_memoize_param_lazycls(LazyClassData);
TypedValue serialize_memoize_param_dbl(double);

extern const StaticString
  s_nullMemoKey,
  s_trueMemoKey,
  s_falseMemoKey,
  s_emptyArrMemoKey,
  s_emptyStrMemoKey;

///////////////////////////////////////////////////////////////////////////////
}

