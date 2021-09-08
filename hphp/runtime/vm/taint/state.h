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

#include <boost/functional/hash.hpp>

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

struct Path {
  // Print trace in JSON-line format to stderr.
  void dump() const;
  std::vector<const Func*> hops;
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
 * Represents granularity of what we store on the heap. This can either
 * be an `lval` or a combination of `base->attribute`.
 */
struct AccessPath {
  bool operator==(const AccessPath& other) const {
    // We don't care about read-only in member keys.
    return base == other.base && ((!memberKey && !other.memberKey) ||
        (memberKey->mcode == other.memberKey->mcode
         && memberKey->iva == other.memberKey->iva));
  }
  tv_lval base;
  Optional<MemberKey> memberKey;
};

} // namespace taint
} // namespace HPHP

template <>
struct std::hash<HPHP::MemberKey> {
  std::size_t operator()(const HPHP::MemberKey& memberKey) const {
    std::size_t seed = 0;
    // We don't care about read-only in member keys.
    boost::hash_combine(seed, memberKey.mcode);
    boost::hash_combine(seed, memberKey.iva);
    return seed;
  }
};

template <>
struct std::hash<HPHP::taint::AccessPath> {
  std::size_t operator()(const HPHP::taint::AccessPath& accessPath) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, std::hash<HPHP::tv_lval>()(accessPath.base));
    if (accessPath.memberKey) {
      boost::hash_combine(seed, std::hash<HPHP::MemberKey>()(*accessPath.memberKey));
    }
    return seed;
  }
};

namespace HPHP {
namespace taint {

/*
 * Our shadow heap is not replicating the full VM heap but keeps track
 * of tainted values (cells) on the heap.
 */
struct Heap {
  void set(const AccessPath& accessPath, const Value& value);
  Value get(const AccessPath& accessPath) const;

  void clear();

 private:
  hphp_fast_map<AccessPath, Value> m_heap;
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
