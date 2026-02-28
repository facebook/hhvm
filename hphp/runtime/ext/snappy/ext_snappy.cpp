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

#include "hphp/runtime/ext/extension.h"

#include <snappy.h>

namespace HPHP {

Variant HHVM_FUNCTION(snappy_compress, const String& data) {
  String s = String(snappy::MaxCompressedLength(data.size()), ReserveString);
  size_t size;
  snappy::RawCompress(data.data(), data.size(), s.mutableData(), &size);
  return s.shrink(size);
}

Variant HHVM_FUNCTION(snappy_uncompress, const String& data) {
  size_t dsize;
  if (!snappy::GetUncompressedLength(data.data(), data.size(), &dsize)) {
    return false;
  }

  String s = String(dsize, ReserveString);
  if (!snappy::RawUncompress(data.data(), data.size(), s.mutableData())) {
    return false;
  }
  return s.setSize(dsize);
}

static struct SnappyExtension final : Extension {
  SnappyExtension() : Extension("snappy", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_FE(snappy_compress);
    HHVM_FE(snappy_uncompress);
    HHVM_FALIAS(sncompress, snappy_compress);
    HHVM_FALIAS(snuncompress, snappy_uncompress);
  }
} s_snappy_extension;

}
