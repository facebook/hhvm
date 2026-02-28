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
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

#include <folly/ScopeGuard.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Exception.h>

namespace apache::thrift::op {

/**
 * A deterministic stack-context based accumulator that guarantees the
 * accumulated results to be stable and consistent across different languages
 * and implementations. Accumulators know how to combine individual thrift
 * primitives and know how to handle ordered/unordered elements.
 *
 * Context:
 *   Context wraps a vector of Hasher instances (hashers), the number of ordered
 *   collection of elements (ordered_count), and a flag that indicates whether
 *   the context was created with unordered element (has_unordered_context). The
 *   context is unordered when the count is zero and ordered if the count is
 *   non-zero. If the context is ordered, the last element of hashers will
 *   always point to the ordered hasher.
 *
 * Ordered Elements:
 *   Each time we start combining an ordered collection of elements we need to
 *   check if the previous context is ordered or unordered. If the previous
 *   context is ordered, we can elide creating a new Hasher by accumulating the
 *   hash with the Hasher in the previous ordered context and keeping count of
 *   accumulated Hasher. If the previous context is unordered, we need to create
 *   a new Hasher instance and push back into the vector of Hasher in the
 *   previous context. All subsequent combine operations will use that context
 *   to accumulate the hash. If the stack is empty, we still need to push a new
 *   ordered context to the stack.
 *
 * Unordered Elements:
 *   We always need to create a new context for unordered elements. All
 *   subsequent combine operations will push front a new Hasher into the
 *   vector of Hasher.
 *
 * Examples:
 *   struct MyData {
 *     1: list<i64> f1 = [1, 2, 3];
 *     2: set<i64>  f2 = {4, 5, 6};
 *   }
 *   MyData data;
 *   DeterministicAccumulator<MyHasherGenerator> acc;
 *   acc.beginUnordered();             // struct data begin
 *    acc.beginOrdered();              // field  f1   begin
 *     acc.combine(TType::T_LIST);     // field  f1   type
 *     acc.combine(1);                 // field  f1   id
 *     acc.beginOrdered();             // list   f1   begin
 *      acc.combine(TType::T_I64);     // list   f1   type
 *      acc.combine(3);                // list   f1   size
 *      acc.beginOrdered();            // list   f1   data begin
 *       acc.combine(1);               // f1[0]
 *       acc.combine(2);               // f1[1]
 *       acc.combine(3);               // f1[2]
 *      acc.endOrdered();              // list   f1   data end
 *     acc.endOrdered();               // list   f1   end
 *    acc.endOrdered();                // field  f1   end
 *    acc.beginOrdered();              // field  f2   begin
 *     acc.combine(TType::T_SET);      // field  f2   type
 *     acc.combine(2);                 // field  f2   id
 *     acc.beginOrdered();             // set    f2   begin
 *      acc.combine(TType::T_STRING);  // set    f2   type
 *      acc.combine(3);                // set    f2   size
 *      acc.beginUnordered();          // set    f2   data begin
 *       acc.combine(4);               // f2[0]
 *       acc.combine(5);               // f2[1]
 *       acc.combine(6);               // f2[2]
 *      acc.endUnordered();            // set    f2   data end
 *     acc.endOrdered();               // set    f2   end
 *    acc.endOrdered();                // field  f2   end
 *   acc.endUnordered();               // struct data end
 */
template <typename HasherGenerator>
class DeterministicAccumulator {
 private:
  using Hasher = decltype(std::declval<HasherGenerator>()());

 public:
  DeterministicAccumulator() = default;
  explicit DeterministicAccumulator(HasherGenerator generator)
      : generator_(std::move(generator)) {}

  Hasher& result() {
    if (context_.empty()) {
      return result_;
    }
    folly::throw_exception<std::logic_error>("empty hash result");
  }
  const Hasher& result() const {
    if (context_.empty()) {
      return result_;
    }
    folly::throw_exception<std::logic_error>("empty hash result");
  }

  template <typename T>
  void combine(const T& val);

  void beginOrdered();
  void endOrdered();
  void beginUnordered() { context_.emplace(Context{true}); }
  void endUnordered();

 private:
  struct Context {
    Context() = default;
    explicit Context(bool has_unordered_ctx)
        : has_unordered_context(has_unordered_ctx) {}

