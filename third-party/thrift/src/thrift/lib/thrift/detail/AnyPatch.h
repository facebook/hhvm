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

#include <vector>
#include <fmt/format.h>
#include <folly/container/F14Map.h>
#include <folly/lang/Badge.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/op/detail/StructPatch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_detail_types.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache::thrift::protocol::detail {
// Use Badge pattern to control access.
// See thrift/lib/cpp2/patch/detail/PatchBadge.h
struct PatchBadgeFactory;
using Badge = folly::badge<PatchBadgeFactory>;
} // namespace apache::thrift::protocol::detail

namespace apache::thrift::op {
namespace detail {

template <typename Patch>
class AnyPatch;

} // namespace detail

class AnyPatchStruct;
class TypeToPatchInternalDoNotUse;
class AnyPatchStruct;
class AnySafePatch;

namespace detail {
[[noreturn]] void throwDuplicatedType(const type::Type& type);
[[noreturn]] void throwTypeNotValid(const type::Type& type);
[[noreturn]] void throwAnyNotValid(const type::AnyStruct& any);
[[noreturn]] void throwUnsupportedAnyProtocol(const type::AnyStruct& any);

inline void throwIfInvalidOrUnsupportedAny(const type::AnyStruct& any) {
  if (!type::AnyData::isValid(any)) {
    throwAnyNotValid(any);
  }
  if (any.protocol() != type::Protocol::get<type::StandardProtocol::Binary>() &&
      any.protocol() !=
          type::Protocol::get<type::StandardProtocol::Compact>()) {
    throwUnsupportedAnyProtocol(any);
  }
}

struct TypeToPatchMapAdapter {
  using StandardType = std::vector<TypeToPatchInternalDoNotUse>;
  using AdaptedType = folly::F14FastMap<type::Type, protocol::DynamicPatch>;

  static AdaptedType fromThrift(StandardType&& vec);
  static StandardType toThrift(const AdaptedType& map);

  template <typename Tag, typename Protocol>
  static uint32_t encode(Protocol& prot, const AdaptedType& map) {
    uint32_t s = 0;
    s += prot.writeListBegin(protocol::TType::T_STRUCT, map.size());
    for (const auto& [type, patch] : map) {
      s += prot.writeStructBegin(
          op::get_class_name_v<TypeToPatchInternalDoNotUse>.data());
      s += prot.writeFieldBegin("type", protocol::TType::T_STRUCT, 1);
      s += op::encode<type::infer_tag<type::Type>>(prot, type);
      s += prot.writeFieldEnd();
      s += prot.writeFieldBegin("patch", protocol::TType::T_STRUCT, 2);
      s +=
          op::encode<type::struct_t<type::AnyStruct>>(prot, toAny(patch, type));
      s += prot.writeFieldEnd();
      s += prot.writeFieldStop();
      s += prot.writeStructEnd();
    }
    s += prot.writeListEnd();
    return s;
  }

  template <typename Tag, typename Protocol>
  static void decode(Protocol& prot, AdaptedType& map) {
    protocol::TType t;
    uint32_t s;
    prot.readListBegin(t, s);
    if (t != protocol::TType::T_STRUCT) {
      while (s--) {
        prot.skip(t);
      }
    } else {
      while (s--) {
        TypeToPatchInternalDoNotUse typeToPatchStruct;
        op::decode<type::struct_t<TypeToPatchInternalDoNotUse>>(
            prot, typeToPatchStruct);
        if (!addDynamicPatchToMap(map, typeToPatchStruct)) {
          throwDuplicatedType(typeToPatchStruct.type().value());
        }
      }
    }
    prot.readListEnd();
  }

  static bool equal(const AdaptedType&, const AdaptedType&);
  static bool isEmpty(const AdaptedType& map) { return map.empty(); }

 private:
  static bool addDynamicPatchToMap(
      AdaptedType&, const TypeToPatchInternalDoNotUse&);

