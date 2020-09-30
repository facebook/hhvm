/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/afdt-util.h"
#include "hphp/util/exception.h"
#include "hphp/util/trace.h"

#include <afdt.h>
#include <functional>

namespace HPHP {

template<class... Args>
void cli_write(int afdt_fd, Args&&... args) {
  TRACE_SET_MOD(clisrv);
  FTRACE(4, "cli_write({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::sendx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::sendRaw: %s", ex.what());
  }
}

template<class... Args>
void cli_read(int afdt_fd, Args&&... args) {
  TRACE_SET_MOD(clisrv);
  FTRACE(4, "cli_read({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::recvx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::recvRaw: %s", ex.what());
  }
}

int cli_read_fd(int afdt_fd);
void cli_write_fd(int afdt_fd, int fd);

}
