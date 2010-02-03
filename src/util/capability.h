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

#ifndef __CAPABILITY_H__
#define __CAPABILITY_H__

#include <sys/types.h>
#include <unistd.h>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Capability {
public:
  /**
   * This sets the  effective user ID of the current process, leaving
   * capability of binding to system ports (< 1024) to the user.
   */
  static bool ChangeUnixUser(uid_t uid);
  static bool ChangeUnixUser(const std::string &username);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CAPABILITY_H__
