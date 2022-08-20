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

#include <memory>
#include <stdexcept>
#include <type_traits>

#include <folly/ConstexprMath.h>
#include <folly/lang/Exception.h>
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

// A pointer for a type that has sufficent alignment to store information
// in the lower bits.
//
// TODO(afuller): Moved to a shared locations, so this can be used to track
// 'ownership' as well.
template <typename T, size_t Bits = folly::constexpr_log2(alignof(T))>
class AlignedPtr {
 public:
  static_assert(
      Bits > 0 && Bits <= folly::constexpr_log2(alignof(T)),
      "insufficent alignment");
  constexpr static std::uintptr_t kMask = ~std::uintptr_t{} << Bits;

  constexpr AlignedPtr() noexcept = default;
  /* implicit */ constexpr AlignedPtr(T* ptr, std::uintptr_t bits = {}) noexcept
      : ptr_((folly::bit_cast<std::uintptr_t>(ptr) & kMask) | (bits & ~kMask)) {
    assert((bits & kMask) == 0); // Programming error.
    // Never happens because of T's alignment.
    assert((folly::bit_cast<std::uintptr_t>(ptr) & ~kMask) == 0);
  }

  T* get() const noexcept { return reinterpret_cast<T*>(ptr_ & kMask); }

  template <size_t Bit>
  constexpr bool get() const noexcept {
    return ptr_ & bitMask<Bit>();
  }

  template <size_t Bit>
  constexpr void set() noexcept {
    ptr_ |= bitMask<Bit>();
  }

  template <size_t Bit>
  constexpr void clear() noexcept {
    ptr_ &= ~bitMask<Bit>();
  }

 private:
  std::uintptr_t ptr_ = {};

  template <size_t Bit>
  constexpr static auto bitMask() noexcept {
    static_assert(Bit < Bits, "out of range");
    return std::uintptr_t{1} << Bit;
  }
};

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

// A base class for qualifier-preserving type-erased runtime types.
class RuntimeBase {
 public:
  RuntimeBase() noexcept = default;
  RuntimeBase(RuntimeType type, void* ptr) noexcept : type_(type), ptr_(ptr) {}
  RuntimeBase(RuntimeType type, const void* ptr) noexcept
      : type_(type.withContext(true)), ptr_(const_cast<void*>(ptr)) {}

  // Type accessors.
  const Type& type() const noexcept { return type_->thriftType; }
  const std::type_info& typeId() const noexcept { return type_->cppType; }

  // Throws on mismatch.
  template <typename Tag>
  const native_type<Tag>& as() const {
    return type_->as<native_type<Tag>>(ptr_);
  }

  // Returns nullptr on mismatch.
  template <typename Tag>
  const native_type<Tag>* tryAs() const noexcept {
    return type_->tryAs<native_type<Tag>>(ptr_);
  }

  bool empty() const { return type_->empty(ptr_); }
  bool identical(const RuntimeBase& rhs) const {
    return type() == rhs.type() && type_->identical(ptr_, rhs);
  }

  bool equal(const RuntimeBase& rhs) const { return type_->equal(ptr_, rhs); }

 protected:
  RuntimeType type_;
  void* ptr_ = nullptr;

  ~RuntimeBase() = default; // Abstract base class;

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
  void append(const RuntimeBase& val) const { type_.mut().append(ptr_, val); }
  bool add(const RuntimeBase& val) const { return type_.mut().add(ptr_, val); }
  bool put(const RuntimeBase& key, const RuntimeBase& val) const {
    return type_.mut().put(ptr_, {}, &key, val);
  }
  bool put(FieldId id, const RuntimeBase& val) const {
    return type_.mut().put(ptr_, id, nullptr, val);
  }

  // TODO(afuller): Make context an enum, instead of pair of bools.
  Ptr get(const RuntimeBase& key) const;
  Ptr get(FieldId id) const;
  Ptr get(size_t pos) const;
  Ptr get(const RuntimeBase& key, bool ctxConst, bool ctxRvalue = false) const;
  Ptr get(FieldId id, bool ctxConst, bool ctxRvalue = false) const;
  Ptr get(size_t pos, bool ctxConst, bool ctxRvalue = false) const;

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
  friend bool operator==(
      const RuntimeBase& lhs, const RuntimeBase& rhs) noexcept {
    return lhs.equal(rhs);
  }
};

// An un-owning pointer to a thrift value.
class Ptr final : public RuntimeBase {
 public:
  using RuntimeBase::RuntimeBase;

