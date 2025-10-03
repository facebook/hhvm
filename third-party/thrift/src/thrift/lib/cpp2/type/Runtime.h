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

namespace apache::thrift::type {

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

  template <typename Tag, typename T>
  ConstRef(Tag, T&& val) : Base(op::detail::getAnyType<Tag, T>(), &val) {}

  template <typename IdT>
  ConstRef operator[](IdT&& id) const {
    return at(std::forward<IdT>(id));
  }

 private:
  friend Base;
  template <typename, typename, typename>
  friend class detail::BaseDyn;
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
  template <ThriftTypeTag Tag>
  static Value create() {
    return Value{Tag{}, nullptr};
  }
  template <typename T>
    requires(!ThriftTypeTag<T>)
  static Value create() {
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

  ConstRef asRef() const { return ref_; }

 protected:
  Ref ref_;

  BaseDynView(detail::Ptr ptr, BaseType baseType) : ref_(ptr) {
    checkBaseType(ref_, baseType);
  }

  ~BaseDynView() = default;

  static void checkBaseType(const Dyn& val, BaseType baseType) {
    if (val.type().baseType() != baseType) {
      folly::throw_exception<std::bad_any_cast>();
    }
  }
};

class BaseDynKeyView : public BaseDynView {
  using Base = BaseDynView;

 public:
  using key_type = ConstRef;

  // Returns the number of elements with key that compares equal to the
  // specified argument `key`, which is either 1 or 0 since this container does
  // not allow duplicates.
  FOLLY_NODISCARD size_t count(ConstRef key) const { return contains(key); }
  FOLLY_NODISCARD size_t count(const std::string& key) const {
    return contains(key);
  }

  // Checks if there is an element with key equivalent to `key` in the
  // container.
  FOLLY_NODISCARD bool contains(ConstRef key) const {
    return ref_.get(key).has_value();
  }
  FOLLY_NODISCARD bool contains(const std::string& key) const {
    return ref_.get(key).has_value();
  }

 protected:
  using Base::Base;
  ~BaseDynKeyView() = default;
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
  DynList& operator=(ConstRef other) { return operator=(other.asList()); }
  template <typename RefT>
  DynList& operator=(DynList<RefT> other) {
    return (ref_.assign(other.asRef()), *this);
  }

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

  // Erases all elements from the container.
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

  Ref asRef() { return ref_; }
  using Base::asRef;
};

// The constant portions of an unordered c++ set.
template <>
class DynSet<ConstRef> : public detail::BaseDynKeyView {
  using Base = detail::BaseDynKeyView;

 public:
  // Finds an element with key equivalent to `key`.
  [[noreturn]] Base::const_iterator find(ConstRef) const {
    detail::BaseErasedOp::unimplemented();
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

  DynSet& operator=(ConstRef other) { return operator=(other.asSet()); }
  template <typename RefT>
  DynSet& operator=(DynSet<RefT> other) {
    return (ref_.assign(other.asRef()), *this);
  }

  // Erases all elements from the container.
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
  template <class T = ConstRef>
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

// The constant portions of an unordered c++ map.
template <>
class DynMap<ConstRef> : public detail::BaseDynKeyView {
  using Base = detail::BaseDynKeyView;

 public:
  using mapped_type = ConstRef;
  using value_type = std::pair<ConstRef, Ref>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const pointer;
  using const_iterator = detail::MapIter<ConstRef, Ref>;
  using iterator = const_iterator;

  explicit DynMap(detail::Ptr ptr) : Base(ptr, BaseType::Map) {}

  // Returns an iterator to the first element of the container.
  const_iterator begin() const { return cbegin(); }
  const_iterator cbegin() const {
    return const_iterator{ref_.detail::Dyn::keys()};
  }

  // Returns an iterator to the last element of the container.
  const_iterator end() const { return cend(); }
  const_iterator cend() const { return const_iterator{}; }

  // Returns a reference to the mapped value of the element with key
  // equivalent to `key`. If no such element exists, an exception of type
  // std::out_of_range is thrown.
  ConstRef at(ConstRef key) const { return ref_.at(key); }
  ConstRef at(const std::string& key) const { return ref_.at(key); }
  ConstRef operator[](ConstRef key) const { return ref_.at(key); }
  ConstRef operator[](const std::string& key) const { return ref_.at(key); }
};

// The mutable portions of an unordered c++ map.
//
// TODO(afuller): Add type-erased iterator features, needed for full API.
template <>
class DynMap<Ref> : public DynMap<ConstRef> {
  using Base = DynMap<ConstRef>;

 public:
  using Base::Base;
  using mapped_type = Ref;
  using iterator = detail::MapIter<ConstRef, Ref, true>;

  DynMap& operator=(ConstRef other) { return operator=(other.asMap()); }
  template <typename RefT>
  DynMap& operator=(DynMap<RefT> other) {
    return (ref_.assign(other.asRef()), *this);
  }

  // Erases all elements from the container.
  void clear() { ref_.clear(); }

  // Returns an iterator to the first element of the container.
  iterator begin() { return iterator{ref_.detail::Dyn::keys()}; }
  using Base::begin;

  // Returns an iterator to the last element of the container.
  iterator end() { return iterator{}; }
  using Base::end;

  // Inserts `value`.
  [[noreturn]] std::pair<iterator, bool> insert(const value_type&) {
    detail::BaseErasedOp::unimplemented();
  }

  // Inserts `value`, using `hint` as a non-binding suggestion to where the
  // search should start.
  iterator insert(const_iterator, const value_type& value) {
    return insert(value).first;
  }

  // Inserts elements from range `[first, last)`.
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      ref_.ensure(first->first, first->second);
    }
  }

