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

#ifndef incl_HPHP_CLI_SERVER_IO_H_
#define incl_HPHP_CLI_SERVER_IO_H_

#include "hphp/util/afdt-util.h"
#include "hphp/util/trace.h"

#include <afdt.h>
#include <functional>

namespace HPHP {

int cli_get_cli_sock();

template<class... Args>
void cli_write(int afdt_fd, Args&&... args) {
  FTRACE(4, "cli_write({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::sendx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::sendRaw: %s [%s]",
                    ex.what(), folly::errnoStr(errno).c_str());
  }
}

template<class... Args>
void cli_read(int afdt_fd, Args&&... args) {
  FTRACE(4, "cli_read({}, nargs={})\n", afdt_fd, sizeof...(args) + 1);
  try {
    afdt::recvx(afdt_fd, std::forward<Args>(args)...);
  } catch (const std::runtime_error& ex) {
    throw Exception("Failed in afdt::recvRaw: %s [%s]",
                    ex.what(), folly::errnoStr(errno).c_str());
  }
}

int cli_read_fd(int afdt_fd);
void cli_write_fd(int afdt_fd, int fd);

void cli_register_handler(const char* name, std::function<void(int)> impl);
template<class... Args> void cli_invoke_handler(const char* name, Args&&... args) {
  int afdt_fd = cli_get_cli_sock();
  cli_write(afdt_fd, "ext", name, std::forward<Args>(args)...);
  bool handler_defined;
  cli_read(afdt_fd, handler_defined);
  if (!handler_defined) {
    throw Exception("Undefined CLI extension handler: %s", name);
  }
}

}

#define CLI_INVOKE_HANDLER(x, ...) \
  static_assert(&x##_cli_client); \
  cli_invoke_handler(#x, __VA_ARGS__);

#define CLI_CLIENT_HANDLER(x, ...) \
x##_cli_client(__VA_ARGS__)

#define CLI_REGISTER_HANDLER(name) \
  cli_register_handler(#name, name##_cli_client);

#endif
