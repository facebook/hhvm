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

#ifndef incl_HPHP_RUNTIME_BASE_ZENDINI_H_
#define incl_HPHP_RUNTIME_BASE_ZENDINI_H_

#include "hphp/runtime/base/ini-setting.h"

///////////////////////////////////////////////////////////////////////////////
// defined in zend-ini.ll

int ini_parse();
void ini_error(const char *msg);
int ini_lex(std::string *ini_lval, void *loc);

void zend_ini_scan(const std::string& str,
                   int scanner_mode,
                   const std::string& filename,
                   HPHP::IniSetting::ParserCallback &callback,
                   void *arg);

void zend_ini_scan_cleanup();

void zend_ini_on_section(const std::string &name);
void zend_ini_on_label(const std::string &name);
void zend_ini_on_entry(const std::string &key, const std::string &value);
void zend_ini_on_pop_entry(const std::string &key, const std::string &value,
                           const std::string &offset);
void zend_ini_on_constant(std::string &result, const std::string &name);
void zend_ini_on_var(std::string &result, const std::string &name);
void zend_ini_on_op(std::string &result, char type, const std::string& op1,
                    const std::string& op2 = std::string());

///////////////////////////////////////////////////////////////////////////////
// defined in zend-ini.y

bool zend_parse_ini_string(const std::string &str,
                           const std::string &filename,
                           int scanner_mode,
                           HPHP::IniSetting::ParserCallback &callback,
                           void *arg);

#endif // incl_HPHP_RUNTIME_BASE_ZENDINI_H_
