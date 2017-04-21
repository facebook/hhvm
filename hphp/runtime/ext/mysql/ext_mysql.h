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

#ifndef incl_HPHP_EXT_MYSQL_H_
#define incl_HPHP_EXT_MYSQL_H_

#include <folly/Optional.h>

#include "hphp/runtime/ext/extension.h"
#include "mysql.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/config.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

Variant HHVM_FUNCTION(mysql_num_fields, const Resource& result);
Variant HHVM_FUNCTION(mysql_fetch_lengths, const Resource& result);
Variant HHVM_FUNCTION(mysql_num_rows, const Resource& result);
String HHVM_FUNCTION(mysql_get_client_info);
Variant HHVM_FUNCTION(mysql_affected_rows, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_error, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_errno, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_get_host_info, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_info, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_insert_id, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_get_proto_info, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_get_server_info, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_thread_id, const Variant& link_identifier);
Variant HHVM_FUNCTION(mysql_warning_count, const Variant& link_identifier);

struct mysqlExtension final : Extension {
  mysqlExtension() : Extension("mysql", "1.0") {}

  // implementing IDebuggable
  virtual int debuggerSupport() override;
  virtual void debuggerInfo(InfoVec &info) override;

  static bool ReadOnly;
#ifdef FACEBOOK
  static bool Localize;
#endif
  static int ConnectTimeout;
  static int ReadTimeout;
  static int WaitTimeout;
  static int SlowQueryThreshold;
  static bool KillOnTimeout;
  static int MaxRetryOpenOnFail;
  static int MaxRetryQueryOnFail;
  static std::string Socket;
  static bool TypedResults;

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) override;
  void moduleInit() override;
};

extern mysqlExtension s_mysql_extension;

///////////////////////////////////////////////////////////////////////////////
extern const int64_t k_ASYNC_OP_INVALID;
extern const int64_t k_ASYNC_OP_UNSET;
extern const int64_t k_ASYNC_OP_CONNECT;
extern const int64_t k_ASYNC_OP_QUERY;
extern const int64_t k_ASYNC_OP_FETCH_ROW;

}

#endif // incl_HPHP_EXT_MYSQL_H_