    std::vector<Hasher> hashers;
    // Number of ordered collections that this context represents.
    // An ordered hasher is always located at the back of the hashers.
    // Each time when we open a new ordered context and elide the creation,
    // this ordered_count gets bumped. We decrease it each time we leave an
    // ordered context (finish an ordered collection of elements).
    size_t ordered_count{};
    bool has_unordered_context{};
  };

  HasherGenerator generator_;
  std::stack<Context, std::vector<Context>> context_;
  Hasher result_ = generator_();

  constexpr auto& context() {
    if (context_.empty()) {
      folly::throw_exception<std::logic_error>("empty hash context");
    }
    return context_.top();
  }

  void exitContext(Hasher result);
};

// Creates a deterministic accumulator using the provided hasher.
template <class Hasher>
[[nodiscard]] auto makeDeterministicAccumulator() {
  // C++14 can not implicitly generate deduction guide for lambda.
  auto hashGen = [] { return Hasher{}; };
  return DeterministicAccumulator<decltype(hashGen)>(std::move(hashGen));
}

template <class Accumulator>
[[nodiscard]] auto makeOrderedHashGuard(Accumulator& accumulator) {
  accumulator.beginOrdered();
  return folly::makeGuard([&] { accumulator.endOrdered(); });
}

template <class Accumulator>
[[nodiscard]] auto makeUnorderedHashGuard(Accumulator& accumulator) {
  accumulator.beginUnordered();
  return folly::makeGuard([&] { accumulator.endUnordered(); });
}

template <typename HasherGenerator>
void DeterministicAccumulator<HasherGenerator>::beginOrdered() {
  // If the context stack is empty, push new context to the stack and use it.
  if (context_.empty()) {
    context_.emplace();
  }
  auto& ctx = context();
  // If the previous context is not an ordered context, insert a new hasher
  // back of the vector.
  if (ctx.ordered_count == 0) {
    ctx.hashers.push_back(generator_());
  }
  ++ctx.ordered_count;
  return;
}

template <typename HasherGenerator>
void DeterministicAccumulator<HasherGenerator>::endOrdered() {
  auto& ctx = context();
  // If the previous context is not an ordered context, throw.
  if (ctx.ordered_count == 0) {
    folly::throw_exception<std::logic_error>("ordering context mistmatch");
  }
  if (--ctx.ordered_count == 0) {
    if (ctx.has_unordered_context) {
      // If the context was created with an unordered element, we should not
      // exit the context.
      ctx.hashers.back().finalize();
    } else {
      exitContext(std::move(ctx.hashers.back()));
    }
  }
}

template <typename HasherGenerator>
void DeterministicAccumulator<HasherGenerator>::endUnordered() {
  auto& ctx = context();
  // If the previous context is not an unordered context, throw.
  if (ctx.ordered_count > 0) {
    folly::throw_exception<std::logic_error>("ordering context mistmatch");
  }
  std::sort(ctx.hashers.begin(), ctx.hashers.end());
  auto result = generator_();
  for (const auto& hasher : ctx.hashers) {
    result.combine(hasher);
  }
  exitContext(std::move(result));
}

template <typename HasherGenerator>
void DeterministicAccumulator<HasherGenerator>::exitContext(Hasher result) {
  context_.pop();
  result.finalize();
  // If the context stack is empty, set the result.
  if (context_.empty()) {
    result_ = std::move(result);
    return;
  }
  auto& ctx = context();
  // If the previous context is an ordered context, combine with the
  // existing ordered hasher which exists in the back of the vector.
  if (ctx.ordered_count > 0) {
    ctx.hashers.back().combine(result);
    return;
  }
  // If the previous context is an unordered context, insert a new hasher to the
  // vector.
  ctx.hashers.push_back(std::move(result));
}

template <typename HasherGenerator>
template <typename T>
void DeterministicAccumulator<HasherGenerator>::combine(const T& val) {
  // If the context stack is empty, set the result.
  if (context_.empty()) {
    result_ = generator_();
    result_.combine(val);
    result_.finalize();
    return;
  }
  auto& ctx = context();
  // If the previous context is an ordered context, combine with the
  // existing ordered hasher.
  if (ctx.ordered_count > 0) {
    ctx.hashers.back().combine(val);
    return;
  }
  // If the previous context is an unordered context, insert a new hasher
  // and combine with the new hasher.
  ctx.hashers.push_back(generator_());
  ctx.hashers.back().combine(val);
  ctx.hashers.back().finalize();
}

} // namespace apache::thrift::op
