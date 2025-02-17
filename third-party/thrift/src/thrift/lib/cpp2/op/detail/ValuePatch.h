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

#include <thrift/lib/cpp2/op/detail/BasePatch.h>

namespace apache::thrift::op::detail {

/// Patch for a Thrift bool.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse bool invert`
template <typename Patch>
class BoolPatch : public BaseClearPatch<Patch, BoolPatch<Patch>> {
  using Base = BaseClearPatch<Patch, BoolPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using Base::clear;

  /// Inverts the bool.
  void invert() {
    auto& val = assignOr(*data_.invert());
    val = !val;
  }

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(bool);
  ///       void clear();
  ///       void invert();
  ///     }
  ///
  /// For example:
  ///
  ///     auto patch = BoolPatch::createClear();
  ///     patch = !patch;
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.clear();
  ///     v.invert();
  template <typename Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.invert();
    }
    if (!Base::customVisitAssignAndClear(v) && *data_.invert()) {
      std::forward<Visitor>(v).invert();
    }
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      void assign(T b) { v = b; }
      void clear() { v = false; }
      void invert() { v = !v; }
    };

    return customVisit(Visitor{val});
  }

 private:
  using Base::assignOr;
  using Base::data_;

  friend BoolPatch operator!(BoolPatch val) { return (val.invert(), val); }
};

/// Patch for a numeric Thrift types.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse T add`
template <typename Patch>
class NumberPatch : public BaseClearPatch<Patch, NumberPatch<Patch>> {
  using Base = BaseClearPatch<Patch, NumberPatch>;
  using T = typename Base::value_type;
  using Tag = type::infer_tag<T>;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using Base::clear;

  /// Increases the value.
  template <typename U>
  void add(U&& val) {
    assignOr(*data_.add()) += std::forward<U>(val);
  }

  /// Decreases the value.
  template <typename U>
  void subtract(U&& val) {
    assignOr(*data_.add()) -= std::forward<U>(val);
  }

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(Number);
  ///       void clear();
  ///       void add(Number);
  ///     }
  ///
  /// For example:
  ///
  ///     auto patch = I32Patch::createClear();
  ///     patch += 10;
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.clear();
  ///     v.add(10);
  template <typename Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.add(T{});
    }
    if (!Base::customVisitAssignAndClear(v)) {
      v.add(*data_.add());
    }
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      void assign(const T& t) { v = t; }
      void clear() { v = 0; }
      void add(const T& t) { v += t; }
    };

    return customVisit(Visitor{val});
  }

  /// Increases the value.
  template <typename U>
  NumberPatch& operator+=(U&& val) {
    add(std::forward<U>(val));
    return *this;
  }

  /// Decreases the value.
  template <typename U>
  NumberPatch& operator-=(U&& val) {
    subtract(std::forward<T>(val));
    return *this;
  }

 private:
  using Base::assignOr;
  using Base::data_;

  template <typename U>
  friend NumberPatch operator+(NumberPatch lhs, U&& rhs) {
    lhs.add(std::forward<U>(rhs));
    return lhs;
  }

  template <typename U>
  friend NumberPatch operator+(U&& lhs, NumberPatch rhs) {
    rhs.add(std::forward<U>(lhs));
    return rhs;
  }

  template <typename U>
  friend NumberPatch operator-(NumberPatch lhs, U&& rhs) {
    lhs.subtract(std::forward<U>(rhs));
    return lhs;
  }
};

/// Base class for string/binary patch types.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional T assign`
/// * `terse bool clear`
/// * `terse T append`
/// * `terse T prepend`
template <typename Patch, typename Derived>
class BaseStringPatch : public BaseContainerPatch<Patch, Derived> {
  using Base = BaseContainerPatch<Patch, Derived>;
  using T = typename Base::value_type;

 public:
  using Base::Base;
  using Base::operator=;

  /// Appends a string.
  template <typename U>
  Derived& operator+=(U&& val) {
    derived().append(std::forward<U>(val));
    return derived();
  }

 private:
  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(const String&);
  ///       void clear();
  ///       void prepend(const String&);
  ///       void append(const String&);
  ///     }
  ///
  /// For example:
  ///
  ///     auto patch = StringPatch::createPrepend("(");
  ///     patch += ")";
  ///
  /// `patch.customVisit(v)` will invoke the following methods
  ///
  ///     v.prepend("(");
  ///     v.append(")");
  template <class Self, class Visitor>
  static void customVisitImpl(Self&& self, Visitor&& v) {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(T{});
      v.clear();
      v.prepend(T{});
      v.append(T{});
    }
    if (!std::forward<Self>(self).customVisitAssignAndClear(
            std::forward<Visitor>(v))) {
      std::forward<Visitor>(v).prepend(
          *std::forward<Self>(self).data_.prepend());
      std::forward<Visitor>(v).append(*std::forward<Self>(self).data_.append());
    }
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(
      customVisit, customVisitImpl);

