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

#include <type_traits>

namespace apache::thrift::fast_thrift::thrift::server {

// Forward declaration of the framework's constrained-extension adapter (defined
// in extension/ThriftExtensionPipelineHandler.h). Declared here — rather than
// included — so this allowlist can exempt the adapter without framework/ taking
// a dependency on extension/, and without an include cycle through
// ThriftPipelineHandler.h.
template <typename H>
class ThriftExtensionPipelineHandler;

// A compile-time list of handler types. Membership is the allowlist.
template <typename... Ts>
struct NativeHandlerList {};

namespace native_allowlist_detail {
template <typename T, typename List>
inline constexpr bool contains = false;
template <typename T, typename... Ts>
inline constexpr bool contains<T, NativeHandlerList<Ts...>> =
    (std::is_same_v<T, Ts> || ...);
} // namespace native_allowlist_detail

/**
 * The Thrift-governed allowlist of raw ("native") thrift pipeline handler
 * types.
 *
 * Native handlers own message lifetime and the raw pipeline context, so they
 * can destabilize the server; they are gated to an as-needed basis. Adding a
 * type to THIS list is the only way to permit installing it as a native handler
 * — membership cannot be extended from outside this file, which is
 * Thrift-owned.
 *
 * Default: empty (deny all raw native handlers). Prefer the constrained
 * observer/modifier extension API (FastServerModule::addThriftExtension); reach
 * for a native handler only when an extension genuinely cannot express the
 * need, and grant it here in the same change.
 */
using AllowedNativeThriftHandlers = NativeHandlerList<
    // Grant a native handler by adding its type here, e.g.:
    // acme::MyLowLevelHandler,
    >;

/**
 * True iff `T` may be installed as a native thrift pipeline handler — either it
 * is on AllowedNativeThriftHandlers, or it is the framework's own
 * ThriftExtensionPipelineHandler (the safe wrapper the extension API funnels
 * through, which is always permitted).
 */
template <typename T>
inline constexpr bool kIsAllowedNativeThriftHandler =
    native_allowlist_detail::contains<T, AllowedNativeThriftHandlers>;

template <typename H>
inline constexpr bool
    kIsAllowedNativeThriftHandler<ThriftExtensionPipelineHandler<H>> = true;

} // namespace apache::thrift::fast_thrift::thrift::server
