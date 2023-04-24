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

#include <folly/net/NetworkSocket.h>

#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {
namespace rocket {
namespace plugin {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    void,
    applyCustomQosMarkingToSocket,
    ::folly::NetworkSocket,
    const ::apache::thrift::RequestSetupMetadata&);

}
} // namespace rocket
} // namespace thrift
} // namespace apache
