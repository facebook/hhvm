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

#include <folly/lang/Badge.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/protocol/detail/DynamicCursorSerializer.h>

namespace apache::thrift::op::detail {
template <typename>
class AnyPatch;
}

namespace apache::thrift::op {
class AnyPatchStruct;
using AnyPatch = detail::AnyPatch<AnyPatchStruct>;
} // namespace apache::thrift::op

namespace apache::thrift::protocol {
class DiffVisitorBase;

class DynamicListPatch;
class DynamicSetPatch;
class DynamicMapPatch;
class DynamicStructPatch;
class DynamicUnionPatch;

using ValueList = std::vector<Value>;
using ValueSet = folly::F14VectorSet<Value>;
using ValueMap = folly::F14FastMap<Value, Value>;

namespace detail {

// Use Badge pattern to control access.
// See thrift/lib/cpp2/patch/detail/PatchBadge.h
struct PatchBadgeFactory;
using Badge = folly::badge<PatchBadgeFactory>;

template <typename T, typename U>
using if_same_type_after_remove_cvref = std::enable_if_t<
    std::is_same_v<folly::remove_cvref_t<T>, folly::remove_cvref_t<U>>>;

using apache::thrift::detail::DynamicCursorSerializationWrapper;

template <class PatchType>
PatchType createPatchFromObject(Badge badge, Object obj) {
  PatchType patch;
  if constexpr (requires { patch.fromObject(badge, std::move(obj)); }) {
    patch.fromObject(badge, std::move(obj));
  } else {
    if (!ProtocolValueToThriftValue<type::infer_tag<PatchType>>{}(obj, patch)) {
      throw std::runtime_error(
          fmt::format(
              "Object does not match patch ({}) schema. Object = {}",
              folly::pretty_name<PatchType>(),
              debugStringViaEncode(obj)));
    }
  }
  return patch;
}

void convertStringToBinary(Value& v);
void convertStringToBinary(ValueList& v);
void convertStringToBinary(ValueSet& v);
void convertStringToBinary(ValueMap& v);
void convertStringToBinary(Object& obj);

struct DynamicPatchOptions {
  // If users are confident their input doesn't contain string, we can skip
  // converting it to binary.
  bool doNotConvertStringToBinary = false;
};

void checkSameType(
    const apache::thrift::protocol::Value& v1,
    const apache::thrift::protocol::Value& v2);
void checkCompatibleType(
    const ValueList& l, const apache::thrift::protocol::Value& v);
void checkCompatibleType(
    const ValueSet& s, const apache::thrift::protocol::Value& v);
void checkHomogeneousContainer(const ValueList& l);
void checkHomogeneousContainer(const ValueSet& s);
void checkHomogeneousContainer(const ValueMap& m);

/// Copy the next field in `in` to `out`
template <class Reader, bool _>
void forwardOneField(
    thrift::detail::StructuredDynamicCursorReader<Reader, _>& in,
    thrift::detail::StructuredDynamicCursorWriter<
        typename Reader::ProtocolWriter>& out) {
  auto id = in.fieldId();
  auto type = in.fieldType();
  if (type == protocol::TType::T_BOOL) {
    // readRaw won't work with bool in compact protocol
    out.write(id, type::bool_t{}, in.read(type::bool_t{}));
  } else {
    out.writeRaw(id, type, in.readRawCursor());
  }
}
} // namespace detail

class DynamicPatch;

class DynamicPatchBase {
 public:
  DynamicPatchBase() = default;
  DynamicPatchBase(const DynamicPatchBase&) = default;
  DynamicPatchBase(DynamicPatchBase&&) = default;
  DynamicPatchBase& operator=(const DynamicPatchBase&) = default;
  DynamicPatchBase& operator=(DynamicPatchBase&&) = default;

  void clear() {
    patch_.members()->clear();
    get(op::PatchOp::Clear).emplace_bool(true);
  }

  void fromObject(detail::Badge, Object obj) { patch_ = std::move(obj); }

  [[nodiscard]] Object toObject() const& {
    return DynamicPatchBase(*this).toObject();
  }

  [[nodiscard]] Object toObject() && {
    if (!options_.doNotConvertStringToBinary) {
      detail::convertStringToBinary(patch_);
    }
    return std::move(patch_);
  }

  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

  // The number of added/removed/replaced/patched elements in Patch, it will be
  // used to decide whether to use assign patch.
  [[nodiscard]] std::size_t patchedElementCount(detail::Badge) const;

  [[nodiscard]] bool empty(detail::Badge) const { return patch_.empty(); }

 protected:
  ~DynamicPatchBase() = default;

  [[nodiscard]] Value& get(op::PatchOp patchOp) {
    return patch_[static_cast<FieldId>(patchOp)];
  }

  [[nodiscard]] Value* get_ptr(op::PatchOp patchOp) {
    return folly::get_ptr(*patch_.members(), folly::to_underlying(patchOp));
  }

  [[nodiscard]] const Value* get_ptr(op::PatchOp patchOp) const {
    return folly::get_ptr(*patch_.members(), folly::to_underlying(patchOp));
  }

