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

#include <cstddef>
#include <initializer_list>
#include <string>
#include <typeinfo>
#include <utility>

#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/op/detail/AnyOp.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/cpp2/type/detail/Runtime.h>

namespace apache {
namespace thrift {
namespace type {

// A light weight (pass-by-value), non-owning *const* reference to a runtime
// Thrift value.
//
// Should typically be passed by value as it only holds two
// ponters; a pointer to the value being reference and a pointer to the static
// runtime metadata associated with the type of the value.
class ConstRef final : public detail::BaseRef<ConstRef> {
  using Base = detail::BaseRef<ConstRef>;

 public:
  using Base::get;

  ConstRef() noexcept = default;
  // 'capture' any other runtime type.
  /* implicit */ ConstRef(const detail::Dyn& other) noexcept : Base(other) {}
  // 'capture' any native type we can safely infer the tag for.
  template <typename T, typename Tag = infer_tag<T>>
  /* implicit */ ConstRef(T&& val) noexcept
      : ConstRef(Tag{}, std::forward<T>(val)) {}
  // 'capture' a std::string.
  // TODO: infer_tag a string_view type instead.
  /* implicit */ ConstRef(const std::string& val) noexcept
      : ConstRef(binary_t{}, val) {}

  template <typename IdT>
  ConstRef operator[](IdT&& id) const {
    return at(std::forward<IdT>(id));
  }

 private:
  friend Base;
  template <typename, typename, typename>
  friend class detail::BaseDyn;

