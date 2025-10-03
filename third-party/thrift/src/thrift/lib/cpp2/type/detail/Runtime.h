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
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/type/AlignedPtr.h>
#include <thrift/lib/cpp2/type/Id.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/detail/TypeInfo.h>

namespace apache::thrift::type {
class Ref;
template <typename RefT>
class DynList;
template <typename RefT>
class DynSet;
template <typename RefT>
class DynMap;
namespace detail {

const TypeInfo& voidTypeInfo();

// The type information associated with a runtime Thrift value.
//
// C++ only tracks const- and rvalue-ness at compiletime, so type-erased value
// wrappers must track it at runtime.
//
// This class only stores a single AlignedPtr, so should be passed by value.
class RuntimeType {
 public:
  template <typename U>
  static RuntimeType create(const TypeInfo& info) noexcept {
    return {
        info,
        std::is_const_v<std::remove_reference_t<U>>,
        std::is_rvalue_reference_v<U>};
  }

  RuntimeType() noexcept {}
  explicit RuntimeType(const TypeInfo& info) noexcept : info_(&info) {}
  RuntimeType(
      const TypeInfo& info, bool isConst, bool isRvalue = false) noexcept
      : info_(&info, (isConst << kConst) | (isRvalue << kRvalue)) {}

  bool isConst() const noexcept { return info_.template get<kConst>(); }
  bool isRvalue() const noexcept { return info_.template get<kRvalue>(); }
  const TypeInfo& info() const noexcept { return *info_.get(); }

  // Throws if const.
  const TypeInfo& mut() const { return (ensureMut(), info()); }

  // Returns the appropriate runtime type, based on the given context.
  //
  // The runtime type is const, if either the context or type is const.
  // The runtime type is r-value, if both the context and the type are r-value.
  RuntimeType withContext(bool ctxConst, bool ctxRvalue = false) const {
    return {info(), isConst() || ctxConst, isRvalue() && ctxRvalue};
  }

  const TypeInfo* operator->() const noexcept { return &info(); }

 private:
  // Stash the runtime qualifer information in the TypeInfo pointer, as
  // we know it has sufficent alignment.
  enum Qualifier { kConst, kRvalue, kQualSize };
  AlignedPtr<const TypeInfo, kQualSize> info_ = &voidTypeInfo();

  void ensureMut() const {
    if (isConst()) {
      folly::throw_exception<std::logic_error>("cannot modify a const ref");
    }
  }
};

// TODO(afuller): Benchmark and optimize.
class Cursor {
 public:
  Cursor() noexcept = default;

  // Returns the next value, or nullPtr() if there are no more values.
  Ptr next();

 private:
  friend class Dyn;
  template <typename, typename, typename>
  friend class BaseMapIter;

  RuntimeType type_;
  void* ptr_ = nullptr;
  IterType iterType_;
  std::any itr_;

  Cursor(RuntimeType type, void* ptr, IterType iterType = IterType::Default)
      : type_(type), ptr_(ptr), iterType_(iterType) {}
};

// A base class for qualifier-preserving type-erased runtime-typed dynamic
// value.
class Dyn {
 public:
  Dyn() noexcept = default;
  Dyn(RuntimeType type, void* ptr) noexcept : type_(type), ptr_(ptr) {}
  Dyn(RuntimeType type, const void* ptr) noexcept
      : type_(type.withContext(true)), ptr_(const_cast<void*>(ptr)) {}

  // Type accessors.
  const Type& type() const noexcept { return type_->thriftType; }
  const std::type_info& typeId() const noexcept { return type_->cppType; }

  // Throws on mismatch.
  template <ThriftTypeTag Tag>
  const native_type<Tag>& as() const {
    // TODO(afuller): Check thrift types match.
    return type_->as<native_type<Tag>>(ptr_);
  }
  template <typename T>
    requires(!ThriftTypeTag<T>)
  const T& as() const {
    return as<infer_tag<T>>();
  }

  // Returns nullptr on mismatch.
  template <ThriftTypeTag Tag>
  FOLLY_NODISCARD const native_type<Tag>* tryAs() const noexcept {
    // TODO(afuller): Check thrift types match.
    return type_->tryAs<native_type<Tag>>(ptr_);
  }
  template <typename T>
    requires(!ThriftTypeTag<T>)
  const T* tryAs() const {
    return tryAs<infer_tag<T>>();
  }

