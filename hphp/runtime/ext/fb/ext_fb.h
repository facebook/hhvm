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

extern const int64_t k_FB_SERIALIZE_HACK_ARRAYS;
extern const int64_t k_FB_SERIALIZE_HACK_ARRAYS_AND_KEYSETS;
extern const int64_t k_FB_SERIALIZE_VARRAY_DARRAY;
extern const int64_t k_FB_SERIALIZE_POST_HACK_ARRAY_MIGRATION;

int64_t HHVM_FUNCTION(fb_utf8_strlen, const String& input);

///////////////////////////////////////////////////////////////////////////////

Variant fb_unserialize(const char* str,
                       int len,
                       bool& success,
                       int64_t options);
String fb_compact_serialize(const Variant& thing, int64_t options);
Variant fb_compact_unserialize(const char* str, int len,
                               bool& success,
                               Variant& errcode);

///////////////////////////////////////////////////////////////////////////////
}

