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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {

  Array HHVM_FUNCTION(HH_io_pipe) {
    int fds[2];
    if (::pipe(fds) != 0) {
      raise_error("Failed to pipe(2)");
    }
    return make_varray(
      req::make<PlainFile>(fds[0]),
      req::make<PlainFile>(fds[1])
    );
  }

  struct IOExtension final : Extension {

    IOExtension() : Extension("hsl_io", "1.0") {}

    void moduleInit() override {
      HHVM_FALIAS(
        HH\\Lib\\_Private\\Native\\pipe,
        HH_io_pipe
      );
      loadSystemlib();
    }
  } s_io_extension;

} // anonymous namespace
} // namespace HPHP
