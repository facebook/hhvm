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

#if YYDEBUG
extern int ini_debug;
#endif
int ini_parse();
void ini_error(const char *msg);
int ini_lex(std::string *ini_lval, void *loc);

void zend_ini_scan(const std::string& str,
                   int scanner_mode,
                   const std::string& filename,
                   HPHP::IniSetting::PFN_PARSER_CALLBACK callback,
                   void *arg);

void zend_ini_scan_cleanup();

void zend_ini_callback(std::string *arg1,
                       std::string *arg2,
                       std::string *arg3,
                       int callback_type);


///////////////////////////////////////////////////////////////////////////////
// defined in zend-ini.y

bool zend_parse_ini_string(const std::string& str,
                           const std::string& filename,
                           int scanner_mode,
                           HPHP::IniSetting::PFN_PARSER_CALLBACK callback,
                           void *arg);



#endif // incl_HPHP_RUNTIME_BASE_ZENDINI_H_
