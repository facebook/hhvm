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

namespace apache {
namespace thrift {
namespace type {
class Ref;
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

// TODO(afuller): Expose a more standard iterator iterface.
// TODO(afuller): Benchmark and and consider optimizing.
class Cursor {
 public:
  // Returns the next value, or nullPtr() if there are no more values.
  Ptr next();

 private:
  friend class Dyn;

  RuntimeType type_;
  void* ptr_ = nullptr; // Only needed for end().
  IterType iterType_;
  std::any itr_;

  Cursor(RuntimeType type, void* ptr, IterType iterType)
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
  template <typename Tag>
  const native_type<Tag>& as() const {
    // TODO(afuller): Check thrift types match.
    return type_->as<native_type<Tag>>(ptr_);
  }
  template <typename T>
  if_not_thrift_type_tag<T, const T&> as() const {
    return as<infer_tag<T>>();
  }

  // Returns nullptr on mismatch.
  template <typename Tag>
  FOLLY_NODISCARD const native_type<Tag>* tryAs() const noexcept {
    // TODO(afuller): Check thrift types match.
    return type_->tryAs<native_type<Tag>>(ptr_);
  }
  template <typename T>
  if_not_thrift_type_tag<T, const T*> tryAs() const {
    return tryAs<infer_tag<T>>();
  }

  FOLLY_NODISCARD bool empty() const { return type_->empty(ptr_); }
  FOLLY_NODISCARD bool identical(const Dyn& rhs) const {
    return type() == rhs.type() && type_->identical(ptr_, rhs);
  }

  FOLLY_NODISCARD bool equal(const Dyn& rhs) const {
    return type_->equal(ptr_, rhs);
  }
  FOLLY_NODISCARD folly::ordering compare(const Dyn& rhs) const {
    return type_->compare(ptr_, rhs);
  }

  FOLLY_NODISCARD bool has_value() const { return !type().empty(); }

 protected:
  friend class Cursor;
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
  void append(const Dyn& val) const { type_.mut().append(ptr_, val); }
  bool add(const Dyn& val) const { return type_.mut().add(ptr_, val); }
  bool put(const Dyn& key, const Dyn& val) const {
    return type_.mut().put(ptr_, key, val);
  }
  bool put(FieldId id, const Dyn& val) const {
    return type_.mut().put(ptr_, id, val);
  }

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
  using Dyn::clear;
  using Dyn::ensure;
  using Dyn::get;
  using Dyn::keys;
  using Dyn::mut;
  using Dyn::put;
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