  template <typename Tag, typename T>
  ConstRef(Tag, T&& val) : Base(op::detail::getAnyType<Tag, T>(), &val) {}
};

// A light weight (pass-by-value), non-owning reference to a runtime Thrift
// value.
//
// Should typically be passed by value as it only holds two
// ponters; a pointer to the value being reference and a pointer to the static
// runtime metadata associated with the type of the value.
class Ref final : private detail::DynCmp<Ref, ConstRef>,
                  public detail::BaseRef<Ref, ConstRef> {
  using Base = detail::BaseRef<Ref, ConstRef>;
  using Base::if_not_index;

 public:
  Ref() noexcept = default;

  // Sets the referenced value to it's intrinsic default (e.g. ignoring custom
  // field defaults).
  using Base::clear;

  // Assign to the given value.
  using Base::assign;
  using Base::operator=;

  // Prepend to a list, string, etc.
  using Base::prepend;
  // Append to a list, string, etc.
  using Base::append;

  // Add to a set, number, etc.
  using Base::add;
  using Base::operator+=;
  using Base::operator++;

  // Put a key-value pair, overwriting any existing entry in a map, struct, etc.
  //
  // Returns true if an existing value was replaced.
  using Base::put;

  // Insert a list element.
  using Base::insert;

  // Remove a list element or set key.
  //
  // Returns true if an existing value was removed.
  using Base::remove;

  // Add a key-value pair, if not already present, using the given default if
  // provided.
  //
  // Returns a reference to the associated value.
  using Base::ensure;

  // Throws on mismatch or if const.
  template <typename Tag>
  native_type<Tag>& mut() {
    return Base::mut<Tag>();
  }

  // Returns nullptr on mismatch or if const.
  template <typename Tag>
  native_type<Tag>* tryMut() noexcept {
    return Base::tryMut<Tag>();
  }

  using Base::at;
  using Base::get;
  Ref operator[](Ordinal ord) & { return at(ord); }
  Ref operator[](Ordinal ord) && { return at(ord); }
  Ref operator[](size_t pos) & { return at(pos); }
  Ref operator[](size_t pos) && { return at(pos); }
  ConstRef operator[](Ordinal ord) const& { return at(ord); }
  ConstRef operator[](Ordinal ord) const&& { return at(ord); }
  ConstRef operator[](size_t pos) const& { return at(pos); }
  ConstRef operator[](size_t pos) const&& { return at(pos); }
  template <typename IdT>
  if_not_index<IdT, Ref> operator[](IdT&& id) & {
    return ensure(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT, Ref> operator[](IdT&& id) && {
    return ensure(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT> operator[](IdT&& id) const& {
    return at(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT> operator[](IdT&& id) const&& {
    return at(std::forward<IdT>(id));
  }

  explicit Ref(detail::Ptr data) noexcept : Base(data) {}
  // Enable `const Ref&` to implicitly capture ConstRef&&
  /*implicit*/ Ref(ConstRef&& ref) noexcept : Base(ref) {}

 private:
  friend class detail::Ptr;
  friend Base;
  using Base::asRef;

  template <typename Tag, typename T>
  Ref(Tag, T&& val) : Base(op::detail::getAnyType<Tag>(), &val) {}
};

namespace detail {
inline Ref Ptr::operator*() const noexcept {
  return Ref{*this};
}
} // namespace detail

// A runtime Thrift value that owns it's own memory.
//
// TODO(afuller): Store small values in-situ.
class Value : private detail::DynCmp<Value, ConstRef>,
              private detail::DynCmp<Value, Ref>,
              public detail::BaseDyn<ConstRef, Ref, Value> {
  using Base = detail::BaseDyn<ConstRef, Ref, Value>;
  using Dyn = detail::Dyn;

 public:
  template <typename Tag>
  static if_thrift_type_tag<Tag, Value> create() {
    return Value{Tag{}, nullptr};
  }
  template <typename T>
  static if_not_thrift_type_tag<T, Value> create() {
    return create<infer_tag<T>>();
  }
  template <typename Tag>
  static Value of(const native_type<Tag>& val) {
    return {Tag{}, val};
  }
  template <typename Tag>
  static Value of(native_type<Tag>&& val) {
    return {Tag{}, std::move(val)};
  }
  template <typename Tag>
  static Value of(std::unique_ptr<native_type<Tag>> val) {
    if (val == nullptr) {
      return {};
    }
    return Value{Tag{}, std::move(val)};
  }
  template <typename U>
  static Value of(U&& val) {
    return of<infer_tag<U>>(std::forward<U>(val));
  }
  template <typename U>
  static Value of(std::unique_ptr<U> val) {
    return of<infer_tag<U>>(std::move(val));
  }

  Value() noexcept = default; // A void/null value.
  Value(const Value& other) noexcept
      : Base(other.type_, other.type_->make(other.ptr_, false)) {}

  Value(Value&& other) noexcept : Base(other.type_, other.ptr_) {
    other.Base::reset();
  }
  ~Value() { reset(); }

  void reset();

  // Sets the referenced value to it's intrinsic default (e.g. ignoring custom
  // field defaults).
  using Base::clear;

  // Assign to the given value.
  using Base::assign;
  using Base::operator=;
  Value& operator=(const Value& other) noexcept;
  Value& operator=(Value&& other) noexcept;

  // Prepend to a list, string, etc.
  using Base::prepend;
  // Append to a list, string, etc.
  using Base::append;

  // Add to a set, number, etc.
  using Base::add;
  using Base::operator+=;
  using Base::operator++;

  // Put a key-value pair, overwriting any existing entry in a map, struct, etc.
  //
  // Returns true if an existing value was replaced.
  using Base::put;

  // Insert a list element.
  using Base::insert;

  // Remove a list element or set key.
  //
  // Returns true if an existing value was removed.
  using Base::remove;

  // Add a key-value pair, if not already present, using the given default if
  // provided.
  //
  // Returns a reference to the associated value.
  using Base::ensure;

  // Throwing type-safe casting functions.
  using Base::as;
  template <typename Tag>
  native_type<Tag>& as() & {
    return type_->as<native_type<Tag>>(ptr_);
  }
  template <typename T>
  if_not_thrift_type_tag<T, T&> as() & {
    return as<infer_tag<T>>();
  }
  template <typename Tag>
  native_type<Tag>&& as() && {
    return std::move(type_->as<native_type<Tag>>(ptr_));
  }
  template <typename T>
  if_not_thrift_type_tag<T, T&&> as() && {
    return as<infer_tag<T>>();
  }

  // Non-throwing type-safe casting functions.
  using Base::tryAs;
  template <typename Tag>
  native_type<Tag>* tryAs() noexcept {
    return type_->tryAs<native_type<Tag>>(ptr_);
  }
  template <typename T>
  if_not_thrift_type_tag<T, T*> tryAs() noexcept {
    return tryAs<infer_tag<T>>();
  }

  using Base::at;
  using Base::get;
  Ref operator[](Ordinal ord) & { return at(ord); }
  Ref operator[](Ordinal ord) && { return at(ord); }
  Ref operator[](size_t pos) & { return at(pos); }
  Ref operator[](size_t pos) && { return at(pos); }
  ConstRef operator[](Ordinal ord) const& { return at(ord); }
  ConstRef operator[](Ordinal ord) const&& { return at(ord); }
  ConstRef operator[](size_t pos) const& { return at(pos); }
  ConstRef operator[](size_t pos) const&& { return at(pos); }
  template <typename IdT>
  if_not_index<IdT, Ref> operator[](IdT&& id) & {
    return ensure(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT, Ref> operator[](IdT&& id) && {
    return ensure(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT> operator[](IdT&& id) const& {
    return get(std::forward<IdT>(id));
  }
  template <typename IdT>
  if_not_index<IdT> operator[](IdT&& id) const&& {
    return get(std::forward<IdT>(id));
  }

 private:
  using Base::Base;

  template <typename Tag, typename T = native_type<Tag>>
  explicit Value(Tag, std::nullptr_t)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(nullptr, false)) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, std::unique_ptr<T> val)
      : Base(op::detail::getAnyType<Tag, T>(), val.release()) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, const T& val)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(&val, false)) {
    assert(ptr_ != nullptr);
  }
  template <typename Tag, typename T>
  Value(Tag, T&& val)
      : Base(
            op::detail::getAnyType<Tag, T>(),
            op::detail::getAnyType<Tag, T>()->make(&val, true)) {
    assert(ptr_ != nullptr);
  }
};

namespace detail {
class BaseDynView {
 public:
  using value_type = ConstRef;
  using size_type = size_t;
  using reference = ConstRef;
  using const_reference = ConstRef;
  using pointer = ConstRef*;
  using const_pointer = ConstRef*;
  using iterator = Ref::const_iterator;
  using const_iterator = Ref::const_iterator;

  // Returns an iterator to the first element of the container.
  const_iterator begin() const { return ref_.cbegin(); }
  const_iterator cbegin() const { return ref_.cbegin(); }

  // Returns an iterator to the last element of the container.
  const_iterator end() const { return ref_.cend(); }
  const_iterator cend() const { return ref_.cend(); }

  // Checks if the container has no elements.
  FOLLY_NODISCARD bool empty() const { return ref_.empty(); }

  // Returns the number of elements in the container.
  FOLLY_NODISCARD size_t size() const { return ref_.size(); }

 protected:
  Ref ref_;

  BaseDynView(detail::Ptr ptr, BaseType baseType) : ref_(ptr) {
    if (ref_.type().baseType() != baseType) {
      folly::throw_exception<std::bad_any_cast>();
    }
  }

  ~BaseDynView() = default;
};

} // namespace detail

// The constant portions of a c++ 'SequenceContainer'.
//
// See: https://en.cppreference.com/w/cpp/named_req/SequenceContainer
template <>
class DynList<ConstRef> : public detail::BaseDynView {
  using Base = detail::BaseDynView;

 public:
  // Returns a reference to the element at specified location pos, with bounds
  // checking.
  ConstRef at(size_t pos) const { return ref_.at(pos); }
  ConstRef operator[](size_t pos) const { return at(pos); }

  // Returns a reference to the first element in the container.
  ConstRef front() const { return at(0); }

  // Returns a reference to the last element in the container.
  ConstRef back() const { return at(ref_.size() - 1); }

  explicit DynList(detail::Ptr ptr) : Base(ptr, BaseType::List) {}
};

// The mutable portions of a c++ 'SequenceContainer'.
//
// See: https://en.cppreference.com/w/cpp/named_req/SequenceContainer
//
// TODO(afuller): Add type-erased iterator features, needed for full API.
template <>
class DynList<Ref> : public DynList<ConstRef> {
  using Base = DynList<ConstRef>;

 public:
  using Base::Base;
  using value_type = Ref;
  using reference = Ref;
  using pointer = Ref*;
  using iterator = Ref::iterator;

  // Replaces the contents with count copies of value value
  [[noreturn]] void assign(size_type, ConstRef) {
    detail::BaseErasedOp::unimplemented();
  }
  [[noreturn]] void assign(ConstRef) { detail::BaseErasedOp::unimplemented(); }
  DynList& operator=(ConstRef other) { return (assign(other), *this); }

  // Returns a reference to the element at specified location pos, with bounds
  // checking.
  Ref at(size_type pos) { return ref_.at(pos); }
  using Base::at;
  Ref operator[](size_type pos) { return at(pos); }
  using Base::operator[];

  // Returns a reference to the first element in the container.
  Ref front() { return at(0); }
  using Base::front;

  // Returns a reference to the last element in the container.
  Ref back() { return at(ref_.size() - 1); }
  using Base::back;

  // Returns an iterator to the first element of the container.
  iterator begin() { return ref_.begin(); }
  using Base::begin;

  // Returns an iterator to the last element of the container.
  iterator end() { return ref_.end(); }
  using Base::end;

  // Erases all elements from the container. After this call, size() returns
  // zero.
  void clear() { ref_.clear(); }

  // Inserts `value` before `pos`.
  [[noreturn]] iterator insert(const_iterator, ConstRef) {
    detail::BaseErasedOp::unimplemented(); // TODO(afuller): Implement.
  }
  // Removes the element at `pos`.
  [[noreturn]] iterator erase(const_iterator) {
    detail::BaseErasedOp::unimplemented(); // TODO(afuller): Implement.
  }
  // Removes the elements in the range `[first, last)`.
  [[noreturn]] iterator erase(const_iterator, const_iterator) {
    detail::BaseErasedOp::unimplemented(); // TODO(afuller): Implement.
  }

  // Appends the given element `value` to the end of the container.
  void push_back(ConstRef value) { ref_.insert(size(), value); }
  void push_back(const std::string& value) { ref_.insert(size(), value); }

  // Prepent the given element `value` to the beginning of the container.
  void push_front(ConstRef value) { ref_.insert(0, value); }
  void push_front(const std::string& value) { ref_.insert(0, value); }

  // Removes the last element of the container.
  void pop_back() { ref_.remove(std::max<size_type>(1, size()) - 1); }
  // Removes the first element of the container.
  void pop_front() { ref_.remove(0); }
};

// The constant portions of an unordered c++ set.
template <>
class DynSet<ConstRef> : public detail::BaseDynView {
  using Base = detail::BaseDynView;

 public:
  using key_type = ConstRef;

  // Returns the number of elements with key that compares equal to the
  // specified argument `key`, which is either 1 or 0 since this container does
  // not allow duplicates.
  FOLLY_NODISCARD size_t count(ConstRef key) const { return contains(key); }
  FOLLY_NODISCARD size_t count(const std::string& key) const {
    return contains(key);
  }

  // Finds an element with key equivalent to `key`.
  [[noreturn]] Base::const_iterator find(ConstRef) const {
    detail::BaseErasedOp::unimplemented();
  }

  // Checks if there is an element with key equivalent to `key` in the
  // container.
  FOLLY_NODISCARD bool contains(ConstRef key) const {
    return !ref_.get(key).type().empty();
  }
  FOLLY_NODISCARD bool contains(const std::string& key) const {
    return !ref_.get(key).type().empty();
  }

  explicit DynSet(detail::Ptr ptr) : Base(ptr, BaseType::Set) {}
};

// The mutable portions of an unordered c++ set.
//
// TODO(afuller): Add type-erased iterator features, needed for full API.
template <>
class DynSet<Ref> : public DynSet<ConstRef> {
  using Base = DynSet<ConstRef>;

 public:
  using Base::Base;

  [[noreturn]] void assign(ConstRef) { detail::BaseErasedOp::unimplemented(); }
  DynSet& operator=(ConstRef other) { return (assign(other), *this); }

  // Erases all elements from the container. After this call, size() returns
  // zero.
  void clear() { ref_.clear(); }

  // Inserts value.
  [[noreturn]] std::pair<iterator, bool> insert(ConstRef) {
    detail::BaseErasedOp::unimplemented();
  }

  // Inserts value, using hint as a non-binding suggestion to where the search
  // should start.
  iterator insert(const_iterator, ConstRef value) {
    // TODO(afuller): consider passing through hint.
    return insert(value).first;
  }

  // Inserts elements from range `[first, last)`.
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      ref_.add(*first);
    }
  }

  // Inserts elements from initializer list `ilist`.
  template <class T>
  void insert(std::initializer_list<T> ilist) {
    insert(ilist.begin(), ilist.end());
  }

  // Removes the element (if one exists) with the key equivalent to `key`.
  size_t erase(ConstRef key) { return ref_.remove(key); }
  size_t erase(const std::string& key) { return ref_.remove(key); }

  // Removes the element at `pos`.
  [[noreturn]] iterator erase(const_iterator) {
    detail::BaseErasedOp::unimplemented(); // TODO(afuller): Implement.
  }
  // Removes the elements in the range `[first, last)`.
  [[noreturn]] iterator erase(const_iterator, const_iterator) {
    detail::BaseErasedOp::unimplemented(); // TODO(afuller): Implement.
  }

  // Finds an element with key equivalent to `key`.
  [[noreturn]] iterator find(ConstRef) {
    detail::BaseErasedOp::unimplemented();
  }
  using Base::find;
};

} // namespace type
} // namespace thrift
} // namespace apache