  Object patch_;
  detail::DynamicPatchOptions options_;
};

/// DynamicPatch for ambiguous type.
///
/// When DynamicPatch is created from serialized blob, there could be an
/// ambiguity to deduce the exact patch type. It consists of the
/// following operations: `assign`, `clear`, `patchIfSet`, `removeMulti`. See
/// DynamicUnknownPatch::Category for all possible scenarios.
///
/// If the exact patch type is known, `DynamicPatch::getStoredPatchByTag` can be
/// used to inform the exact patch type.
///
/// Good example:
///
/// DynamicPatch dynPatch; // patch with only `patchIfSet`.
/// EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
/// // Assert Category::StructuredPatch is union patch.
/// dynPatch.getStoredPatchByTag<type::union_c>();
///
///
/// Bad example:
///
/// DynamicPatch dynPatch; // patch with only `removeMulti`.
/// EXPECT_TRUE(dynPatch.isPatchTypeAmbiguous());
/// EXPECT_THROW(dynPatch.getStoredPatchByTag<type::union_c>());
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       // Category::StructuredOrAnyPatch
///       void assign(const Object&);
///       // Category::ClearPatch
///       void clear();
///       // Category::AssociativeContainerPatch
///       void removeMulti(const ValueSet&);
///       // Category::StructuredPatch
///       void patchIfSet(FieldId, const DynamicPatch&);
///     }
class DynamicUnknownPatch : public DynamicPatchBase {
 public:
  void assign(Object v);
  void removeMulti(ValueSet v);
  void patchIfSet(FieldId, const DynamicPatch&);

  void fromObject(detail::Badge badge, Object obj) {
    DynamicPatchBase::fromObject(badge, std::move(obj));
    validateAndGetCategory();
  }

  /// Checks if it is convertible to the patch type.
  template <class Patch>
  void checkConvertible() const {
    auto category = validateAndGetCategory();
    switch (category) {
      case Category::EmptyPatch:
        // Empty patch is compatible with any patch type.
        break;
      case Category::ClearPatch:
        // Clear patch is compatible with any patch type.
        break;
      case Category::StructuredPatch:
        // Structured patch is compatible with struct or union patch.
        if constexpr (
            !std::is_same_v<Patch, DynamicStructPatch> &&
            !std::is_same_v<Patch, DynamicUnionPatch>) {
          throwIncompatibleConversion();
        }
        break;
      case Category::StructuredOrAnyPatch:
        // Structured or any patch is compatible with struct, union, or any
        // patch.
        if constexpr (
            !std::is_same_v<Patch, DynamicStructPatch> &&
            !std::is_same_v<Patch, DynamicUnionPatch> &&
            !std::is_same_v<Patch, op::AnyPatch>) {
          throwIncompatibleConversion();
        }
        break;
      case Category::AssociativeContainerPatch:
        // Associative container patch is compatible with set or map patch.
        if constexpr (
            !std::is_same_v<Patch, DynamicSetPatch> &&
            !std::is_same_v<Patch, DynamicMapPatch>) {
          throwIncompatibleConversion();
        }
        break;
    }
  }

  protocol::ExtractedMasksFromPatch extractMaskFromPatch() const;

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&&, Visitor&&);

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, Value&) const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicUnknownPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }

 private:
  // The unknown patch can be classified into the following categories
  enum class Category {
    // The patch has no operation. It can be any kinds of patch.
    EmptyPatch,

    // The patch only has `clear` operation. It can be any kinds of patch.
    ClearPatch,

    // The patch has `patchPrior/patchAfter`. It can be a struct patch or
    // union patch.
    StructuredPatch,

    // The patch only has `assign` of Object. It can be a struct, union, or
    // any patch.
    StructuredOrAnyPatch,

    // The patch only has `remove` operation. It can be a set or map patch.
    AssociativeContainerPatch
  };

  Category validateAndGetCategory() const;

  [[noreturn]] void throwIncompatibleConversion() const;
  [[noreturn]] void throwIncompatibleCategory(std::string_view method) const;

  bool isOneOfCategory(std::initializer_list<Category> c) const {
    return std::find(c.begin(), c.end(), validateAndGetCategory()) != c.end();
  }
};

/// DynamicPatch for a Thrift list.
///
/// It consists of the following operations: `assign`, `clear`, and
/// `push_back`.
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       void assign(const ValueList&);
///       void clear();
///       void push_back(const Value&);
///     }
class DynamicListPatch : public DynamicPatchBase {
 public:
  using DynamicPatchBase::DynamicPatchBase;
  void assign(ValueList v) {
    checkHomogeneousContainer(v);
    patch_.members()->clear();
    get(op::PatchOp::Assign).emplace_list(std::move(v));
  }

  [[deprecated("Use append(value) instead")]]
  void push_back(Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      detail::checkCompatibleType(assign->as_list(), v);
      assign->as_list().push_back(std::move(v));
      return;
    }
    auto& l = get(op::PatchOp::Put).ensure_list();
    detail::checkCompatibleType(l, v);
    l.push_back(std::move(v));
  }

  void append(Value v) { push_back(std::move(v)); }
  void appendMulti(ValueList v) {
    for (auto&& i : v) {
      append(std::move(i));
    }
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v) {
    if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
      std::forward<Visitor>(v).assign(
          folly::forward_like<Self>(assign->as_list()));
      return;
    }

    if (auto clear = self.get_ptr(op::PatchOp::Clear);
        clear && clear->as_bool()) {
      std::forward<Visitor>(v).clear();
    }

    if (auto put = self.get_ptr(op::PatchOp::Put)) {
      for (auto& i : put->as_list()) {
        std::forward<Visitor>(v).push_back(folly::forward_like<Self>(i));
      }
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, ValueList& v) const;

  protocol::ExtractedMasksFromPatch extractMaskFromPatch() const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicListPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }
};

