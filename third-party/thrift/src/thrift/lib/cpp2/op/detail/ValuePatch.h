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

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// A patch adapter that only supports 'assign',
// which is the minimum any patch should support.
//
// Patch must have the following fields:
//   optional T assign;
template <typename Patch>
class AssignPatch : public BaseAssignPatch<Patch, AssignPatch<Patch>> {
  using Base = BaseAssignPatch<Patch, AssignPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  void apply(T& val) const { applyAssign(val); }
  template <typename U>
  void merge(U&& next) {
    mergeAssign(std::forward<U>(next));
  }

 private:
  using Base::applyAssign;
  using Base::mergeAssign;
};

// Patch must have the following fields:
//   optional T assign;
//   bool clear;
//   bool invert;
template <typename Patch>
class BoolPatch : public BaseClearPatch<Patch, BoolPatch<Patch>> {
  using Base = BaseClearPatch<Patch, BoolPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  static BoolPatch createInvert() { return !BoolPatch{}; }

  void invert() {
    auto& val = assignOr(*data_.invert());
    val = !val;
  }

  void apply(T& val) const {
    if (!Base::template applyAssignAndClear<type::bool_t>(val) &&
        *data_.invert()) {
      val = !val;
    }
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      *data_.invert() ^= *next.toThrift().invert();
    }
  }

 private:
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;

  friend BoolPatch operator!(BoolPatch val) { return (val.invert(), val); }
};

// Patch must have the following fields:
//   optional T assign;
//   bool clear;
//   T add;
template <typename Patch>
class NumberPatch : public BaseClearPatch<Patch, NumberPatch<Patch>> {
  using Base = BaseClearPatch<Patch, NumberPatch>;
  using T = typename Base::value_type;
  using Tag = type::infer_tag<T>;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename U>
  static NumberPatch createAdd(U&& val) {
    NumberPatch patch;
    patch.add(std::forward<U>(val));
    return patch;
  }
  template <typename U>
  static NumberPatch createSubtract(U&& val) {
    NumberPatch patch;
    patch.subtract(std::forward<U>(val));
    return patch;
  }

  template <typename U>
  void add(U&& val) {
    assignOr(*data_.add()) += std::forward<U>(val);
  }

  template <typename U>
  void subtract(U&& val) {
    assignOr(*data_.add()) -= std::forward<U>(val);
  }

  void apply(T& val) const {
    if (!Base::template applyAssignAndClear<Tag>(val)) {
      val += *data_.add();
    }
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      *data_.add() += *next.toThrift().add();
    }
  }

  template <typename U>
  NumberPatch& operator+=(U&& val) {
    add(std::forward<U>(val));
    return *this;
  }

  template <typename U>
  NumberPatch& operator-=(U&& val) {
    subtract(std::forward<T>(val));
    return *this;
  }

 private:
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;

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

// Base class for string/binary patch types.
//
// Patch must have the following fields:
//   optional T assign;
//   bool clear;
//   T append;
//   T prepend;
template <typename Patch, typename Derived>
class BaseStringPatch : public BaseContainerPatch<Patch, Derived> {
  using Base = BaseContainerPatch<Patch, Derived>;

 public:
  using Base::Base;
  using Base::operator=;

  template <typename U>
  static Derived createPrepend(U&& val) {
    Derived patch;
    patch.prepend(std::forward<U>(val));
    return patch;
  }

  template <typename... Args>
  static Derived createAppend(Args&&... args) {
    Derived patch;
    patch.append(std::forward<Args>(args)...);
    return patch;
  }

  template <typename U>
  Derived& operator+=(U&& val) {
    derived().append(std::forward<U>(val));
    return derived();
  }

 protected:
  using Base::assignOr;
  using Base::data_;
  using Base::derived;

 private:
  template <typename U>
  friend Derived operator+(Derived lhs, U&& rhs) {
    return lhs += std::forward<U>(rhs);
  }
  template <typename U>
  friend Derived operator+(U&& lhs, Derived rhs) {
    rhs.prepend(std::forward<U>(lhs));
    return rhs;
  }
};

// Patch must have the following fields:
//   optional string assign;
//   bool clear;
//   string append;
//   string prepend;
template <typename Patch>
class StringPatch : public BaseStringPatch<Patch, StringPatch<Patch>> {
  using Base = BaseStringPatch<Patch, StringPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename... Args>
  void append(Args&&... args) {
    assignOr(*data_.append()).append(std::forward<Args>(args)...);
  }

  template <typename U>
  void prepend(U&& val) {
    T& cur = assignOr(*data_.prepend());
    cur = std::forward<U>(val) + std::move(cur);
  }

  void apply(T& val) const {
    if (!applyAssignOrClear(val)) {
      val = *data_.prepend() + std::move(val) + *data_.append();
    }
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      *data_.prepend() = *std::forward<U>(next).toThrift().prepend() +
          std::move(*data_.prepend());
      data_.append()->append(*std::forward<U>(next).toThrift().append());
    }
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

// Patch must have the following fields:
//   optional standard.ByteBuffer assign;
//   bool clear;
//   standard.ByteBuffer append;
//   standard.ByteBuffer prepend;
template <typename Patch>
class BinaryPatch : public BaseStringPatch<Patch, BinaryPatch<Patch>> {
  using Base = BaseStringPatch<Patch, BinaryPatch>;
  using T = typename Base::value_type;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;

  template <typename... Args>
  void append(Args&&... args) {
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    auto& cur = assignOr(*data_.append());
    queue.append(cur);
    queue.append(std::forward<Args>(args)...);
    cur = queue.moveAsValue();
  }

  template <typename U>
  void prepend(U&& val) {
    auto& cur = assignOr(*data_.prepend());
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    queue.append(std::forward<U>(val));
    queue.append(cur);
    cur = queue.moveAsValue();
  }

  void apply(T& val) const {
    if (applyAssignOrClear(val)) {
      return;
    }
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    queue.append(std::move(*data_.prepend()));
    queue.append(val);
    queue.append(std::move(*data_.append()));
    val = queue.moveAsValue();
  }

  template <typename U>
  void merge(U&& next) {
    if (!mergeAssignAndClear(std::forward<U>(next))) {
      folly::IOBufQueue prependQueue{folly::IOBufQueue::cacheChainLength()};
      prependQueue.append(*std::forward<U>(next).toThrift().prepend());
      prependQueue.append(std::move(*data_.prepend()));
      data_.prepend() = prependQueue.moveAsValue();

      folly::IOBufQueue appendQueue{folly::IOBufQueue::cacheChainLength()};
      appendQueue.append(std::move(*data_.append()));
      appendQueue.append(*std::forward<U>(next).toThrift().append());
      data_.append() = appendQueue.moveAsValue();
    }
  }

 private:
  using Base::applyAssignOrClear;
  using Base::assignOr;
  using Base::data_;
  using Base::mergeAssignAndClear;
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
