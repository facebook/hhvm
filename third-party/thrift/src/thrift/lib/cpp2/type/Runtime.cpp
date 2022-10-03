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

#include <thrift/lib/cpp2/type/Runtime.h>

namespace apache::thrift::type {
namespace detail {
namespace {
Ptr checkFound(Ptr result) {
  if (!result.has_value()) {
    folly::throw_exception<std::out_of_range>("not found");
  }
  return result;
}
} // namespace

Ptr TypeInfo::get(void* ptr, FieldId id) const {
  return get_(ptr, id, std::string::npos, nullPtr());
}
Ptr TypeInfo::get(void* ptr, size_t pos) const {
  return get_(ptr, {}, pos, nullPtr());
}
Ptr TypeInfo::get(void* ptr, const Dyn& val) const {
  return get_(ptr, {}, std::string::npos, val);
}

bool TypeInfo::put(void* ptr, FieldId id, const Dyn& val) const {
  return put_(ptr, id, std::string::npos, nullPtr(), val);
}
bool TypeInfo::put(void* ptr, const Dyn& key, const Dyn& val) const {
  return put_(ptr, {}, std::string::npos, key, val);
}
void TypeInfo::insert(void* ptr, size_t pos, const Dyn& val) const {
  put_(ptr, {}, pos, nullPtr(), val);
}
bool TypeInfo::remove(void* ptr, size_t pos) const {
  return put_(ptr, {}, pos, nullPtr(), nullPtr());
}
bool TypeInfo::remove(void* ptr, const Dyn& key) const {
  return put_(ptr, {}, std::string::npos, key, nullPtr());
}
bool TypeInfo::remove(void* ptr, FieldId id) const {
  return put_(ptr, id, std::string::npos, nullPtr(), nullPtr());
}

Ptr TypeInfo::ensure(void* ptr, FieldId id) const {
  return ensure(ptr, id, nullPtr());
}
Ptr TypeInfo::ensure(void* ptr, FieldId id, const Dyn& defVal) const {
  return ensure_(ptr, id, nullPtr(), defVal);
}
Ptr TypeInfo::ensure(void* ptr, const Dyn& key) const {
  return ensure(ptr, key, nullPtr());
}
Ptr TypeInfo::ensure(void* ptr, const Dyn& key, const Dyn& defVal) const {
  return ensure_(ptr, {}, key, defVal);
}

Ptr Cursor::next() {
  return type_->next(ptr_, iterType_, itr_);
}

bool Dyn::contains(const Dyn& key) const {
  return get(key).has_value();
}

Ptr Dyn::ensure(const Dyn& key) const {
  return type_.mut().ensure(ptr_, key);
}
Ptr Dyn::ensure(const Dyn& key, const Dyn& val) const {
  return type_.mut().ensure(ptr_, key, val);
}
Ptr Dyn::ensure(FieldId id) const {
  return type_.mut().ensure(ptr_, id);
}
Ptr Dyn::ensure(FieldId id, const Dyn& val) const {
  return type_.mut().ensure(ptr_, id, val);
}

Ptr Dyn::get(const Dyn& key) const {
  return type_->get(ptr_, key);
}
Ptr Dyn::get(FieldId id) const {
  return type_->get(ptr_, id);
}
Ptr Dyn::get(size_t pos) const {
  return type_->get(ptr_, pos);
}
Ptr Dyn::get(const Dyn& key, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, key);
}
Ptr Dyn::get(FieldId id, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, id);
}
Ptr Dyn::get(size_t pos, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, pos);
}

Ptr Dyn::at(const Dyn& key) const {
  return checkFound(get(key));
}
Ptr Dyn::at(size_t pos) const {
  return checkFound(get(pos));
}
Ptr Dyn::at(const Dyn& key, bool ctxConst, bool ctxRvalue) const {
  return checkFound(get(key, ctxConst, ctxRvalue));
}

Ptr Dyn::at(size_t pos, bool ctxConst, bool ctxRvalue) const {
  return checkFound(get(pos, ctxConst, ctxRvalue));
}

Ptr Dyn::withContext(bool ctxConst, bool ctxRvalue) const noexcept {
  return {type_.withContext(ctxConst, ctxRvalue), ptr_};
}

} // namespace detail

void Value::reset() {
  if (ptr_ != nullptr) {
    type_->delete_(ptr_);
    Base::reset();
  }
}

Value& Value::operator=(const Value& other) noexcept {
  reset();
  Base::reset(other.type_, other.type_->make(other.ptr_, false));
  return *this;
}

Value& Value::operator=(Value&& other) noexcept {
  reset();
  Base::reset(other.type_, other.ptr_);
  other.Base::reset();
  return *this;
}

} // namespace apache::thrift::type
