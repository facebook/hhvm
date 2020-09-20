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

#include "hphp/runtime/server/cli-server-ext.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"

TRACE_SET_MOD(clisrv);

#include "hphp/runtime/server/cli-server-impl.h"

namespace HPHP {

namespace detail {

CLIServerExtensionInterface::CLIServerExtensionInterface(int fd):
  afdt_fd(fd)
#ifndef NDEBUG
, state(State::INIT)
#endif
{}

CLIServerExtensionInterface::~CLIServerExtensionInterface() {
  for (int fd: borrowed_fds) {
    ::close(fd);
  }
#ifndef NDEBUG
  assertx(state == State::DONE_RESULT);
#endif
}

void CLIServerExtensionInterface::assertCount(int count) {
  int expected;
  cli_read(afdt_fd, expected);
  if (count != expected) {
    throw Exception("Expected count %d, got %d", expected, count);
  }
}

// CLIClientInterface

void CLIClientInterface::writeInvokeHeader(int count) {
  cli_write(afdt_fd, "ext", id, count);
}

void CLIClientInterface::readResultHeader() {
  bool function_exists_on_client;
  cli_read(afdt_fd, function_exists_on_client);
  if (!function_exists_on_client) {
    throw new Exception("Function %s does not exist on client", id.c_str());
  }
}


} // namespace HPHP::detail

} // namespace HPHP
