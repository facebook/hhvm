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

#include "hphp/runtime/ext/scrypt/crypto/crypto_scrypt.h"
#include "hphp/runtime/ext/scrypt/crypto/params.h"

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/ext/string/ext_string.h"

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>

namespace {

using namespace HPHP;

static const std::string s_DsD("$s$"), s_D("$");

Variant HHVM_FUNCTION(scrypt_enc, const String& password, const String& salt,
                      int64_t N, int64_t r, int64_t p) {

  int cryptN = N;
  uint32_t cryptR = r;
  uint32_t cryptP = p;
  unsigned char buf[64];

  // crypto_scrypt sanity-checks the actual parameter values, but make
  // sure we don't overflow.
  if (r < 0 || r > INT32_MAX || p < 0 || p > INT32_MAX) {
    return false;
  }

  // 1 << N must fit in a uint64_t
  if (N < 0 || N > 63) {
    return false;
  }

  if (cryptN == 0 || cryptR == 0 || cryptP == 0) {
    pickparams(0, 0, .15, &cryptN, &cryptR, &cryptP);
  }

  int ret = crypto_scrypt((uint8_t *)password.c_str(),
                          (size_t)password.size(), (uint8_t *)salt.c_str(),
                          (size_t)salt.size(),
                          1ULL << cryptN, cryptR, cryptP,
                          buf, sizeof(buf));

  if (ret)
    return false;

  String buf_str((char *) buf, sizeof(buf), CopyString);
  return s_DsD + String((int)cryptN) +
         s_D + String((int)cryptR) +
         s_D + String((int)cryptP) +
         s_D + StringUtil::Base64Encode(salt) +
         s_D + StringUtil::Base64Encode(buf_str);
}

struct ScryptExtension : Extension {
  public:
    ScryptExtension(): Extension("scrypt", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
    void moduleRegisterNative() override {
      HHVM_FE(scrypt_enc);
    }
} s_scrypt_extension;

} // anonymous namespace
