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
#include <string>
#include <string_view>
#include <folly/Range.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/util/ManagedStringView.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

enum class FunctionQualifier : uint8_t {
  Unspecified,
  OneWay,
  Idempotent,
  ReadOnly,
};

enum class InteractionMethodPosition : uint8_t {
  None,
  Factory,
  Member,
};

inline std::string_view qualifierToString(FunctionQualifier fq) {
  switch (fq) {
    case FunctionQualifier::Unspecified:
      return "Unspecified";
    case FunctionQualifier::OneWay:
      return "OneWay";
    case FunctionQualifier::Idempotent:
      return "Idempotent";
    case FunctionQualifier::ReadOnly:
      return "ReadOnly";
  }
  folly::assume_unreachable();
}

/*
 * A structure for storing thrift method metadata.
 * When created from static data, the structure creates a view of the data
 * When created from temporary data, the structure takes the ownership of
 * the data by creating a dynamic storage for it.
 */
class MethodMetadata {
 public:
  struct Data {
    Data(
        std::string name,
        FunctionQualifier qualifier,
        std::string uriOrName = {},
        InteractionMethodPosition position = InteractionMethodPosition::None,
        std::string interaction = {})
        : methodName(std::move(name)),
          functionQualifier(qualifier),
          thriftServiceUriOrName(std::move(uriOrName)),
          interactionPosition(position),
          interactionName(std::move(interaction)) {}

    FOLLY_COLD
    Data(
        const char* name,
        FunctionQualifier qualifier,
        const char* uriOrName = "",
        InteractionMethodPosition position = InteractionMethodPosition::None,
        const char* interaction = "") noexcept
        : methodName(name),
          functionQualifier(qualifier),
          thriftServiceUriOrName(uriOrName),
          interactionPosition(position),
          interactionName(interaction) {}
    std::string methodName;
    FunctionQualifier functionQualifier;
    std::string thriftServiceUriOrName;
    InteractionMethodPosition interactionPosition;
    std::string interactionName;
  };

  MethodMetadata(const MethodMetadata& that) {
    if (that.isOwning()) {
      isOwning_ = true;
      data_ = new Data(*that.data_);
    } else {
      isOwning_ = false;
      data_ = that.data_;
    }
  }

  MethodMetadata& operator=(const MethodMetadata& that) {
    if (that.isOwning()) {
      if (isOwning()) {
        *data_ = *that.data_;
      } else {
        isOwning_ = true;
        data_ = new Data(*that.data_);
      }
    } else {
      if (isOwning()) {
        delete data_;
        isOwning_ = false;
      }
      data_ = that.data_;
    }
    return *this;
  }

  MethodMetadata(MethodMetadata&& that) noexcept
      : isOwning_(std::exchange(that.isOwning_, false)),
        data_(std::exchange(that.data_, nullptr)) {}

  MethodMetadata& operator=(MethodMetadata&& that) noexcept {
    MethodMetadata tmp(std::move(that));
    swap(*this, tmp);
    return *this;
  }

  ~MethodMetadata() noexcept {
    if (isOwning()) {
      delete data_;
    }
  }

  /* implicit */ MethodMetadata(std::string_view methodName)
      : MethodMetadata(
            std::in_place,
            std::string(methodName),
            FunctionQualifier::Unspecified) {}

  /* implicit */ MethodMetadata(std::string&& methodName)
      : MethodMetadata(
            std::in_place,
            std::move(methodName),
            FunctionQualifier::Unspecified) {}

  /* implicit */ MethodMetadata(const std::string& methodName)
      : MethodMetadata(
            std::in_place, methodName, FunctionQualifier::Unspecified) {}

  /* implicit */ MethodMetadata(const char* methodName)
      : MethodMetadata(
            std::in_place, methodName, FunctionQualifier::Unspecified) {}

  /* implicit */ MethodMetadata(folly::StringPiece methodName)
      : MethodMetadata(
            std::in_place,
            std::string(methodName),
            FunctionQualifier::Unspecified) {}

  /* implicit */ MethodMetadata(const Data& data)
      : MethodMetadata(std::in_place, data) {}

  static MethodMetadata from_static(Data* mPtr) {
    return MethodMetadata(mPtr, NonOwningTag{});
  }

  ManagedStringView name_managed() const& {
    if (isOwning()) {
      return ManagedStringView(name_view());
    } else {
      return ManagedStringView::from_static(name_view());
    }
  }

  ManagedStringView name_managed() && {
    if (isOwning()) {
      return ManagedStringView(std::move(data_->methodName));
    } else {
      return ManagedStringView::from_static(name_view());
    }
  }

  std::string name_str() const& { return data_->methodName; }

  std::string name_str() && {
    return isOwning() ? std::move(data_->methodName) : data_->methodName;
  }

  std::string_view name_view() const {
    return std::string_view(data_->methodName);
  }

  FunctionQualifier qualifier() const { return data_->functionQualifier; }

  ManagedStringView thriftServiceUriOrName_managed() const& {
    return isOwning()
        ? ManagedStringView(thriftServiceUriOrName_view())
        : ManagedStringView::from_static(thriftServiceUriOrName_view());
  }

  ManagedStringView thriftServiceUriOrName_managed() && {
    return isOwning()
        ? ManagedStringView(std::move(data_->thriftServiceUriOrName))
        : ManagedStringView::from_static(thriftServiceUriOrName_view());
  }

  std::string thriftServiceUriOrName_str() const& {
    return data_->thriftServiceUriOrName;
  }

  std::string thriftServiceUriOrName_str() && {
    return isOwning() ? std::move(data_->thriftServiceUriOrName)
                      : data_->thriftServiceUriOrName;
  }

  std::string_view thriftServiceUriOrName_view() const {
    return data_->thriftServiceUriOrName;
  }

  InteractionMethodPosition interactionPosition() const {
    return data_->interactionPosition;
  }

  ManagedStringView interactionName_managed() const& {
    if (isOwning()) {
      return ManagedStringView(std::string_view(data_->interactionName));
    } else {
      return ManagedStringView::from_static(
          std::string_view(data_->interactionName));
    }
  }

  ManagedStringView interactionName_managed() && {
    if (isOwning()) {
      return ManagedStringView(std::move(data_->interactionName));
    } else {
      return ManagedStringView::from_static(
          std::string_view(data_->interactionName));
    }
  }

  bool isOwning() const { return isOwning_; }

 private:
  struct NonOwningTag {};
  MethodMetadata(Data* mPtr, NonOwningTag) : isOwning_(false), data_(mPtr) {}

  template <typename... Args>
  MethodMetadata(std::in_place_t, Args&&... args)
      : isOwning_(true), data_(new Data(std::forward<Args>(args)...)) {}

  friend void swap(MethodMetadata& left, MethodMetadata& right) noexcept {
    std::swap(left.isOwning_, right.isOwning_);
    std::swap(left.data_, right.data_);
  }

  bool isOwning_;
  Data* data_;
};

} // namespace apache::thrift
