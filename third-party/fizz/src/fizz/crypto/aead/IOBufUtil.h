/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <algorithm>

namespace fizz {

/**
 * Trims trimmed.size() bytes at the end of a chained IOBuf and fills in
 * trimmed. The caller is responsible for making sure that buf has enough
 * bytes to trim trimmed.size().
 */
void trimBytes(folly::IOBuf& buf, folly::MutableByteRange trimmed);

/**
 * Trim bytes from the start of an IOBuf potentially spanning multiple
 * buffers in the chain.
 */
void trimStart(folly::IOBuf& buf, size_t toTrim);

/**
 * XOR first and second and store the result in second.
 */
void XOR(folly::ByteRange first, folly::MutableByteRange second);

/**
 * Useful when we need to run a function that performs some operation on
 * an IOBuf and then stores the result in another IOBuf.
 * We assume that the caller ensures that out size >= in size.
 * Apart from that you can pass in chained buffers for output or input
 * buffers or even the same buffer. In the case you pass the same buffer,
 * the Func needs to make sure it can safely deal with this case.
 */
template <typename Func>
void transformBuffer(const folly::IOBuf& in, folly::IOBuf& out, Func func) {
  auto current = &in;
  folly::IOBuf* currentOut = &out;
  size_t offset = 0;

  // Iterate using the buffers instead of iterating with
  // the iobuf iterators.
  do {
    size_t currentLength = current->length();

    while (currentLength != 0) {
      size_t selected = std::min(
          static_cast<size_t>(currentOut->length() - offset), currentLength);
      func(
          currentOut->writableData() + offset,
          current->data() + (current->length() - currentLength),
          selected);
      currentLength -= selected;
      offset += selected;
      if (offset == currentOut->length()) {
        offset = 0;
        currentOut = currentOut->next();
      }
    }
    current = current->next();
  } while (current != &in);
}

/**
 * Useful when we need to run a function that performs operations in chunks
 * and transforms data from in -> out, regardless of whether in or out is
 * chained.  We assume out size >= in size.
 *
 * func is expected to take data from some input buffer, do some operation on it
 * and then place the result of the operation into an output buffer.  It only
 * should write to output in blocks of size BlockSize.  Data from the input
 * buffer must be internally buffered by func if it can not write a full block
 * to output.
 */
template <size_t BlockSize, typename Func>
folly::io::RWPrivateCursor
transformBufferBlocks(const folly::IOBuf& in, folly::IOBuf& out, Func func) {
  size_t internallyBuffered = 0;
  folly::io::RWPrivateCursor output(&out);
  folly::io::Cursor input(&in);

  // block to handle writes along output buffer boundaries
  std::array<uint8_t, BlockSize> blockBuffer = {};

  // ensure buffers are fine
  while (!input.isAtEnd()) {
    auto inputRange = input.peekBytes();
    auto inputLen = inputRange.size();
    auto outputLen = output.peekBytes().size();
    if (inputLen + internallyBuffered < BlockSize) {
      // input doesn't have enough - we can call func and it should
      // internally buffer.
      // This should be safe to just internally buffer since we took into
      // account what was existing in the internal buffer already
      auto numWritten = func(blockBuffer.data(), inputRange.data(), inputLen);
      DCHECK_EQ(numWritten, 0) << "expected buffering. wrote " << numWritten;
      // only update input offsets
      internallyBuffered += inputLen;
      input.skip(inputLen);
    } else if (outputLen < BlockSize) {
      // we have at least a block to write from input + internal buffer, so
      // output didn't have enough room in this case
      // copy a block from input in temp and then push onto output
      auto numWritten = func(
          blockBuffer.data(),
          inputRange.data(),
          // only provide it the amount needed for one block
          BlockSize - internallyBuffered);
      DCHECK_EQ(numWritten, BlockSize)
          << "did not write full block bs=" << BlockSize
          << " wrote=" << numWritten;

      // push the block onto output
      output.push(blockBuffer.data(), BlockSize);

      // advance input
      input.skip(BlockSize - internallyBuffered);
      internallyBuffered = 0;
    } else {
      // we have at least one block that can be written from input to output
      // calculate shared bytes while taking into account internal buffer
      auto numSharedBytes = std::min(outputLen, inputLen + internallyBuffered);

      // this is the number we can safely (and expect) to write to output
      auto numBlockBytes = numSharedBytes - (numSharedBytes % BlockSize);

      // try to grab as much from input - we can grab up to BlockSize - 1 more
      auto maxToTake = (numBlockBytes - internallyBuffered) + (BlockSize - 1);
      auto numToTake = std::min(inputLen, maxToTake);
      auto numWritten =
          func(output.writableData(), inputRange.data(), numToTake);

      DCHECK_EQ(numWritten, numBlockBytes);

      // advance cursors
      input.skip(numToTake);
      output.skip(numWritten);
      // recalculate internal buffer state
      internallyBuffered = (internallyBuffered + numToTake) % BlockSize;
    }
  }
  return output;
}
} // namespace fizz
