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

#include <utility>

#include <folly/json.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/op/Patch.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>

namespace apache::thrift {
namespace ident {
struct assign;
}
class BinaryProtocolReader;
class BinaryProtocolWriter;
class CompactProtocolReader;
class CompactProtocolWriter;

namespace op::detail {

template <class Protocol>
inline constexpr bool kProtocolSupportsDynamicPatch =
    std::is_same_v<Protocol, BinaryProtocolReader> ||
    std::is_same_v<Protocol, BinaryProtocolWriter> ||
    std::is_same_v<Protocol, CompactProtocolReader> ||
    std::is_same_v<Protocol, CompactProtocolWriter> ||
    std::is_same_v<Protocol, protocol::detail::ObjectWriter>;

// TODO(dokwon): Consider removing AssignPatch and provide full patch
// support with DynamicPatch.

/// A patch adapter that only supports 'assign',
/// which is the minimum any patch should support.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
///
/// If the assign only patch is deserialized from a dynamic patch, it might have
/// other operations besides assign operation.
template <typename Patch>
class AssignPatch : public BaseAssignPatch<Patch, AssignPatch<Patch>> {
  using Base = BaseAssignPatch<Patch, AssignPatch>;
  using T = typename Base::value_type;
  using Tag = get_type_tag<Patch, ident::assign>;

 public:
  using Base::apply;
  using Base::Base;

  void assign(T a) {
    Base::assign(std::move(a));
    dynPatch_.reset();
  }

  auto& operator=(T a) {
    assign(std::move(a));
    return *this;
  }

  void apply(T& val) const {
    if (dynPatch_) {
      auto value = protocol::asValueStruct<Tag>(val);
      dynPatch_->apply(value);
      val = protocol::fromValueStruct<Tag>(value);
    } else if (auto p = data_.assign()) {
      val = data_.assign().value();
    }
  }

  void merge(AssignPatch other) {
    if (dynPatch_ && other.dynPatch_) {
      XLOG_EVERY_MS(CRITICAL, 10000)
          << "Merging dynamic patch is not implemented. "
             "The merged result will be incorrect.\n"
             "First Patch = "
          << folly::toPrettyJson(
                 apache::thrift::protocol::toDynamic(dynPatch_->toObject()))
          << "\nSecond Patch = "
          << folly::toPrettyJson(
                 apache::thrift::protocol::toDynamic(
                     other.dynPatch_->toObject()));

      // Do nothing, which is the old behavior
      return;
    }

    if (!other.dynPatch_) {
      if (auto p = other.data_.assign()) {
        assign(std::move(*p));
      }
      return;
    }

    if (auto p = data_.assign()) {
      other.apply(*p);
    } else {
      dynPatch_ = std::move(other.dynPatch_);
    }
  }

  /// @cond
  template <class Protocol>
  uint32_t encode(Protocol& prot) const {
    if (!kProtocolSupportsDynamicPatch<Protocol> || !dynPatch_) {
      return op::encode<type::struct_t<Patch>>(prot, data_);
    }

    return dynPatch_->encode(prot);
  }

  template <class Protocol>
  void decode(Protocol& prot) {
    if (!kProtocolSupportsDynamicPatch<Protocol>) {
      return op::decode<type::struct_t<Patch>>(prot, data_);
    }

    createFromObject(
        protocol::detail::parseValue(prot, TType::T_STRUCT).as_object());
  }

  bool empty() const { return !dynPatch_.has_value() && Base::empty(); }

  void reset() {
    dynPatch_.reset();
    Base::reset();
  }

  template <class..., bool IsStructured = is_thrift_class_v<T>>
  static auto fromSafePatch(
      const typename std::enable_if_t<IsStructured, SafePatchType<Tag>>::type&
          safePatch) {
    return fromSafePatchImpl<AssignPatch>(safePatch);
  }

  auto toSafePatch() const {
    return toSafePatchImpl<typename SafePatchType<Tag>::type>(*this);
  }
  /// @endcond

 private:
  using Base::data_;
  std::optional<protocol::DynamicPatch> dynPatch_;

  void createFromObject(protocol::Object v) {
    data_ = protocol::fromObjectStruct<type::struct_t<Patch>>(v);
    if (data_.assign()) {
      dynPatch_.reset();
    } else {
      dynPatch_ = protocol::DynamicPatch::fromObject(std::move(v));
      Base::reset();
    }
  }

  template <typename>
  friend struct protocol::detail::ProtocolValueToThriftValue;
};

} // namespace op::detail

namespace protocol::detail {

// When converting protocol::Object to AssignPatch, we need special logic here
// to handle the case when protocol::Object is a dynamic patch and it might
// contain operations other than assign.
template <typename PatchStruct>
struct ProtocolValueToThriftValue<type::adapted<
    InlineAdapter<op::detail::AssignPatch<PatchStruct>>,
    type::struct_t<PatchStruct>>> {
  bool operator()(
      const Object& obj, op::detail::AssignPatch<PatchStruct>& patch) {
    patch.createFromObject(obj);
    return true;
  }
  bool operator()(
      const Value& obj, op::detail::AssignPatch<PatchStruct>& patch) {
    if (auto p = obj.if_object()) {
      operator()(*p, patch);
      return true;
    }
    return false;
  }
};
} // namespace protocol::detail
} // namespace apache::thrift
