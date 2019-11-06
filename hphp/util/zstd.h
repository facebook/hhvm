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

#include <zstd.h>

#include "hphp/util/string-holder.h"
#include "hphp/util/compression-ctx-pool.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ZstdCompressor {
 public:
  explicit ZstdCompressor(int compression_level, bool should_checksum = false);

  StringHolder compress(const void* data, size_t& len, bool last);

  void setChecksum(bool should_checksum);

 protected:
  static void zstd_cctx_deleter(ZSTD_CCtx* ctx);

  using ContextPool = CompressionContextPool<
      ZSTD_CCtx, ZSTD_createCCtx, zstd_cctx_deleter>;

  // we use two context pools because the streaming contexts will always use
  // exactly the same params, which lets us avoid resetting the internal
  // tables, whereas the singleshot pools will churn through different params.

  static ContextPool streaming_cctx_pool;

  static ContextPool single_shot_cctx_pool;

  static ContextPool::Ref make_zstd_cctx(bool last);

 public:
  static bool s_useLocalArena;
 protected:
  int compression_level_;
  bool should_checksum_;
  ContextPool::Ref ctx_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
