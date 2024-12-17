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
#include <thrift/lib/cpp2/async/AlwaysAllowSheddingInteractionOverloadPolicy.h>
#include <thrift/lib/cpp2/async/TerminateInteractionOverloadPolicy.h>

THRIFT_FLAG_DEFINE_string(interaction_overload_protection_policy, "default");

namespace apache::thrift {

/*static*/ std::unique_ptr<InteractionOverloadPolicy>
InteractionOverloadPolicy::createFromThriftFlag() {
  if (THRIFT_FLAG(interaction_overload_protection_policy) == "terminate") {
    return std::make_unique<TerminateInteractionOverloadPolicy>();
  } else {
    return std::make_unique<DefaultInteractionOverloadPolicy>();
  }
}

} // namespace apache::thrift
