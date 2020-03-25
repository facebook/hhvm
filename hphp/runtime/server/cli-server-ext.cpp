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

namespace {

enum CLISrvArgType {
  CLISRV_ARG_BOOL,
  CLISRV_ARG_INT,
  CLISRV_ARG_INT64,
  CLISRV_ARG_STRING,
  CLISRV_ARG_FD,
  CLISRV_ARG_SOCKADDR,
};

} // namespace

CLIServerExtensionInterface::CLIServerExtensionInterface(int fd):
  afdt_fd(fd)
#ifndef NDEBUG
, state(State::INIT)
#endif
{}

CLIServerExtensionInterface::~CLIServerExtensionInterface() {
#ifndef NDEBUG
  assertx(state == State::DONE_RESULT);
#endif
}

#define CHECK_TYPE(expected) \
  { \
    int type; \
    cli_read(afdt_fd, type); \
    if (type != expected) { \
      throw Exception("Incorrect type - expected %d (%s), got %d", expected, #expected, type); \
    }\
  }

// readers

void CLIServerExtensionInterface::read(bool& arg) {
  CHECK_TYPE(CLISRV_ARG_BOOL);
  cli_read(afdt_fd, arg);
}

void CLIServerExtensionInterface::read(std::string& arg) {
  CHECK_TYPE(CLISRV_ARG_STRING);
  cli_read(afdt_fd, arg);
}

void CLIServerExtensionInterface::read(int& arg) {
  CHECK_TYPE(CLISRV_ARG_INT);
  cli_read(afdt_fd, arg);
}

void CLIServerExtensionInterface::read(int64_t& arg) {
  CHECK_TYPE(CLISRV_ARG_INT64);
  cli_read(afdt_fd, arg);
}

void CLIServerExtensionInterface::read(FdData& arg) {
  CHECK_TYPE(CLISRV_ARG_FD);
  arg.fd = cli_read_fd(afdt_fd);
}

void CLIServerExtensionInterface::assertCount(int count) {
  int expected;
  cli_read(afdt_fd, expected);
  if (count != expected) {
    throw Exception("Expected count %d, got %d", expected, count);
  }
}

void CLIServerExtensionInterface::read(sockaddr_storage& arg) {
  CHECK_TYPE(CLISRV_ARG_SOCKADDR);
  cli_read(afdt_fd, arg);
}

// writers

void CLIServerExtensionInterface::write(bool arg) {
  cli_write(afdt_fd, CLISRV_ARG_BOOL, arg);
}

void CLIServerExtensionInterface::write(FdData arg) {
  cli_write(afdt_fd, CLISRV_ARG_FD);
  cli_write_fd(afdt_fd, arg.fd);
}

void CLIServerExtensionInterface::write(const std::string& arg) {
  cli_write(afdt_fd, CLISRV_ARG_STRING, arg);
}

void CLIServerExtensionInterface::write(int arg) {
  cli_write(afdt_fd, CLISRV_ARG_INT, arg);
}

void CLIServerExtensionInterface::write(int64_t arg) {
  cli_write(afdt_fd, CLISRV_ARG_INT64, arg);
}

void CLIServerExtensionInterface::write(const sockaddr_storage& arg) {
  cli_write(
    afdt_fd,
    CLISRV_ARG_SOCKADDR,
    arg
  );
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
