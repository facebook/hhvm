/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/ZstdStreamCompressor.h>

#include <folly/compression/Compression.h>

namespace proxygen {

ZstdStreamCompressor::ZstdStreamCompressor(int compressionLevel,
                                           bool independentChunks)
    : codec_(nullptr),
      compressionLevel_(compressionLevel),
      independent_(independentChunks) {
}

folly::io::StreamCodec& ZstdStreamCompressor::getCodec() {
  if (!codec_) {
    codec_ = folly::io::getStreamCodec(folly::io::CodecType::ZSTD,
                                       compressionLevel_);
  }
  return *codec_;
}

std::unique_ptr<folly::IOBuf> ZstdStreamCompressor::compress(
    const folly::IOBuf* in, bool last) {
  if (error_) {
    return nullptr;
  }

  if (in == nullptr) {
    error_ = true;
    return nullptr;
  }

  try {
    folly::IOBuf clone;
    if (in->isChained()) {
      clone = in->cloneCoalescedAsValueWithHeadroomTailroom(0, 0);
      in = &clone;
    }

    auto op = last || independent_ ? folly::io::StreamCodec::FlushOp::END
                                   : folly::io::StreamCodec::FlushOp::FLUSH;
    auto& codec = getCodec();
    if (independent_) {
      codec.resetStream(in->length());
    }

    auto compressBound = codec.maxCompressedLength(in->length());
    auto out = folly::IOBuf::create(compressBound + 1);

    folly::ByteRange inrange{in->data(), in->length()};
    folly::MutableByteRange outrange{out->writableTail(), out->tailroom()};

    auto success = codec.compressStream(inrange, outrange, op);

    if (!success) {
      error_ = true;
      return {};
    }

    DCHECK_EQ(inrange.size(), 0);
    DCHECK_GT(outrange.size(), 0);

    out->append(outrange.begin() - out->tail());

    if (op == folly::io::StreamCodec::FlushOp::END) {
      codec_.reset();
    }

    return out;
  } catch (const std::exception&) {
    error_ = true;
  }
  return {};
}

} // namespace proxygen