/// DynamicPatch for a Thrift set.
///
/// It consists of the following operations: `assign`, `clear`, `addMulti`, and
/// `removeMulti`.
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       void assign(const ValueSet&);
///       void clear();
///       void removeMulti(const ValueSet&);
///       void addMulti(const ValueSet&);
///     }
class DynamicSetPatch : public DynamicPatchBase {
 public:
  using DynamicPatchBase::DynamicPatchBase;
  void assign(ValueSet v) {
    detail::checkHomogeneousContainer(v);
    patch_.members()->clear();
    get(op::PatchOp::Assign).emplace_set(std::move(v));
  }

  void insert(Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      detail::checkCompatibleType(assign->as_set(), v);
      assign->as_set().insert(std::move(v));
      return;
    }
    if (auto remove = get_ptr(op::PatchOp::Remove)) {
      remove->as_set().erase(v);
    }
    auto& s = get(op::PatchOp::Add).ensure_set();
    detail::checkCompatibleType(s, v);
    s.insert(std::move(v));
  }

  void erase(Value v) {
    if (auto assign = get_ptr(op::PatchOp::Assign)) {
      assign->as_set().erase(v);
      return;
    }
    if (auto add = get_ptr(op::PatchOp::Add)) {
      add->as_set().erase(v);
    }
    get(op::PatchOp::Remove).ensure_set().insert(std::move(v));
  }

  void addMulti(ValueSet add) {
    add.eraseInto(
        add.begin(), add.end(), [&](auto&& k) { insert(std::move(k)); });
  }

  void removeMulti(ValueSet remove) {
    remove.eraseInto(
        remove.begin(), remove.end(), [&](auto&& k) { erase(std::move(k)); });
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v) {
    if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
      std::forward<Visitor>(v).assign(
          folly::forward_like<Self>(assign->as_set()));
      return;
    }

    if (auto clear = self.get_ptr(op::PatchOp::Clear);
        clear && clear->as_bool()) {
      std::forward<Visitor>(v).clear();
    }

    if (auto remove = self.get_ptr(op::PatchOp::Remove)) {
      std::forward<Visitor>(v).removeMulti(
          folly::forward_like<Self>(remove->as_set()));
    }

    if (auto add = self.get_ptr(op::PatchOp::Add)) {
      std::forward<Visitor>(v).addMulti(
          folly::forward_like<Self>(add->as_set()));
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(detail::Badge, ValueSet& v) const;

  protocol::ExtractedMasksFromPatch extractMaskFromPatch() const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicSetPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }
};

/// Schema-less Patch.
///
/// When Static Patch is unavailable, Dynamic Patch offers a schema-less patch
/// apply, merge, and introspection (requires advanced knowledge of Thrift
/// Patch).
class DynamicPatch {
 public:
  /// @cond
  DynamicPatch();
  DynamicPatch(const DynamicPatch&);
  DynamicPatch& operator=(const DynamicPatch&);
  DynamicPatch(DynamicPatch&&) noexcept;
  DynamicPatch& operator=(DynamicPatch&&) noexcept;
  ~DynamicPatch();
  /// @endcond

  template <class T>
  explicit DynamicPatch(T t) : patch_(std::make_unique<Patch>(std::move(t))) {}

  /// Convert DynamicPatch to Protocol Object
  Object toObject() &&;
  Object toObject() const&;

  /// Returns if the patch is no-op.
  [[nodiscard]] bool empty() const;

  /// Applies the patch to the given value. Throws if the patch is not
  /// applicable to the value.
  void apply(Value&) const;
  /// Applies the patch to the given Thrift Any. Throws if the patch is not
  /// applicable.
  void applyToDataFieldInsideAny(type::AnyStruct&) const;
  /// @brief Applies the patch to the given blob and returns the result as a
  /// blob. Throws if the patch is not applicable.
  ///
  /// Note, this method uses introspection API `extractMaskFromPatch` to avoid
  /// full deserialization when applying Thrift Patch to serialized structured
  /// object in a blob.
  template <type::StandardProtocol Protocol>
  std::unique_ptr<folly::IOBuf> applyToSerializedObject(
      const folly::IOBuf& buf) const;

  /// The behavior is identical to applyToSerializedObject(...), though
  /// sometimes it's faster to deserialize the whole iobuf then apply patches.
  template <type::StandardProtocol Protocol>
  std::unique_ptr<folly::IOBuf> applyToSerializedObjectWithoutExtractingMask(
      const folly::IOBuf& buf) const;

  /// Converts SafePatch stored in Thrift Any to DynamicPatch.
  [[nodiscard]] static DynamicPatch fromSafePatch(const type::AnyStruct& any);
  /// Stores DynamicPatch as SafePatch in Thrift Any with the provided type
  /// using CompactProtocol.
  type::AnyStruct toSafePatch(type::Type type) const;

