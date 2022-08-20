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

#include <thrift/lib/cpp/transport/THeader.h>

namespace apache {
namespace thrift {

/**
 * HeaderChannelTrait manages THeader specific channel level information.
 *
 * TODO(ckwalsh): This should be refactored out into a state object belonging
 * to each channel at some point in the future.
 */
class HeaderChannelTrait {
 public:
  static bool isSupportedClient(CLIENT_TYPE ct);
  static void checkSupportedClient(CLIENT_TYPE ct);
};
} // namespace thrift
} // namespace apache
