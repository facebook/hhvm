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
#include <thrift/lib/cpp2/patch/DynamicPatch.h>

namespace apache::thrift::protocol {

namespace detail {
struct PatchBadgeFactory;
using Badge = folly::badge<PatchBadgeFactory>;

// This is a wrapper of the underlying dynamic patch, with type safe APIs that
// are deduced from Tag. The APIs in principle should be identical to static
// patches.
template <class Tag>
class TaggedPatchRef;

class BadgeHolder {
  static Badge get();
  template <class>
  friend class TaggedPatchRef;
  template <class>
  friend class TaggedStructurePatchRef;
  friend class TaggedPatchAnyRef;
};

template <class Tag>
class TaggedPatchRef<type::list<Tag>> {
  using List = type::list<Tag>;

 public:
  using value_type = type::native_type<List>;
  using underlying_dynamic_patch = DynamicListPatch;

  explicit TaggedPatchRef(DynamicListPatch& patch) : patch_(patch) {}

  void assign(const value_type& v) {
    patch_.get().assign(badge, asValueStruct<List>(v).as_list());
  }

  void operator=(const value_type& v) { assign(v); }

  void clear() { patch_.get().clear(badge); }

  void push_back(const typename type::native_type<Tag>& v) {
    patch_.get().push_back(badge, asValueStruct<Tag>(v));
  }

  void apply(value_type& v) {
    auto l = asValueStruct<List>(v);
    patch_.get().apply(badge, l.as_list());
    v = protocol::fromValueStruct<List>(l);
  }

  void merge(const TaggedPatchRef& other) {
    patch_.get().merge(badge, other.patch_.get());
  }

  auto toObject() const { return patch_.get().toObject(); }

 private:
  static inline const auto badge = BadgeHolder::get();

  std::reference_wrapper<DynamicListPatch> patch_;
};

template <class Tag>
class TaggedPatchRef<type::set<Tag>> {
  using Set = type::set<Tag>;

 public:
  using value_type = type::native_type<Set>;
  using underlying_dynamic_patch = DynamicSetPatch;

  explicit TaggedPatchRef(DynamicSetPatch& patch) : patch_(patch) {}

  void assign(const value_type& v) {
    patch_.get().assign(badge, asValueStruct<Set>(v).as_set());
  }

  void operator=(const value_type& v) { assign(v); }

  void clear() { patch_.get().clear(badge); }

  void insert(const typename type::native_type<Tag>& v) {
    patch_.get().insert(badge, asValueStruct<Tag>(v));
  }

  void erase(const typename type::native_type<Tag>& v) {
    patch_.get().erase(badge, asValueStruct<Tag>(v));
  }

  void apply(value_type& v) {
    auto l = asValueStruct<Set>(v);
    patch_.get().apply(badge, l.as_set());
    v = protocol::fromValueStruct<Set>(l);
  }

  void merge(const TaggedPatchRef& other) {
    patch_.get().merge(badge, other.patch_.get());
  }

  auto toObject() const { return patch_.get().toObject(); }

 private:
  static inline const auto badge = BadgeHolder::get();

  std::reference_wrapper<DynamicSetPatch> patch_;
};

// The TaggedPatchAnyRef is a type-erased container that holds any
// TaggedPatchRef. It provides helper function to refer to any particular
// DynamicPatch with Tag attached. This is needed for patchByKey(...) and
// patchIfSet(...) since the sub-patch can be any type.
class TaggedPatchAnyRef {
 public:
  template <class Tag>
  auto& bind(DynamicPatch& patch) {
    auto& stored = patch.getStoredPatchByTag<Tag>(BadgeHolder::get());
    if constexpr (type::is_a_v<Tag, type::primitive_c>) {
      // For primitive patches we return the stored patch directly.
      return stored;
    } else {
      // For non-primitive patches, we wrap the stored patch with TaggedPatchRef
      // so that users can have type safe APIs.
      storage_ = TaggedPatchRef<Tag>(stored);
      return std::any_cast<TaggedPatchRef<Tag>&>(storage_);
    }
  }

 private:
  // In theory we can deduce the exact patch type from Tag, we don't need to use
  // std::any, though it's easier to implement if we just store the
  // TaggedPatchRef in a std::any.
  std::any storage_;
};

template <class K, class V>
class TaggedPatchRef<type::map<K, V>> {
  using Map = type::map<K, V>;

 public:
  using value_type = type::native_type<Map>;
  using underlying_dynamic_patch = DynamicMapPatch;

  explicit TaggedPatchRef(DynamicMapPatch& patch) : patch_(patch) {}

  void assign(const value_type& v) {
    patch_.get().assign(badge, asValueStruct<Map>(v).as_map());
  }

  void operator=(const value_type& v) { assign(v); }

  void clear() { patch_.get().clear(badge); }

  void insert_or_assign(
      const type::native_type<K>& key, const type::native_type<V>& value) {
    patch_.get().insert_or_assign(
        badge, asValueStruct<K>(key), asValueStruct<V>(value));
  }

  void add(const type::native_type<Map>& map) {
    patch_.get().tryPutMulti(badge, asValueStruct<Map>(map).as_map());
  }

  void erase(const type::native_type<K>& key) {
    patch_.get().erase(badge, asValueStruct<K>(key));
  }

  auto& patchByKey(const type::native_type<K>& key) {
    auto& subPatch = patch_.get().patchByKey(badge, asValueStruct<K>(key));
    return valuePatchRef_.bind<V>(subPatch);
  }

  auto& ensureAndPatchByKey(const type::native_type<K>& key) {
    add({{key, {}}});
    return patchByKey(key);
  }

