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

#include <thrift/lib/cpp2/async/InteractionOverloadPolicy.h>

namespace apache::thrift {

/**
 * A InteractionOverloadPolicy that marks the interaction as "load-shedded" if
 * any request is rejected due to load-shedding. Any subsequent requests
 * (including retries of rejected requests) will fail.
 *
 * NOTE: This policy maintains ordering guarantees of interactions.
 */
class TerminateInteractionOverloadPolicy : public InteractionOverloadPolicy {
 public:
  void onRequestLoadshed() override { isLoadshedded_ = true; }

  bool allowNewRequest() override { return !isLoadshedded_; }

 private:
  bool isLoadshedded_{false};
};

} // namespace apache::thrift