  FOLLY_NODISCARD bool empty() const { return type_->empty(ptr_); }
  FOLLY_NODISCARD bool identical(const Dyn& rhs) const {
    return type() == rhs.type() && type_->identical(ptr_, rhs);
  }
  FOLLY_NODISCARD bool contains(const Dyn& key) const;

  FOLLY_NODISCARD bool equal(const Dyn& rhs) const {
    return type_->equal(ptr_, rhs);
  }
  FOLLY_NODISCARD folly::ordering compare(const Dyn& rhs) const {
    return type_->compare(ptr_, rhs);
  }
  // TODO(dokwon): Only use op::isEmpty after migrating TypeStruct to terse
  // write.
  FOLLY_NODISCARD bool has_value() const {
    return !type().empty() && type().toThrift() != Type::underlying_type{};
  }

 protected:
  // TODO(afuller): Simplify friends model.
  friend class Cursor;
  template <typename, typename>
  friend class Iter;
  template <typename, typename>
  friend class BaseIter;
  template <typename, typename, typename>
  friend class BaseMapIter;
  template <typename>
  friend class type::DynMap;

  RuntimeType type_;
  void* ptr_ = nullptr;

  ~Dyn() = default; // Abstract base class;

  // Throws on mismatch or if const.
  template <typename Tag>
  native_type<Tag>& mut() const {
    return type_.mut().as<native_type<Tag>>(ptr_);
  }

  // Returns nullptr on mismatch or if const.
  template <typename Tag>
  native_type<Tag>* tryMut() const noexcept {
    return type_.isConst() ? nullptr : type_->tryAs<native_type<Tag>>(ptr_);
  }

  void clear() const { type_.mut().clear(ptr_); }
  void assign(const Dyn& val) const { type_.mut().assign(ptr_, val); }
  void prepend(const Dyn& val) const { type_.mut().prepend(ptr_, val); }
  void append(const Dyn& val) const { type_.mut().append(ptr_, val); }
  bool add(const Dyn& val) const { return type_.mut().add(ptr_, val); }
  bool put(const Dyn& key, const Dyn& val) const {
    return type_.mut().put(ptr_, key, val);
  }
  bool put(FieldId id, const Dyn& val) const {
    return type_.mut().put(ptr_, id, val);
  }
  void insert(size_t pos, const Dyn& val) const {
    type_.mut().insert(ptr_, pos, val);
  }

  bool remove(const Dyn& val) const { return type_.mut().remove(ptr_, val); }
  bool remove(FieldId id) const { return type_.mut().remove(ptr_, id); }
  void remove(size_t pos) const { type_.mut().remove(ptr_, pos); }

  Ptr ensure(const Dyn& key) const;
  Ptr ensure(const Dyn& key, const Dyn& val) const;
  Ptr ensure(FieldId id) const;
  Ptr ensure(FieldId id, const Dyn& val) const;

  // TODO(afuller): Make context an enum, instead of pair of bools.
  Ptr get(const Dyn& key) const;
  Ptr get(FieldId id) const;
  Ptr get(size_t pos) const;
  Ptr get(const Dyn& key, bool ctxConst, bool ctxRvalue = false) const;
  Ptr get(FieldId id, bool ctxConst, bool ctxRvalue = false) const;
  Ptr get(size_t pos, bool ctxConst, bool ctxRvalue = false) const;

  // Like get, but throws when not found.
  Ptr at(const Dyn& key) const;
  Ptr at(size_t pos) const;
  Ptr at(const Dyn& key, bool ctxConst, bool ctxRvalue = false) const;
  Ptr at(size_t pos, bool ctxConst, bool ctxRvalue = false) const;

  Cursor items() const { return {type_, ptr_}; }
  Cursor items(bool ctxConst, bool ctxRvalue = false) const {
    return {type_.withContext(ctxConst, ctxRvalue), ptr_};
  }
  Cursor keys() const { return {type_, ptr_, IterType::Key}; }
  Cursor keys(bool ctxConst, bool ctxRvalue = false) const {
    return {type_.withContext(ctxConst, ctxRvalue), ptr_, IterType::Key};
  }
  Cursor values() const { return {type_, ptr_, IterType::Value}; }
  Cursor values(bool ctxConst, bool ctxRvalue = false) const {
    return {type_.withContext(ctxConst, ctxRvalue), ptr_, IterType::Value};
  }