  /// Merges another patch into this patch. After the merge
  /// (`patch.merge(next)`), `patch.apply(value)` is equivalent to
  /// `next.apply(patch.apply(value))`.
  template <class Other>
  detail::if_same_type_after_remove_cvref<Other, DynamicPatch> merge(Other&&);

  /// Convert Patch stored in Protocol Object to DynamicPatch.
  [[nodiscard]] static DynamicPatch fromObject(Object);

  /// Retrieves the stored patch by the specified tag. Can be used to assert the
  /// type of DynamicUnknownPatch. Throws if the patch cannot be retrieved with
  /// the specified tag.
  template <class Tag>
  auto& getStoredPatchByTag() {
    return getStoredPatchByTag(Tag{});
  }

  /// Returns if the patch type is ambiguous.
  [[nodiscard]] bool isPatchTypeAmbiguous() const;

  /// @brief Constructs read and write Thrift Masks that only contain fields
  /// that are read / written to respectively by the Patch. The read mask
  /// specifies entries that must be known in advance to correctly apply Thrift
  /// Patch. The write mask specifies entries that are potentially affected by
  /// the patch.
  ///
  /// It constructs nested Mask for map, struct, union, and any patches. For
  /// map, it only supports integer or string key. If the type of key map is not
  /// integer or string, it throws.
  ///
  /// Disclaimer: Mask-extraction only guarentees recall (i.e. all fields that
  /// are read/written to *will* be selected). However, it provides only a
  /// best-effort guarentee of precision (selected fields are not guarenteed to
  /// be relevant to a given patch). If high precision is important, the user is
  /// advised to use visitPatch or customVisit to introspect the patch directly.
  ExtractedMasksFromPatch extractMaskFromPatch() const;

  /// Convert Patch stored in Thrift Any to DynamicPatch.
  [[nodiscard]] static DynamicPatch fromPatch(const type::AnyStruct& any);
  /// Stores DynamicPatch as Patch in Thrift Any with the provided type
  /// using CompactProtocol.
  type::AnyStruct toPatch(type::Type type) const;

  /// @cond
  template <typename Protocol>
  std::uint32_t encode(Protocol& prot) const;
  template <typename Protocol>
  std::unique_ptr<folly::IOBuf> encode() const;

  template <typename Protocol>
  void decode(Protocol& prot);
  template <typename Protocol>
  void decode(const folly::IOBuf& buf);

  template <class T>
  bool holds_alternative(detail::Badge) const {
    return std::get_if<T>(patch_.get()) != nullptr;
  }

  template <class PatchType>
  PatchType* get_if() {
    return std::get_if<PatchType>(patch_.get());
  }
  template <class PatchType>
  const PatchType* get_if() const {
    return std::get_if<PatchType>(patch_.get());
  }

  template <class PatchType>
  PatchType& get() {
    // std::unique_ptr::operator* requires complete type.
    return std::get<PatchType>(*patch_.get());
  }
  template <class PatchType>
  const PatchType& get() const {
    return std::get<PatchType>(*patch_.get());
  }

  /// @endcond

  template <class Reader, bool _>
  void applyOneFieldInStream(
      detail::Badge badge,
      thrift::detail::StructuredDynamicCursorReader<Reader, _>& in,
      thrift::detail::StructuredDynamicCursorWriter<
          typename Reader::ProtocolWriter>& out) const;

 private:
  DynamicListPatch& getStoredPatchByTag(type::list_c);
  DynamicSetPatch& getStoredPatchByTag(type::set_c);
  DynamicMapPatch& getStoredPatchByTag(type::map_c);
  DynamicStructPatch& getStoredPatchByTag(type::struct_c);
  DynamicUnionPatch& getStoredPatchByTag(type::union_c);
  template <class T, class Tag>
  auto& getStoredPatchByTag(type::cpp_type<T, Tag>) {
    return getStoredPatchByTag(Tag{});
  }
  op::I32Patch& getStoredPatchByTag(type::enum_c);
  op::AnyPatch& getStoredPatchByTag(type::struct_t<type::AnyStruct>);
  template <class Tag>
  std::enable_if_t<type::is_a_v<Tag, type::primitive_c>, op::patch_type<Tag>&>
  getStoredPatchByTag(Tag) {
    return getStoredPatch<op::patch_type<Tag>>();
  }

  template <class Patch>
  Patch& getStoredPatch() {
    // patch already has the correct type, return it directly.
    if (auto p = get_if<Patch>()) {
      return *p;
    }

    // Use merge to change patch's type.
    merge(DynamicPatch{Patch{}});
    return get<Patch>();
  }

