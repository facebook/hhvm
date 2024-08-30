/*

The source code contained in this file is based on the original code by
Daniel Sipka (https://github.com/no1msd/mstch). The original license by Daniel
Sipka can be read below:

The MIT License (MIT)

Copyright (c) 2015 Daniel Sipka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace apache {
namespace thrift {
namespace mstch {

namespace internal {

template <class N>
class object_t {
 public:
  const N& at(const std::string& name) const { return methods_.at(name)(); }

  bool has(const std::string& name) const {
    return (methods_.find(name) != methods_.end());
  }

  std::vector<std::string> property_names() const {
    std::vector<std::string> result;
    for (auto& entry : methods_) {
      result.push_back(entry.first);
    }
    return result;
  }

 protected:
  // Volatile (uncached) methods are re-invoked every time their value is needed
  // during a template evaluation.
  //
  // This is potentially useful if mutating state during evaluation, but has a
  // performance cost. There are usually better ways to express such logic.
  template <typename F>
  std::enable_if_t<std::is_same_v<std::invoke_result_t<F>, N>>
  register_volatile_method(std::string name, F method) {
    do_register_method(
        std::move(name),
        [method = std::move(method),
         uncache = std::optional<N>()]() mutable -> const N& {
          uncache = method();
          return *uncache;
        });
  }

  // Cached methods are invoked at most once on the same object.
  template <typename F>
  std::enable_if_t<std::is_same_v<std::invoke_result_t<F>, N>>
  register_cached_method(std::string name, F method) {
    do_register_method(
        std::move(name),
        [method = std::move(method),
         cache = std::optional<N>()]() mutable -> const N& {
          if (!cache) {
            cache = method();
          }
          return *cache;
        });
  }

  template <class S>
  void register_volatile_methods(
      S* s, const std::unordered_map<std::string, N (S::*)()>& methods) {
    for (const auto& method : methods) {
      register_volatile_method(std::move(method.first), [s, m = method.second] {
        return (s->*m)();
      });
    }
  }

  template <class S>
  void register_cached_methods(
      S* s, const std::unordered_map<std::string, N (S::*)()>& methods) {
    for (const auto& method : methods) {
      register_cached_method(std::move(method.first), [s, m = method.second] {
        return (s->*m)();
      });
    }
  }

 private:
  void do_register_method(std::string name, std::function<const N&()> method) {
    auto result = methods_.emplace(std::move(name), std::move(method));
    if (!result.second) {
      throw std::runtime_error(
          "Method already registered: " + result.first->first);
    }
  }

  std::unordered_map<std::string, std::function<const N&()>> methods_;
};

template <typename Node>
using node_base = std::variant<
    std::nullptr_t,
    std::string,
    int,
    double,
    bool,
    std::shared_ptr<internal::object_t<Node>>,
    std::map<const std::string, Node>,
    std::vector<Node>>;

} // namespace internal

struct node : internal::node_base<node> {
  using base = internal::node_base<node>;

  using base::base;
  /* implicit */ node(std::string_view sv) : base(std::string(sv)) {}
  /* implicit */ node(std::size_t i);

  // Equivalent to the int constructor brought in by `using base::base`, but
  // having this here forces integer callers which are neither int nor size_t to
  // intentionally convert to one or the other. Without the following line,
  // calls like node(int64_t) would silently disambiguate to the size_t
  // constructor, which is potentially surprising.
  /* implicit */ node(int i) : base(i) {}

  template <typename... Visitor>
  decltype(auto) visit(Visitor&&... visitor) {
    return std::visit(visitor..., static_cast<base&>(*this));
  }
  template <typename... Visitor>
  decltype(auto) visit(Visitor&&... visitor) const {
    return std::visit(visitor..., static_cast<const base&>(*this));
  }
};

template <typename Node, typename... A>
node make_shared_node(A&&... a) {
  return node(std::make_shared<Node>(static_cast<A&&>(a)...));
}

using object = internal::object_t<node>;
using map = std::map<const std::string, node>;
using array = std::vector<node>;

std::string render(
    const std::string& tmplt,
    const node& root,
    const std::map<std::string, std::string>& partials =
        std::map<std::string, std::string>());

} // namespace mstch
} // namespace thrift
} // namespace apache
