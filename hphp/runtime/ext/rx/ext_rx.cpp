/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
  const StaticString s_IS_ENABLED("HH\\Rx\\IS_ENABLED");
}

struct RxExtension final : Extension {
  RxExtension() : Extension("rx") {}

  void moduleInit() override {
    auto const isEnabled = RuntimeOption::EvalRxPretendIsEnabled;
    Native::registerConstant<KindOfBoolean>(s_IS_ENABLED.get(), isEnabled);
  }
} s_rx_extension;

///////////////////////////////////////////////////////////////////////////////
}
