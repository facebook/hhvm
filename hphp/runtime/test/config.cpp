/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <gtest/gtest.h>

#include "hphp/runtime/base/config.h"

namespace HPHP {

TEST(Config, HdfToIni) {
  EXPECT_EQ("hhvm.jit", Config::IniName("Eval.Jit"));
  EXPECT_EQ("hhvm.jit_llvm", Config::IniName("Eval.JitLLVM"));

  EXPECT_EQ("hhvm.server.ssl_port", Config::IniName("Server.SSLPort"));
  EXPECT_EQ("max_file_uploads",
            Config::IniName("Server.Upload.MaxFileUploads"));
  EXPECT_EQ("hhvm.force_hh", Config::IniName("Eval.EnableHipHopSyntax"));
  EXPECT_EQ("server.ssl_port", Config::IniName("Server.SSLPort", false));
}

}
