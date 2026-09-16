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

#include <gtest/gtest.h>

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/ini-setting.h"

namespace HPHP {

TEST(Config, HdfToIni) {
  EXPECT_EQ("hhvm.jit", Config::IniName("Eval.Jit"));

  EXPECT_EQ("hhvm.server.ssl_port", Config::IniName("Server.SSLPort"));
  EXPECT_EQ("max_file_uploads",
            Config::IniName("Server.Upload.MaxFileUploads"));
  EXPECT_EQ("server.ssl_port", Config::IniName("Server.SSLPort", false));
}

TEST(Config, MatchHdfPattern) {
  IniSettingMap ini;
  Hdf hdf;
  hdf["task"] =
    R"REGEX(/^tsp_(?:cln|rva)\/[^\/]+\/c3\.web(-[0-9]+)?\//)REGEX";

  EXPECT_TRUE(Config::matchHdfPattern(
    "tsp_cln/example/c3.web/", ini, hdf, "task"
  ));
  EXPECT_FALSE(Config::matchHdfPattern(
    "tsp_ash/example/c3.web/", ini, hdf, "task"
  ));
}

TEST(Config, MatchHdfPatternRejectsMalformedRegex) {
  IniSettingMap ini;
  Hdf hdf;
  hdf["task"] =
    R"REGEX(/^tsp_(?:cln|rva)\/[^/]+\/c3\.web(-[0-9]+)?\//)REGEX";

  EXPECT_THROW(
    Config::matchHdfPattern(
      "tsp_cln/example/c3.web/", ini, hdf, "task"
    ),
    std::runtime_error
  );
}

TEST(Config, MatchHdfPatternSetRejectsMalformedRegexAfterNonMatch) {
  IniSettingMap ini;
  Hdf hdf;
  hdf["tagset"][0] = "/does-not-match/";
  hdf["tagset"][1] =
    R"REGEX(/^tsp_(?:cln|rva)\/[^/]+\/c3\.web(-[0-9]+)?\//)REGEX";

  EXPECT_THROW(
    Config::matchHdfPatternSet("example", ini, hdf, "tagset"),
    std::runtime_error
  );
}

}