  // Inserts elements from initializer list `ilist`.
  template <class T = std::pair<ConstRef, ConstRef>>
  void insert(std::initializer_list<T> ilist) {
    insert(ilist.begin(), ilist.end());
  }

  // If a key equivalent to `key` already exists in the container, assigns
  // `val` to the corresponding mapped_type. If the `key` does not exist,
  // inserts the new value as if by `insert`.
  [[noreturn]] std::pair<iterator, bool> insert_or_assign(ConstRef, ConstRef) {
    detail::BaseErasedOp::unimplemented();
  }
  iterator insert_or_assign(const_iterator, ConstRef key, ConstRef val) {
    return insert_or_assign(key, val).first;
  }

  // Inserts a new element into the container constructed with the
  // given key-value pair, if there is no element with the key in the
  // container.
  [[noreturn]] std::pair<iterator, bool> emplace(ConstRef, ConstRef) {
    detail::BaseErasedOp::unimplemented();
  }
  iterator emplace(const_iterator, ConstRef key, ConstRef value) {
    return emplace(key, value).first;
  }

  // Inserts a new element into the container with key `key` and `value`, if
  // there is no element with the key in the container.
  std::pair<iterator, bool> try_emplace(ConstRef key, ConstRef value) {
    return emplace(key, value);
  }
  iterator try_emplace(const_iterator hint, ConstRef key, ConstRef value) {
    return emplace(hint, key, value);
  }

  // Removes the element at `pos`.
  [[noreturn]] iterator erase(const_iterator) {
    detail::BaseErasedOp::unimplemented();
  }

  // Removes the elements in the range `[first; last)`, which must be a valid
  // range in `*this`.
  [[noreturn]] iterator erase(const_iterator, const_iterator) {
    detail::BaseErasedOp::unimplemented();
  }

  // Removes the element (if one exists) with the key equivalent to `key`.
  size_type erase(ConstRef key) { return ref_.remove(key); }
  size_type erase(const std::string& key) { return ref_.remove(key); }

  // Returns a reference to the mapped value of the element with key
  // equivalent to `key`. If no such element exists, an exception of type
  // std::out_of_range is thrown.
  Ref at(ConstRef key) { return ref_.at(key); }
  Ref at(const std::string& key) { return ref_.at(key); }
  using Base::at;
  Ref operator[](ConstRef key) { return ref_.ensure(key); }
  Ref operator[](const std::string& key) { return ref_.ensure(key); }
  using Base::operator[];

  Ref asRef() { return ref_; }
  using Base::asRef;
};

} // namespace apache::thrift::type
