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

#include <string>
#include <string_view>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <thrift/lib/cpp2/Thrift.h>

namespace apache::thrift {
// A view into a string that may or may not be stored inline.
// Unlike string_view, should be passed by move by default.
class ManagedStringView {
 public:
  ManagedStringView() = default;
  /* implicit */ ManagedStringView(std::string_view view) : string_{view} {}
  /* implicit */ ManagedStringView(const std::string& string)
      : string_{string} {}
  /* implicit */ ManagedStringView(std::string&& string) noexcept
      : string_{std::move(string)} {}
  /* implicit */ ManagedStringView(const char* buf) : string_{buf} {}
  /* implicit */ ManagedStringView(folly::StringPiece view) : string_{view} {}

  template <typename string_view>
  static ManagedStringView from_static(string_view view) noexcept {
    static_assert(
        std::is_same_v<string_view, std::string_view>,
        "This method is only safe to call with a constexpr std::string_view.");
    return ManagedStringView{view, FromStaticTag{}};
  }

  std::string str() const& { return is_owned() ? string_ : std::string{view_}; }
  std::string str() && {
    return is_owned() ? std::move(string_) : std::string{view_};
  }

  std::string_view view() const {
    return is_owned() ? std::string_view{string_} : view_;
  }

  const char* data() const { return view().data(); }
  std::size_t size() const { return view().size(); }

  friend bool operator==(const ManagedStringView& a, std::string_view b) {
    return a.view() == b;
  }
  friend bool operator==(std::string_view b, const ManagedStringView& a) {
    return a.view() == b;
  }
  friend bool operator==(
      const ManagedStringView& a, const ManagedStringView& b) {
    return a.view() == b.view();
  }
  friend bool operator!=(const ManagedStringView& a, std::string_view b) {
    return a.view() != b;
  }
  friend bool operator!=(std::string_view b, const ManagedStringView& a) {
    return a.view() != b;
  }
  friend bool operator!=(
      const ManagedStringView& a, const ManagedStringView& b) {
    return a.view() != b.view();
  }
  friend bool operator<(const ManagedStringView& a, std::string_view b) {
    return a.view() < b;
  }
  friend bool operator<(std::string_view b, const ManagedStringView& a) {
    return a.view() > b;
  }
  friend bool operator<(
      const ManagedStringView& a, const ManagedStringView& b) {
    return a.view() < b.view();
  }

 protected:
  struct FromStaticTag {};

  explicit ManagedStringView(std::string_view view, FromStaticTag) noexcept
      : view_{view} {}

  bool is_owned() const noexcept { return !view_.data(); }

  std::string_view view_;
  std::string string_;
};

// Separate adaptor to work with FieldRef without making the main type
// convertible to everything.
class ManagedStringViewWithConversions : public ManagedStringView {
 public:
  using value_type = char;

  using ManagedStringView::ManagedStringView;
  /* implicit */ ManagedStringViewWithConversions(const ManagedStringView& view)
      : ManagedStringView(view) {}
  /* implicit */ ManagedStringViewWithConversions(ManagedStringView&& view)
      : ManagedStringView(std::move(view)) {}

  /* implicit */ operator folly::StringPiece() const { return view(); }
  void clear() { *this = ManagedStringViewWithConversions(); }
  void reserve(size_t n) {
    check_owned("view: cannot reserve");
    string_.reserve(n);
  }

  void append(const char* data, size_t n) {
    check_owned("view: cannot append");
    string_.append(data, n);
  }
  void append(std::string_view in) {
    check_owned("view: cannot append");
    string_.append(in.data(), in.size());
  }

  ManagedStringView& operator+=(std::string_view in) {
    append(in);
    return *this;
  }
  ManagedStringView& operator+=(unsigned char in) {
    check_owned("view: cannot append");
    string_ += in;
    return *this;
  }

 private:
  void check_owned(const char* what) {
    if (!is_owned()) {
      folly::terminate_with<std::runtime_error>(what);
    }
  }
};

inline folly::Expected<folly::StringPiece, folly::ConversionCode> parseTo(
    folly::StringPiece in, ManagedStringViewWithConversions& out) noexcept {
  out = ManagedStringViewWithConversions(in);
  return folly::StringPiece{in.end(), in.end()};
}

} // namespace apache::thrift
