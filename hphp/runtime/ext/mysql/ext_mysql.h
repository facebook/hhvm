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
#include "hphp/runtime/base/config.h"
#include "mysql.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

struct mysqlExtension final : Extension {
  mysqlExtension() : Extension("mysql", "1.0", "mysql_gateway") {}

  static bool ReadOnly;
#ifdef HHVM_FACEBOOK
  static bool Localize;
#endif
  static int ConnectTimeout;
  static int ReadTimeout;
  static int WaitTimeout;
  static int SlowQueryThreshold;
  static int MaxRetryOpenOnFail;
  static int MaxRetryQueryOnFail;
  static std::string Socket;
  static bool TypedResults;

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) override;
  void moduleInit() override;
};

extern mysqlExtension s_mysql_extension;

}
