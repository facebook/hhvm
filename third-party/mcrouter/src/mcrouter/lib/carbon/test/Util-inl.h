/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/uio.h>

#include <cstring>

#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/CarbonProtocolReader.h"
#include "mcrouter/lib/carbon/CarbonProtocolWriter.h"
#include "mcrouter/lib/carbon/CommonSerializationTraits.h"

namespace carbon {
namespace test {
namespace util {
namespace detail {
template <class Out>
Out deserialize(CarbonQueueAppenderStorage& storage, size_t& bytesWritten) {
  // Fill the serialized data into an IOBuf
  folly::IOBuf buf(folly::IOBuf::CREATE, 2048);
  auto* curBuf = &buf;
  const auto iovs = storage.getIovecs();
  bytesWritten = 0;
  for (size_t i = 0; i < iovs.second; ++i) {
    const struct iovec* iov = iovs.first + i;
    size_t written = 0;
    while (written < iov->iov_len) {
      const auto bytesToWrite =
          std::min(iov->iov_len - written, curBuf->tailroom());
      std::memcpy(
          curBuf->writableTail(),
          reinterpret_cast<const uint8_t*>(iov->iov_base) + written,
          bytesToWrite);
      curBuf->append(bytesToWrite);
      written += bytesToWrite;
      bytesWritten += written;

      if (written < iov->iov_len) {
        // Append new buffer with enough room for remaining data in this
        // iovec,
        // plus a bit more space for the next iovec's data
        curBuf->appendChain(
            folly::IOBuf::create(iov->iov_len - written + 2048));
        curBuf = curBuf->next();
      }
    }
  }

  // Deserialize the serialized data
  Out deserialized;
  CarbonProtocolReader reader{carbon::CarbonCursor(&buf)};
  deserialized.deserialize(reader);

  return deserialized;
}
} // namespace detail

template <class T, class Out>
Out serializeAndDeserialize(const T& toSerialize, size_t& bytesWritten) {
  // Serialize the request
  CarbonQueueAppenderStorage storage;
  CarbonProtocolWriter writer(storage);
  toSerialize.serialize(writer);

  return detail::deserialize<Out>(storage, bytesWritten);
}

template <class T, class Out>
Out serializeAndDeserialize(const T& toSerialize) {
  size_t tmp;
  return serializeAndDeserialize<T, Out>(toSerialize, tmp);
}

template <class T>
void expectEqSimpleStruct(const T& a, const T& b) {
  EXPECT_EQ(a.int32Member(), b.int32Member());
  EXPECT_EQ(a.stringMember(), b.stringMember());
  EXPECT_EQ(a.enumMember(), b.enumMember());
}

} // namespace util
} // namespace test
} // namespace carbon
