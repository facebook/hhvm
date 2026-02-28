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

#include <lz4.h>
#include <lz4hc.h>

namespace HPHP {

// Varint helper functions for lz4
static int VarintSize(int val) {
  int s = 1;
  while (val >= 128) {
    ++s;
    val >>= 7;
  }
  return s;
}

static void VarintEncode(int val, char** dest) {
  char* p = *dest;
  while (val >= 128) {
    *p++ = 0x80 | (static_cast<char>(val) & 0x7f);
    val >>= 7;
  }
  *p++ = static_cast<char>(val);
  *dest = p;
}

static int VarintDecode(const char** src, int max_size) {
  const char* p = *src;
  int val = 0;
  int shift = 0;
  while (*p & 0x80) {
    if (max_size <= 1) { return -1; }
    --max_size;
    val |= static_cast<int>(*p++ & 0x7f) << shift;
    shift += 7;
  }
  val |= static_cast<int>(*p++) << shift;
  *src = p;
  return val;
}

Variant HHVM_FUNCTION(lz4_compress, const String& uncompressed,
                                    bool high /* = false */) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  String s = String(bufsize + headerSize, ReserveString);
  char* compressed = s.mutableData();
  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int size;
  if (high) {
    size = LZ4_compress_HC(
        uncompressed.data(), compressed, uncompressed.size(), bufsize, 0);
  } else {
    size = LZ4_compress_default(
        uncompressed.data(), compressed, uncompressed.size(), bufsize);
  }
  if (size < 0) {
    return false;
  }
  return s.shrink(size + headerSize);
}

Variant HHVM_FUNCTION(lz4_uncompress, const String& compressed) {
  const char* compressed_ptr = compressed.data();
  int dsize = VarintDecode(&compressed_ptr, compressed.size());
  if (dsize < 0) {
    return false;
  }

  int inSize = compressed.size() - (compressed_ptr - compressed.data());
  String s = String(dsize, ReserveString);
  char* uncompressed = s.mutableData();
  int ret = LZ4_decompress_safe(compressed_ptr, uncompressed, inSize, dsize);
  if (ret != dsize) {
    return false;
  }

  return s.setSize(dsize);
}

static struct LZ4Extension final : Extension {
  LZ4Extension() : Extension("lz4", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_FE(lz4_compress);
    HHVM_FE(lz4_uncompress);
  }
} s_lz4_extension;

}
