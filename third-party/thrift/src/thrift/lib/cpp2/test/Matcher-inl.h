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

#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>
#include <fatal/type/enum.h>
#include <fatal/type/variant_traits.h>
#include <folly/Overload.h>
#include <folly/Traits.h>
#include <folly/lang/Pretty.h>

namespace apache::thrift {

namespace test::detail {

template <typename FieldTag>
std::string_view getFieldName() {
  // Thrift generates the tags in the 'apache::thrift::tag' namespace,
  // so we can just skip it to provide a better name for the users.
  // -1 because we don't want to count the terminating \0 character.
  constexpr std::size_t offset = sizeof("apache::thrift::tag::") - 1;
  // it's safe to return the string_view because pretty_name() returns a
  // statically allocated char *
  std::string_view name{folly::pretty_name<FieldTag>()};
  if (name.size() > offset) {
    name.remove_prefix(offset);
  }
  return name;
}

template <typename FieldTag, typename InnerMatcher>
class ThriftFieldMatcher {
 public:
  explicit ThriftFieldMatcher(InnerMatcher matcher)
      : matcher_(std::move(matcher)) {}

  template <
      typename ThriftStruct,
      typename = std::enable_if_t<
          is_thrift_exception_v<folly::remove_cvref_t<ThriftStruct>> ||
          is_thrift_struct_v<folly::remove_cvref_t<ThriftStruct>>>>
  operator testing::Matcher<ThriftStruct>() const {
    return testing::Matcher<ThriftStruct>(
        new Impl<const ThriftStruct&>(matcher_));
  }

 private:
  using Accessor = access_field_fn<FieldTag>;
  // For field_ref, we want to forward the value pointed to the matcher
  // directly (it's fully transparent).
  // For optional_field_ref, we don't want to do it becase the mental model
  // is as if it was an std::optional<field_ref>. If we were to forward the
  // value, it would be impossible to check if it is empty.
  // Instead, we send the optional_field_ref to the matcher. The user can
  // provide a testing::Optional matcher to match the value contained in it.

  template <typename ThriftStruct>
  class Impl : public testing::MatcherInterface<ThriftStruct> {
   private:
    using FieldRef = decltype(Accessor{}(std::declval<ThriftStruct>()));
    using FieldReferenceType = typename FieldRef::reference_type;
    inline static constexpr bool IsOptionalFieldRef =
        std::is_same_v<optional_field_ref<FieldReferenceType>, FieldRef>;
    // See above on why we do this switch
    using MatchedValue = std::conditional_t<
        IsOptionalFieldRef,
        optional_field_ref<FieldReferenceType>,
        FieldReferenceType>;

   public:
    template <typename MatcherOrPolymorphicMatcher>
    explicit Impl(const MatcherOrPolymorphicMatcher& matcher)
        : concrete_matcher_(testing::MatcherCast<MatchedValue>(matcher)) {}

    void DescribeTo(::std::ostream* os) const override {
      *os << "is an object whose field `" << getFieldName<FieldTag>() << "` ";
      concrete_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "is an object whose field `" << getFieldName<FieldTag>() << "` ";
      concrete_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(
        ThriftStruct obj,
        testing::MatchResultListener* listener) const override {
      *listener << "whose field `" << getFieldName<FieldTag>() << "` is ";
      auto val = Accessor{}(obj);
      // Using gtest internals??
      // This function does some printing and formatting.
      // Here we want the same behaviour as other matchers, so
      // this allows us to save code and stay consistent.
      // If in later gtest versions this breaks, we can either
      //  1. use the new functionality looking at how
      //     testing::FieldMatcher is implemented, or
      //  2. copy the old implementation here.
      using testing::internal::MatchPrintAndExplain;
      if constexpr (IsOptionalFieldRef) {
        return MatchPrintAndExplain(val, concrete_matcher_, listener);
      } else {
        return MatchPrintAndExplain(*val, concrete_matcher_, listener);
      }
    }

   private:
    const testing::Matcher<MatchedValue> concrete_matcher_;
  }; // class Impl

  const InnerMatcher matcher_;
};

template <typename FieldTag, typename InnerMatcher>
class IsThriftUnionWithMatcher {
 public:
  explicit IsThriftUnionWithMatcher(InnerMatcher matcher)
      : matcher_(std::move(matcher)) {}

  template <
      typename ThriftUnion,
      typename = std::enable_if_t<
          is_thrift_union_v<folly::remove_cvref_t<ThriftUnion>>>>
  operator testing::Matcher<ThriftUnion>() const {
    static_assert(
        SupportsReflection<folly::remove_cvref_t<ThriftUnion>>,
        "Include the _fatal_types.h header for the Thrift file defining this union");
    return testing::Matcher<ThriftUnion>(
        new Impl<const ThriftUnion&>(matcher_));
  }

 private:
  // Needed in order to provide a good error message in the static assert
  template <typename T>
  constexpr static inline bool SupportsReflection =
      fatal::has_variant_traits<T>::value;

  using Accessor = access_field_fn<FieldTag>;

  template <typename ThriftUnion>
  class Impl : public testing::MatcherInterface<ThriftUnion> {
   private:
    using FieldRef = decltype(Accessor{}(std::declval<ThriftUnion>()));
    // Unions do not support optional fields, so this is always a concrete
    // value (no optional_ref)
    using FieldReferenceType = typename FieldRef::reference_type;

   public:
    template <typename MatcherOrPolymorphicMatcher>
    explicit Impl(const MatcherOrPolymorphicMatcher& matcher)
        : concrete_matcher_(testing::MatcherCast<FieldReferenceType>(matcher)) {
    }

    void DescribeTo(::std::ostream* os) const override {
      *os << "is an union with `" << getFieldName<FieldTag>()
          << "` active, which ";
      concrete_matcher_.DescribeTo(os);
    }

    void DescribeNegationTo(::std::ostream* os) const override {
      *os << "is an union without `" << getFieldName<FieldTag>()
          << "` active, or the active value ";
      concrete_matcher_.DescribeNegationTo(os);
    }

    bool MatchAndExplain(
        ThriftUnion obj,
        testing::MatchResultListener* listener) const override {
      std::optional<bool> matches;
      using descriptors = typename fatal::variant_traits<
          folly::remove_cvref_t<ThriftUnion>>::descriptors;
      fatal::scalar_search<descriptors, fatal::get_type::id>(
          obj.getType(), [&](auto indexed) {
            using descriptor = decltype(fatal::tag_type(indexed));
            using tag = typename descriptor::metadata::tag;
            auto active = fatal::enum_to_string(obj.getType(), "unknown");
            if constexpr (std::is_same_v<FieldTag, tag>) {
              *listener << "whose active member `" << active << "` is ";
              matches = testing::internal::MatchPrintAndExplain(
                  descriptor::get(obj), concrete_matcher_, listener);
            } else {
              *listener << "whose active member `" << active << "` is "
                        << testing::PrintToString(descriptor::get(obj));
              matches = false;
            }
          });
      if (matches) {
        return *matches;
      }
      *listener << "which is unset";
      return false;
    }

   private:
    const testing::Matcher<FieldReferenceType> concrete_matcher_;
  }; // class Impl

  const InnerMatcher matcher_;
};

} // namespace test::detail
} // namespace apache::thrift