 protected:
  using Base::assignOr;
  using Base::data_;
  using Base::derived;

 private:
  /// Concat two strings.
  template <typename U>
  friend Derived operator+(Derived lhs, U&& rhs) {
    return lhs += std::forward<U>(rhs);
  }
  /// Concat two strings.
  template <typename U>
  friend Derived operator+(U&& lhs, Derived rhs) {
    rhs.prepend(std::forward<U>(lhs));
    return rhs;
  }
};

/// Patch for a Thrift string.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional string assign`
/// * `terse bool clear`
/// * `terse string append`
/// * `terse string prepend`
template <typename Patch>
class StringPatch : public BaseStringPatch<Patch, StringPatch<Patch>> {
  using Base = BaseStringPatch<Patch, StringPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  /// Appends a string.
  template <typename... Args>
  void append(Args&&... args) {
    assignOr(*data_.append()).append(std::forward<Args>(args)...);
  }

  /// Prepends a string.
  template <typename U>
  void prepend(U&& val) {
    T& cur = assignOr(*data_.prepend());
    cur = std::forward<U>(val) + std::move(cur);
  }

 private:
  template <class Self>
  static void applyImpl(Self&& self, T& val) {
    struct Visitor {
      T& v;
      void assign(const T& t) { v = t; }
      void assign(T&& t) { v = std::move(t); }
      void clear() { v.clear(); }
      void prepend(const T& t) { v = t + v; }
      void prepend(T&& t) { v = std::move(t) + v; }
      void append(const T& t) { v += t; }
      void append(T&& t) { v += std::move(t); }
    };

    return std::forward<Self>(self).customVisit(Visitor{val});
  }

 public:
  FOLLY_FOR_EACH_THIS_OVERLOAD_IN_CLASS_BODY_DELEGATE(apply, applyImpl);

 private:
  using Base::assignOr;
  using Base::data_;
};

inline const folly::IOBuf& emptyIOBuf() {
  static const auto empty =
      folly::IOBuf::wrapBufferAsValue(folly::StringPiece(""));
  return empty;
}

/// Patch for a Thrift Binary.
///
/// The `Patch` template parameter must be a Thrift struct with the following
/// fields:
/// * `optional standard.ByteBuffer assign`
/// * `terse bool clear`
/// * `terse standard.ByteBuffer append`
/// * `terse standard.ByteBuffer prepend`
template <typename Patch>
class BinaryPatch : public BaseStringPatch<Patch, BinaryPatch<Patch>> {
  using Base = BaseStringPatch<Patch, BinaryPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::assign;
  using Base::Base;

  void assign(std::string s) {
    assign(*folly::IOBuf::fromString(std::move(s)));
  }

  template <typename T>
  BinaryPatch& operator=(T&& other) {
    return assign(std::forward<T>(other)), *this;
  }

  /// Appends a binary string.
  template <typename... Args>
  void append(Args&&... args) {
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    auto& cur = assignOr(*data_.append());
    queue.append(cur);
    queue.append(std::forward<Args>(args)...);
    cur = queue.moveAsValue();
  }

  /// Prepends a binary string.
  template <typename U>
  void prepend(U&& val) {
    auto& cur = assignOr(*data_.prepend());
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    queue.append(std::forward<U>(val));
    queue.append(cur);
    cur = queue.moveAsValue();
  }

  void apply(T& val) const {
    struct Visitor {
      T& v;
      std::deque<const T*> bufs = {&v};
      void assign(const T& t) { bufs = {&t}; }
      void clear() { bufs = {&emptyIOBuf()}; }
      void prepend(const T& t) { bufs.push_front(&t); }
      void append(const T& t) { bufs.push_back(&t); }
      ~Visitor() {
        folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
        for (auto buf : bufs) {
          queue.append(*buf);
        }
        v = queue.moveAsValue();
      }
    };

    return Base::customVisit(Visitor{val});
  }

  void apply(std::string& val) const {
    auto buf = folly::IOBuf::fromString(std::move(val));
    apply(*buf);
    val = buf->toString();
  }

 private:
  using Base::assignOr;
  using Base::data_;
};

} // namespace apache::thrift::op::detail
