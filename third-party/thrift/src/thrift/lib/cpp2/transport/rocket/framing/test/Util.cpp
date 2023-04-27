/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transport/rocket/framing/test/Util.h>

#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace test {

folly::StringPiece getRange(folly::IOBuf& iobuf) {
  return folly::StringPiece(iobuf.coalesce());
}

folly::StringPiece getRange(const folly::IOBuf& iobuf) {
  EXPECT_FALSE(iobuf.isChained());
  return folly::StringPiece{
      reinterpret_cast<const char*>(iobuf.data()), iobuf.length()};
}

std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>
splitMetadataAndData(const Payload& p) {
  EXPECT_NE(nullptr, p.buffer());
  folly::IOBufQueue q;
  q.append(p.buffer()->clone());
  auto metadata = q.split(p.metadataSize());
  if (!metadata) {
    metadata = std::make_unique<folly::IOBuf>();
  }
  metadata->coalesce();
  auto data = q.move();
  if (!data) {
    data = std::make_unique<folly::IOBuf>();
  }
  data->coalesce();
  return {std::move(metadata), std::move(data)};
}

} // namespace test
} // namespace rocket
} // namespace thrift
} // namespace apache