  void mergeContext(bool ctxConst, bool ctxRvalue = false) {
    type_ = type_.withContext(ctxConst, ctxRvalue);
  }

  void reset(const Dyn& other) noexcept { reset(other.type_, other.ptr_); }
  void reset(RuntimeType type, void* ptr) noexcept {
    type_ = type;
    ptr_ = ptr;
  }
  void reset(RuntimeType type, const void* ptr) noexcept {
    type_ = type.withContext(true);
    ptr_ = const_cast<void*>(ptr);
  }
  void reset() noexcept {
    type_ = {};
    ptr_ = {};
  }

  Ptr withContext(bool ctxConst, bool ctxRvalue = false) const noexcept;

 private:
  friend bool operator==(const Dyn& lhs, std::nullptr_t) {
    return !lhs.has_value();
  }
  friend bool operator==(std::nullptr_t, const Dyn& rhs) {
    return !rhs.has_value();
  }
  friend bool operator!=(const Dyn& lhs, std::nullptr_t) {
    return lhs.has_value();
  }
  friend bool operator!=(std::nullptr_t, const Dyn& rhs) {
    return rhs.has_value();
  }
};

// An un-owning pointer to a thrift value.
class Ptr final : public Dyn {
 public:
  using Dyn::Dyn;

  // Pointers do not share constness with the value, so expose functions
  // directly.
  using Dyn::add;
  using Dyn::append;
  using Dyn::assign;
  using Dyn::at;
  using Dyn::clear;
  using Dyn::ensure;
  using Dyn::get;
  using Dyn::insert;
  using Dyn::keys;
  using Dyn::mut;
  using Dyn::prepend;
  using Dyn::put;
  using Dyn::remove;
  using Dyn::tryMut;
  using Dyn::values;

  // Deref.
  Ref operator*() const noexcept;

 private:
  friend class Dyn;
};

inline Ptr nullPtr() {
  return {};
}

// A base struct provides helpers for throwing exceptions and default throwing
// implementations for type-specific ops.
struct BaseErasedOp {
  [[noreturn]] static void bad_op(const char* msg = "not supported") {
    folly::throw_exception<std::logic_error>(msg);
  }
  static void check_op(bool cond, const char* msg = "not supported") {
    if (!cond) {
      bad_op(msg);
    }
  }
  static void check_found(bool found, const char* msg = "not found") {
    if (!found) {
      folly::throw_exception<std::out_of_range>(msg);
    }
  }
  [[noreturn]] static void bad_type() {
    folly::throw_exception<std::bad_any_cast>();
  }
  [[noreturn]] static void unimplemented(const char* msg = "not implemented") {
    folly::throw_exception<std::runtime_error>(msg);
  }

  [[noreturn]] static void prepend(void*, const Dyn&) { bad_op(); }
  [[noreturn]] static void append(void*, const Dyn&) { bad_op(); }
  [[noreturn]] static bool add(void*, const Dyn&) { bad_op(); }
  [[noreturn]] static bool put(void*, FieldId, size_t, const Dyn&, const Dyn&) {
    bad_op();
  }
  [[noreturn]] static Ptr ensure(void*, FieldId, const Dyn&, const Dyn&) {
    bad_op();
  }
  [[noreturn]] static Ptr next(void*, IterType, std::any&) { bad_op(); }
  [[noreturn]] static Ptr get(void*, FieldId, size_t, const Dyn&) { bad_op(); }
  [[noreturn]] static size_t size(const void*) { bad_op(); }
};

template <typename L, typename R>
class BaseDynCmp {
 private:
  friend bool operator==(const L& lhs, const R& rhs) { return lhs.equal(rhs); }
  friend bool operator!=(const L& lhs, const R& rhs) { return !lhs.equal(rhs); }
  friend bool operator<(const L& lhs, const R& rhs) {
    return op::detail::is_lt(lhs.compare(rhs));
  }
  friend bool operator<=(const L& lhs, const R& rhs) {
    return op::detail::is_lteq(lhs.compare(rhs));
  }
  friend bool operator>(const L& lhs, const R& rhs) {
    return op::detail::is_gt(lhs.compare(rhs));
  }
  friend bool operator>=(const L& lhs, const R& rhs) {
    return op::detail::is_gteq(lhs.compare(rhs));
  }
};

template <typename T1, typename T2 = T1>
class DynCmp : public BaseDynCmp<T1, T2>, public BaseDynCmp<T2, T1> {};
template <typename T>
class DynCmp<T, T> : public BaseDynCmp<T, T> {};

// A wrapper for a Cursor that support the standard iterator interface.
template <typename RefT, typename Derived>
class BaseIter : public BaseDerived<Derived> {
 public:
  using value_type = RefT;
  using reference_type = RefT;
  using pointer_type = RefT*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  BaseIter() noexcept = default;
  BaseIter(const BaseIter& other) = default;
  BaseIter(BaseIter&& other) noexcept = default;
  explicit BaseIter(Cursor cur) : cur_(std::move(cur)), next_(cur_.next()) {}
  template <typename MutT, typename D>
  explicit BaseIter(BaseIter<MutT, D>&& other) noexcept
      : cur_(std::move(other.cur_)), next_(other.next_) {}

