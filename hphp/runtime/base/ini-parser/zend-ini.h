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

#ifndef incl_HPHP_RUNTIME_BASE_ZENDINI_H_
#define incl_HPHP_RUNTIME_BASE_ZENDINI_H_

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/ini-setting.h"

///////////////////////////////////////////////////////////////////////////////
// defined in zend-ini.ll

int ini_parse();
void ini_error(const char *msg);
int ini_lex(HPHP::String *ini_lval, void *loc);

void zend_ini_scan(const HPHP::String& str,
                   int scanner_mode,
                   const HPHP::String& filename,
                   HPHP::IniSetting::PFN_PARSER_CALLBACK callback,
                   void *arg);

void zend_ini_scan_cleanup();

void zend_ini_callback(HPHP::String *arg1,
                       HPHP::String *arg2,
                       HPHP::String *arg3,
                       int callback_type);


///////////////////////////////////////////////////////////////////////////////
// defined in zend-ini.y

bool zend_parse_ini_string(const HPHP::String& str,
                           const HPHP::String& filename,
                           int scanner_mode,
                           HPHP::IniSetting::PFN_PARSER_CALLBACK callback,
                           void *arg);



#endif // incl_HPHP_RUNTIME_BASE_ZENDINI_H_
