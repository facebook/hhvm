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

#include <cstdint>

#include <boost/operators.hpp>

namespace reflection_indirection {

using CppFakeI32 = std::int32_t;

struct CppHasANumber : private boost::totally_ordered<CppHasANumber> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(number);

  std::int32_t number{};
  CppHasANumber() {}
  explicit CppHasANumber(std::int32_t number_) : number(number_) {}
  bool operator==(CppHasANumber that) const { return number == that.number; }
  bool operator<(CppHasANumber that) const { return number < that.number; }
};

class CppHasAResult : private boost::totally_ordered<CppHasAResult> {
 public:
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(foo().result());

  class Foo {
   public:
    explicit Foo(std::int32_t& obj) : obj_(obj) {}
    std::int32_t& result() & { return obj_; }
    std::int32_t&& result() && { return std::move(obj_); }
    const std::int32_t& result() const& { return obj_; }

   private:
    std::int32_t& obj_;
  };

  CppHasAResult() {}
  explicit CppHasAResult(std::int32_t result) : result_(result) {}
  CppHasAResult(const CppHasAResult& that) : result_(that.result_) {}
  CppHasAResult& operator=(const CppHasAResult& that) {
    this->~CppHasAResult();
    return *::new (this) CppHasAResult(that);
  }

  bool operator==(CppHasAResult that) const { return result_ == that.result_; }
  bool operator<(CppHasAResult that) const { return result_ < that.result_; }

  Foo& foo() & { return foo_; }
  Foo&& foo() && { return static_cast<Foo&&>(foo_); }
  const Foo& foo() const& { return foo_; }

 private:
  std::int32_t result_{};
  Foo foo_{result_};
};

struct CppHasAPhrase : private boost::totally_ordered<CppHasAPhrase> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(phrase);

  std::string phrase{};
  CppHasAPhrase() {}
  explicit CppHasAPhrase(std::string phrase_) : phrase(std::move(phrase_)) {}
  bool operator==(CppHasAPhrase that) const { return phrase == that.phrase; }
  bool operator<(CppHasAPhrase that) const { return phrase < that.phrase; }
};
} // namespace reflection_indirection
