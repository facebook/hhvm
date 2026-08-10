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

#include <google/protobuf/arena.h> // @manual
#include <google/protobuf/stubs/common.h> // @manual

// Arena-allocates a message so that its sub-allocations also land on the arena.
// `Arena::Create` only gained that behaviour once `CreateMessage` was removed;
// threshold matches `velox/dwio/common/Arena.h`.
template <typename Struct>
Struct* arenaCreate(google::protobuf::Arena& arena) {
#if GOOGLE_PROTOBUF_VERSION >= 5030000
  return google::protobuf::Arena::Create<Struct>(&arena);
#else
  return google::protobuf::Arena::CreateMessage<Struct>(&arena);
#endif
}
