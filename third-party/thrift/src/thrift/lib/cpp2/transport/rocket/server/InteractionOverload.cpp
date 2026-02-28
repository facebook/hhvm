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

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/transport/rocket/server/InteractionOverload.h>

namespace apache::thrift {

bool shouldTerminateInteraction(
    bool isInteractionCreatePresent,
    std::optional<int64_t> interactionId,
    Cpp2ConnContext* connContext) {
  using namespace apache::thrift::detail;
  if (!THRIFT_FLAG(enable_interaction_overload_protection_server)) {
    return false;
  }

  // First request - If this request is being rejected at this stage, the
  // Tile/TilePromise will never get created and the interaction will be in a
  // bad state resulting in all subsequent requests to fail, regardless of
  // interaction type. We send a terminal error instead to inform the client of
  // this, regardless of policy.
  if (isInteractionCreatePresent) {
    return true;
  }

  // Invalid - either interactionCreate must be present or interactionId must be
  // set.
  if (!interactionId) {
    return true;
  }

  auto interaction =
      Cpp2ConnContextInternalAPI(*connContext).findTile(*interactionId);
  if (!interaction) {
    // If no interaction is found, this means interaction is in a bad state
    // (Tile/TilePromise was never created) and we send terminal error.
    return true;
  }

  // Subsequent request - We use policy to decide if we should send terminal
  // error or not.
  if (auto* overloadPolicy =
          TileInternalAPI(*interaction).getOverloadPolicy()) {
    overloadPolicy->onRequestLoadshed();
    return !overloadPolicy->allowNewRequest();
  }

  // If policy is not found, use legacy behavior.
  return false;
}

std::string_view mapToTerminalError(std::string_view code) noexcept {
  if (code == kOverloadedErrorCode) {
    return kInteractionLoadsheddedOverloadErrorCode;
  } else if (code == kAppOverloadedErrorCode) {
    return kInteractionLoadsheddedAppOverloadErrorCode;
  } else {
    return code;
  }
}

} // namespace apache::thrift
