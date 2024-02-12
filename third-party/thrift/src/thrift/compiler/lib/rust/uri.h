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

namespace apache {
namespace thrift {
namespace compiler {
namespace rust {
constexpr auto kRustOrdUri = "facebook.com/thrift/annotation/rust/Ord";
constexpr auto kRustBoxUri = "facebook.com/thrift/annotation/rust/Box";
constexpr auto kRustTypeUri = "facebook.com/thrift/annotation/rust/Type";
constexpr auto kRustNewTypeUri = "facebook.com/thrift/annotation/rust/NewType";
constexpr auto kRustAdapterUri = "facebook.com/thrift/annotation/rust/Adapter";
constexpr auto kRustDeriveUri = "facebook.com/thrift/annotation/rust/Derive";
constexpr auto kRustServiceExnUri =
    "facebook.com/thrift/annotation/rust/ServiceExn";
constexpr auto kRustExhaustiveUri =
    "facebook.com/thrift/annotation/rust/Exhaustive";
constexpr auto kRustArcUri = "facebook.com/thrift/annotation/rust/Arc";
constexpr auto kRustRequestContextUri =
    "facebook.com/thrift/annotation/rust/RequestContext";
constexpr auto kRustCopyUri = "facebook.com/thrift/annotation/rust/Copy";
constexpr auto kRustNameUri = "facebook.com/thrift/annotation/rust/Name";

} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
