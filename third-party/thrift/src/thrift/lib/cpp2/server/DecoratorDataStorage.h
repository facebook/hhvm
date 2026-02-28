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

#include <thrift/lib/cpp2/util/AllocationColocator.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>

namespace apache::thrift {

constexpr std::size_t kMaxDecoratorDataSize = 128;

using DecoratorDataEntry = util::TypeErasedValue<kMaxDecoratorDataSize>;

/**
 * DecoratorDataStorage stores an array of DecorotorDataEntry - each of such
 * entries can be expected to have been the result of colocating storage.
 */
struct DecoratorDataStorage {
  std::size_t count = 0;
  util::AllocationColocator<>::ArrayPtr<DecoratorDataEntry> decoratorData =
      nullptr;
};

} // namespace apache::thrift