  pointer_type operator->() const { return &next_; }
  reference_type operator*() const { return next_; }
  Derived& operator++() { return (next_.reset(cur_.next()), derived()); }
  Derived operator++(int) {
    Derived result = derived();
    operator++();
    return result;
  }

  Derived& operator=(const BaseIter& other) {
    cur_ = other.cur_;
    next_.reset(other.next_);
    return derived();
  }
  Derived& operator=(BaseIter&& other) noexcept {
    cur_ = std::move(other.cur_);
    next_.reset(other.next_);
    return derived();
  }

 protected:
  using BaseDerived<Derived>::derived;
  Cursor cur_;
  RefT next_;

  ~BaseIter() = default;

 private:
  template <typename, typename>
  friend class BaseIter;
  template <typename, typename>
  friend class Iter;
  template <typename>
  friend class type::DynMap;

  friend bool operator==(const BaseIter& lhs, const BaseIter& rhs) noexcept {
    return lhs.next_.ptr_ == rhs.next_.ptr_;
  }
  friend bool operator!=(const BaseIter& lhs, const BaseIter& rhs) noexcept {
    return lhs.next_.ptr_ != rhs.next_.ptr_;
  }
};

// A wrapper for a Cursor that support the standard map iterator interface.
template <typename ConstT, typename RefT, typename Derived>
class BaseMapIter : public BaseDerived<Derived> {
  using Base = BaseDerived<Derived>;

 public:
  using value_type = std::pair<ConstT, RefT>;
  using reference_type = value_type&;
  using pointer_type = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  BaseMapIter() = default;
  BaseMapIter(const BaseMapIter& other) = default;
  BaseMapIter(BaseMapIter&& other) noexcept = default;
  explicit BaseMapIter(detail::Cursor cur) : cur_(std::move(cur)) {
    operator++();
  }
  template <typename MutT, typename D>
  explicit BaseMapIter(BaseMapIter<ConstT, MutT, D>&& other) noexcept
      : cur_(std::move(other.cur_)), next_(other.next_) {}

  pointer_type operator->() const { return &next_; }
  reference_type operator*() const { return next_; }
  BaseMapIter& operator++() {
    next_.first.reset(cur_.next());
    if (next_.first.has_value()) { // TODO(afuller): Use a single call.
      next_.second.reset(cur_.type_->get(cur_.ptr_, next_.first));
    }
    return *this;
  }
  BaseMapIter operator++(int) {
    BaseMapIter result = *this;
    operator++();
    return result;
  }

  BaseMapIter& operator=(const BaseMapIter& other) {
    cur_ = other.cur_;
    next_.reset(other.next_);
    return *this;
  }
  BaseMapIter& operator=(BaseMapIter&& other) noexcept {
    cur_ = std::move(other.cur_);
    next_.reset(other.next_);
    return *this;
  }

 protected:
  detail::Cursor cur_;
  mutable value_type next_;

 private:
  template <typename, typename, typename>
  friend class BaseMapIter;

