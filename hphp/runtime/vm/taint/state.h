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

#ifdef HHVM_TAINT

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/vm/func.h"

#include <map>
#include <memory>
#include <optional>
#include <set>

template <>
struct std::hash<HPHP::tv_lval> {
  std::size_t operator()(const HPHP::tv_lval& val) const {
    return val.hash();
  }
};

namespace HPHP {
namespace taint {

/*
 * Single hop in a trace. Can be from callee to caller on trace from source
 * to root; or caller to callee on trace from root to sink.
 */
struct Hop {
  const Func* from;
  const Func* to;
};

struct Path {
  // Print trace in JSON-line format to stderr.
  void dump() const;
  std::vector<Hop> hops;
};

using Value = Optional<Path>;

struct Stack {
  Stack(const std::vector<Value>& stack = {}) : m_stack(stack) {}

  void push(const Value& value);

  Value top() const;
  Value peek(int offset) const;

  void pop(int n = 1);
  void replaceTop(const Value& value);

  size_t size() const;
  std::string show() const;

  void clear();

 private:
  std::vector<Value> m_stack;
};

/*
 * Our shadow heap is not replicating the full VM heap but keeps track
 * of tainted values (cells) on the heap.
 */
struct Heap {
  void set(const tv_lval& typedValue, const Value& value);
  Value get(const tv_lval& typedValue) const;

  void clear();

 private:
  hphp_fast_map<tv_lval, Value> m_heap;
};

struct State {
  static std::shared_ptr<State> get();

  void initialize();
  void reset();

  Stack stack;
  Heap heap;
};

} // namespace taint
} // namespace HPHP

#endif
