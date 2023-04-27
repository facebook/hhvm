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

#pragma once

#include <thrift/lib/cpp2/transport/rocket/Types.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace test {

folly::StringPiece getRange(folly::IOBuf& iobuf);
folly::StringPiece getRange(const folly::IOBuf& iobuf);

std::pair<std::unique_ptr<folly::IOBuf>, std::unique_ptr<folly::IOBuf>>
splitMetadataAndData(const Payload& p);

} // namespace test
} // namespace rocket
} // namespace thrift
} // namespace apache