  friend bool operator==(
      const BaseMapIter& lhs, const BaseMapIter& rhs) noexcept {
    return lhs.next_.first.ptr_ == rhs.next_.first.ptr_;
  }
  friend bool operator!=(
      const BaseMapIter& lhs, const BaseMapIter& rhs) noexcept {
    return lhs.next_.first.ptr_ != rhs.next_.first.ptr_;
  }
};

template <typename RefT, typename MutT = RefT>
class Iter : public BaseIter<RefT, Iter<RefT, MutT>> {
  using Base = BaseIter<RefT, Iter>;

 public:
  using Base::Base;
  using Base::operator=;

  // Implicit conversion from MutT iterator.
  /*implicit*/ Iter(const Iter<MutT>& other) : Base(other) {}
  /*implicit*/ Iter(Iter<MutT>&& other) : Base(std::move(other)) {}

 private:
  friend bool operator==(const Iter& lhs, const Iter<MutT>& rhs) noexcept {
    return lhs.next_.ptr_ == rhs.next_.ptr_;
  }
  friend bool operator==(const Iter<MutT>& lhs, const Iter& rhs) noexcept {
    return lhs.next_.ptr_ == rhs.next_.ptr_;
  }
  friend bool operator!=(const Iter& lhs, const Iter<MutT>& rhs) noexcept {
    return lhs.next_.ptr_ != rhs.next_.ptr_;
  }
  friend bool operator!=(const Iter<MutT>& lhs, const Iter& rhs) noexcept {
    return lhs.next_.ptr_ != rhs.next_.ptr_;
  }
};

template <typename ConstT, typename MutT, bool Mut = false>
class MapIter : public BaseMapIter<ConstT, MutT, MapIter<ConstT, MutT>> {
  using Base = BaseMapIter<ConstT, MutT, MapIter>;
  using MutItr = MapIter<ConstT, MutT, true>;

 public:
  using Base::Base;
  using Base::operator=;

  // Implicit conversion from MutT iterator.
  /*implicit*/ MapIter(const MutItr& other) : Base(other) {}
  /*implicit*/ MapIter(MutItr&& other) : Base(std::move(other)) {}

 private:
  friend bool operator==(const MapIter& lhs, const MutItr& rhs) noexcept {
    return lhs.next_.ptr_ == rhs.next_.ptr_;
  }
  friend bool operator==(const MutItr& lhs, const MapIter& rhs) noexcept {
    return lhs.next_.ptr_ == rhs.next_.ptr_;
  }
  friend bool operator!=(const MapIter& lhs, const MutItr& rhs) noexcept {
    return lhs.next_.ptr_ != rhs.next_.ptr_;
  }
  friend bool operator!=(const MutItr& lhs, const MapIter& rhs) noexcept {
    return lhs.next_.ptr_ != rhs.next_.ptr_;
  }
};

template <typename MutT>
class Iter<MutT> : public BaseIter<MutT, Iter<MutT>> {
  using Base = BaseIter<MutT, Iter>;

 public:
  using Base::Base;
  using Base::operator=;
};

template <typename ConstT, typename MutT>
class MapIter<ConstT, MutT, true>
    : public BaseMapIter<MutT, MutT, MapIter<ConstT, MutT, true>> {
  using Base = BaseMapIter<MutT, MutT, MapIter>;

 public:
  using Base::Base;
  using Base::operator=;
};

template <typename ConstT, typename MutT = ConstT>
class Iterable {
 public:
  using iterator = Iter<MutT>;
  using const_iterator = Iter<ConstT, MutT>;
  using value_type = MutT;

  /*implicit*/ Iterable(Cursor cur) : cur_(cur) {}

  iterator begin() const { return iterator(cur_); }
  const_iterator cbegin() const { return const_iterator(cur_); }
  iterator end() const { return {}; }
  const_iterator cend() const { return {}; }