  template <class Self, class Visitor>
  static decltype(auto) visitPatchImpl(Self&& self, Visitor&& visitor) {
    return std::visit(
        std::forward<Visitor>(visitor), *std::forward<Self>(self).patch_);
  }

  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, detail::Badge badge, Visitor&& v) {
    std::forward<Self>(self).visitPatch([&](auto&& patch) {
      using PatchType = folly::remove_cvref_t<decltype(patch)>;
      if constexpr (requires {
                      std::forward<PatchType>(patch).customVisit(
                          badge, std::forward<Visitor>(v));
                    }) {
        std::forward<PatchType>(patch).customVisit(
            badge, std::forward<Visitor>(v));
      } else {
        std::forward<PatchType>(patch).customVisit(std::forward<Visitor>(v));
      }
    });
  }

  // A SafePatch storage for DynamicPatch.
  class DynamicSafePatch {
   public:
    DynamicSafePatch() = default;
    DynamicSafePatch(std::int32_t version, std::unique_ptr<folly::IOBuf> data)
        : version_(version), data_(std::move(data)) {}
    DynamicSafePatch(const DynamicSafePatch&) = delete;
    DynamicSafePatch& operator=(const DynamicSafePatch&) = delete;
    DynamicSafePatch(DynamicSafePatch&&) = default;
    DynamicSafePatch& operator=(DynamicSafePatch&&) = default;
    ~DynamicSafePatch() = default;

    template <typename Protocol>
    std::uint32_t encode(Protocol& prot) const;
    template <typename Protocol>
    void decode(Protocol& prot);
    template <typename Protocol>
    std::unique_ptr<folly::IOBuf> encode() const {
      folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
      Protocol prot;
      prot.setOutput(&queue);
      encode(prot);
      return queue.move();
    }
    template <typename Protocol>
    void decode(const folly::IOBuf& buf) {
      Protocol prot;
      prot.setInput(&buf);
      return decode(prot);
    }

    std::int32_t version() const { return version_; }
    const std::unique_ptr<folly::IOBuf>& data() const { return data_; }

   private:
    std::int32_t version_;
    std::unique_ptr<folly::IOBuf> data_;
  };

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      visitPatch, visitPatchImpl);

  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

 private:
  using Patch = std::variant<
      DynamicUnknownPatch,
      op::BoolPatch,
      op::BytePatch,
      op::I16Patch,
      op::I32Patch,
      op::I64Patch,
      op::FloatPatch,
      op::DoublePatch,
      op::StringPatch,
      op::BinaryPatch,
      op::AnyPatch,
      DynamicListPatch,
      DynamicSetPatch,
      DynamicMapPatch,
      DynamicStructPatch,
      DynamicUnionPatch>;
  std::unique_ptr<Patch> patch_;
};

/// DynamicPatch for a Thrift map.
///
/// It consists of the following operations: `assign`, `clear`, `putMulti`,
/// `tryPutMulti`, `removeMulti`, and `patchByKey`.
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       void assign(const ValueMap&);
///       void clear();
///       void putMulti(const ValueMap&);
///       void tryPutMulti(const ValueMap&);
///       void removeMulti(const ValueSet&);
///       void patchByKey(const Value&, const DynamicPatch&);
///     }
class DynamicMapPatch {
 public:
  void assign(ValueMap v) {
    detail::checkHomogeneousContainer(v);
    *this = {};
    setOrCheckMapType(v);
    assign_ = std::move(v);
  }

  void clear() {
    *this = {};
    clear_ = true;
  }

  void insert_or_assign(Value k, Value v);
  void erase(Value k);

  void tryPutMulti(ValueMap v);
  void removeMulti(ValueSet v) {
    v.eraseInto(v.begin(), v.end(), [&](auto&& k) { erase(std::move(k)); });
  }
  void putMulti(ValueMap m) {
    detail::checkHomogeneousContainer(m);
    m.eraseInto(m.begin(), m.end(), [&](auto&& k, auto&& v) {
      insert_or_assign(std::move(k), std::move(v));
    });
  }

  // Return the subPatch. We can use it to provide similar APIs to static patch.
  DynamicPatch& patchByKey(Value k, const DynamicPatch& p) {
    return patchByKeyImpl(std::move(k), p);
  }
  DynamicPatch& patchByKey(Value k, DynamicPatch&& p) {
    return patchByKeyImpl(std::move(k), std::move(p));
  }

  DynamicPatch& patchByKey(Value&&);
  DynamicPatch& patchByKey(const Value&);

  [[nodiscard]] Object toObject() &&;
  [[nodiscard]] Object toObject() const&;
  void fromObject(detail::Badge, Object obj);

  [[nodiscard]] std::size_t patchedElementCount(detail::Badge) const {
    return add_.size() + remove_.size() + put_.size() + patchPrior_.size() +
        patchAfter_.size();
  }

  [[nodiscard]] bool empty(detail::Badge) const {
    return !assign_ && !clear_ && add_.empty() && remove_.empty() &&
        put_.empty() && patchPrior_.empty() && patchAfter_.empty();
  }

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v);

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);
  void apply(detail::Badge, ValueMap& v) const;

  protocol::ExtractedMasksFromPatch extractMaskFromPatch() const;

  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicMapPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }

  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

 private:
  void undoChanges(const Value& k);
  void ensurePatchable();
  void setOrCheckMapType(const protocol::Value& k, const protocol::Value& v);
  void setOrCheckMapType(const ValueMap& m) {
    if (!m.empty()) {
      setOrCheckMapType(m.begin()->first, m.begin()->second);
    }
  }

  template <class SubPatch>
  DynamicPatch& patchByKeyImpl(Value k, SubPatch&& p);

  std::optional<ValueMap> assign_;
  bool clear_ = false;
  folly::F14VectorMap<Value, DynamicPatch> patchPrior_;
  ValueMap add_;
  folly::F14VectorMap<Value, DynamicPatch> patchAfter_;
  ValueSet remove_;
  ValueMap put_;
  detail::DynamicPatchOptions options_;

  std::optional<detail::Value::Type> keyType_;
  std::optional<detail::Value::Type> valueType_;
};

