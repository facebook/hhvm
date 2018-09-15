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

#ifndef incl_HPHP_UTIL_ZSTD_HELPERS_H_
#define incl_HPHP_UTIL_ZSTD_HELPERS_H_

#include <cstddef>
#include <memory>

#include <folly/Memory.h>

#include <zstd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ZstdCompressor {
 public:
  explicit ZstdCompressor(int compression_level);

  const char* compress(const void* data,
                       size_t& len,
                       bool last);

 private:
  static void zstd_cctx_deleter(ZSTD_CCtx* ctx);

  using ZSTD_CCtx_Ptr = std::unique_ptr<
      ZSTD_CCtx,
      folly::static_function_deleter<ZSTD_CCtx, zstd_cctx_deleter>>;

  static ZSTD_CCtx_Ptr make_zstd_cctx();

  int compression_level_;
  ZSTD_CCtx_Ptr ctx_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