  // Pointers do not share constness with the value, so expose functions
  // directly.
  using RuntimeBase::add;
  using RuntimeBase::append;
  using RuntimeBase::clear;
  using RuntimeBase::get;
  using RuntimeBase::mut;
  using RuntimeBase::put;
  using RuntimeBase::tryMut;

  // Deref.
  Ref operator*() const noexcept;

 private:
  friend class RuntimeBase;
};

inline Ptr TypeInfo::get(void* ptr, FieldId id) const {
  return get_(ptr, id, std::string::npos, nullptr);
}
inline Ptr TypeInfo::get(void* ptr, size_t pos) const {
  return get_(ptr, {}, pos, nullptr);
}
inline Ptr TypeInfo::get(void* ptr, const RuntimeBase& val) const {
  return get_(ptr, {}, std::string::npos, &val);
}

inline Ptr RuntimeBase::get(const RuntimeBase& key) const {
  return type_->get(ptr_, key);
}
inline Ptr RuntimeBase::get(FieldId id) const {
  return type_->get(ptr_, id);
}
inline Ptr RuntimeBase::get(size_t pos) const {
  return type_->get(ptr_, pos);
}
inline Ptr RuntimeBase::get(
    const RuntimeBase& key, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, key);
}
inline Ptr RuntimeBase::get(FieldId id, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, id);
}
inline Ptr RuntimeBase::get(size_t pos, bool ctxConst, bool ctxRvalue) const {
  return type_.withContext(ctxConst, ctxRvalue)->get(ptr_, pos);
}

// A base struct provides helpers for throwing exceptions and default throwing
// impls type-specific ops.
struct BaseErasedOp {
  [[noreturn]] static void bad_op(const char* msg = "not supported") {
    folly::throw_exception<std::logic_error>(msg);
  }
  [[noreturn]] static void bad_type() {
    folly::throw_exception<std::bad_any_cast>();
  }
  [[noreturn]] static void unimplemented(const char* msg = "not implemented") {
    folly::throw_exception<std::runtime_error>(msg);
  }

  [[noreturn]] static void append(void*, const RuntimeBase&) { bad_op(); }
  [[noreturn]] static bool add(void*, const RuntimeBase&) { bad_op(); }
  [[noreturn]] static bool put(
      void*, FieldId, const RuntimeBase*, const RuntimeBase&) {
    bad_op();
  }
  [[noreturn]] static Ptr get(void*, FieldId, size_t, const RuntimeBase*) {
    bad_op();
  }
  [[noreturn]] static size_t size(const void*) { bad_op(); }
};

// TODO(afuller): Consider adding asMap(), asList(), etc, to create type-safe
// views, with APIs that match c++ standard containers (vs the Thrift 'op' names
// used in the core API).
template <typename ConstT, typename MutT>
class RuntimeAccessBase : public RuntimeBase {
  using Base = RuntimeBase;

 public:
  using Base::Base;
  explicit RuntimeAccessBase(const Base& other) : Base(other) {}

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

  // TODO(afuller): Make these 'ensuring' gets, aka insert_or_assign.
  template <typename Id>
  MutT operator[](Id&& id) & {
    return get(std::forward<Id>(id));
  }
  template <typename Id>
  MutT operator[](Id&& id) && {
    return get(std::forward<Id>(id));
  }
  template <typename Id>
  ConstT operator[](Id&& id) const& {
    return get(std::forward<Id>(id));
  }
  template <typename Id>
  ConstT operator[](Id&& id) const&& {
    return get(std::forward<Id>(id));
  }

 protected:
  static ConstT asRef(const std::string& name) {
    return ConstT::template to<type::string_t>(name);
  }
};

template <typename Derived, typename ConstT = Derived>
class BaseRef : public RuntimeAccessBase<ConstT, Derived> {
  using Base = RuntimeAccessBase<ConstT, Derived>;

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
};

// The ops for the empty type 'void'.
struct VoidErasedOp : BaseErasedOp {
  static void delete_(void*) {}
  static void* make(void* ptr, bool) {
    assert(ptr == nullptr);
    return ptr = nullptr;
  }
  static bool empty(const void*) { return true; }
  static bool identical(const void*, const RuntimeBase&) { return true; }
  static int compare(const void*, const RuntimeBase& rhs) {
    if (!rhs.type().empty()) {
      bad_op();
    }
    return 0;
  }
  static void clear(void*) {}
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
