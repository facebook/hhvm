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

namespace apache::thrift {

/*
 * InteractionOverloadPolicy defines the interface for determining the behavior
 * of overload protection mechanisms (Host Overload, App Overload, Queue
 * Timeout) when using interactions.
 *
 * NOTE: This policy is only used **after** the first interaction request has
 * been processed (not necessarily executed). The behavior **before** the first
 * request is processed is the same regardless of the policy used.
 */
class InteractionOverloadPolicy {
 public:
  virtual ~InteractionOverloadPolicy() = default;
  /*
   * Must be called when an interaction request is rejected due to load-shedding
   * (App Overload, Host Overload, or Queue Timeout).
   */
  virtual void onRequestLoadshed() = 0;

  /*
   * Used to check if the overload policy allows a new request to be accepted.
   * Some policies may allow new requests to be accepted even if there has been
   * requests load-shedded previously. While others may reject any new requests
   * if there has been any previous request load-shedded.
   */
  virtual bool allowNewRequest() = 0;
};

} // namespace apache::thrift