  void apply(value_type& v) {
    auto l = asValueStruct<Map>(v);
    patch_.get().apply(badge, l.as_map());
    v = protocol::fromValueStruct<Map>(l);
  }

  void merge(const TaggedPatchRef& other) {
    patch_.get().merge(badge, other.patch_.get());
  }

  auto toObject() const { return patch_.get().toObject(); }

 private:
  static inline const auto badge = BadgeHolder::get();

  std::reference_wrapper<DynamicMapPatch> patch_;
  TaggedPatchAnyRef valuePatchRef_;
};

template <class T>
class TaggedStructurePatchRef {
  using Tag = type::infer_tag<T>;

 public:
  using value_type = T;
  using underlying_dynamic_patch = std::conditional_t<
      is_thrift_union_v<T>,
      DynamicUnionPatch,
      DynamicStructPatch>;

  explicit TaggedStructurePatchRef(underlying_dynamic_patch& patch)
      : patch_(patch) {}

  void assign(const value_type& v) {
    patch_.get().assign(badge, asValueStruct<Tag>(v).as_object());
  }

  void operator=(const value_type& v) { assign(v); }

  void clear() { patch_.get().clear(badge); }

  template <class Id>
  void ensure() {
    ensureImpl<Id>({});
  }

  template <class Id>
  void ensure(const op::get_native_type<value_type, Id>& d) {
    static_assert(type::is_optional_or_union_field_v<T, Id>);
    ensureImpl<Id>(d);
  }

  template <class Id>
  auto& patchIfSet() {
    if (!type::is_optional_or_union_field_v<T, Id>) {
      ensure<Id>();
    }

    FieldId id = op::get_field_id_v<value_type, Id>;
    auto& subPatch = patch_.get().patchIfSet(badge, id);
    return fieldPatchRef_.bind<op::get_type_tag<value_type, Id>>(subPatch);
  }

  template <class Id>
  auto& patch() {
    ensure<Id>();
    return patchIfSet<Id>();
  }

  template <class Id>
  void remove() {
    patch_.get().remove(badge, op::get_field_id_v<value_type, Id>);
  }

  void apply(value_type& v) {
    auto obj = asValueStruct<Tag>(v);
    patch_.get().apply(badge, obj.as_object());
    v = protocol::fromValueStruct<Tag>(obj);
  }

  void merge(const TaggedStructurePatchRef& other) {
    patch_.get().merge(badge, other.patch_.get());
  }

  auto toObject() const { return patch_.get().toObject(); }

  auto toSafePatch() const {
    op::safe_patch_type<type::infer_tag<T>> safePatch;
    safePatch.data() = protocol::serializeObject<CompactProtocolWriter>(
        patch_.get().toObject());
    safePatch.version() = 2;
    return safePatch;
  }

  template <class Id>
  bool modifies() const {
    return patch_.get().modifies(badge, op::get_field_id_v<value_type, Id>);
  }

  bool empty() { return patch_.get().empty(badge); }

 private:
  static inline const auto badge = BadgeHolder::get();

  template <class Id>
  void ensureImpl(const op::get_native_type<value_type, Id>& d) {
    patch_.get().ensure(
        badge,
        op::get_field_id_v<value_type, Id>,
        asValueStruct<op::get_type_tag<value_type, Id>>(d));
  }

  std::reference_wrapper<underlying_dynamic_patch> patch_;
  TaggedPatchAnyRef fieldPatchRef_;

 protected:
  ~TaggedStructurePatchRef() = default;
};

template <class T>
class TaggedPatchRef<type::struct_t<T>> : public TaggedStructurePatchRef<T> {
 public:
  using TaggedStructurePatchRef<T>::TaggedStructurePatchRef;
  using TaggedStructurePatchRef<T>::operator=;
};

template <class T>
class TaggedPatchRef<type::union_t<T>> : public TaggedStructurePatchRef<T> {
 public:
  using TaggedStructurePatchRef<T>::TaggedStructurePatchRef;
  using TaggedStructurePatchRef<T>::operator=;
};

// A wrapper to make unowned types own the underlying data
template <class Ref>
class Owned : public Ref {
  using underlying_dynamic_patch = typename Ref::underlying_dynamic_patch;
  struct Secret {};

 public:
  Owned() : Owned(Secret{}, std::make_unique<underlying_dynamic_patch>()) {}

  Owned(const Owned& other) : Owned() {
    // We don't need to copy `Ref` base class -- it's a reference that doesn't
    // own any data.
    *value_ = *other.value_;
  }

  Owned& operator=(const Owned& other) {
    *value_ = *other.value_;
    return *this;
  }

  Owned(Owned&&) = default;
  Owned& operator=(Owned&&) = default;
  ~Owned() = default;

  using Ref::operator=;

  static Owned createClear() {
    Owned ret;
    ret.clear();
    return ret;
  }

  template <class T = typename Ref::value_type>
  static Owned createAssign(T t) {
    Owned ret;
    ret.assign(std::move(t));
    return ret;
  }

  void reset() { *this = Owned{}; }

 private:
  // We need a Secret argument, otherwise code like `ListPatch = {0}` won't work
  // since it will pick this constructor rather than invoking
  // `ListPatch.assign({0})`.
  Owned(Secret, std::unique_ptr<underlying_dynamic_patch> patch)
      : Ref(*patch), value_(std::move(patch)) {}

  std::unique_ptr<underlying_dynamic_patch> value_;
};
} // namespace detail

// Tagged Patch works similar to op::patch_type, but it does not require the
// generated patch struct
template <class T>
using tagged_patch =
    typename detail::Owned<detail::TaggedPatchRef<type::infer_tag<T>>>;

} // namespace apache::thrift::protocol
