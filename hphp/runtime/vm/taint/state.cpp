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

#ifdef HHVM_TAINT

#include <sstream>

#include <folly/Singleton.h>

#include "hphp/runtime/base/init-fini-node.h"

#include "hphp/runtime/vm/taint/state.h"

#include "hphp/util/trace.h"

namespace HPHP {
namespace taint {

TRACE_SET_MOD(taint);

void Path::dump() const {
  std::stringstream stream;
  stream << "{\"hops\": [";
  for (int i = 0; i < hops.size(); i++) {
    stream << "\"" << hops[i]->fullName()->data() << "\"";
    if (i != hops.size() - 1) {
      stream << ", ";
    }
  }
  stream << "]}";
  std::cerr << stream.str() << std::endl;
}

std::ostream& operator<<(std::ostream& out, const HPHP::taint::Value& value) {
  if (!value) {
    return out << "_";
  } else {
    return out << "S";
  }
}

void Stack::push(const Value& value) {
  m_stack.push_back(value);
}

Value Stack::top() const {
  return peek(0);
}

Value Stack::peek(int offset) const {
  // TODO(T93491972): replace with assertions once we can run the integration
  // tests.
  if (m_stack.size() <= offset) {
    FTRACE(
        3,
        "taint: (WARNING) called `Stack::peek({})` on stack of size {}\n",
        offset,
        m_stack.size());
    return std::nullopt;
  }
  return m_stack[m_stack.size() - 1 - offset];
}

void Stack::pop(int n) {
  if (m_stack.size() < n) {
    FTRACE(
        3,
        "taint: (WARNING) called `Stack::pop({})` on stack of size {}\n",
        n,
        m_stack.size());
    n = m_stack.size();
  }

  for (int i = 0; i < n; i++) {
    m_stack.pop_back();
  }
}

void Stack::replaceTop(const Value& value) {
  if (m_stack.empty()) {
    FTRACE(3, "taint: (WARNING) called `Stack::replaceTop()` on empty stack\n");
    return;
  }
  m_stack.back() = value;
}

size_t Stack::size() const {
  return m_stack.size();
}

std::string Stack::show() const {
  std::stringstream stream;
  for (const auto value : m_stack) {
    stream << value << " ";
  }
  stream << "(t)";
  return stream.str();
}

void Stack::clear() {
  m_stack.clear();
}

void Heap::set(const tv_lval& typedValue, const Value& value) {
  m_heap[typedValue] = value;
}

Value Heap::get(const tv_lval& typedValue) const {
  auto value = m_heap.find(typedValue);
  if (value != m_heap.end()) {
    return value->second;
  }

  return std::nullopt;
}

void Heap::clear() {
  m_heap.clear();
}

namespace {

struct SingletonTag {};

InitFiniNode s_stateInitialization(
    []() {
      State::get()->initialize();
    },
    InitFiniNode::When::ProcessInit);

folly::Singleton<State, SingletonTag> kSingleton{};

} // namespace

/* static */ std::shared_ptr<State> State::get() {
  return kSingleton.try_get();
}

void State::initialize() {
  // Stack is initialized with 4 values before any operation happens.
  // We don't care about these values but mirroring simplifies
  // consistency checks.
  for (int i = 0; i < 4; i++) {
    stack.push(std::nullopt);
  }
}

void State::reset() {
  stack.clear();
  heap.clear();
  initialize();
}

} // namespace taint
} // namespace HPHP

#endif
