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
#include "hphp/util/arena.h"

#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <set>

#include "hphp/runtime/vm/taint/configuration.h"

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

/*
 * A Path is a full trace.
 *
 * This datastructure is optimized for writes at the expense of reads.
 * We expect a single tainted value may flow to many others, so we emulate
 * a tree structure ourselves to make adding children cheap.
 *
 * We assume all pointers to Paths are kept alive by the caller.
 */
struct Path {
  std::string jsonLine() const;

  // Creates a new, empty Path
  Path();

  // Creates a new path originating here
  static Path* origin(Arena* arena, Hop hop);

  // Creates a path going from this path to the new hop
  // Takes an allocator to help with memory management
  Path* to(Arena* arena, Hop hop) const;

  ~Path() = default;

 private:
  // The hop taken here. Can be empty for the root (both pointers null)
  Hop hop;
  const Path* parent;
};

using Value = Path*;

struct Stack {
  Stack(const std::deque<Value>& stack = {}) : m_stack(stack) {}

  void push(Value value);
  void pushFront(Value value);

  Value top() const;
  Value peek(int offset) const;

  void pop(int n = 1);
  void popFront();
  void replaceTop(Value value);

  size_t size() const;
  std::string show() const;

  void clear();

 private:
  std::deque<Value> m_stack;
};

/*
 * Our shadow heap is not replicating the full VM heap but keeps track
 * of tainted values (cells) on the heap.
 */
struct Heap {
  void set(tv_lval typedValue, Value value);
  Value get(const tv_lval& typedValue) const;

  void clear();

 private:
  hphp_fast_map<tv_lval, Value> m_heap;
};

struct State {
  static rds::local::RDSLocal<State, rds::local::Initialize::FirstUse> instance;

  State();

  void initialize();
  void teardown();

  std::vector<Source> sources(const Func* func);
  std::vector<Sink> sinks(const Func* func);

  Stack stack;
  Heap heap;

  // Arena to hold all the paths
  std::unique_ptr<Arena> arena;
  // Contains all the taint flows found in this request
  std::vector<Path*> paths;

  std::shared_ptr<TaintedFunctionSet<Source>> m_sources;
  std::shared_ptr<TaintedFunctionSet<Sink>> m_sinks;
};

} // namespace taint
} // namespace HPHP

#endif
