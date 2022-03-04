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

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>

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

typedef ArenaImpl<32768> PathArena;
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
  static Path* origin(PathArena* arena, Hop hop);

  // Creates a path going from this path to the new hop
  // Takes an allocator to help with memory management
  Path* to(PathArena* arena, Hop hop) const;

  ~Path() = default;

 private:
  // The hop taken here. Can be empty for the root (both pointers null)
  Hop hop;
  const Path* parent;
};

using Value = Path*;

/**
 * Use a stack based on a dequeue with a fixed size
 * (based on HHVM's VM stack size) to avoid allocating
 * after creation.
 */
struct Stack {
  Stack();

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
  bool full() const;
  bool empty() const;
  std::vector<Value> m_stack;
  size_t m_top;
  size_t m_bottom;
};

/*
 * Our shadow heap is not replicating the full VM heap but keeps track
 * of tainted values (cells) on the heap.
 *
 * We keep a few different heaps around, for modeling different types
 * of things on the heap.
 */

// Model locals, which are identified by `tv_lval`s
struct LocalsHeap {
  void set(tv_lval typedValue, Value value);
  Value get(const tv_lval& typedValue) const;

  void clear();

 private:
  hphp_fast_map<tv_lval, Value> m_heap;
};

// Model objects, which are identified by `tv_lval`s
// and have their properties identified by strings.
struct ObjectsHeap {
  void set(ObjectData* object, folly::StringPiece property, Value value);
  Value get(ObjectData* object, folly::StringPiece property) const;

  void clear();

 private:
  hphp_fast_map<ObjectData*, folly::F14FastMap<std::string, Value>> m_heap;
};

// Model collections, which are identified by `tv_lval`s.
// We just store whether the whole collection is tainted or not right now,
// and do not support un-tainting on removal.
struct CollectionsHeap {
  void set(tv_lval typedValue, Value value);
  Value get(const tv_lval& typedValue) const;

  void clear();

 private:
  hphp_fast_map<tv_lval, Value> m_heap;
};

// Model classes, which are identified by `Class*`s
// and have their static properties identified by strings.
struct ClassesHeap {
  void set(Class* klass, folly::StringPiece property, Value value);
  Value get(Class* klass, folly::StringPiece property) const;

  void clear();

 private:
  hphp_fast_map<Class*, folly::F14FastMap<std::string, Value>> m_heap;
};

// Model globals, which are just a map of string -> value
struct GlobalsHeap {
  void set(folly::StringPiece key, Value value);
  Value get(folly::StringPiece key) const;

  void clear();

 private:
  folly::F14FastMap<std::string, Value> m_heap;
};

struct State {
  static rds::local::RDSLocal<State, rds::local::Initialize::FirstUse> instance;

  State();

  void initialize();
  void teardown();

  std::vector<Source> sources(const Func* func);
  std::vector<Sink> sinks(const Func* func);

  Stack stack;
  LocalsHeap heap_locals;
  ObjectsHeap heap_objects;
  CollectionsHeap heap_collections;
  ClassesHeap heap_classes;
  GlobalsHeap heap_globals;

  // Arena to hold all the paths
  std::unique_ptr<PathArena> arena;
  // Contains all the taint flows found in this request
  std::vector<Path*> paths;

  std::shared_ptr<FunctionMetadataTracker> m_function_metadata = nullptr;

  std::chrono::time_point<std::chrono::system_clock> m_request_start;
};

} // namespace taint
} // namespace HPHP

#endif
