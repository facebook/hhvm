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

#ifndef incl_HPHP_DB_MYSQL_H_
#define incl_HPHP_DB_MYSQL_H_

#include "hphp/util/base.h"
#include "mysql.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
class MySQLUtil {
public:
  enum TimeoutType {
    ConnectTimeout,
    ReadTimeout,
    WriteTimeout
  };

  static int set_mysql_timeout(MYSQL *mysql, MySQLUtil::TimeoutType type, int ms);
};
///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DB_MYSQL_H_