extern template DynamicPatch& DynamicMapPatch::patchByKeyImpl(
    Value k, const DynamicPatch& p);

extern template DynamicPatch& DynamicMapPatch::patchByKeyImpl(
    Value k, DynamicPatch&& p);

template <bool IsUnion>
class DynamicStructurePatch {
 public:
  void assign(Object v) {
    *this = {};
    assign_ = std::move(v);
  }

  void clear() {
    *this = {};
    clear_ = true;
  }

  void remove(FieldId id) {
    undoChanges(id);
    remove_.insert(id);
  }

  void ensure(FieldId id, Value v) {
    return IsUnion ? ensureUnion(id, std::move(v))
                   : ensureStruct(id, std::move(v));
  }

  // patchIfSet
  template <class Tag>
  auto& patchIfSet(FieldId id) {
    auto& subPatch = patchIfSet(id);
    return subPatch.template getStoredPatchByTag<Tag>();
  }

  DynamicPatch& patchIfSet(FieldId id) {
    ensurePatchable();
    return ensure_.contains(id) ? patchAfter_[id] : patchPrior_[id];
  }

  [[nodiscard]] bool empty(detail::Badge) const {
    return !assign_ && !clear_ && remove_.empty() && ensure_.empty() &&
        patchPrior_.empty() && patchAfter_.empty();
  }

  [[nodiscard]] Object toObject() &&;
  [[nodiscard]] Object toObject() const& {
    return DynamicStructurePatch(*this).toObject();
  }
  void fromObject(detail::Badge, Object obj);
  void doNotConvertStringToBinary(detail::Badge) {
    options_.doNotConvertStringToBinary = true;
  }

  bool modifies(detail::Badge, FieldId id) const {
    return assign_ || clear_ || patchPrior_.contains(id) ||
        ensure_.contains(id) || patchAfter_.contains(id) ||
        remove_.contains(id);
  }

  // Ensures and patches fields with assign operation from the given object.
  void ensureAndAssignFieldsFromObject(Object obj);

 private:
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v);

  // Needed for merge(...). We can consider making this a public API.
  template <class SubPatch>
  void patchIfSet(FieldId id, SubPatch&& patch) {
    patchIfSet(id).merge(std::forward<SubPatch>(patch));
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);
  void apply(detail::Badge, Object& obj) const;

  /// @brief Applies the patch to the given blob and returns the result as a
  /// blob. Throws if the patch is not applicable.
  template <type::StandardProtocol Protocol>
  std::unique_ptr<folly::IOBuf> applyToSerializedObject(
      std::unique_ptr<folly::IOBuf> buf) const;

  template <class Reader, bool _>
  void applyAllFieldsInStream(
      detail::Badge badge,
      thrift::detail::StructuredDynamicCursorReader<Reader, _>& in,
      thrift::detail::StructuredDynamicCursorWriter<
          typename Reader::ProtocolWriter>& out) const {
    static_assert(
        (folly::always_false<Reader> || IsUnion) == false,
        "apply to union stream is not implemented");
    if (assign_) {
      for (auto&& [k, v] : *assign_) {
        out.writeValue(k, v);
      }
      return;
    }

    // TODO: optimize it by using folly::sorted_vector_set
    folly::F14FastSet<FieldId> processed;

    if (!clear_) {
      while (in.fieldType() != protocol::TType::T_STOP) {
        auto id = static_cast<FieldId>(in.fieldId());
        processed.insert(id);
        if (remove_.contains(id)) {
          in.skip();
          continue;
        }

        auto prior = folly::get_ptr(patchPrior_, id);
        auto after = folly::get_ptr(patchAfter_, id);

        if (!prior && !after) {
          // The field is not modified. Forward the raw bytes to output.
          detail::forwardOneField(in, out);
          continue;
        }

        if (!prior) {
          after->applyOneFieldInStream(badge, in, out);
          continue;
        }

        if (!after) {
          prior->applyOneFieldInStream(badge, in, out);
          continue;
        }

        // Fallback to protocol::Value
        auto value = in.readValue();
        prior->apply(value);
        after->apply(value);
        out.writeValue(folly::to_underlying(id), value);
      }
    }

    for (const auto& [id, v] : ensure_) {
      if (processed.contains(id)) {
        continue;
      }

      if (remove_.contains(id)) {
        throw std::runtime_error(
            "Patch that deleted ensured id: " +
            std::to_string(folly::to_underlying(id)));
      }

      auto value = v;
      if (auto p = folly::get_ptr(patchAfter_, id)) {
        p->apply(value);
      }
      out.writeValue(folly::to_underlying(id), value);
    }
  }

 private:
  void undoChanges(FieldId);
  void ensurePatchable();

  void ensureStruct(FieldId, Value);
  void ensureUnion(FieldId, Value);

 private:
  std::optional<Object> assign_;
  bool clear_ = false;
  folly::F14VectorMap<FieldId, DynamicPatch> patchPrior_;
  folly::F14FastMap<FieldId, Value> ensure_;
  folly::F14VectorMap<FieldId, DynamicPatch> patchAfter_;
  folly::F14FastSet<FieldId> remove_;
  detail::DynamicPatchOptions options_;
};

