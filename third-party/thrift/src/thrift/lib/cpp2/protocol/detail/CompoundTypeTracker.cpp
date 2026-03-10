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

#include <thrift/lib/cpp2/protocol/detail/CompoundTypeTracker.h>

#include <folly/Exception.h>

namespace apache::thrift::json5::detail {

void CompoundTypeTracker::beginList() {
  containerStack_.push_back(List{});
}

void CompoundTypeTracker::beginSet() {
  containerStack_.push_back(Set{});
}

void CompoundTypeTracker::beginMap(MapForm form) {
  containerStack_.push_back(
      Map{.expectingToken = MapToken::Key, .encodedAs = form});
}

void CompoundTypeTracker::beginStruct() {
  containerStack_.push_back(Struct{});
}

template <class Type, class Stack>
static void pop(Stack& stack) {
  CHECK_THROW(std::holds_alternative<Type>(stack.back()), std::logic_error);
  stack.pop_back();
}

void CompoundTypeTracker::endList() {
  pop<List>(containerStack_);
}
void CompoundTypeTracker::endSet() {
  pop<Set>(containerStack_);
}
void CompoundTypeTracker::endMap() {
  pop<Map>(containerStack_);
}
void CompoundTypeTracker::endStruct() {
  pop<Struct>(containerStack_);
}

bool CompoundTypeTracker::inMap() const {
  return std::holds_alternative<Map>(containerStack_.back());
}

const CompoundTypeTracker::MapState* CompoundTypeTracker::mapState() const {
  return std::get_if<Map>(&containerStack_.back());
}

void CompoundTypeTracker::toggleExpectingKeyValue() {
  if (auto* map = std::get_if<Map>(&containerStack_.back())) {
    map->expectingToken = (map->expectingToken == MapToken::Key)
        ? MapToken::Value
        : MapToken::Key;
  }
}

bool CompoundTypeTracker::inObjectMap() const {
  auto* map = mapState();
  return map != nullptr && map->encodedAs == MapForm::Object;
}

bool CompoundTypeTracker::inObjectMapExpectingKey() const {
  auto* map = mapState();
  return map != nullptr && map->encodedAs == MapForm::Object &&
      map->expectingToken == MapToken::Key;
}

} // namespace apache::thrift::json5::detail
