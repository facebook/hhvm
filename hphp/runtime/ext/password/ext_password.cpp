/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {
const StaticString s_PASSWORD_BCRYPT("PASSWORD_BCRYPT");
const StaticString s_PASSWORD_DEFAULT("PASSWORD_DEFAULT");

const int64_t k_PASSWORD_BCRYPT = 1;
const int64_t k_PASSWORD_DEFAULT = k_PASSWORD_BCRYPT;

class PasswordExtension final : public Extension {
 public:
  PasswordExtension() : Extension("password") {}
  void moduleInit() override {
    Native::registerConstant<KindOfInt64>(
      s_PASSWORD_BCRYPT.get(), k_PASSWORD_BCRYPT
    );
    Native::registerConstant<KindOfInt64>(
      s_PASSWORD_DEFAULT.get(), k_PASSWORD_DEFAULT
    );

    loadSystemlib();
  }
} s_password_extension;
}
