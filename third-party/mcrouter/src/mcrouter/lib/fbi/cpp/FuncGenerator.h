/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>
#include <utility>

#include <boost/iterator/iterator_facade.hpp>

namespace facebook {
namespace memcache {

/**
 * Container-like class that provides a way to generate range of objects
 * using a function.
 *
 * Example:
 *  auto gen = makeFuncGenerator([](size_t id) { return <object #id>; }, 10);
 *  for (auto& it : gen) {
 *    auto obj = (*it)();
 *    // use obj
 *  }
 */
template <class Func>
struct FuncGenerator {
  static_assert(
      !std::is_reference<Func>::value,
      "FuncGenerator doesn't work with reference types");

  struct Functor {
    Functor(Func* f, size_t id) : f_(f), id_(id) {}

    typename std::result_of<Func(size_t)>::type operator()() const {
      assert(f_);
      return (*f_)(id_);
    }

   private:
    Func* f_{nullptr};
    size_t id_{0};
  };

  class Iterator : public boost::iterator_facade<
                       Iterator,
                       Functor,
                       std::random_access_iterator_tag,
                       Functor,
                       int64_t> {
   public:
    Iterator() = default;

    Iterator(Func& f, size_t id) : f_(&f), id_(id) {}

   private:
    friend class boost::iterator_core_access;
    // Iterator should be default constructible
    Func* f_{nullptr};
    size_t id_{0};

    Functor dereference() const {
      return Functor(f_, id_);
    }

    bool equal(const Iterator& other) const {
      return id_ == other.id_ && f_ == other.f_;
    }

    void increment() {
      ++id_;
    }

    void decrement() {
      assert(id_ > 0);
      --id_;
    }

    void advance(int64_t n) {
      assert(id_ + n >= 0);
      id_ += n;
    }

    int64_t distance_to(const Iterator& other) const {
      return static_cast<int64_t>(other.id_) - id_;
    }
  };

  FuncGenerator(Func&& f, size_t n) : f_(std::move(f)), n_(n) {}

  Iterator begin() {
    return Iterator(f_, 0);
  }

  Iterator end() {
    return Iterator(f_, n_);
  }

 private:
  Func f_;
  size_t n_;
};

template <class Func>
FuncGenerator<typename std::remove_reference<Func>::type> makeFuncGenerator(
    Func&& f,
    size_t n) {
  return {std::forward<Func>(f), n};
}
} // namespace memcache
} // namespace facebook
