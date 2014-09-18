/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/Optional.h"

#include "hphp/runtime/base/base-includes.h"
#include "mysql.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/persistent-resource-store.h"
#include "hphp/runtime/base/config.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

class mysqlExtension : public Extension {
public:
  mysqlExtension() : Extension("mysql", "1.0") {}

  // implementing IDebuggable
  virtual int  debuggerSupport() {
    return SupportInfo;
  }
  virtual void debuggerInfo(InfoVec &info) {
    int count = g_persistentResources->getMap("mysql::persistent_conns").size();
    Add(info, "Persistent", FormatNumber("%" PRId64, count));

    AddServerStats(info, "sql.conn"       );
    AddServerStats(info, "sql.reconn_new" );
    AddServerStats(info, "sql.reconn_ok"  );
    AddServerStats(info, "sql.reconn_old" );
    AddServerStats(info, "sql.query"      );
  }

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

  virtual void moduleLoad(const IniSetting::Map& ini, Hdf config) {
    Hdf mysql = config["MySQL"];
    Config::Bind(ReadOnly, ini, mysql["ReadOnly"], false);
#ifdef FACEBOOK
    Config::Bind(Localize, ini, mysql["Localize"], false);
#endif
    Config::Bind(ConnectTimeout, ini, mysql["ConnectTimeout"], 1000);
    Config::Bind(ReadTimeout, ini, mysql["ReadTimeout"], 60000);
    Config::Bind(WaitTimeout, ini, mysql["WaitTimeout"], -1);
    Config::Bind(SlowQueryThreshold, ini, mysql["SlowQueryThreshold"], 1000);
    Config::Bind(KillOnTimeout, ini, mysql["KillOnTimeout"], false);
    Config::Bind(MaxRetryOpenOnFail, ini, mysql["MaxRetryOpenOnFail"], 1);
    Config::Bind(MaxRetryQueryOnFail, ini, mysql["MaxRetryQueryOnFail"], 1);
    Config::Bind(Socket, ini, mysql["Socket"], "");
    Config::Bind(TypedResults, ini, mysql["TypedResults"], true);
  }

  void moduleInit();
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