  [[noreturn]] static void append(void*, const Dyn&) { bad_op(); }
  [[noreturn]] static bool add(void*, const Dyn&) { bad_op(); }
  [[noreturn]] static bool put(void*, FieldId, const Dyn&, const Dyn&) {
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

template <typename RefT>
class RefCursor : public Cursor {
 public:
  /*implicit*/ RefCursor(Cursor&& cur) : Cursor(cur) {}

  RefT next() { return RefT{Cursor::next()}; }
};

// TODO(afuller): Consider adding asMap(), asList(), etc, to create type-safe
// views, with APIs that match c++ standard containers (vs the Thrift 'op' names
// used in the core API).
template <typename ConstT, typename MutT, typename Derived>
class BaseDyn : private DynCmp<Derived>,
                public BaseDerived<Derived>,
                public Dyn {
  using Base = Dyn;

 public:
  using Base::Base;
  explicit BaseDyn(const Base& other) : Base(other) {}

  bool identical(const ConstT& rhs) const { return Base::identical(rhs); }

  // Get by value.
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
  MutT get(const std::string& name) & { return get(asRef(name)); }
  MutT get(const std::string& name) && { return get(asRef(name)); }
  ConstT get(const std::string& name) const& { return get(asRef(name)); }
  ConstT get(const std::string& name) const&& { return get(asRef(name)); }

  // Get by position.
  MutT get(size_t pos) & { return MutT{Base::get(pos)}; }
  MutT get(size_t pos) && { return MutT{Base::get(pos, false, true)}; }
  ConstT get(size_t pos) const& { return ConstT{Base::get(pos, true)}; }
  ConstT get(size_t pos) const&& { return ConstT{Base::get(pos, true, true)}; }

  // Get by ordinal.
  MutT get(Ordinal ord) & { return get(type::toPosition(ord)); }
  MutT get(Ordinal ord) && { return get(type::toPosition(ord)); }
  ConstT get(Ordinal ord) const& { return get(type::toPosition(ord)); }
  ConstT get(Ordinal ord) const&& { return get(type::toPosition(ord)); }

  size_t size() const { return type_->size(ptr_); }

  // Iterate over keys.
  RefCursor<MutT> keys() & { return Base::keys(); }
  RefCursor<MutT> keys() && { return Base::keys(false, true); }
  RefCursor<ConstT> keys() const& { return Base::keys(true); }
  RefCursor<ConstT> keys() const&& { return Base::keys(true, true); }

  // Iterate over values.
  RefCursor<MutT> values() & { return Base::values(); }
  RefCursor<MutT> values() && { return Base::values(false, true); }
  RefCursor<ConstT> values() const& { return Base::values(true); }
  RefCursor<ConstT> values() const&& { return Base::values(true, true); }

 protected:
  using BaseDerived<Derived>::derived;
  template <typename IdT>
  constexpr static bool is_index_type_v =
      std::is_same<IdT, Ordinal>::value || std::is_integral<IdT>::value;
  template <typename IdT, typename R = ConstT>
  using if_not_index = std::enable_if_t<!is_index_type_v<IdT>, R>;

  template <typename Tag = type::string_t>
  static ConstT asRef(const std::string& name) {
    return ConstT::template to<Tag>(name);
  }

  // Re-map mut calls from base, without 'const' qualifier.

  void assign(ConstT val) { Base::assign(val); }
  void assign(const std::string& val) { assign(asRef<binary_t>(val)); }
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

  void append(ConstT val) { Base::append(val); }
  void append(const std::string& val) { append(asRef<binary_t>(val)); }

  bool add(ConstT val) { return Base::add(val); }
  bool add(const std::string& val) { return add(asRef<binary_t>(val)); }

  bool put(FieldId id, ConstT val) { return Base::put(id, val); }
  bool put(ConstT key, ConstT val) { return Base::put(key, val); }
  bool put(FieldId id, const std::string& val) {
    return Base::put(id, asRef(val));
  }
  bool put(const std::string& name, ConstT val) {
    return put(asRef(name), val);
  }
  bool put(const std::string& name, const std::string& val) {
    return put(asRef(name), asRef(val));
  }

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
  MutT ensure(const std::string& name) { return ensure(asRef(name)); }
  MutT ensure(const std::string& name, ConstT defVal) {
    return ensure(asRef(name), defVal);
  }
  MutT ensure(const std::string& name, const std::string& defVal) {
    return ensure(asRef(name), asRef(defVal));
  }

  void clear() { Base::clear(); }
  void clear(FieldId id) { Base::put(id, ConstT{}); }
  void clear(ConstT key) { Base::put(key, ConstT{}); }
  void clear(std::string name) { Base::put(asRef(name), ConstT{}); }

 private:
  friend class BaseDerived<Derived>;

  // TODO(afuller): Support capturing string literals directly and remove these.
  friend bool operator==(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) == rhs;
  }
  friend bool operator!=(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) != rhs;
  }
  friend bool operator<(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) < rhs;
  }
  friend bool operator<=(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) <= rhs;
  }
  friend bool operator>(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) > rhs;
  }
  friend bool operator>=(const std::string& lhs, const Derived& rhs) {
    return asRef<binary_t>(lhs) >= rhs;
  }
  friend bool operator==(const Derived& lhs, const std::string& rhs) {
    return lhs == asRef<binary_t>(rhs);
  }
  friend bool operator!=(const Derived& lhs, const std::string& rhs) {
    return lhs != asRef<binary_t>(rhs);
  }
  friend bool operator<(const Derived& lhs, const std::string& rhs) {
    return lhs < asRef<binary_t>(rhs);
  }
  friend bool operator<=(const Derived& lhs, const std::string& rhs) {
    return lhs <= asRef<binary_t>(rhs);
  }
  friend bool operator>(const Derived& lhs, const std::string& rhs) {
    return lhs > asRef<binary_t>(rhs);
  }
  friend bool operator>=(const Derived& lhs, const std::string& rhs) {
    return lhs >= asRef<binary_t>(rhs);
  }
};

template <typename Derived, typename ConstT = Derived>
class BaseRef : public BaseDyn<ConstT, Derived, Derived> {
  using Base = BaseDyn<ConstT, Derived, Derived>;

 public:
  using Base::Base;

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
  static partial_ordering compare(const void*, const Dyn& rhs) {
    check_op(!rhs.has_value());
    return partial_ordering::eq;
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

} // namespace type
} // namespace thrift
} // namespace apache
