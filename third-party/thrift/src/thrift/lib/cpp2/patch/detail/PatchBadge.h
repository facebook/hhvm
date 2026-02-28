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
#include <folly/lang/Badge.h>
namespace apache::thrift::protocol::detail {

// We use the badge pattern to control access to certain methods in our
// codebase. Some methods should be private to end users but public within the
// detail namespace. To achieve this, we require a `Badge` as an argument for
// these methods.
//
// For example:
//
//   class MyClass {
//    public:
//     void semiPrivateMethod(Badge, args...);
//   };
//
// The Badge can only be obtained by including a specific file that is protected
// by visibility. This ensures that end users cannot invoke the
// `semiPrivateMethod(...)` directly.
//
// One might ask why we don't use the `private + friend` approach instead. The
// reason is that we have multiple classes with semi-private methods and many
// places in the detail namespace where these methods need to be invoked. Using
// friend would require creating a large number of friend relationships (O(NM)),
// which is not scalable. With the badge pattern, we centralize access control
// in a single place.
//
// For more information on the badge pattern, see folly/lang/Badge.h.
struct PatchBadgeFactory {
  constexpr static auto create() { return folly::badge<PatchBadgeFactory>(); }
};

using Badge = folly::badge<PatchBadgeFactory>;
inline constexpr Badge badge = PatchBadgeFactory::create();
} // namespace apache::thrift::protocol::detail
