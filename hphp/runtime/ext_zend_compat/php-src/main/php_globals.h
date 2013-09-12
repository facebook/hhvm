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
   | Author: Zeev Suraski <zeev@zend.com>                                 |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_GLOBALS_H
#define PHP_GLOBALS_H

#include "zend_globals.h"

#include "zend_objects_API.h"
#include "hphp/runtime/base/externals.h"

const HPHP::StaticString
  s__COOKIE("_COOKIE"),
  s__ENV("_ENV"),
  s__FILES("_FILES"),
  s__GET("_GET"),
  s__POST("_POST"),
  s__REQUEST("_REQUEST"),
  s__SERVER("_SERVER");

#define PG(v) PG_##v()

/* Track vars */
#define TRACK_VARS_POST   0
#define TRACK_VARS_GET    1
#define TRACK_VARS_COOKIE 2
#define TRACK_VARS_SERVER 3
#define TRACK_VARS_ENV    4
#define TRACK_VARS_FILES  5
#define TRACK_VARS_REQUEST  6

typedef struct _arg_separators {
  char *output;
  char *input;
} arg_separators;

const zval** PG_http_globals();
_arg_separators PG_arg_separator();

#endif /* PHP_GLOBALS_H */