/// DynamicPatch for a Thrift struct.
///
/// It consists of the following operations: `assign`, `clear`, `patchIfSet`,
/// `ensure`, `patch`, and `remove`.
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       void assign(const Object&);
///       void clear();
///       void ensure(FieldId, const Value&);
///       void patchIfSet(FieldId, const DynamicPatch&);
///       void remove(FieldId);
///     }
class DynamicStructPatch : public DynamicStructurePatch<false> {
 public:
  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicStructPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }

  ExtractedMasksFromPatch extractMaskFromPatch() const;
};

/// DynamicPatch for a Thrift union.
///
/// It consists of the following operations: `assign`, `clear`, `patchIfSet`,
/// `ensure`, and `patch`.
///
/// For introspection, users should provide a visitor with the following methods
/// for `customVisit`:
///
///     struct Visitor {
///       void assign(const Object&);
///       void clear();
///       void ensure(FieldId, const Value&);
///       void patchIfSet(FieldId, const DynamicPatch&);
///     }
class DynamicUnionPatch : public DynamicStructurePatch<true> {
 public:
  template <class Next>
  detail::if_same_type_after_remove_cvref<Next, DynamicUnionPatch> merge(
      detail::Badge, Next&& other) {
    std::forward<Next>(other).customVisit(*this);
  }

  ExtractedMasksFromPatch extractMaskFromPatch() const;

 private:
  // Hide `remove` operation since we can't remove field from union
  using DynamicStructurePatch::remove;
};

template <class Self, class Visitor>
void DynamicUnknownPatch::customVisitImpl(Self&& self, Visitor&& v) {
  // We don't need to check EnsureStruct and EnsureUnion fields since they won't
  // exists -- otherwise we are able to identify the exact patch type and it
  // should not be unknown patch.
  if (auto assign = self.get_ptr(op::PatchOp::Assign)) {
    std::forward<Visitor>(v).assign(
        folly::forward_like<Self>(assign->as_object()));
    return;
  }

  if (auto clear = self.get_ptr(op::PatchOp::Clear);
      clear && clear->as_bool()) {
    std::forward<Visitor>(v).clear();
  }

  for (auto op : {op::PatchOp::PatchPrior, op::PatchOp::PatchAfter}) {
    if (auto subPatch = self.get_ptr(op)) {
      for (auto& [fieldId, fieldPatch] : subPatch->as_object()) {
        std::forward<Visitor>(v).patchIfSet(
            static_cast<FieldId>(fieldId),
            DynamicPatch::fromObject(
                folly::forward_like<Self>(fieldPatch.as_object())));
      }
    }
  }

  if (auto remove = self.get_ptr(op::PatchOp::Remove)) {
    std::forward<Visitor>(v).removeMulti(
        folly::forward_like<Self>(remove->as_set()));
  }
}

template <class Self, class Visitor>
void DynamicMapPatch::customVisitImpl(Self&& self, Visitor&& v) {
  if (self.assign_) {
    std::forward<Visitor>(v).assign(*std::forward<Self>(self).assign_);
    return;
  }

  if (self.clear_) {
    std::forward<Visitor>(v).clear();
  }

  for (auto& [k, p] : self.patchPrior_) {
    std::forward<Visitor>(v).patchByKey(k, folly::forward_like<Self>(p));
  }

  std::forward<Visitor>(v).tryPutMulti(std::forward<Self>(self).add_);
  std::forward<Visitor>(v).removeMulti(std::forward<Self>(self).remove_);
  std::forward<Visitor>(v).putMulti(std::forward<Self>(self).put_);

  for (auto& [k, p] : self.patchAfter_) {
    std::forward<Visitor>(v).patchByKey(k, folly::forward_like<Self>(p));
  }
}

template <bool IsUnion>
template <class Self, class Visitor>
void DynamicStructurePatch<IsUnion>::customVisitImpl(Self&& self, Visitor&& v) {
  if (self.assign_) {
    std::forward<Visitor>(v).assign(*std::forward<Self>(self).assign_);
    return;
  }

  if (self.clear_) {
    std::forward<Visitor>(v).clear();
  }

  for (auto& [id, p] : self.patchPrior_) {
    std::forward<Visitor>(v).patchIfSet(id, folly::forward_like<Self>(p));
  }

  for (auto& [id, value] : self.ensure_) {
    std::forward<Visitor>(v).ensure(id, folly::forward_like<Self>(value));
  }

  for (auto& [id, p] : self.patchAfter_) {
    std::forward<Visitor>(v).patchIfSet(id, folly::forward_like<Self>(p));
  }

  if constexpr (!IsUnion) {
    for (const auto& id : self.remove_) {
      std::forward<Visitor>(v).remove(id);
    }
  }
}

