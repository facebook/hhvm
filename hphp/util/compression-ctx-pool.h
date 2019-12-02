/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <memory>
#include <string>

#include <folly/Memory.h>
#include <folly/Synchronized.h>

namespace HPHP {

template <typename T, T* (*Creator)(), void (*Deleter)(T*)>
class CompressionContextPool {
 private:
  using InternalRef =
      std::unique_ptr<T, folly::static_function_deleter<T, Deleter>>;

  class ReturnToPoolDeleter {
   public:
    using Pool = CompressionContextPool<T, Creator, Deleter>;

    // lets us default-construct non-attached deleters, so we can
    // default-construct empty Refs.
    ReturnToPoolDeleter() : pool_(nullptr) {}

    explicit ReturnToPoolDeleter(Pool* pool) : pool_(pool) {}

    void operator()(T* t) {
      InternalRef ptr(t);
      if (pool_ != nullptr) {
        pool_->add(std::move(ptr));
      }
    }

   private:
    Pool* pool_;
  };

 public:
  using Object = T;
  using Ref = std::unique_ptr<T, ReturnToPoolDeleter>;

  explicit CompressionContextPool() {}

  Ref get() {
    auto stack = stack_.wlock();
    if (stack->empty()) {
      T* t = Creator();
      if (t == nullptr) {
        throw std::runtime_error("Failed to allocate new pool object!");
      }
      return Ref(t, ReturnToPoolDeleter(this));
    }
    auto ptr = std::move(stack->back());
    stack->pop_back();
    return Ref(ptr.release(), ReturnToPoolDeleter(this));
  }

 private:
  void add(InternalRef ptr) {
    DCHECK(ptr);
    stack_.wlock()->push_back(std::move(ptr));
  }

  folly::Synchronized<std::vector<InternalRef>> stack_;
};
} // namespace HPHP
