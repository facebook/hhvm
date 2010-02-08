/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "db_mysql.h"
#include <mysql/mysql.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

int MySQLUtil::set_mysql_timeout(MYSQL *mysql, MySQLUtil::TimeoutType type, int ms) {
   mysql_option opt = MYSQL_OPT_CONNECT_TIMEOUT;
#ifdef MYSQL_MILLISECOND_TIMEOUT
  switch (type) {
   case MySQLUtil::ConnectTimeout: opt = MYSQL_OPT_CONNECT_TIMEOUT_MS; break;
   case MySQLUtil::ReadTimeout: opt =  MYSQL_OPT_READ_TIMEOUT_MS; break;
   case MySQLUtil::WriteTimeout: opt =  MYSQL_OPT_WRITE_TIMEOUT_MS; break;
   default: ASSERT(false); break;
  }
#else
  switch (type) {
    case MySQLUtil::ConnectTimeout: opt = MYSQL_OPT_CONNECT_TIMEOUT; break;
    case MySQLUtil::ReadTimeout: opt =  MYSQL_OPT_READ_TIMEOUT; break;
    case MySQLUtil::WriteTimeout: opt =  MYSQL_OPT_WRITE_TIMEOUT; break;
    default: ASSERT(false); break;
  }
  ms = (ms + 999) / 1000;
#endif

  return mysql_options(mysql, opt, (const char*)&ms);
}

///////////////////////////////////////////////////////////////////////////////
}