 private:
  Cursor cur_;
};

// TODO(afuller): Consider adding asMap(), to create type-safe
// views, with APIs that match c++ standard containers (vs the Thrift 'op' names
// used in the core API).
template <typename ConstT, typename MutT, typename Derived>
class BaseDyn : public Dyn,
                public BaseDerived<Derived>,
                private DynCmp<Derived> {
  using Base = Dyn;

 public:
  using iterable = Iterable<ConstT, MutT>;
  using const_iterable = Iterable<ConstT>;
  using iterator = Iter<MutT>;
  using const_iterator = Iter<ConstT, MutT>;
  using value_type = MutT;
  using size_type = size_t;

  using Base::Base;
  explicit BaseDyn(const Base& other) : Base(other) {}

  bool identical(const ConstT& rhs) const { return Base::identical(rhs); }

  size_t size() const { return type_->size(ptr_); }

  iterator begin() const { return iterator(Base::items()); }
  const_iterator cbegin() const { return const_iterator(Base::items()); }
  iterator end() const { return {}; }
  const_iterator cend() const { return {}; }

  using Base::contains;
  bool contains(const std::string& key) const { return contains(asRef(key)); }

  // Get by value.
  // TODO(afuller): These should be accepting ConstT.
  MutT get(const Base& key) & { return MutT{Base::get(key)}; }
  MutT get(const Base& key) && { return MutT{Base::get(key, false, true)}; }
  ConstT get(const Base& key) const& { return ConstT{Base::get(key, true)}; }
  ConstT get(const Base& key) const&& {
    return ConstT{Base::get(key, true, true)};
  }

  // Get by field id.
  MutT get(FieldId id) & { return MutT{Base::get(id)}; }
  MutT get(FieldId id) && { return MutT{Base::get(id, false, true)}; }
  ConstT get(FieldId id) const& { return ConstT{Base::get(id, true)}; }
  ConstT get(FieldId id) const&& { return ConstT{Base::get(id, true, true)}; }

  // Get by name.
  MutT get(const std::string& name) & { return get(asRef<string_t>(name)); }
  MutT get(const std::string& name) && { return get(asRef<string_t>(name)); }
  ConstT get(const std::string& name) const& {
    return get(asRef<string_t>(name));
  }
  ConstT get(const std::string& name) const&& {
    return get(asRef<string_t>(name));
  }
  // Like `get`, but throws std::out_of_range when a value is not found.
  MutT at(const std::string& key) & { return at(asRef(key)); }
  MutT at(const std::string& key) && { return at(asRef(key)); }
  ConstT at(const std::string& key) const& { return at(asRef(key)); }
  ConstT at(const std::string& key) const&& { return at(asRef(key)); }

  // TODO(afuller): These should be accepting ConstT.
  MutT at(const Base& key) & { return MutT{Base::at(key)}; }
  MutT at(const Base& key) && { return MutT{Base::at(key, false, true)}; }
  ConstT at(const Base& key) const& { return ConstT{Base::at(key, true)}; }
  ConstT at(const Base& key) const&& {
    return ConstT{Base::at(key, true, true)};
  }

  // Get by position.
  MutT get(size_t pos) & { return MutT{Base::get(pos)}; }
  MutT get(size_t pos) && { return MutT{Base::get(pos, false, true)}; }
  ConstT get(size_t pos) const& { return ConstT{Base::get(pos, true)}; }
  ConstT get(size_t pos) const&& { return ConstT{Base::get(pos, true, true)}; }
  // Like `get`, but throws std::out_of_range when a value is not found.
  MutT at(size_t pos) & { return MutT{Base::at(pos)}; }
  MutT at(size_t pos) && { return MutT{Base::at(pos, false, true)}; }
  ConstT at(size_t pos) const& { return ConstT{Base::at(pos, true)}; }
  ConstT at(size_t pos) const&& { return ConstT{Base::at(pos, true, true)}; }

  // Get by ordinal.
  MutT get(Ordinal ord) & { return get(type::toPosition(ord)); }
  MutT get(Ordinal ord) && { return get(type::toPosition(ord)); }
  ConstT get(Ordinal ord) const& { return get(type::toPosition(ord)); }
  ConstT get(Ordinal ord) const&& { return get(type::toPosition(ord)); }
  // Like `get`, but throws std::out_of_range when a value is not found.
  MutT at(Ordinal ord) & { return at(type::toPosition(ord)); }
  MutT at(Ordinal ord) && { return at(type::toPosition(ord)); }
  ConstT at(Ordinal ord) const& { return at(type::toPosition(ord)); }
  ConstT at(Ordinal ord) const&& { return at(type::toPosition(ord)); }

  // Iterate over keys.
  iterable keys() & { return Base::keys(); }
  iterable keys() && { return Base::keys(false, true); }
  const_iterable keys() const& { return Base::keys(true); }
  const_iterable keys() const&& { return Base::keys(true, true); }

  // Iterate over values.
  iterable values() & { return Base::values(); }
  iterable values() && { return Base::values(false, true); }
  const_iterable values() const& { return Base::values(true); }
  const_iterable values() const&& { return Base::values(true, true); }

  DynList<MutT> asList() & { return DynList<MutT>{Base::withContext(false)}; }
  DynList<MutT> asList() && {
    return DynList<MutT>{Base::withContext(false, true)};
  }
  DynList<ConstT> asList() const& {
    return DynList<ConstT>{Base::withContext(true, false)};
  }
  DynList<ConstT> asList() const&& {
    return DynList<ConstT>{Base::withContext(true, true)};
  }

  DynSet<MutT> asSet() & { return DynSet<MutT>{Base::withContext(false)}; }
  DynSet<MutT> asSet() && {
    return DynSet<MutT>{Base::withContext(false, true)};
  }
  DynSet<ConstT> asSet() const& {
    return DynSet<ConstT>{Base::withContext(true, false)};
  }
  DynSet<ConstT> asSet() const&& {
    return DynSet<ConstT>{Base::withContext(true, true)};
  }

  DynMap<MutT> asMap() & { return DynMap<MutT>{Base::withContext(false)}; }
  DynMap<MutT> asMap() && {
    return DynMap<MutT>{Base::withContext(false, true)};
  }
  DynMap<ConstT> asMap() const& {
    return DynMap<ConstT>{Base::withContext(true, false)};
  }
  DynMap<ConstT> asMap() const&& {
    return DynMap<ConstT>{Base::withContext(true, true)};
  }

 protected:
  using BaseDerived<Derived>::derived;
  template <typename IdT>
  constexpr static bool is_index_type_v =
      std::is_same_v<IdT, Ordinal> || std::is_integral_v<IdT>;
  template <typename IdT, typename R = ConstT>
  using if_not_index = std::enable_if_t<!is_index_type_v<IdT>, R>;

  template <typename Tag = binary_t>
  static ConstT asRef(const native_type<Tag>& val) {
    return ConstT::template to<Tag>(val);
  }

  // Re-map mut calls from base, without 'const' qualifier.

  void assign(ConstT val) { Base::assign(val); }
  void assign(const std::string& val) { assign(asRef(val)); }
  Derived& operator=(ConstT val) & { return (assign(val), derived()); }
  Derived&& operator=(ConstT val) && {
    return (assign(val), std::move(derived()));
  }
  Derived& operator=(const std::string& val) & {
    return (assign(val), derived());
  }
  Derived&& operator=(const std::string& val) && {
    return (assign(val), std::move(derived()));
  }

  void prepend(ConstT val) { Base::prepend(val); }
  void prepend(const std::string& val) { prepend(asRef(val)); }
  void append(ConstT val) { Base::append(val); }
  void append(const std::string& val) { append(asRef(val)); }

  bool add(ConstT val) { return Base::add(val); }
  bool add(const std::string& val) { return add(asRef(val)); }
  Derived& operator+=(ConstT val) & { return (add(val), derived()); }
  Derived&& operator+=(ConstT val) && { return (add(val), derived()); }
  Derived& operator+=(const std::string& val) & {
    return (add(asRef(val)), derived());
  }
  Derived&& operator+=(const std::string& val) && {
    return (add(asRef(val)), derived());
  }
  Derived& operator++() & { return (add(asRef<i32_t>(1)), derived()); }
  Derived&& operator++() && { return (add(asRef<i32_t>(1)), derived()); }

  bool put(FieldId id, ConstT val) { return Base::put(id, val); }
  bool put(ConstT key, ConstT val) { return Base::put(key, val); }
  bool put(FieldId id, const std::string& val) { return put(id, asRef(val)); }
  bool put(ConstT key, const std::string& val) { return put(key, asRef(val)); }
  bool put(const std::string& name, ConstT val) {
    return put(asRef<string_t>(name), val);
  }
  bool put(const std::string& name, const std::string& val) {
    return put(asRef<string_t>(name), asRef(val));
  }

  void insert(size_t pos, ConstT val) { Base::insert(pos, val); }
  void insert(size_t pos, const std::string& val) {
    Base::insert(pos, asRef(val));
  }

  bool remove(ConstT key) { return Base::remove(key); }
  void remove(size_t pos) { Base::remove(pos); }
  bool remove(const std::string& key) { return Base::remove(asRef(key)); }

  MutT ensure(FieldId id) { return MutT{Base::ensure(id)}; }
  MutT ensure(FieldId id, ConstT defVal) {
    return MutT{Base::ensure(id, defVal)};
  }
  MutT ensure(FieldId id, const std::string& defVal) {
    return MutT{Base::ensure(id, asRef(defVal))};
  }
  MutT ensure(ConstT key) { return MutT{Base::ensure(key)}; }
  MutT ensure(ConstT key, ConstT defVal) {
    return MutT{Base::ensure(key, defVal)};
  }
  MutT ensure(const std::string& name) { return ensure(asRef<string_t>(name)); }
  MutT ensure(const std::string& name, ConstT defVal) {
    return ensure(asRef<string_t>(name), defVal);
  }
  MutT ensure(const std::string& name, const std::string& defVal) {
    return ensure(asRef<string_t>(name), asRef(defVal));
  }

  void clear() { Base::clear(); }
  void clear(FieldId id) { Base::remove(id); }
  void clear(ConstT key) { Base::remove(key); }
  void clear(std::string name) { Base::remove(asRef<string_t>(name)); }

 private:
  // TODO(afuller): Support capturing string literals directly and remove these.
  friend bool operator==(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) == rhs;
  }
  friend bool operator!=(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) != rhs;
  }
  friend bool operator<(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) < rhs;
  }
  friend bool operator<=(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) <= rhs;
  }
  friend bool operator>(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) > rhs;
  }
  friend bool operator>=(const std::string& lhs, const Derived& rhs) {
    return asRef(lhs) >= rhs;
  }
  friend bool operator==(const Derived& lhs, const std::string& rhs) {
    return lhs == asRef(rhs);
  }
  friend bool operator!=(const Derived& lhs, const std::string& rhs) {
    return lhs != asRef(rhs);
  }
  friend bool operator<(const Derived& lhs, const std::string& rhs) {
    return lhs < asRef(rhs);
  }
  friend bool operator<=(const Derived& lhs, const std::string& rhs) {
    return lhs <= asRef(rhs);
  }
  friend bool operator>(const Derived& lhs, const std::string& rhs) {
    return lhs > asRef(rhs);
  }
  friend bool operator>=(const Derived& lhs, const std::string& rhs) {
    return lhs >= asRef(rhs);
  }
};

