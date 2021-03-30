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

#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {
  bool s_enabled;

  struct HslSystemlibExtension final : Extension {

    HslSystemlibExtension() : Extension("hsl_systemlib", "1.0") {}

    void moduleLoad(const IniSetting::Map& ini, const Hdf hdf) override {
      Config::Bind(s_enabled, ini, hdf, "Eval.HSLSystemlibEnabled", true);
    }

    void moduleInit() override {
      if (s_enabled) {
        loadSystemlib();
      }
    }

    const DependencySet getDeps() const override {
      return DependencySet({"string"});
    }

  } s_hsl_systemlib_extension;

} // anonymous namespace
} // namespace HPHP
