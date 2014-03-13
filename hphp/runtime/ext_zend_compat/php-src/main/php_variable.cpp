/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include <stdio.h>
#include "php.h"
#include "php_variables.h"
#include "php_globals.h"
#include "SAPI.h"
#include "zend_globals.h"

#include "hphp/runtime/server/http-protocol.h"

SAPI_API SAPI_TREAT_DATA_FUNC(php_default_treat_data) {
  assert(arg == PARSE_STRING);
  auto& var = HPHP::tvAsVariant(destArray->tv());
  auto& varArr = HPHP::forceToArray(var);
  HPHP::HttpProtocol::DecodeParameters(varArr, str, strlen(str));
}