template <typename Derived, typename ConstT = Derived>
class BaseRef : public BaseDyn<ConstT, Derived, Derived> {
  using Base = BaseDyn<ConstT, Derived, Derived>;

 public:
  using Base::Base;
  using Base::reset;

  template <typename Tag>
  static Derived to(native_type<Tag>& val) {
    return {Tag{}, val};
  }
  template <typename Tag>
  static Derived to(const native_type<Tag>& val) {
    return {Tag{}, val};
  }
  template <typename Tag>
  static Derived to(native_type<Tag>&& val) {
    return {Tag{}, std::move(val)};
  }
  template <typename Tag>
  static Derived to(const native_type<Tag>&& val) {
    return {Tag{}, std::move(val)};
  }
  template <typename T>
  static Derived to(T&& val) {
    return to<type::infer_tag<T>>(std::forward<T>(val));
  }

 protected:
  using Base::operator=;
  using Base::operator+=;
  using Base::operator++;
};

// The ops for the empty type 'void'.
struct VoidErasedOp : BaseErasedOp {
  static void delete_(void*) {}
  static void* make(void* ptr, bool) {
    assert(ptr == nullptr);
    return ptr = nullptr;
  }
  static bool empty(const void*) { return true; }
  static bool identical(const void*, const Dyn&) { return true; }
  static folly::partial_ordering compare(const void*, const Dyn& rhs) {
    check_op(!rhs.has_value());
    return folly::partial_ordering::equivalent;
  }
  static void clear(void*) {}
  static void assign(void*, const Dyn& val) { check_op(!val.has_value()); }
};

inline const TypeInfo& voidTypeInfo() {
  return getTypeInfo<VoidErasedOp, void_t>();
}

} // namespace detail

// An un-owning pointer to a thrift value.
using Ptr = detail::Ptr;

} // namespace apache::thrift::type
