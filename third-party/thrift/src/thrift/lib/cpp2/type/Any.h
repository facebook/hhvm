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

#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache {
namespace thrift {
namespace type {

// TODO(afuller): Add an 'Any' class that can be any of:
// - Deserialized-Value (AnyValue)
// - Deserialized-Reference (AnyRef)
// - Serialized-Value (AnyData)
// - Serialized-Reference (AnyData?),  Such a rep would be able to
// efficiently 'snip' serialized values out of other serialized values, without
// deserializing. Note: As type::ByteBuffer is actually a folly::IOBuf, AnyData
// might already be sufficent to represent this. In which case all that would be
// needed is the 'snipping' logic.

// An Any value, that may not contain all the required information
// to deserilize the value.
//
// TODO(afuller): Add native wrappers.
using SemiAny = SemiAnyStruct;

// A serilaized Any value, with all the required information to deserilize the
// value.
class AnyData : public detail::Wrap<AnyStruct> {
  using Base = detail::Wrap<AnyStruct>;

 public:
  using Base::Base;

  // Throws an std::runtime_error if the SemiAny is missing the type or
  // protocol.
  explicit AnyData(SemiAny semiAny);

  const Type& type() const { return *data_.type(); }
  const Protocol& protocol() const { return *data_.protocol(); }
  const folly::IOBuf& data() const { return *data_.data(); }
};

} // namespace type
} // namespace thrift
} // namespace apache
