/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/brotli.h"
#include <enc/encode.h>
#include <folly/ScopeGuard.h>

// The version of brotli we are using does implement these helper
// methods that we need. However the methods are not declared in
// the core brotli libraries. Lets declare those here.
namespace brotli {
size_t CopyOneBlockToRingBuffer(BrotliIn* r,
                                BrotliCompressor* compressor);
bool BrotliInIsFinished(brotli::BrotliIn* r);
}

using namespace brotli;


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char* compressBrotli(BrotliCompressor* compressor,
                           const void* data,
                           size_t& len,
                           bool last) {
  // Brotli does not have a utility to compute max size of the buffer
  // in case data is incompressible. Below link discusses some numbers
  // and formula where 16MB block would use only 6 extra bytes.
  // For all practical usage we should be fine with 20 bytes.
  // https://github.com/google/brotli/issues/274
  // We should also allow 6 extra bytes for an empty meta-block at
  // the end of each chunk to force "flush".
  size_t availableBytes = len + 30;
  auto available = (char *)malloc(len + availableBytes);
  auto deleter = folly::makeGuard([&] { free(available); });

  BrotliMemIn in(data, len);
  BrotliMemOut out(available, availableBytes);
  bool finalBlock = false;
  while (!finalBlock) {
    auto inBytes = CopyOneBlockToRingBuffer(&in, compressor);
    finalBlock = inBytes == 0 || BrotliInIsFinished(&in);
    size_t outBytes = 0;
    uint8_t* output = nullptr;
    if (!compressor->WriteBrotliData(last && finalBlock,
                                             /* force_flush */ finalBlock,
                                             &outBytes,
                                             &output)) {
      return nullptr;
    }

    // This deserves an explanation as brotli's documentation is
    // really incomplete on the topic.
    // 'force_flush' is what they call a "soft flush" and it stops at the byte
    // boundary of the compressed buffer. As such, there is a 7/8 chance that
    // last few bytes will be held by the compressor. To force the compressor
    // stop at the byte boundary one can write an empty meta-block.
    if (!last && finalBlock) {
      size_t bytes = 6;
      compressor->WriteMetadata(
          0, nullptr, false, &bytes, output + outBytes);
      outBytes += bytes;
    }

    if (outBytes > 0 && !out.Write(output, outBytes)) {
      return nullptr;
    }
  }

  deleter.dismiss();
  len = out.position();

  return available;
}

///////////////////////////////////////////////////////////////////////////////
}
