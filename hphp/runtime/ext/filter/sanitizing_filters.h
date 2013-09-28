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
#ifndef HPHP_EXT_FILTER_SANITIZING_FILTERS_H
#define HPHP_EXT_FILTER_SANITIZING_FILTERS_H

#include "hphp/runtime/ext/filter/filter_private.h"

namespace HPHP {

Variant php_filter_string(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_encoded(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_special_chars(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_full_special_chars(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_unsafe_raw(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_email(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_url(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_number_int(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_number_float(PHP_INPUT_FILTER_PARAM_DECL);
Variant php_filter_magic_quotes(PHP_INPUT_FILTER_PARAM_DECL);

}

#endif /* HPHP_EXT_FILTER_SANITIZING_FILTERS_H */
