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

#include <thrift/lib/cpp2/server/DecoratorDataRuntime.h>

namespace apache::thrift::server {

DecoratorDataHandleFactory::DecoratorDataHandleFactory(
    std::vector<const UntypedDecoratorKey*>& keysToBeAllocated)
    : keysToBeAllocated_{keysToBeAllocated} {}

DecoratorDataPerRequestBlueprint::Setup::Setup() : keysToBeAllocated_{} {}

DecoratorDataHandleFactory
DecoratorDataPerRequestBlueprint::Setup::getHandleFactory() {
  return DecoratorDataHandleFactory(keysToBeAllocated_);
}

DecoratorDataPerRequestBlueprint
DecoratorDataPerRequestBlueprint::Setup::finalize() && {
  return DecoratorDataPerRequestBlueprint(keysToBeAllocated_.size());
}

DecoratorDataPerRequestBlueprint::DecoratorDataPerRequestBlueprint(
    std::size_t numEntries)
    : numEntries_{numEntries} {}

} // namespace apache::thrift::server