  static type::AnyStruct toAny(
      const protocol::DynamicPatch&, const type::Type&);
};

/// Patch for Thrift Any.
/// * `optional AnyStruct assign`
/// * `terse bool clear`
/// * `terse map<Type, DynamicPatch> patchIfTypeIsPrior`
/// * `optional AnyStruct ensureAny`
/// * `terse map<Type, DynamicPatch> patchIfTypeIsAfter`
template <typename Patch>
class AnyPatch : public BaseClearPatch<Patch, AnyPatch<Patch>> {
  using Base = BaseClearPatch<Patch, AnyPatch>;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using Base::assign;
  using Base::clear;

 private:
  /// @copybrief StructPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const AnyStruct&);
  ///       void clear();
  ///       void patchIfTypeIs(const Type&, const DynamicPatch&);
  ///       void ensureAny(const AnyStruct&);
  ///     }
  ///
  template <typename Self, typename Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v) {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(type::AnyStruct{});
      v.clear();
      v.patchIfTypeIs(type::Type{}, protocol::DynamicPatch{});
      v.ensureAny(type::AnyStruct{});
    }
    if (!std::forward<Self>(self).customVisitAssignAndClear(
            std::forward<Visitor>(v))) {
      // patchIfTypeIsPrior
      for (auto&& [type, patch] :
           std::forward<Self>(self).data_.patchIfTypeIsPrior().value()) {
        std::forward<Visitor>(v).patchIfTypeIs(
            folly::forward_like<Self>(type), folly::forward_like<Self>(patch));
      }

      // ensureAny
      if (self.data_.ensureAny().has_value()) {
        std::forward<Visitor>(v).ensureAny(
            std::forward<Self>(self).data_.ensureAny().value());
      }

      // patchIfTypeIsAfter
      for (auto&& [type, patch] :
           std::forward<Self>(self).data_.patchIfTypeIsAfter().value()) {
        std::forward<Visitor>(v).patchIfTypeIs(
            folly::forward_like<Self>(type), folly::forward_like<Self>(patch));
      }
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

  void apply(type::AnyStruct& val) const;

  template <type::StandardProtocol Protocol>
  std::unique_ptr<folly::IOBuf> applyToSerializedObject(
      std::unique_ptr<folly::IOBuf> buf) const {
    static_assert(
        Protocol == type::StandardProtocol::Binary ||
            Protocol == type::StandardProtocol::Compact,
        "Unsupported protocol");
    using Serializer = std::conditional_t<
        Protocol == type::StandardProtocol::Binary,
        apache::thrift::BinarySerializer,
        apache::thrift::CompactSerializer>;
    auto any = Serializer::template deserialize<type::AnyStruct>(buf.get());
    apply(any);
    return Serializer::template serialize<folly::IOBufQueue>(any).move();
  }

  /// Ensures the given type is set in Thrift Any.
  void ensureAny(type::AnyStruct ensureAny) {
    throwIfInvalidOrUnsupportedAny(ensureAny);
    ensureAnyWithoutValidation(std::move(ensureAny));
  }

  /// Patches the value in Thrift Any if the type matches with the provided
  /// patch.
  template <typename VPatch>
  void patchIfTypeIs(const VPatch& patch) {
    static_assert(op::is_patch_v<VPatch>);
    tryPatchable<VPatch>();
    patchIfTypeIsImpl(patch, ensures<op::patched_type_tag_t<VPatch>>());
  }

  /// Ensures the given value type is set in Thrift Any, and patches the value
  /// in Thrift Any if the type matches with the provided patch.
  template <typename VPatch>
  void ensureAndPatch(const VPatch& patch) {
    static_assert(op::is_patch_v<VPatch>);
    using VTag = op::patched_type_tag_t<VPatch>;
    ensureAnyWithoutValidation(type::AnyData::toAny<VTag>({}).toThrift());
    patchIfTypeIsImpl(patch, true);
  }

  // The provided type MUST match with the value type of patch stored in
  // provided patch as Thrift Any.
  void patchIfTypeIs(type::Type type, type::AnyStruct patch) {
    if (!type.isValid()) {
      throwTypeNotValid(type);
    }
    throwIfInvalidOrUnsupportedAny(patch);
    tryPatchable(type);
    bool ensure = ensures(type);
    patchIfTypeIsImpl(std::move(type), std::move(patch), ensure);
  }

  // The provided type in ensureAny MUST match with the value type of patch
  // stored in provided patch as Thrift Any.
  void ensureAndPatch(type::AnyStruct ensure, type::AnyStruct patch) {
    throwIfInvalidOrUnsupportedAny(patch);
    type::Type type = ensure.type().value();
    ensureAny(std::move(ensure));
    patchIfTypeIsImpl(std::move(type), std::move(patch), true);
  }

  // WARNING TO USERS: This method does not do any type checking on the patch,
  // meaning you need to make sure the DynamicPatch you provide applies to the
  // type represented by the ensured Any. If not you will corrupt your data.
  void ensureAndPatch(type::AnyStruct ensure, protocol::DynamicPatch patch) {
    type::Type type = ensure.type().value();
    ensureAny(std::move(ensure));
    patchIfTypeIsImpl(std::move(type), std::move(patch), true);
  }

  // Interop with AnyData
  // TODO(pranavtbhat): Replace with actual adapter/typedef support in patch
  void assign(const type::AnyData& val) { assign(val.toThrift()); }
  void assign(type::AnyData&& val) { assign(std::move(val).toThrift()); }

  AnyPatch& operator=(const type::AnyData& val) {
    return this->operator=(val.toThrift());
  }
  AnyPatch& operator=(type::AnyData&& val) {
    return this->operator=(std::move(val).toThrift());
  }

  void apply(type::AnyData& val) const { apply(val.toThrift()); }

  /// Extracts patch operations for a specific type from this AnyPatch.
  ///
  /// This function works when the AnyPatch does not change the underlying type
  /// during application. The extracted patch is equivalent to applying to the
  /// underlying value directly.
  ///
  /// Example:
  /// ```
  /// Foo before = ...;
  /// Any any = AnyData::toAny(before);
  /// anyPatch.apply(any);
  /// if (any.type() == type::create<Foo>()) {
  ///   Foo after = any.get<Foo>();
  ///   FooPatch patch = anyPatch.extractPatchAsIf<FooPatch>();
  ///   patch.apply(before);
  ///   assert(before == after); // Guaranteed
  /// }
  /// ```
  ///
  /// If the AnyPatch changes the underlying type, the extracted patch behavior
  /// is unspecified since type changes cannot be expressed through VPatch.
  template <typename VPatch>
  VPatch extractPatchAsIf() const {
    using VTag = op::patched_type_tag_t<VPatch>;
    using VPatchTag = type::infer_tag<VPatch>;
    struct AnyPatchExtractionVisitor {
      void assign(const type::AnyStruct& any) {
        if (type::identicalType(any.type().value(), type_)) {
          patch_ = type::AnyData{any}.get<VTag>();
        }
      }
      void clear() {
        patch_.clear();
        cleared_ = true;
      }
      void patchIfTypeIs(
          const type::Type& type, const protocol::DynamicPatch& dpatch) {
        if (!type::identicalType(type, type_)) {
          return;
        }
        auto patchObj = dpatch.toObject();
        patch_.merge(protocol::fromObjectStruct<VPatchTag>(patchObj));
      }
      void ensureAny(const type::AnyStruct& any) {
        // TODO(dokwon): Consider short circuiting `patchIfTypeIsPrior` if
        // `ensureAny` ensures to different type.
        if (!type::identicalType(any.type().value(), type_)) {
          op::clear<VPatchTag>(patch_);
        } else if (cleared_) {
          // After clear(), ensureAny() replaces the entire value since
          // the type was removed. Model as assign in the extracted patch.
          auto value = type::AnyData{any};
          if (type::identicalType(value.type(), type_)) {
            patch_ = value.get<VTag>();
          }
        }
        cleared_ = false;
      }

      explicit AnyPatchExtractionVisitor(VPatch& patch) : patch_(patch) {}

      VPatch& patch_;
      const type::Type& type_ = type::Type::get<VTag>();
      bool cleared_ = false;
    };
    VPatch patch;
    AnyPatchExtractionVisitor visitor(patch);
    customVisit(visitor);
    return patch;
  }

  /// Extracts dynamic patch for the given patch type.
  protocol::DynamicPatch extractDynamicPatchAsIf(const type::Type& type) const {
    protocol::DynamicPatch patch;
    DynamicPatchExtractionVisitor visitor(patch, type);
    customVisit(visitor);
    return patch;
  }

  static auto fromSafePatch(const AnySafePatch& patch) {
    return fromSafePatchImpl<AnyPatch>(patch);
  }
  auto toSafePatch() const { return toSafePatchImpl<AnySafePatch>(*this); }

  protocol::ExtractedMasksFromPatch extractMaskFromPatch() const;

 private:
  using Base::assignOr;
  using Base::data_;
  using Base::hasAssign;

  bool ensures(const type::Type& type) {
    return data_.ensureAny().has_value() &&
        type::identicalType(data_.ensureAny()->type().value(), type);
  }

  template <typename Tag>
  bool ensures() {
    return ensures(type::Type::get<Tag>());
  }

  // If assign has value and specified 'VPatch' is the corresponding patch
  // type to the type in 'assign' operation, we ensure the patch is patchable
  // by making it to 'clear' + 'ensureAny' operation.
  template <typename VPatch>
  void tryPatchable() {
    using VTag = patched_type_tag_t<VPatch>;
    tryPatchable(type::Type::get<VTag>());
  }
  void tryPatchable(const type::Type& type) {
    if (data_.assign().has_value()) {
      if (!type::identicalType(data_.assign()->type().value(), type)) {
        return;
      }
      data_.clear() = true;
      data_.ensureAny() = std::move(data_.assign().value());
      data_.assign().reset();
    }
  }

  // Callers that have already validated the AnyStruct should use this to avoid
  // redundant work.
  void ensureAnyWithoutValidation(type::AnyStruct ensureAny) {
    if (data_.assign().has_value()) {
      data_.clear() = true;
      data_.ensureAny() = std::move(data_.assign().value());
      data_.assign().reset();
    }

    if (ensures(ensureAny.type().value())) {
      return;
    }

    data_.ensureAny() = std::move(ensureAny);
  }

  template <typename VPatch>
  void patchIfTypeIsImpl(const VPatch& patch, bool after) {
    auto type = type::Type::get<op::patched_type_tag_t<VPatch>>();
    patchIfTypeIsImpl(
        type, protocol::DynamicPatch::fromStaticPatch(patch), after);
  }

  void patchIfTypeIsImpl(type::Type type, type::AnyStruct any, bool after);
  void patchIfTypeIsImpl(
      type::Type type, protocol::DynamicPatch dynamicPatch, bool after);

  // Needed for merge.
  void patchIfTypeIs(
      const type::Type& type, const protocol::DynamicPatch& patch);

  // A visitor to extract DynamicPatch for the given patch type.
  struct DynamicPatchExtractionVisitor {
    void assign(const type::AnyStruct& any);
    void clear();
    void patchIfTypeIs(
        const type::Type& type, const protocol::DynamicPatch& dpatch);
    void ensureAny(const type::AnyStruct& any);

    DynamicPatchExtractionVisitor(
        protocol::DynamicPatch& patch, const type::Type& type)
        : patch_(patch), type_(type) {}

    protocol::DynamicPatch& patch_;
    const type::Type& type_;
    bool cleared_ = false;
  };
};

template <class T>
struct PatchType;
template <class T>
struct SafePatchType;
template <class T>
struct SafePatchValueType;

template <>
struct PatchType<type::struct_t<::apache::thrift::type::AnyStruct>> {
  using type = AnyPatch<::apache::thrift::op::AnyPatchStruct>;
};

template <>
struct SafePatchType<type::struct_t<::apache::thrift::type::AnyStruct>> {
  using type = ::apache::thrift::op::AnySafePatch;
};

template <>
struct SafePatchValueType<::apache::thrift::op::AnySafePatch> {
  using type = ::apache::thrift::type::AnyStruct;
};

// Adapters for Thrift Any
template <typename T>
using AnyPatchAdapter = InlineAdapter<AnyPatch<T>>;

} // namespace detail
} // namespace apache::thrift::op