template <class Reader, bool _>
void DynamicPatch::applyOneFieldInStream(
    detail::Badge badge,
    thrift::detail::StructuredDynamicCursorReader<Reader, _>& in,
    thrift::detail::StructuredDynamicCursorWriter<
        typename Reader::ProtocolWriter>& out) const {
  const auto id = in.fieldId();
  auto byTag = [&](auto& patch, auto tag) {
    auto value = in.read(tag);
    patch.apply(value);
    out.write(id, tag, std::move(value));
  };
  auto fallback = [&](auto&) {
    auto value = in.readValue();
    apply(value);
    out.writeValue(id, value);
  };
  auto applier = folly::overload(
      [&](const op::BoolPatch& patch) { byTag(patch, type::bool_t{}); },
      [&](const op::BytePatch& patch) { byTag(patch, type::byte_t{}); },
      [&](const op::I16Patch& patch) { byTag(patch, type::i16_t{}); },
      [&](const op::I32Patch& patch) { byTag(patch, type::i32_t{}); },
      [&](const op::I64Patch& patch) { byTag(patch, type::i64_t{}); },
      [&](const op::FloatPatch& patch) { byTag(patch, type::float_t{}); },
      [&](const op::DoublePatch& patch) { byTag(patch, type::double_t{}); },
      [&](const op::StringPatch& patch) { byTag(patch, type::string_t{}); },
      [&](const op::BinaryPatch& patch) { byTag(patch, type::binary_t{}); },
      [&](const DynamicStructPatch& patch) {
        auto reader = in.beginReadStructured();
        auto writer = out.beginWriteStructured(id);
        patch.applyAllFieldsInStream(badge, reader, writer);
        in.endRead(std::move(reader));
        out.endWrite(std::move(writer));
      },
      fallback);
  visitPatch(applier);
}

class DiffVisitorBase {
 public:
  [[nodiscard]] DynamicPatch diff(const Object& src, const Object& dst);
  [[nodiscard]] DynamicPatch diff(
      detail::Badge, const Value& src, const Value& dst);
  [[nodiscard]] DynamicPatch diff(const Value& src, const Value& dst);

  virtual ~DiffVisitorBase() = default;

 protected:
  virtual op::BoolPatch diffBool(bool, bool dst);
  [[nodiscard]] virtual op::BytePatch diffByte(
      std::int8_t src, std::int8_t dst);
  [[nodiscard]] virtual op::I16Patch diffI16(
      std::int16_t src, std::int16_t dst);
  [[nodiscard]] virtual op::I32Patch diffI32(
      std::int32_t src, std::int32_t dst);
  [[nodiscard]] virtual op::I64Patch diffI64(
      std::int64_t src, std::int64_t dst);
  [[nodiscard]] virtual op::FloatPatch diffFloat(float src, float dst);
  [[nodiscard]] virtual op::DoublePatch diffDouble(double src, double dst);
  [[nodiscard]] virtual op::BinaryPatch diffBinary(
      const folly::IOBuf& src, const folly::IOBuf& dst);
  [[nodiscard]] virtual DynamicListPatch diffList(
      const ValueList& src, const ValueList& dst);
  [[nodiscard]] virtual DynamicSetPatch diffSet(
      const ValueSet& src, const ValueSet& dst);
  [[nodiscard]] virtual DynamicMapPatch diffMap(
      const ValueMap& src, const ValueMap& dst);
  [[nodiscard]] virtual DynamicPatch diffStructured(
      const Object& src, const Object& dst);

 protected:
  [[nodiscard]] DynamicUnionPatch diffUnion(
      const Object& src, const Object& dst);
  [[nodiscard]] virtual op::AnyPatch diffAny(
      const Object& src, const Object& dst);
  void diffField(
      const Object& src,
      const Object& dst,
      FieldId id,
      DynamicStructPatch& patch);
  void diffElement(
      const ValueMap& src,
      const ValueMap& dst,
      const Value& key,
      DynamicMapPatch& patch);

  static DynamicPatch createDynamicUnknownPatchWithAssign(const Object& obj);

  void pushField(FieldId);
  void pushType(type::Type);
  void pushKey(const Value&);
  void pop();

  const Mask& getCurrentPath() const { return path_; }

 private:
  Mask path_ = allMask();

  // Keep tracking the stack of mask in path_ for optimization.
  std::stack<Mask*> maskInPath_{{&path_}};
};

namespace detail {
std::string toSafePatchUri(std::string s);
std::string fromSafePatchUri(std::string s);
} // namespace detail

/// @cond
// Convert a normal struct uri to patch uri
std::string toPatchUri(std::string s);
std::string fromPatchUri(std::string s);
/// @endcond

/// Convert a struct/union type to SafePatch type.
type::Type toSafePatchType(type::Type input);
/// Convert SafePatch type to struct/union type.
type::Type fromSafePatchType(type::Type input, bool isUnion);

/// Convert struct/union type to Patch type.
/// Currently, it does not support primitive types (e.g., i32 --> I32Patch).
type::Type toPatchType(type::Type input);
/// Convert Patch type to struct/union type.
/// Currently, it does not support primitive types (e.g., I32Patch --> i32).
type::Type fromPatchType(type::Type input, bool isUnion);

} // namespace apache::thrift::protocol
