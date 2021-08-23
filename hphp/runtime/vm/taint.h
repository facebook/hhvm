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

#include "hphp/runtime/vm/hhbc.h"

#include <memory>
#include <set>

namespace HPHP {
namespace taint {

using Source = int;
constexpr Source kNoSource = 0;
constexpr Source kTestSource = 1;

struct Configuration {
  static std::shared_ptr<Configuration> get();

  void read(const std::string& path);

  std::set<std::string> sources;
  std::set<std::string> sinks;
};

struct Issue {
  Source source;
  std::string sink_function;

  bool operator==(const Issue& other) const {
    return source == other.source && sink_function == other.sink_function;
  }
};

struct Stack {
  Stack(const std::vector<Source>& stack = {}) : m_stack(stack) {}

  void push(Source source);
  Source top() const;
  void pop(int n = 1);
  void replaceTop(Source source);

  size_t size() const;
  void clear();
 private:
  std::vector<Source> m_stack;
};

struct State {
  static std::shared_ptr<State> get();

  void initialize();

  void reset() {
    stack.clear();
    issues.clear();
  }

  Stack stack;
  std::vector<Issue> issues;
};

#define O(name, imm, in, out, flags) \
void iop##name();

OPCODES

#undef O

} // namespace taint
} // namespace HPHP

#endif
